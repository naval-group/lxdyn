const std = @import("std");
const builtin = @import("builtin");

// =============================================================================
// Every -Dtarget resolves its own dependency closure. A missing one warns rather than
// fails, so `zig build --help` still works on a machine that has none.
//
// Build options: -Ddeps=<path>  -Deigen=<path>  -Ddeps-host=<path>  -Dgit-sha=<sha>
// (`zig build --help` lists them all)
// =============================================================================

var deps_root: []const u8 = undefined;
var eigen_include: ?[]const u8 = null;
var target_is_windows = false;
var codegen: Codegen = undefined;
var debug_build = false;
var proto_flags: []const []const u8 = undefined;

const native_default: std.Target.Query = if (builtin.os.tag == .linux)
    .{ .abi = .gnu, .glibc_version = .{ .major = 2, .minor = 28, .patch = 0 } }
else
    .{};

// =============================================================================
// What gets built — everything below is a list you edit; the code is further down.
// =============================================================================

const Executable = struct {
    name: []const u8,
    source: []const u8,
    protos: []const []const u8 = &.{},
};

const executables = [_]Executable{
    .{ .name = "xdyn", .source = "xdyn/executables/xdyn.cpp" },
    .{ .name = "gz", .source = "xdyn/executables/gz.cpp" },
    .{ .name = "xdyn-for-cs", .source = "xdyn/executables/xdyn_for_cs.cpp", .protos = &.{"cosimulation"} },
    .{ .name = "xdyn-for-me", .source = "xdyn/executables/xdyn_for_me.cpp", .protos = &.{"model_exchange"} },
    .{ .name = "xdyn-grpc-airy", .source = "xdyn/executables/xdyn_grpc_airy.cpp" },
    .{ .name = "test_orbital_velocities", .source = "xdyn/executables/test_orbital_velocities_and_dynamic_pressures.cpp" },
    .{ .name = "test_hs", .source = "xdyn/executables/test_hs.cpp" },
    .{ .name = "yml2test", .source = "xdyn/executables/yml2test.cpp" },
    .{ .name = "convert_stl_files_to_code", .source = "xdyn/executables/convert_stl_files_to_code.cpp" },
    .{ .name = "convert_quaternion_to_euler", .source = "xdyn/executables/convert_quaternion_to_euler.cpp" },
    .{ .name = "generate_yaml_example", .source = "xdyn/executables/generate_yaml_examples.cpp" },
    .{ .name = "generate_stl_examples", .source = "xdyn/executables/generate_stl_examples.cpp" },
    .{ .name = "generate_fmi_xml", .source = "xdyn/executables/generate_fmi_xml.cpp" },
};

const ssc_modules = [_][]const u8{
    "external/ssc/ssc/csv_file_reader", "external/ssc/ssc/data_source",
    "external/ssc/ssc/decode_unit",     "external/ssc/ssc/exception_handling",
    "external/ssc/ssc/integrate",       "external/ssc/ssc/interpolation",
    "external/ssc/ssc/json",            "external/ssc/ssc/kinematics",
    "external/ssc/ssc/numeric",         "external/ssc/ssc/random_data_generator",
    "external/ssc/ssc/solver",          "external/ssc/ssc/text_file_reader",
};

// Warnings the tree already satisfies, kept satisfied. This list only *enables*;
// -Werror on each hand-written flag set is what makes any of them fail a build.
// Severity has to be explicit somewhere: the build runner surfaces a C diagnostic
// only when it is an error, so a warning nobody promotes is a warning nobody sees.
const warning_flags = [_][]const u8{
    "-Wcast-align",           "-Wcast-qual",
    "-Wcomma",                "-Wctad-maybe-unsupported",
    "-Wduplicate-enum",       "-Wembedded-directive",
    "-Wextra-semi",           "-Wfour-char-constants",
    "-Wimplicit-fallthrough", "-Winvalid-noreturn",
    "-Wkeyword-macro",        "-Wloop-analysis",
    "-Wmissing-prototypes",   "-Wmissing-variable-declarations",
    "-Wnon-virtual-dtor",     "-Wover-aligned",
    "-Woverloaded-virtual",   "-Wpedantic",
    "-Wpessimizing-move",     "-Wpointer-arith",
    "-Wrange-loop-analysis",  "-Wredundant-decls",
    "-Wself-assign",          "-Wshift-sign-overflow",
    "-Wsigned-enum-bitfield", "-Wstring-conversion",
    "-Wtautological-compare", "-Wthread-safety",
    "-Wundef",                "-Wunused-member-function",
    "-Wunused-template",      "-Wused-but-marked-unused",
    "-Wvla",                  "-Wwrite-strings",
    "-Wformat=2",
};

