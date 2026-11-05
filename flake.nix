{
  description = "xdyn — zig/libc++ development shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

    # The closure sources: keep in sync with tools/deps/fetch-sources.sh. `refs/tags/` is spelled
    # out because a bare ref resolves as a branch.
    src-yaml-cpp   = { url = "github:jbeder/yaml-cpp/yaml-cpp-0.9.0"; flake = false; };
    src-googletest = { url = "github:google/googletest/v1.18.0";      flake = false; };
    src-hdf5       = { url = "github:HDFGroup/hdf5/2.2.0";            flake = false; };
    src-grpc       = { url = "git+https://github.com/grpc/grpc?ref=refs/tags/v1.85.1&submodules=1"; flake = false; };
    src-boost      = { url = "tarball+https://archives.boost.io/release/1.92.0/source/boost_1_92_0.tar.bz2"; flake = false; };
  };

  outputs = { self, nixpkgs, ... }@inputs:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

      # Closures wrap tools/deps/ rather than reimplement it. stdenvNoCC keeps every compiler but
      # zig out; src is the recipes alone, so neither an xdyn edit nor a new asset hash invalidates them.
      depsRecipes = pkgs.lib.fileset.toSource {
        root = ./tools/deps;
        fileset = pkgs.lib.fileset.difference ./tools/deps (pkgs.lib.fileset.unions [
          ./tools/deps/assets.sha256 ./tools/deps/fetch.sh ./tools/deps/pack.sh
        ]);
      };
      sandboxPrelude = ''
        export HOME=$TMPDIR/home
        export ZIG_GLOBAL_CACHE_DIR=$TMPDIR/zig XDG_CACHE_HOME=$TMPDIR/cache
        export WINEPREFIX=$TMPDIR/wine WINEDEBUG=-all
        mkdir -p $HOME $ZIG_GLOBAL_CACHE_DIR $XDG_CACHE_HOME $WINEPREFIX

        # Copied, since the recipes build inside $SRC. u+w keeps the exec bit b2's bootstrap needs.
        export XDYN_DEPS_SRC=$TMPDIR/src
        mkdir -p $XDYN_DEPS_SRC
        cp -r ${inputs.src-yaml-cpp}   $XDYN_DEPS_SRC/yaml-cpp
        cp -r ${inputs.src-googletest} $XDYN_DEPS_SRC/googletest
        cp -r ${inputs.src-hdf5}       $XDYN_DEPS_SRC/hdf5
        cp -r ${inputs.src-grpc}       $XDYN_DEPS_SRC/grpc
        cp -r ${inputs.src-boost}      $XDYN_DEPS_SRC/boost_1_92_0
        chmod -R u+w $XDYN_DEPS_SRC
      '';

      # Static protoc and grpc_cpp_plugin: a closure's own are dynamic, which a sandbox can't run.
      # Built here only to build the closures and to be packed; xdyn builds with the fetched asset.
      hostTools = pkgs.stdenvNoCC.mkDerivation {
        pname = "lxdyn-deps-host-tools";
        version = "1.85.1";
        src = depsRecipes;
        nativeBuildInputs = [ pkgs.zig pkgs.cmake pkgs.ninja pkgs.nasm pkgs.git pkgs.llvm ];
        dontFixup = true;
        buildPhase = ''
          runHook preBuild
          ${sandboxPrelude}
          export XDYN_DEPS=$out XDYN_DEPS_HOST=$out
          sh ./fetch-sources.sh
          sh ./build-host-tools.sh
          runHook postBuild
        '';
        installPhase = "runHook preInstall; runHook postInstall";
        meta.description = "protoc and grpc_cpp_plugin for the build machine, static";
      };

      mkClosure = { triple, host ? null, emulators ? [ ], steps ? null }:
        pkgs.stdenvNoCC.mkDerivation {
          pname = "libcxx-deps-${triple}";
          version = "1.85.1-1.92.0-2.2.0";   # grpc-boost-hdf5
          src = depsRecipes;
          nativeBuildInputs = [ pkgs.zig pkgs.cmake pkgs.ninja pkgs.nasm pkgs.git pkgs.llvm ] ++ emulators;
          # Fixup's binutils strip silently skips foreign-target archives.
          dontFixup = true;
          buildPhase = ''
            runHook preBuild
            ${sandboxPrelude}
            export XDYN_DEPS=$out
            ${if host == null then "" else "export XDYN_DEPS_HOST=${host}"}
            ${if steps == null then "" else "export XDYN_DEPS_STEPS='${steps}'"}
            sh ./build-all.sh ${triple}
            runHook postBuild
          '';
          installPhase = "runHook preInstall; runHook postInstall";
          meta.description = "xdyn C++ dependency closure, libc++, ${triple}";
        };

      closures = {
        host-tools = hostTools;

        # Sandbox smoke test: boost is the b2 step, gRPC (the hours) is left out.
        probe = mkClosure {
          triple = "x86_64-linux-gnu";
          host = hostTools;
          steps = "fetch-sources build-yaml-cpp build-gtest build-boost";
        };

        libcxx-deps-x86_64-linux-gnu = mkClosure { triple = "x86_64-linux-gnu"; host = hostTools; };
        libcxx-deps-x86_64-linux-musl = mkClosure { triple = "x86_64-linux-musl"; host = hostTools; };
        # b2 runs its configure probes, so foreign targets need their emulator.
        libcxx-deps-aarch64-linux-musl = mkClosure {
          triple = "aarch64-linux-musl";
          host = hostTools;
          emulators = [ pkgs.qemu ];
        };
        libcxx-deps-x86_64-windows-gnu = mkClosure {
          triple = "x86_64-windows-gnu";
          host = hostTools;
          emulators = [ pkgs.wine64 ];
        };
      };

      # The published closures, by the tag and hashes fetch.sh also reads from assets.sha256.
      assetLines = pkgs.lib.splitString "\n" (builtins.readFile ./tools/deps/assets.sha256);
      assetTag = pkgs.lib.removePrefix "# tag: " (pkgs.lib.findFirst (pkgs.lib.hasPrefix "# tag: ")
        (throw "tools/deps/assets.sha256 has no '# tag:' line") assetLines);

      fetchAsset = flavor: asset:
        let
          line = pkgs.lib.findFirst (pkgs.lib.hasSuffix "  ${asset}")
            (throw "tools/deps/assets.sha256 has no entry for ${asset}") assetLines;
        in pkgs.stdenvNoCC.mkDerivation {
          name = "deps-${flavor}-${assetTag}";
          src = pkgs.fetchurl {
            url = "https://github.com/naval-group/lxdyn/releases/download/${assetTag}/${asset}";
            sha256 = builtins.head (pkgs.lib.splitString " " line);
          };
          nativeBuildInputs = [ pkgs.zstd ];
          # The stamp fetch.sh writes, which build.zig checks.
          buildCommand = ''
            mkdir $out
            zstd -d -q -c $src | tar -C $out -xf -
            echo '${line}' > $out/asset.sha256
          '';
        };

      fetched = pkgs.lib.genAttrs
        [ "x86_64-linux-gnu" "x86_64-linux-musl" "aarch64-linux-musl" "x86_64-windows-gnu" ]
        (triple: fetchAsset triple "lxdyn-deps-${triple}.tar.zst")
        // { host-tools = fetchAsset "host-tools" "lxdyn-deps-host-tools-x86_64-linux.tar.zst"; };

      # Deny-list, so a docs or CI edit doesn't rebuild xdyn.
      xdynSource = pkgs.lib.cleanSourceWith {
        name = "xdyn-source";
        src = self;
        filter = path: _type:
          let rel = pkgs.lib.removePrefix "${self}/" path; in
          !(pkgs.lib.hasPrefix ".github" rel)
          && !(pkgs.lib.hasPrefix "doxygen" rel)
          && !(pkgs.lib.hasSuffix ".md" rel);
      };

      prepareXdyn = ''
        export HOME=$TMPDIR/home ZIG_GLOBAL_CACHE_DIR=$TMPDIR/zig XDG_CACHE_HOME=$TMPDIR/cache
        mkdir -p $HOME $ZIG_GLOBAL_CACHE_DIR $XDG_CACHE_HOME
      '';

      # The sandbox has no .git for headSha() and no pkg-config hook for the eigen probe.
      zigFlags = triple: pkgs.lib.concatStringsSep " " [
        "-Ddeps=${fetched.${triple}}"
        "-Ddeps-host=${fetched.host-tools}"
        "-Deigen=${pkgs.eigen_5}/include/eigen3"
        "-Dcommit=${self.shortRev or self.dirtyShortRev or "unknown"}"
      ];

      # Static musl binaries for consumers such as LOTUSim: no loader or closure at runtime.
      xdyn = pkgs.stdenvNoCC.mkDerivation {
        pname = "xdyn";
        # A flake sees no git tags, so the version comes from build.zig.zon.
        version = builtins.head (builtins.match
          ".*\\.version = \"([^\"]+)\".*" (builtins.readFile ./build.zig.zon));
        src = xdynSource;
        nativeBuildInputs = [ pkgs.zig pkgs.llvm pkgs.removeReferencesTo ];
        dontFixup = true;
        buildPhase = ''
          runHook preBuild
          ${prepareXdyn}
          zig build -Dtarget=x86_64-linux-musl ${zigFlags "x86_64-linux-musl"} --prefix $out
          runHook postBuild
        '';
        installPhase = "runHook preInstall; runHook postInstall";
        postInstall = ''
          # The other eleven include a `gz` that would shadow Gazebo's on a consumer's PATH.
          find $out/bin -mindepth 1 \
            -not -name xdyn -not -name xdyn-for-cs -not -name xdyn-for-me -delete
          # Debug info and Boost's assert strings carry the closure's store path.
          llvm-strip --strip-debug $out/bin/* $out/lib/*.a
          remove-references-to -t ${fetched.x86_64-linux-musl} $out/bin/* $out/lib/*.a
        '';
        meta.description = "xdyn simulator binaries, libc++, static x86_64-linux-musl";
      };

      # An importer dlopens the FMU library, so it is a shared object for the importer's libc:
      # glibc on Linux, where xdyn's binaries are static musl.
      mkFmu = { pname, triple, platform, target ? triple }: pkgs.stdenvNoCC.mkDerivation {
        inherit pname;
        inherit (xdyn) version;
        src = xdynSource;
        nativeBuildInputs = [ pkgs.zig pkgs.removeReferencesTo ];
        dontFixup = true;
        buildPhase = ''
          runHook preBuild
          ${prepareXdyn}
          zig build fmu -Dtarget=${target} ${zigFlags triple} --prefix $TMPDIR/prefix
          runHook postBuild
        '';
        installPhase = ''
          runHook preInstall
          install -Dm755 -t $out/lib $TMPDIR/prefix/fmu/*
          remove-references-to -t ${fetched.${triple}} $out/lib/*
          runHook postInstall
        '';
        meta.description = "xdyn as an FMI 2.0 library, ${platform}";
      };

      # The library plus xdyn-make-fmu, which packs a model with it: `xdyn-make-fmu ship.yml -o ship.fmu`.
      # An explicit gnu target takes zig's newest glibc, not build.zig's native 2.28 floor.
      xdyn-fmu = (mkFmu {
        pname = "xdyn-fmu"; triple = "x86_64-linux-gnu"; platform = "linux64";
        target = "x86_64-linux-gnu.2.28";
      })
        .overrideAttrs (old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [ pkgs.makeWrapper ];
          postInstall = ''
            install -Dm644 -t $out/share/xdyn-fmu xdyn_wrapper_fmi/make_fmu.py
            makeWrapper ${pkgs.python3}/bin/python3 $out/bin/xdyn-make-fmu \
              --add-flags "$out/share/xdyn-fmu/make_fmu.py --library $out/lib/xdyn_fmu.so"
          '';
          doInstallCheck = true;
          installCheckPhase = ''
            runHook preInstallCheck
            $out/bin/xdyn-make-fmu xdyn_wrapper_fmi/unit_tests/propelled_ship.yml -o $TMPDIR/ship.fmu
            ${pkgs.python3}/bin/python3 -m zipfile -l $TMPDIR/ship.fmu > $TMPDIR/ship.list
            grep -q modelDescription.xml $TMPDIR/ship.list
            grep -q binaries/linux64/xdyn_fmu.so $TMPDIR/ship.list
            runHook postInstallCheck
          '';
        });

      xdyn-fmu-win64 = mkFmu { pname = "xdyn-fmu-win64"; triple = "x86_64-windows-gnu"; platform = "win64"; };

      # The release asset, for importers without Nix: both libraries and the packer.
      xdyn-fmu-zip = pkgs.runCommandLocal "lxdyn-fmu-${xdyn.version}" { } ''
        dir=lxdyn-fmu-${xdyn.version}
        mkdir -p $dir/linux64 $dir/win64 $out
        cp ${xdyn-fmu}/lib/xdyn_fmu.so $dir/linux64/
        cp ${xdyn-fmu-win64}/lib/xdyn_fmu.dll $dir/win64/
        cp ${xdyn-fmu}/share/xdyn-fmu/make_fmu.py $dir/
        # Store files are dated 1970, which zip cannot hold: strict_timestamps=False clamps them to 1980.
        ${pkgs.python3}/bin/python3 -c '
        import pathlib, sys, zipfile
        with zipfile.ZipFile(sys.argv[1], "w", zipfile.ZIP_DEFLATED, strict_timestamps=False) as z:
            for f in sorted(pathlib.Path(sys.argv[2]).rglob("*")):
                z.write(f)
        ' $out/$dir.zip $dir
      '';

      # One wheel per CPython upstream still supports. libxdyn compiles once; only the bindings
      # rebuild per interpreter, against its Python.h.
      wheelPythons = [ pkgs.python311 pkgs.python312 pkgs.python313 pkgs.python314 pkgs.python315 ];
      # CalVer is YY.M[.P]: build.zig.zon's 26.11.0 is the release 26.11.
      wheelVersion = let m = builtins.match "([0-9]+\\.[0-9]+)\\.0" xdyn.version;
        in if m == null then xdyn.version else builtins.head m;
      lxdyn-wheels = pkgs.stdenvNoCC.mkDerivation {
        pname = "lxdyn-wheels";
        version = wheelVersion;
        src = xdynSource;
        # python3 by path only: its setup hook would point every interpreter at its own sysconfig.
        nativeBuildInputs = [ pkgs.zig pkgs.llvm ];
        dontFixup = true;
        buildPhase = ''
          runHook preBuild
          ${prepareXdyn}
          ${pkgs.lib.concatMapStrings (py: ''
            zig build python -Dtarget=x86_64-linux-gnu.2.28 ${zigFlags "x86_64-linux-gnu"} \
              -Dpython=${py.interpreter} -Dpybind11=${pkgs.python3Packages.pybind11}/include \
              --prefix $TMPDIR/prefix
          '') wheelPythons}
          runHook postBuild
        '';
        installPhase = ''
          runHook preInstall
          # Under its own name: a store path's basename carries the hash.
          cp ${./LICENSE.md} LICENSE.md
          for so in $TMPDIR/prefix/python/*.so; do
            llvm-strip --strip-debug $so
            # The wheel leaves Nix, so every store path goes, including the closure's build prefix HDF5 bakes in.
            sed -i -E 's|${builtins.storeDir}/[0-9a-z]{32}|${builtins.storeDir}/eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee|g' $so
            # manylinux_2_28 is a promise about the symbols the module needs, so it is checked.
            glibc=$(llvm-nm -D --undefined-only $so | grep -oE 'GLIBC_[0-9.]+' | sed 's/^GLIBC_//' | sort -V | tail -1)
            [ "$(printf '%s\n2.28\n' "$glibc" | sort -V | tail -1)" = 2.28 ] \
              || { echo "$so needs GLIBC_$glibc, above manylinux_2_28" >&2; exit 1; }
            ! llvm-readelf -d $so | grep -qiE 'rpath|runpath' || { echo "$so carries an RPATH" >&2; exit 1; }
            ${pkgs.python3.interpreter} xdyn_wrapper_python/make_wheel.py $so --version ${wheelVersion} --out $out \
              --pyproject xdyn_wrapper_python/pyproject.toml \
              --readme ${./xdyn_wrapper_python/README.md} --license LICENSE.md
          done
          [ "$(ls $out/*.whl | wc -l)" = ${toString (builtins.length wheelPythons)} ]
          runHook postInstall
        '';
        meta.description = "the lxdyn Python wheels, one per CPython, manylinux_2_28 x86_64";
      };

      # Hermetic unit suites from a cold cache: what CI trusts, too slow for the edit-test loop.
      mkUnitCheck = { triple, emulators ? [ ], test }:
        pkgs.stdenvNoCC.mkDerivation {
          name = "xdyn-unit-${triple}";
          src = xdynSource;
          nativeBuildInputs = [ pkgs.zig ] ++ emulators;
          dontFixup = true;
          buildPhase = ''
            runHook preBuild
            ${prepareXdyn}
            export WINEPREFIX=$TMPDIR/wine WINEDEBUG=-all
            ${test}
            runHook postBuild
          '';
          installPhase = "touch $out";
        };

      checks = {
        # The suite wants /lib64/ld-linux-x86-64.so.2, which the sandbox lacks: run it via nixpkgs' loader.
        unit-x86_64-linux-gnu = mkUnitCheck {
          triple = "x86_64-linux-gnu";
          test = ''
            zig build ${zigFlags "x86_64-linux-gnu"} --prefix $TMPDIR/prefix
            mkdir $TMPDIR/scratch && cd $TMPDIR/scratch
            for suite in run_all_tests run_ssc_tests; do
              ${pkgs.glibc}/lib/ld-linux-x86-64.so.2 --library-path ${pkgs.glibc}/lib \
                $TMPDIR/prefix/bin/$suite
            done
          '';
        };
        # -fqemu/-fwine, or the host's binfmt_misc would run the suite without either.
        unit-aarch64-linux-musl = mkUnitCheck {
          triple = "aarch64-linux-musl";
          emulators = [ pkgs.qemu ];
          test = "zig build test -Dtarget=aarch64-linux-musl -fqemu ${zigFlags "aarch64-linux-musl"}";
        };
        unit-x86_64-windows-gnu = mkUnitCheck {
          triple = "x86_64-windows-gnu";
          emulators = [ pkgs.wine64 ];
          test = "zig build test -Dtarget=x86_64-windows-gnu -fwine ${zigFlags "x86_64-windows-gnu"}";
        };
        fmu-x86_64-linux-gnu = xdyn-fmu;
        inherit lxdyn-wheels;
        lint = pkgs.runCommandLocal "xdyn-lint" { src = xdynSource; } ''
          cp -r $src/. . && sh tools/lint.sh && touch $out
        '';
      };
    in {
      packages.${system} = closures // { inherit xdyn xdyn-fmu xdyn-fmu-win64 xdyn-fmu-zip lxdyn-wheels; }
        // pkgs.lib.mapAttrs' (triple: drv: pkgs.lib.nameValuePair "deps-${triple}" drv) fetched;
      checks.${system} = checks;

      # ⛔ Tools only. No pkgs.boost/grpc/hdf5/yaml-cpp/gtest: they are libstdc++ builds, the ABI
      # wall. No pkgs.binutils: its wrapped ld breaks uv's host cc. See decisions.md.
      devShells.${system} = rec {
        # mkShellNoCC: mkShell's cc-wrapper exports CPATH, which zig cc honours.
        default = pkgs.mkShellNoCC {
          packages = [
            pkgs.zig
            pkgs.mise
            pkgs.git            # build.zig stamps the commit into binaries
            pkgs.curl.bin       # .bin: the full package puts its dev closure on PKG_CONFIG_PATH
            pkgs.pkg-config     # build.zig's eigen probe
            pkgs.uv             # owns the interpreters: a nixpkgs python leaks PYTHONPATH
            pkgs.gdb
            pkgs.doxygen
            pkgs.llvm           # llvm-nm/llvm-strip read every target, unlike binutils
            pkgs.zstd
            pkgs.hdf5.bin       # h5dump only; pkgs.hdf5 is a C++ library
          ];
          # buildInputs, so pkg-config's hook finds it. eigen_5 clears a -Werror=uninitialized.
          buildInputs = [ pkgs.eigen_5 ];
          # Headers only, like Eigen: a path, so none of the package's Python lands in the shell.
          XDYN_PYBIND11 = "${pkgs.python3Packages.pybind11}/include";
          shellHook = ''
            echo "xdyn devShell: $(zig version), $(uv --version)"
          '';
        };

        # qemu and wine only run the cross suites; wine alone is bigger than everything above.
        cross = default.overrideAttrs (old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [ pkgs.qemu pkgs.wine64 ];
          shellHook = old.shellHook + ''
            echo "  cross runners: $(qemu-aarch64 --version | head -1), $(wine --version)"
          '';
        });

        # cmake, ninja and nasm exist only to build a closure. gRPC turns BoringSSL's assembly off
        # under Ninja when it finds no nasm, on every target, though only Windows' is NASM syntax.
        deps = cross.overrideAttrs (old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [ pkgs.cmake pkgs.ninja pkgs.nasm ];
          shellHook = old.shellHook + ''
            echo "  closure toolchain: $(cmake --version | head -1), ninja $(ninja --version)"
          '';
        });
      };
    };
}
