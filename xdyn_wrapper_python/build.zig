// The pybind11 extension module: `zig build python`, installed as python/xdyn<EXT_SUFFIX>.
const std = @import("std");
const root = @import("../build.zig");

pub fn add(ctx: *const root.Ctx, libxdyn: *std.Build.Step.Compile) void {
    const b = ctx.b;
    const step = b.step("python", "Build the pybind11 extension module (needs -Dpython or a venv)");
    const env = resolvePython(b) orelse {
        step.dependOn(&b.addFail(
            "no usable interpreter: need one with pybind11 importable. " ++
                "Point at it with -Dpython=<python> or $XDYN_PYTHON, or run `mise run python:setup`.",
        ).step);
        return;
    };

    var flags = root.concat(b, ctx.cpp_flags, &.{"-fvisibility=hidden"});
    if (b.option([]const u8, "git-version", "Version stamped into the module's __version__ (default: dev)")) |v|
        flags = root.concat(b, flags, &.{b.fmt("-DGIT_VERSION=\"{s}\"", .{v})});

    const m = ctx.module(.{ .pic = true });
    m.addSystemIncludePath(.{ .cwd_relative = env.include });
    m.addSystemIncludePath(.{ .cwd_relative = env.pybind11 });
    m.addCSourceFiles(.{ .root = b.path("xdyn_wrapper_python/src"), .files = &.{
        "main.cpp",         "py_ssc.cpp",      "py_xdyn_core.cpp",  "py_xdyn_exe.cpp",
        "py_xdyn_data.cpp", "py_xdyn_env.cpp", "py_xdyn_force.cpp", "py_xdyn_hdb.cpp",
    }, .flags = flags });
    m.linkLibrary(libxdyn);
    ctx.linkDeps(m);

    const lib = b.addLibrary(.{ .name = "pyxdyn", .root_module = m, .linkage = .dynamic });
    lib.step.dependOn(ctx.codegen.step);
    lib.linker_allow_shlib_undefined = true;

    const install = b.addInstallFileWithDir(lib.getEmittedBin(), .prefix, b.fmt("python/xdyn{s}", .{env.ext_suffix}));
    step.dependOn(&install.step);
}

const PythonEnv = struct {
    include: []const u8,
    pybind11: []const u8,
    ext_suffix: []const u8,
};

fn resolvePython(b: *std.Build) ?PythonEnv {
    const explicit = b.option([]const u8, "python", "Interpreter to build the extension module for (needs pybind11)") orelse
        b.graph.environ_map.get("XDYN_PYTHON");

    const candidates: []const []const u8 = if (explicit) |e| &.{e} else &.{
        root.fromRoot(b, b.pathJoin(&.{ "build", "venv", "wrapper-3.10", "bin", "python" })),
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
            .include = b.dupe(include),
            .pybind11 = b.dupe(pybind11),
            .ext_suffix = b.dupe(std.mem.trim(u8, ext_suffix, " \r")),
        };
    }
    return null;
}