const ws_dir = "external/ssc/ssc/websocket/src";

// Globbed out of their module and added back below, each with a flag of its own.
const compiled_separately = [_][]const u8{ "demo_scripts.cpp", "generate_test_ship.cpp" };

const f2c_dir = "external/ssc/ssc/f2c";
const f2c_excluded = [_][]const u8{
    "main.c", "arithchk.c", "arithchk_ori.c", // carry main()
    "uninit.c", "signbit.c", // need a generated arith.h
    "pow_qq.c", "qbitbits.c", "qbitshft.c", "ftell64_.c", // need longint
    "dtime_.c", "etime_.c", // POSIX-only: break the windows target
    "getarg_.c", "iargc_.c", "s_paus.c", "i_ceiling.c", "i_len_trim.c", // unused
};

const proto_files = [_][]const u8{
    "wave_types", "wave_grpc", "force", "controller", "cosimulation", "model_exchange",
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{ .default_target = native_default });
    target_is_windows = target.result.os.tag == .windows;
    deps_root = resolveDepsRoot(b, target);
    eigen_include = resolveEigen(b);
    debug_build = b.option(bool, "debug", "Build xdyn -O0 -g for gdb (deps stay optimized)") orelse false;

    installUnderBuildDir(b, target);
    codegen = addCodegen(b);
    const shim_hpp = b.pathFromRoot("xdyn/compat/ssc_serialize_compat.hpp");
    const optimize: std.builtin.OptimizeMode = .ReleaseFast;

    // -Wno-date-time: zig build enables -Wdate-time itself, and h5_version.c's banner
    // wants __DATE__.
    //
    // -Wno-error=stack-exhausted: the one diagnostic here that describes the *compiler's*
    // resources rather than the code. stl_data.cpp builds a fixture from a 1462-term << chain,
    // which is deep enough that whether clang warns depends on the stack the machine gave it —
    // clean here, fired on a CI runner. A build must not pass or fail on that, so it stays a
    // warning. The chain itself is the real defect: clang segfaults on that file at
    // `ulimit -s 5120`, -Werror or not.
    const cpp_flags = concat(b, withDebug(b, &.{ "-std=gnu++17", "-Wall", "-Wextra", "-Werror", "-Wno-date-time", "-Wno-error=stack-exhausted", "-fPIC", "-include", shim_hpp }), &warning_flags);
    // SSC is under a minimal-divergence policy, so its 11 misleading-indentation sites are
    // suppressed rather than fixed. All 11 are in .cpp files, which is why a flag set of its
    // own is enough — a header site would leak into every xdyn translation unit.
    const ssc_flags = concat(b, cpp_flags, &.{"-Wno-misleading-indentation"});
    const sir_flags = withDebug(b, &.{ "-std=gnu++17", "-Wall", "-Werror", "-fPIC" });
    const c_flags = withDebug(b, &.{ "-std=gnu11", "-Wall", "-Wextra", "-Werror", "-Wno-date-time", "-fno-common", "-fPIC" });
    // No -Werror on the two generated-code sets below: f2c's Fortran-to-C output and protoc's
    // are not ours to keep clean, and their warning counts move when the generator moves.
    const f2c_flags = withDebug(b, &.{ "-std=gnu11", "-Wall", "-Wextra", "-fno-common", "-fPIC", "-Iexternal/ssc/ssc/f2c" });
    proto_flags = withDebug(b, &.{ "-std=gnu++17", "-fPIC" });
    const ws_flags = withDebug(b, &.{
        "-std=gnu++17",                 "-Wall",                            "-Wextra",                          "-Werror",                            "-fPIC",
        "-D_WEBSOCKETPP_CPP11_STL_",    "-D_WEBSOCKETPP_CPP11_THREAD_",     "-D_WEBSOCKETPP_CPP11_FUNCTIONAL_", "-D_WEBSOCKETPP_CPP11_SYSTEM_ERROR_", "-D_WEBSOCKETPP_CPP11_RANDOM_DEVICE_",
        "-D_WEBSOCKETPP_CPP11_MEMORY_", "-Iexternal/ssc/ssc/websocket/inc", "-isystem",                         "external/websocketpp",
    });

    // =========================================================================
    // libxdyn — one static archive of all SSC + xdyn + generated objects
    // =========================================================================
    const xdyn = b.createModule(.{ .target = target, .optimize = optimize, .link_libcpp = true });
    addCommonIncludes(b, xdyn);

    // --- SSC modules: every *.cpp directly in each; SSC keeps its own tests one level
    // deeper, in <module>/unit_tests/, so a non-recursive glob skips them ---
    for (ssc_modules) |dir|
        xdyn.addCSourceFiles(.{ .root = b.path(dir), .files = sourcesIn(b, dir, ".cpp", &.{}), .flags = ssc_flags });
    // The f2c-derived C in these wants the f2c flags, not the C++ ones.
    for ([_][]const u8{ "external/ssc/ssc/integrate", "external/ssc/ssc/interpolation" }) |dir|
        xdyn.addCSourceFiles(.{ .root = b.path(dir), .files = sourcesIn(b, dir, ".c", &.{}), .flags = f2c_flags });
    xdyn.addCSourceFiles(.{ .root = b.path(f2c_dir), .files = sourcesIn(b, f2c_dir, ".c", &f2c_excluded), .flags = f2c_flags });
    // sir_rand.c uses C++ function-style casts → compile as C++ (Zig language override).
    xdyn.addCSourceFiles(.{ .root = b.path("external/ssc/ssc/random_data_generator"), .files = &.{"sir_rand.c"}, .flags = sir_flags, .language = .cpp });
    xdyn.addCSourceFiles(.{ .root = b.path(ws_dir), .files = sourcesIn(b, ws_dir, ".cpp", &.{}), .flags = ws_flags });

    // --- xdyn modules: every xdyn/<module>/*.cpp. Depth 2 on purpose — it takes the module
    // directories without naming them, and leaves xdyn/*/unit_tests/ to the test runner ---
    xdyn.addCSourceFiles(.{ .root = b.path("xdyn"), .files = xdynModuleSources(b), .flags = cpp_flags });
    xdyn.addCSourceFile(.{ .file = codegen.git_sha_c, .flags = c_flags });
    xdyn.addCSourceFiles(.{ .root = b.path("xdyn/interface_hdf5"), .files = &.{ "h5_tools.c", "h5_version.c" }, .flags = c_flags });
    xdyn.addCSourceFiles(.{ .root = b.path("xdyn/observers_and_api"), .files = &.{"demo_scripts.cpp"}, .flags = concat(b, cpp_flags, &.{"-Wno-c23-extensions"}) });
    // one 253906-character string literal; ISO requires only 65536
    xdyn.addCSourceFiles(.{ .root = b.path("xdyn/binary_stl_data"), .files = &.{"generate_test_ship.cpp"}, .flags = concat(b, cpp_flags, &.{"-Wno-overlength-strings"}) });
    addProtoSources(b, xdyn, &.{ "wave_types", "wave_grpc", "force", "controller" });
    xdyn.addCSourceFiles(.{ .root = b.path("external/thirdparty/boost_program_options_descriptions"), .files = &.{
        "CustomOptionDescription.cpp", "OptionPrinter.cpp",
    }, .flags = cpp_flags });

    const libxdyn = b.addLibrary(.{ .name = "xdyn", .root_module = xdyn, .linkage = .static });
    libxdyn.step.dependOn(codegen.step);
    b.installArtifact(libxdyn);

    // =========================================================================
    // Executables — see the `executables` table
    // =========================================================================
    for (executables) |exe| buildExe(b, target, optimize, libxdyn, exe, cpp_flags);

    // =========================================================================
    // Test runner (run_all_tests) — links gtest/gmock too
    // =========================================================================
    const test_mod = b.createModule(.{ .target = target, .optimize = optimize, .link_libcpp = true });
    addCommonIncludes(b, test_mod);
    // The unit tests include their module's headers by bare name, so every module directory
    // has to be on the include path.
    for (xdynDirs(b)) |dir| test_mod.addIncludePath(b.path(b.pathJoin(&.{ "xdyn", dir })));
    test_mod.addCSourceFiles(.{ .root = b.path("xdyn/executables"), .files = &.{"run_all_tests.cpp"}, .flags = cpp_flags });
    // tests touch SSC serializers transitively → cpp_flags already force-includes the shim
    test_mod.addCSourceFiles(.{ .root = b.path("xdyn"), .files = unitTestSources(b), .flags = cpp_flags });
    test_mod.linkLibrary(libxdyn);
    test_mod.addObjectFile(.{ .cwd_relative = b.fmt("{s}/libxdyndeps_test.a", .{deps_root}) });
    test_mod.addObjectFile(.{ .cwd_relative = b.fmt("{s}/libxdyndeps_core.a", .{deps_root}) });
    linkWindowsSystemLibs(test_mod);

    const test_exe = b.addExecutable(.{ .name = "run_all_tests", .root_module = test_mod });
    test_exe.step.dependOn(codegen.step);
    if (!target_is_windows) test_exe.pie = false;
    b.installArtifact(test_exe);

    const run_tests = b.addRunArtifact(test_exe);
    const scratch = b.addWriteFiles();
    _ = scratch.add(".keep", "");
    run_tests.setCwd(scratch.getDirectory());
    if (b.args) |args| run_tests.addArgs(args);
    const test_step = b.step("test", "Run all C++ unit tests");
    test_step.dependOn(&run_tests.step);

    // =========================================================================
    // Python wrapper (pyxdyn) — the one shared object. Opt-in: `zig build python`
    // =========================================================================
    addPythonWrapper(b, target, optimize, libxdyn, cpp_flags);
}

