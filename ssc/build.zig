// SSC, absorbed from naval-group/scientific_computing (PROVENANCE.md): compiled into libxdyn and
// tested by its own runner, run_ssc_tests.
const std = @import("std");
const root = @import("../build.zig");

const modules = [_][]const u8{
    "csv_file_reader", "data_source",           "decode_unit", "exception_handling",
    "integrate",       "interpolation",         "json",        "kinematics",
    "numeric",         "random_data_generator", "solver",      "text_file_reader",
};

// Included as <ssc/<module>.hpp>, each including every header at the top of its module.
const umbrellas = [_][]const u8{
    "csv_file_reader", "data_source",           "decode_unit", "exception_handling",
    "integrate",       "interpolation",         "json",        "kinematics",
    "macros",          "random_data_generator", "solver",      "text_file_reader",
    "websocket",
};

const tested = [_][]const u8{
    "csv_file_reader", "data_source",           "integrate", "interpolation",    "kinematics",
    "numeric",         "random_data_generator", "solver",    "text_file_reader", "websocket",
};

const Flags = struct { cpp: []const []const u8, f2c: []const []const u8, ws: []const []const u8 };

// Every file that includes websocketpp needs the same set: its templates otherwise get two layouts in one binary.
const websocketpp_defines = [_][]const u8{
    "-D_WEBSOCKETPP_CPP11_STL_",          "-D_WEBSOCKETPP_CPP11_THREAD_",        "-D_WEBSOCKETPP_CPP11_FUNCTIONAL_",
    "-D_WEBSOCKETPP_CPP11_SYSTEM_ERROR_", "-D_WEBSOCKETPP_CPP11_RANDOM_DEVICE_", "-D_WEBSOCKETPP_CPP11_MEMORY_",
};

fn flags(ctx: *const root.Ctx) Flags {
    const b = ctx.b;
    return .{
        // SSC's 11 misleading-indentation sites are all in .cpp files, so suppressing them here stays out of xdyn's headers.
        .cpp = root.concat(b, ctx.cpp_flags, &.{"-Wno-misleading-indentation"}),
        // No -Werror: f2c's Fortran-to-C output is not ours to keep clean.
        .f2c = ctx.withDebug(&.{ "-std=gnu11", "-Wall", "-Wextra", "-fno-common", "-fPIC", "-Issc/f2c" }),
        .ws = ctx.withDebug(&([_][]const u8{ "-std=gnu++17", "-Wall", "-Wextra", "-Werror", "-fPIC" } ++ websocketpp_defines ++
            [_][]const u8{ "-Issc/websocket/inc", "-isystem", "external/websocketpp" })),
    };
}

pub fn addSources(ctx: *const root.Ctx, lib: *std.Build.Module) void {
    const b = ctx.b;
    const f = flags(ctx);
    // Non-recursive, which skips each module's unit_tests/.
    for (modules) |name| {
        const dir = b.pathJoin(&.{ "ssc", name });
        lib.addCSourceFiles(.{ .root = b.path(dir), .files = root.sourcesIn(b, dir, ".cpp", &.{}), .flags = f.cpp });
    }
    // The f2c-derived C in these two.
    for ([_][]const u8{ "ssc/integrate", "ssc/interpolation" }) |dir|
        lib.addCSourceFiles(.{ .root = b.path(dir), .files = root.sourcesIn(b, dir, ".c", &.{}), .flags = f.f2c });
    lib.addCSourceFiles(.{ .root = b.path("ssc/f2c"), .files = root.sourcesIn(b, "ssc/f2c", ".c", &.{}), .flags = f.f2c });
    // C++ function-style casts in a .c file.
    lib.addCSourceFiles(.{
        .root = b.path("ssc/random_data_generator"),
        .files = &.{"sir_rand.c"},
        .flags = ctx.withDebug(&.{ "-std=gnu++17", "-Wall", "-Werror", "-fPIC" }),
        .language = .cpp,
    });
    lib.addCSourceFiles(.{ .root = b.path("ssc/websocket/src"), .files = root.sourcesIn(b, "ssc/websocket/src", ".cpp", &.{}), .flags = f.ws });
}

