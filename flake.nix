{
  # Provides TOOLS only, no C++ library -- see the boxed warning below.
  description = "xdyn — zig/libc++ development shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

    # Must stay in sync with tools/deps/fetch-sources.sh's five clones -- nothing couples them.
    # `refs/tags/` is spelled out: a bare `ref=v1.78.1` resolves as refs/heads/ and silently
    # pins a branch of that name instead, if one exists.
    src-yaml-cpp   = { url = "github:jbeder/yaml-cpp/yaml-cpp-0.9.0"; flake = false; };
    src-googletest = { url = "github:google/googletest/v1.15.2";      flake = false; };
    src-hdf5       = { url = "github:HDFGroup/hdf5/hdf5_1.14.6";      flake = false; };
    src-grpc       = { url = "git+https://github.com/grpc/grpc?ref=refs/tags/v1.78.1&submodules=1"; flake = false; };
    src-boost      = { url = "tarball+https://archives.boost.io/release/1.89.0/source/boost_1_89_0.tar.bz2"; flake = false; };
  };

  outputs = { self, nixpkgs, ... }@inputs:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

      # A derivation per target triple, wrapping tools/deps/ rather than reimplementing it --
      # the recipes stay the source of truth for the no-Nix path. This only adds pinned
      # sources and a sandbox, giving each closure a content-addressed identity.
      #
      # stdenvNoCC: no compiler, no libstdc++, no nixpkgs libc++ near this -- zig owns the
      # whole graph or none of it. src is tools/deps/, not `self`, so editing an xdyn .cpp
      # doesn't invalidate a multi-hour build.

      # Writable homes/caches plus the pinned sources, shared by the closures and host tools.
      sandboxPrelude = ''
        export HOME=$TMPDIR/home
        export ZIG_GLOBAL_CACHE_DIR=$TMPDIR/zig XDG_CACHE_HOME=$TMPDIR/cache
        export WINEPREFIX=$TMPDIR/wine WINEDEBUG=-all
        mkdir -p $HOME $ZIG_GLOBAL_CACHE_DIR $XDG_CACHE_HOME $WINEPREFIX

        # Copied rather than referenced: the recipes write into $SRC (b2 bootstraps there,
        # BUILD is $SRC/build/<triple>), and the store paths are read-only.
        export XDYN_DEPS_SRC=$TMPDIR/src
        mkdir -p $XDYN_DEPS_SRC
        cp -r ${inputs.src-yaml-cpp}   $XDYN_DEPS_SRC/yaml-cpp
        cp -r ${inputs.src-googletest} $XDYN_DEPS_SRC/googletest
        cp -r ${inputs.src-hdf5}       $XDYN_DEPS_SRC/hdf5
        cp -r ${inputs.src-grpc}       $XDYN_DEPS_SRC/grpc
        cp -r ${inputs.src-boost}      $XDYN_DEPS_SRC/boost_1_89_0
        # u+w, not --no-preserve=mode: that would strip the executable bit too, and Boost
        # bootstraps b2 by running ./build.sh.
        chmod -R u+w $XDYN_DEPS_SRC
      '';

      # Static protoc/grpc_cpp_plugin for gRPC codegen. Separate from mkClosure because all
      # four flavors share one -- even the host-matching closure, whose own protoc a sandbox
      # can't run either.
      hostTools = pkgs.stdenvNoCC.mkDerivation {
        pname = "lxdyn-deps-host-tools";
        version = "1.78.1";
        src = ./tools/deps;
        nativeBuildInputs = [ pkgs.zig pkgs.cmake pkgs.ninja pkgs.git pkgs.llvm ];
        dontFixup = true;
        buildPhase = ''
          runHook preBuild
          ${sandboxPrelude}
          # Both vars needed: common.sh's $REPO fallback resolves to / inside a derivation,
          # since src is tools/deps/ and $HERE/../.. escapes the build directory.
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
          version = "1.78.1-1.89.0-1.14.6";   # grpc-boost-hdf5, the three that move
          src = ./tools/deps;

          # llvm for llvm-nm: build-boost.sh/build-grpc.sh assert on libc++ mangling.
          nativeBuildInputs = [ pkgs.zig pkgs.cmake pkgs.ninja pkgs.git pkgs.llvm ] ++ emulators;

          # dontFixup: binutils strip is single-arch, no-ops per member on a foreign archive
          # and still exits 0 -- the defect that made an aarch64 asset 264 MB instead of 44.
          dontFixup = true;

          buildPhase = ''
            runHook preBuild
            ${sandboxPrelude}

            # fetch-sources.sh's clones short-circuit on the copies above -- no network. Its
            # b2 bootstrap still runs; that step is zig-built and needs no host compiler.
            export XDYN_DEPS=$out
            ${if host == null then "" else "export XDYN_DEPS_HOST=${host}"}
            ${if steps == null then "" else "export XDYN_DEPS_STEPS='${steps}'"}
            sh ./build-all.sh ${triple}
            runHook postBuild
          '';

          # common.sh creates $out and the recipes install into it directly.
          installPhase = "runHook preInstall; runHook postInstall";

          meta.description = "xdyn C++ dependency closure, libc++, ${triple}";
        };

      closures = rec {
        host-tools = hostTools;

        # Sandbox smoke test, not a closure. Includes boost deliberately -- it's the only step
        # driven by b2 rather than CMake, and the cheaper of the two that assert on llvm-nm.
        # gRPC (the hours) is left out.
        probe = mkClosure {
          triple = "x86_64-linux-gnu";
          host = hostTools;
          steps = "fetch-sources build-yaml-cpp build-gtest build-boost";
        };

        # Every flavor, host-matching one included, takes the same host tools: a native build's
        # own protoc would be dynamically linked, which a /lib64-less sandbox can't run either.
        # No closure depends on another.
        libcxx-deps-x86_64-linux-gnu = mkClosure {
          triple = "x86_64-linux-gnu";
          host = hostTools;
        };
        libcxx-deps-x86_64-linux-musl = mkClosure {
          triple = "x86_64-linux-musl";
          host = hostTools;
        };
        # b2 executes its configure probes, so the two foreign targets need their emulator.
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
    in {
      packages.${system} = closures;

      devShells.${system} = rec {
        # mkShellNoCC, not mkShell: mkShell's cc-wrapper exports CPATH, which zig cc honours --
        # a route for nixpkgs headers, and behind them libstdc++, into a build that must see
        # only zig's libc++ and the closure.
        default = pkgs.mkShellNoCC {
          packages = [
            pkgs.zig            # the compiler and the build system
            pkgs.mise           # task runner only -- pins no tool versions, nix does that here
            pkgs.git            # build.zig stamps the commit into binaries; -Dgit-sha= overrides
            pkgs.curl.bin       # tools/deps/fetch.sh. .bin only: the full package's dev closure
                                # (openssl, krb5, nghttp2, zstd...) lands on PKG_CONFIG_PATH
            pkgs.pkg-config     # build.zig's fourth eigen probe

            pkgs.uv             # owns the Python envs, interpreters included. No pkgs.python*:
                                # a nixpkgs interpreter's PYTHONPATH leaks into every other
                                # interpreter in the shell -- once made a 3.10 venv report 3.13's ABI

            pkgs.gdb            # mise run gdb -- built with Python, for .gdbinit's $_regex
            pkgs.doxygen        # mise run doc:cpp
            pkgs.llvm           # llvm-strip/llvm-nm: target-agnostic, unlike binutils (see
                                # the boxed warning below before reaching for pkgs.binutils)
            pkgs.zstd           # tools/deps/fetch.sh unpacks, pack.sh compresses. Here rather
                                # than .#deps because fetching a closure is the common path
            pkgs.hdf5.bin       # h5dump, for the CLI integration test diffs. .bin only --
                                # pkgs.hdf5 itself is a C++ library and belongs nowhere near this
          ];

          # Header-only (no compiled std ABI to worry about). buildInputs, not packages, so
          # pkg-config's setup hook puts it on PKG_CONFIG_PATH for build.zig's probe. eigen_5
          # specifically clears a -Werror=uninitialized false positive.
          buildInputs = [ pkgs.eigen_5 ];

          shellHook = ''
            echo "xdyn devShell: $(zig version), $(uv --version)"
          '';
        };

        # ⛔ Do not add pkgs.boost / pkgs.grpc / pkgs.hdf5 / pkgs.yaml-cpp / pkgs.gtest here.
        # They're libstdc++ builds -- linking one against a zig cc object fails on mangling,
        # and an accidental one risks two libc++ versions in the graph. Every C++ library xdyn
        # links comes from the tools/deps/ closure, built by zig cc against zig's libc++.
        # This is the rule the first migration attempt died for.

        # ⛔ Do not add pkgs.binutils either, however much `nm`/`ld` is wanted. Its setup hook
        # puts a wrapped ld/as ahead of /usr/bin, so a host cc links host-compiled objects
        # against the nix store and the result aborts before main. Nothing here compiles with
        # a host cc except uv, when filling an interpreter with no wheel (numpy 1.26.4 on
        # 3.13/3.15) -- green here, red in CI is how this was found. llvm-nm covers every flavor.

        # zig cross-*compiles* with nothing added; qemu/wine only *run* the results, so they're
        # a separate shell -- wine alone drags in a multimedia/X11 graph bigger than everything
        # above combined. `mise run cross` passes -fqemu/-fwine explicitly rather than relying
        # on the host's binfmt_misc registration, which would only pass here.

        # cmake/ninja exist only for tools/deps/ (`zig build` invokes neither) -- kept out of
        # `default` since fetching a closure, not building one, is the common path.
        deps = cross.overrideAttrs (old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [ pkgs.cmake pkgs.ninja ];
          shellHook = old.shellHook + ''
            echo "  closure toolchain: $(cmake --version | head -1), ninja $(ninja --version)"
          '';
        });

        cross = default.overrideAttrs (old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [
            pkgs.qemu      # qemu-aarch64, user-mode
            pkgs.wine64    # package is wine64, binary is `wine` -- Wine 11 merged the two
          ];
          shellHook = old.shellHook + ''
            echo "  cross runners: $(qemu-aarch64 --version | head -1), $(wine --version)"
          '';
        });
      };
    };
}