// =============================================================================
// Build layout and codegen
// =============================================================================

fn installUnderBuildDir(b: *std.Build, target: std.Build.ResolvedTarget) void {
    if (b.dest_dir != null) return;
    const default_prefix = b.build_root.join(b.allocator, &.{"zig-out"}) catch return;
    if (!std.mem.eql(u8, b.install_prefix, default_prefix)) return;

    const t = target.result;
    const triple = b.fmt("{s}-{s}-{s}{s}", .{
        @tagName(t.cpu.arch),              @tagName(t.os.tag), @tagName(t.abi),
        if (debug_build) "-debug" else "",
    });
    b.resolveInstallPrefix(b.pathFromRoot(b.pathJoin(&.{ "build", triple })), .{});
}

fn withDebug(b: *std.Build, base: []const []const u8) []const []const u8 {
    if (!debug_build) return base;
    return concat(b, base, &.{ "-O0", "-g", "-fno-omit-frame-pointer" });
}

fn concat(b: *std.Build, base: []const []const u8, extra: []const []const u8) []const []const u8 {
    const out = b.allocator.alloc([]const u8, base.len + extra.len) catch @panic("OOM");
    @memcpy(out[0..base.len], base);
    @memcpy(out[base.len..], extra);
    return out;
}

const Codegen = struct {
    step: *std.Build.Step,
    proto_messages: std.Build.LazyPath,
    proto_services: std.Build.LazyPath,
    git_sha_c: std.Build.LazyPath,
};

