// xdyn's own code: its half of libxdyn, the executables, and the unit-test runner.
// Sources are globbed, so a new .cpp under xdyn/<module>/ or its unit_tests/ registers itself.
const std = @import("std");
const root = @import("../build.zig");

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
};

// Warnings the tree already satisfies, kept satisfied by -Werror: a warning nobody promotes is one nobody sees.
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

// Globbed out of their module and added back by addSources, each with a flag of its own.
const compiled_separately = [_][]const u8{ "demo_scripts.cpp", "generate_test_ship.cpp" };

// The hand-written C++ flag set, shared by every component. -Wno-date-time: h5_version.c wants
// __DATE__. stack-exhausted stays a warning: it fires on stl_data.cpp depending on the runner's stack.
pub fn cppFlags(ctx: *const root.Ctx) []const []const u8 {
    const shim_hpp = root.fromRoot(ctx.b, "xdyn/compat/ssc_serialize_compat.hpp");
    return root.concat(ctx.b, ctx.withDebug(&.{
        "-std=gnu++17", "-Wall",    "-Wextra", "-Werror", "-Wno-date-time", "-Wno-error=stack-exhausted",
        "-fPIC",        "-include", shim_hpp,
    }), &warning_flags);
}

pub fn addSources(ctx: *const root.Ctx, lib: *std.Build.Module) void {
    const b = ctx.b;
    const cpp = ctx.cpp_flags;
    const c_flags = ctx.withDebug(&.{ "-std=gnu11", "-Wall", "-Wextra", "-Werror", "-Wno-date-time", "-fno-common", "-fPIC" });

    // xdyn/<module>/*.cpp: depth 2 leaves xdyn/*/unit_tests/ to the test runner.
    lib.addCSourceFiles(.{ .root = b.path("xdyn"), .files = moduleSources(b), .flags = cpp });
    lib.addCSourceFile(.{ .file = ctx.codegen.git_sha_c, .flags = c_flags });
    lib.addCSourceFiles(.{ .root = b.path("xdyn/interface_hdf5"), .files = &.{ "h5_tools.c", "h5_version.c" }, .flags = c_flags });
    lib.addCSourceFiles(.{ .root = b.path("xdyn/observers_and_api"), .files = &.{"demo_scripts.cpp"}, .flags = root.concat(b, cpp, &.{"-Wno-c23-extensions"}) });
    // One 253906-character string literal; ISO requires only 65536.
    lib.addCSourceFiles(.{ .root = b.path("xdyn/binary_stl_data"), .files = &.{"generate_test_ship.cpp"}, .flags = root.concat(b, cpp, &.{"-Wno-overlength-strings"}) });
    ctx.addProtoSources(lib, &.{ "wave_types", "wave_grpc", "force", "controller" });
}

pub fn addExecutables(ctx: *const root.Ctx, libxdyn: *std.Build.Step.Compile) void {
    const b = ctx.b;
    for (executables) |spec| {
        const m = ctx.module(.{});
        m.addCSourceFiles(.{ .root = b.path("."), .files = &.{spec.source}, .flags = ctx.cpp_flags });
        ctx.addProtoSources(m, spec.protos);
        m.linkLibrary(libxdyn);
        ctx.linkDeps(m);
        const exe = b.addExecutable(.{ .name = spec.name, .root_module = m });
        exe.step.dependOn(ctx.codegen.step);
        if (!ctx.isWindows()) exe.pie = false; // the closure may be non-PIC; PIE is N/A on COFF
        b.installArtifact(exe);
    }
}

// run_all_tests, with gtest and gmock from the closure: `zig build test`.
// Returns the `test` step, which SSC's suite joins.
pub fn addTests(ctx: *const root.Ctx, libxdyn: *std.Build.Step.Compile) *std.Build.Step {
    const b = ctx.b;
    const m = ctx.module(.{});
    // The tests include their module's headers by bare name.
    for (moduleDirs(b)) |dir| m.addIncludePath(b.path(b.pathJoin(&.{ "xdyn", dir })));
    m.addCSourceFiles(.{ .root = b.path("xdyn/executables"), .files = &.{"run_all_tests.cpp"}, .flags = ctx.cpp_flags });
    m.addCSourceFiles(.{ .root = b.path("xdyn"), .files = unitTestSources(b), .flags = ctx.cpp_flags });
    m.linkLibrary(libxdyn);
    m.addObjectFile(.{ .cwd_relative = b.fmt("{s}/libxdyndeps_test.a", .{ctx.deps_root}) });
    ctx.linkDeps(m);

    const exe = b.addExecutable(.{ .name = "run_all_tests", .root_module = m });
    exe.step.dependOn(ctx.codegen.step);
    if (!ctx.isWindows()) exe.pie = false;
    b.installArtifact(exe);

    const run = b.addRunArtifact(exe);
    const scratch = b.addWriteFiles();
    _ = scratch.add(".keep", "");
    run.setCwd(scratch.getDirectory());
    run.addPassthruArgs();
    const step = b.step("test", "Run all C++ unit tests");
    step.dependOn(&run.step);
    return step;
}

fn moduleDirs(b: *std.Build) []const []const u8 {
    var dir = b.root.openDir(b.graph.io, "xdyn", .{ .iterate = true }) catch @panic("cannot open xdyn/");
    defer dir.close(b.graph.io);

    var found: std.ArrayList([]const u8) = .empty;
    var it = dir.iterate();
    while (it.next(b.graph.io) catch @panic("cannot read xdyn/")) |entry| {
        if (entry.kind != .directory) continue;
        found.append(b.allocator, b.dupe(entry.name)) catch @panic("OOM");
    }
    return root.sorted(b, &found);
}

fn moduleSources(b: *std.Build) []const []const u8 {
    var found: std.ArrayList([]const u8) = .empty;
    for (moduleDirs(b)) |dir| {
        if (std.mem.eql(u8, dir, "executables")) continue;
        for (root.sourcesIn(b, b.pathJoin(&.{ "xdyn", dir }), ".cpp", &compiled_separately)) |name|
            found.append(b.allocator, b.pathJoin(&.{ dir, name })) catch @panic("OOM");
    }
    return root.sorted(b, &found);
}

fn unitTestSources(b: *std.Build) []const []const u8 {
    var dir = b.root.openDir(b.graph.io, "xdyn", .{ .iterate = true }) catch @panic("cannot open xdyn/");
    defer dir.close(b.graph.io);

    var found: std.ArrayList([]const u8) = .empty;
    var walker = dir.walk(b.allocator) catch @panic("OOM");
    defer walker.deinit();
    while (walker.next(b.graph.io) catch @panic("cannot walk xdyn/")) |entry| {
        if (entry.kind != .file) continue;
        if (entry.depth() != 3) continue;
        if (!std.mem.endsWith(u8, entry.basename, ".cpp")) continue;
        if (std.mem.indexOf(u8, entry.path, "unit_tests") == null) continue;
        found.append(b.allocator, b.dupe(entry.path)) catch @panic("OOM");
    }
    return root.sorted(b, &found);
}