// Into the cache rather than the source tree, so concurrent builds share nothing they write.
pub fn addUmbrellas(b: *std.Build) *std.Build.Step.WriteFile {
    const io = b.graph.io;
    const notice = std.Io.Dir.cwd().readFileAlloc(io, root.fromRoot(b, "ssc/copyright_notice.txt"), b.allocator, .limited(1 << 16)) catch |err|
        std.debug.panic("reading ssc/copyright_notice.txt: {t}", .{err});
    const files = b.addWriteFiles();
    for (umbrellas) |name| {
        var text: std.ArrayList(u8) = .empty;
        text.print(b.allocator, "/*\n{s}*/\n#ifndef {s}_guard\n#define {s}_guard\n\n", .{ notice, name, name }) catch @panic("OOM");
        for (headersIn(b, b.pathJoin(&.{ "ssc", name }))) |header|
            text.print(b.allocator, "#include \"ssc/{s}/{s}\"\n", .{ name, header }) catch @panic("OOM");
        text.appendSlice(b.allocator, "\n\n#endif\n") catch @panic("OOM");
        _ = files.add(b.fmt("ssc/{s}.hpp", .{name}), text.items);
    }
    return files;
}

// What the shell glob `<module>/*.h*` matched.
fn headersIn(b: *std.Build, dir: []const u8) []const []const u8 {
    var handle = b.root.openDir(b.graph.io, dir, .{ .iterate = true }) catch
        std.debug.panic("cannot open {s} to collect headers", .{dir});
    defer handle.close(b.graph.io);

    var found: std.ArrayList([]const u8) = .empty;
    var it = handle.iterate();
    while (it.next(b.graph.io) catch std.debug.panic("cannot read {s}", .{dir})) |entry| {
        if (entry.kind != .file or std.mem.indexOf(u8, entry.name, ".h") == null) continue;
        found.append(b.allocator, b.dupe(entry.name)) catch @panic("OOM");
    }
    return root.sorted(b, &found);
}

// SSC's suite, as SSC's own CMake built it: the sources each module's unit_tests/CMakeLists.txt listed.
pub fn addTests(ctx: *const root.Ctx, libxdyn: *std.Build.Step.Compile, test_step: *std.Build.Step) void {
    const b = ctx.b;
    const f = flags(ctx);
    const m = ctx.module(.{});
    m.addSystemIncludePath(b.path("external/websocketpp"));
    for (tested) |name| {
        const dir = b.pathJoin(&.{ "ssc", name, "unit_tests" });
        m.addIncludePath(b.path(b.pathJoin(&.{ dir, "inc" })));
        const src = b.pathJoin(&.{ dir, "src" });
        const cpp = if (std.mem.eql(u8, name, "websocket")) root.concat(b, f.cpp, &websocketpp_defines) else f.cpp;
        m.addCSourceFiles(.{ .root = b.path(src), .files = root.sourcesIn(b, src, ".cpp", &.{}), .flags = cpp });
        m.addCSourceFiles(.{ .root = b.path(src), .files = root.sourcesIn(b, src, ".c", &.{}), .flags = f.f2c });
    }
    m.addCSourceFiles(.{ .root = b.path("xdyn/executables"), .files = &.{"run_all_tests.cpp"}, .flags = ctx.cpp_flags });
    m.linkLibrary(libxdyn);
    m.addObjectFile(.{ .cwd_relative = b.fmt("{s}/libxdyndeps_test.a", .{ctx.deps_root}) });
    ctx.linkDeps(m);

    const exe = b.addExecutable(.{ .name = "run_ssc_tests", .root_module = m });
    exe.step.dependOn(ctx.codegen.step);
    if (!ctx.isWindows()) exe.pie = false;
    b.installArtifact(exe);

    const run = b.addRunArtifact(exe);
    const scratch = b.addWriteFiles();
    _ = scratch.add(".keep", "");
    run.setCwd(scratch.getDirectory());
    run.addPassthruArgs();
    test_step.dependOn(&run.step);
}