fn addCodegen(b: *std.Build) Codegen {
    const step = b.step("gen", "Run the code generators on their own");

    const ssc_headers = b.addSystemCommand(&.{ "sh", "generate_module_header.sh" });
    ssc_headers.setCwd(b.path("external/ssc/ssc"));
    ssc_headers.has_side_effects = true;
    step.dependOn(&ssc_headers.step);

    const host = b.option([]const u8, "deps-host", "Host closure providing protoc (default: ./libcxx-x86_64-linux-gnu)") orelse
        b.graph.environ_map.get("XDYN_DEPS_HOST") orelse
        b.pathJoin(&.{ b.build_root.path orelse ".", "libcxx-x86_64-linux-gnu" });
    const bin = b.pathJoin(&.{ host, "install", "bin" });
    const protoc = b.pathJoin(&.{ bin, "protoc" });
    std.Io.Dir.cwd().access(b.graph.io, protoc, .{}) catch {
        std.log.warn("no protoc under '{s}': codegen will fail. Point at the native closure " ++
            "with -Ddeps-host=<path> or $XDYN_DEPS_HOST.", .{bin});
    };

    const gen_proto = b.addSystemCommand(&.{protoc});
    gen_proto.setCwd(b.path("."));
    gen_proto.addFileInput(.{ .cwd_relative = protoc });
    gen_proto.addPrefixedFileArg("--plugin=protoc-gen-grpc=", .{ .cwd_relative = b.pathJoin(&.{ bin, "grpc_cpp_plugin" }) });
    const messages = gen_proto.addPrefixedOutputDirectoryArg("--cpp_out=", "proto");
    const services = gen_proto.addPrefixedOutputDirectoryArg("--grpc_out=", "proto-grpc");
    gen_proto.addPrefixedDirectoryArg("-I", b.path("external/interfaces/proto"));
    for (proto_files) |name|
        gen_proto.addFileArg(b.path(b.fmt("external/interfaces/proto/{s}.proto", .{name})));
    step.dependOn(&gen_proto.step);

    const sha = b.option([]const u8, "git-sha", "Commit stamped into the binaries (default: git rev-parse HEAD)") orelse
        headSha(b);
    const write_sha = b.addWriteFiles();
    const git_sha_c = write_sha.add("get_git_sha.c", b.fmt(
        \\#include "xdyn/get_git_sha/get_git_sha.h"
        \\const char* get_git_sha() {{ return "{s}"; }}
        \\
    , .{sha}));
    step.dependOn(&write_sha.step);

    return .{
        .step = step,
        .proto_messages = messages,
        .proto_services = services,
        .git_sha_c = git_sha_c,
    };
}

fn headSha(b: *std.Build) []const u8 {
    var code: u8 = 0;
    const out = b.runAllowFail(
        &.{ "git", "-C", b.build_root.path orelse ".", "rev-parse", "HEAD" },
        &code,
        .ignore,
    ) catch return "unknown";
    if (code != 0) return "unknown";
    return b.dupe(std.mem.trim(u8, out, " \t\r\n"));
}

fn addProtoSources(b: *std.Build, m: *std.Build.Module, names: []const []const u8) void {
    for (names) |name| {
        m.addCSourceFile(.{
            .file = codegen.proto_messages.path(b, b.fmt("{s}.pb.cc", .{name})),
            .flags = proto_flags,
        });
        m.addCSourceFile(.{
            .file = codegen.proto_services.path(b, b.fmt("{s}.grpc.pb.cc", .{name})),
            .flags = proto_flags,
        });
    }
}

// =============================================================================
// Path resolution — nothing absolute is baked into this file
// =============================================================================

// Keyed on the **full triple**, not the arch: x86_64-linux-gnu and x86_64-linux-musl differ in
// the libc, which is precisely what a closure carries. Resolution order:
//   1. -Ddeps=<path>            explicit
//   2. $XDYN_DEPS_<TRIPLE>      per-target; the one an environment should export, and the only
//                               one that stays correct when a cross build shares the shell
//   3. $XDYN_DEPS               one path for every target — a deliberate override, never exported
//   4. <build root>/libcxx-<triple>   where tools/deps/ builds them, gitignored
// The *host* closure is resolved separately, via -Ddeps-host: codegen runs host protoc.
fn resolveDepsRoot(b: *std.Build, target: std.Build.ResolvedTarget) []const u8 {
    const t = target.result;
    const triple = b.fmt("{s}-{s}-{s}", .{ @tagName(t.cpu.arch), @tagName(t.os.tag), @tagName(t.abi) });

    const env_key = b.fmt("XDYN_DEPS_{s}", .{triple});
    for (env_key) |*c| c.* = switch (c.*) {
        '-' => '_',
        'a'...'z' => c.* - ('a' - 'A'),
        else => c.*,
    };

    const root = b.option([]const u8, "deps", "libc++ dependency closure for the target (default: ./libcxx-<triple>)") orelse
        b.graph.environ_map.get(env_key) orelse
        b.graph.environ_map.get("XDYN_DEPS") orelse
        b.pathJoin(&.{ b.build_root.path orelse ".", b.fmt("libcxx-{s}", .{triple}) });

    std.Io.Dir.cwd().access(b.graph.io, b.pathJoin(&.{ root, "libxdyndeps_core.a" }), .{}) catch {
        std.log.warn("no libxdyndeps_core.a under '{s}': the {s} closure is missing. Fetch it " ++
            "with `mise run deps:fetch {s}`, build it with `mise run deps:{s}`, or name one " ++
            "with -Ddeps=<path> or ${s}.", .{ root, triple, triple, triple, env_key });
    };
    return root;
}

// Eigen is header-only and target-independent, so the host copy serves cross builds too — no
// closure ships it. pkg-config is the devShell's route: flake.nix puts eigen in buildInputs so
// the setup hook exports PKG_CONFIG_PATH, and the system prefixes cover a nix-less clone.
//   1. -Deigen=<path>   2. $XDYN_EIGEN   3. pkg-config eigen3   4. the usual system prefixes
fn resolveEigen(b: *std.Build) ?[]const u8 {
    if (b.option([]const u8, "eigen", "Eigen 3 include directory (default: probe the host)") orelse
        b.graph.environ_map.get("XDYN_EIGEN")) |explicit| return explicit;

    if (probeEigenPkgConfig(b)) |probed| return probed;

    for ([_][]const u8{ "/usr/include/eigen3", "/usr/local/include/eigen3", "/opt/homebrew/include/eigen3" }) |candidate|
        if (hasEigen(b, candidate)) return candidate;

    std.log.warn("Eigen not found (tried pkg-config and the usual prefixes). " ++
        "Point at it with -Deigen=<dir> or $XDYN_EIGEN — the dir that contains Eigen/Core.", .{});
    return null;
}

fn probeEigenPkgConfig(b: *std.Build) ?[]const u8 {
    var code: u8 = 0;
    const out = b.runAllowFail(&.{ "pkg-config", "--cflags-only-I", "eigen3" }, &code, .ignore) catch return null;
    if (code != 0) return null;
    var it = std.mem.tokenizeAny(u8, out, " \t\r\n");
    while (it.next()) |token| {
        if (!std.mem.startsWith(u8, token, "-I")) continue;
        if (hasEigen(b, token[2..])) return b.dupe(token[2..]);
    }
    return null;
}

fn hasEigen(b: *std.Build, dir: []const u8) bool {
    var buf: [std.fs.max_path_bytes]u8 = undefined;
    const probe = std.fmt.bufPrint(&buf, "{s}/Eigen/Core", .{dir}) catch return false;
    std.Io.Dir.cwd().access(b.graph.io, probe, .{}) catch return false;
    return true;
}

// =============================================================================
// Helpers
// =============================================================================

fn linkWindowsSystemLibs(m: *std.Build.Module) void {
    if (!target_is_windows) return;
    for ([_][]const u8{ "ws2_32", "mswsock", "crypt32", "bcrypt", "iphlpapi", "dbghelp", "advapi32" }) |lib|
        m.linkSystemLibrary(lib, .{});
}

fn addCommonIncludes(b: *std.Build, m: *std.Build.Module) void {
    if (target_is_windows) m.addCMacro("YAML_CPP_STATIC_DEFINE", "1");
    // -isystem, not -I: the closure is third-party, and its diagnostics are not ours to fix
    m.addSystemIncludePath(.{ .cwd_relative = b.fmt("{s}/install/include", .{deps_root}) });
    m.addIncludePath(b.path("."));
    m.addIncludePath(b.path("xdyn/cli"));
    m.addIncludePath(b.path("external/ssc"));
    m.addIncludePath(b.path("external"));
    m.addIncludePath(codegen.proto_messages);
    m.addIncludePath(codegen.proto_services);
    m.addSystemIncludePath(b.path("external/thirdparty/eigen3-hdf5"));
    m.addSystemIncludePath(b.path("external/thirdparty"));
    // System include: stb is warning-noisy under -Wall -Wextra and is not ours to fix
    m.addSystemIncludePath(b.path("external/stb"));
    if (eigen_include) |eigen| m.addSystemIncludePath(.{ .cwd_relative = eigen });
}

fn buildExe(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    libxdyn: *std.Build.Step.Compile,
    spec: Executable,
    flags: []const []const u8,
) void {
    const m = b.createModule(.{ .target = target, .optimize = optimize, .link_libcpp = true });
    addCommonIncludes(b, m);
    m.addCSourceFiles(.{ .root = b.path("."), .files = &.{spec.source}, .flags = flags });
    addProtoSources(b, m, spec.protos);
    m.linkLibrary(libxdyn);
    m.addObjectFile(.{ .cwd_relative = b.fmt("{s}/libxdyndeps_core.a", .{deps_root}) });
    linkWindowsSystemLibs(m);
    const exe = b.addExecutable(.{ .name = spec.name, .root_module = m });
    exe.step.dependOn(codegen.step); // for the in-tree half of the codegen
    if (!target_is_windows) exe.pie = false; // prebuilt deps may be non-PIC (PIE is N/A on COFF)
    b.installArtifact(exe);
}

// =============================================================================
// Python wrapper — the one mandatory shared object
// =============================================================================

const PythonEnv = struct {
    exe: []const u8,
    include: []const u8,
    pybind11: []const u8,
    ext_suffix: []const u8,
};

fn addPythonWrapper(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    libxdyn: *std.Build.Step.Compile,
    cpp_flags: []const []const u8,
) void {
    const step = b.step("python", "Build the pybind11 extension module (needs -Dpython or a venv)");
    const env = resolvePython(b) orelse {
        step.dependOn(&b.addFail(
            "no usable interpreter: need one with pybind11 importable. " ++
                "Point at it with -Dpython=<python> or $XDYN_PYTHON, or run `mise run python:setup`.",
        ).step);
        return;
    };

    var flags = concat(b, cpp_flags, &.{"-fvisibility=hidden"});

    if (b.option([]const u8, "git-version", "Version stamped into the module's __version__ (default: dev)")) |v|
        flags = concat(b, flags, &.{b.fmt("-DGIT_VERSION=\"{s}\"", .{v})});

    const m = b.createModule(.{ .target = target, .optimize = optimize, .link_libcpp = true, .pic = true });
    addCommonIncludes(b, m);
    m.addSystemIncludePath(.{ .cwd_relative = env.include });
    m.addSystemIncludePath(.{ .cwd_relative = env.pybind11 });
    m.addCSourceFiles(.{ .root = b.path("xdyn_wrapper_python/src"), .files = &.{
        "main.cpp",         "py_ssc.cpp",      "py_xdyn_core.cpp",  "py_xdyn_exe.cpp",
        "py_xdyn_data.cpp", "py_xdyn_env.cpp", "py_xdyn_force.cpp", "py_xdyn_hdb.cpp",
    }, .flags = flags });
    m.linkLibrary(libxdyn);
    m.addObjectFile(.{ .cwd_relative = b.fmt("{s}/libxdyndeps_core.a", .{deps_root}) });
    linkWindowsSystemLibs(m);

    const lib = b.addLibrary(.{ .name = "pyxdyn", .root_module = m, .linkage = .dynamic });
    lib.step.dependOn(codegen.step);
    lib.linker_allow_shlib_undefined = true;

    const install = b.addInstallFileWithDir(
        lib.getEmittedBin(),
        .prefix,
        b.fmt("python/xdyn{s}", .{env.ext_suffix}),
    );
    step.dependOn(&install.step);
}

fn resolvePython(b: *std.Build) ?PythonEnv {
    const explicit = b.option([]const u8, "python", "Interpreter to build the extension module for (needs pybind11)") orelse
        b.graph.environ_map.get("XDYN_PYTHON");

    const candidates: []const []const u8 = if (explicit) |e| &.{e} else &.{
        b.pathJoin(&.{ b.build_root.path orelse ".", "build", "venv", "wrapper-3.10", "bin", "python" }),
    };

    const probe =
        \\import sysconfig, pybind11
        \\print(sysconfig.get_paths()["include"])
        \\print(pybind11.get_include())
        \\print(sysconfig.get_config_var("EXT_SUFFIX"))
    ;

    for (candidates) |exe| {
        var code: u8 = 0;
        const out = b.runAllowFail(&.{ exe, "-c", probe }, &code, .ignore) catch continue;
        if (code != 0) continue;
        var it = std.mem.tokenizeScalar(u8, out, '\n');
        const include = it.next() orelse continue;
        const pybind11 = it.next() orelse continue;
        const ext_suffix = it.next() orelse continue;
        return .{
            .exe = b.dupe(exe),
            .include = b.dupe(include),
            .pybind11 = b.dupe(pybind11),
            .ext_suffix = b.dupe(std.mem.trim(u8, ext_suffix, " \r")),
        };
    }
    return null;
}

// =============================================================================
// Globbing the source directories
// =============================================================================

fn xdynDirs(b: *std.Build) []const []const u8 {
    var root = b.build_root.handle.openDir(b.graph.io, "xdyn", .{ .iterate = true }) catch
        @panic("cannot open xdyn/");
    defer root.close(b.graph.io);

    var found: std.ArrayList([]const u8) = .empty;
    var it = root.iterate();
    while (it.next(b.graph.io) catch @panic("cannot read xdyn/")) |entry| {
        if (entry.kind != .directory) continue;
        found.append(b.allocator, b.dupe(entry.name)) catch @panic("OOM");
    }
    return sorted(b, &found);
}

fn xdynModuleSources(b: *std.Build) []const []const u8 {
    var found: std.ArrayList([]const u8) = .empty;
    for (xdynDirs(b)) |dir| {
        if (std.mem.eql(u8, dir, "executables")) continue;
        for (sourcesIn(b, b.pathJoin(&.{ "xdyn", dir }), ".cpp", &compiled_separately)) |name|
            found.append(b.allocator, b.pathJoin(&.{ dir, name })) catch @panic("OOM");
    }
    return sorted(b, &found);
}

// Every `ext` file directly in `dir`, minus `excluded`.
fn sourcesIn(b: *std.Build, dir: []const u8, ext: []const u8, excluded: []const []const u8) []const []const u8 {
    var handle = b.build_root.handle.openDir(b.graph.io, dir, .{ .iterate = true }) catch
        std.debug.panic("cannot open {s} to collect sources", .{dir});
    defer handle.close(b.graph.io);

    var found: std.ArrayList([]const u8) = .empty;
    var it = handle.iterate();
    while (it.next(b.graph.io) catch std.debug.panic("cannot read {s}", .{dir})) |entry| {
        if (entry.kind != .file) continue;
        if (!std.mem.endsWith(u8, entry.name, ext)) continue;
        for (excluded) |skip| {
            if (std.mem.eql(u8, entry.name, skip)) break;
        } else found.append(b.allocator, b.dupe(entry.name)) catch @panic("OOM");
    }
    return sorted(b, &found);
}

fn unitTestSources(b: *std.Build) []const []const u8 {
    var root = b.build_root.handle.openDir(b.graph.io, "xdyn", .{ .iterate = true }) catch
        @panic("cannot open xdyn/ to collect unit tests");
    defer root.close(b.graph.io);

    var found: std.ArrayList([]const u8) = .empty;
    var walker = root.walk(b.allocator) catch @panic("OOM");
    defer walker.deinit();

    while (walker.next(b.graph.io) catch @panic("cannot walk xdyn/")) |entry| {
        if (entry.kind != .file) continue;
        if (entry.depth() != 3) continue;
        if (!std.mem.endsWith(u8, entry.basename, ".cpp")) continue;
        if (std.mem.indexOf(u8, entry.path, "unit_tests") == null) continue;
        found.append(b.allocator, b.dupe(entry.path)) catch @panic("OOM");
    }
    return sorted(b, &found);
}

fn sorted(b: *std.Build, found: *std.ArrayList([]const u8)) []const []const u8 {
    std.mem.sort([]const u8, found.items, {}, struct {
        fn lessThan(_: void, a: []const u8, c: []const u8) bool {
            return std.mem.order(u8, a, c) == .lt;
        }
    }.lessThan);
    return found.toOwnedSlice(b.allocator) catch @panic("OOM");
}
