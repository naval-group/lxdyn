// The FMI 2.0 library an .fmu carries: `zig build fmu`, packed by make_fmu.py.
const std = @import("std");
const root = @import("../build.zig");

pub fn add(ctx: *const root.Ctx, libxdyn: *std.Build.Step.Compile) void {
    const b = ctx.b;
    const step = b.step("fmu", "Build the FMI 2.0 library an .fmu carries");
    const os = ctx.target.result.os.tag;

    // Stripped, since it travels inside every .fmu. Not on Windows: debug info goes to a .pdb
    // anyway, and stripping makes lld-link reject the closure's path_traits.obj.
    const m = ctx.module(.{ .pic = true, .strip = !ctx.debug and !ctx.isWindows() });
    m.addIncludePath(b.path("xdyn_wrapper_fmi/fmi2"));
    m.addCSourceFiles(.{
        .root = b.path("xdyn_wrapper_fmi/src"),
        .files = &.{ "fmu.cpp", "co_simulation.cpp", "model_exchange.cpp", "model.cpp" },
        .flags = root.concat(b, ctx.cpp_flags, &.{"-fvisibility=hidden"}),
    });
    m.linkLibrary(libxdyn);
    ctx.linkDeps(m);
    // dladdr and dlopen keep the library loaded (fmu.cpp); in libdl before glibc 2.34.
    if (os == .linux and ctx.target.result.abi.isGnu()) m.linkSystemLibrary("dl", .{});

    const lib = b.addLibrary(.{ .name = "xdyn_fmu", .root_module = m, .linkage = .dynamic });
    lib.step.dependOn(ctx.codegen.step);
    // The closure's archives export everything; only the fmi2* functions may leave the library,
    // or a host that already loaded Boost, yaml-cpp or gRPC would bind to ours.
    if (os == .linux) lib.setVersionScript(b.path("xdyn_wrapper_fmi/src/exports.map"));

    // FMI names the binary after modelIdentifier, with the platform's own extension.
    const name = if (ctx.isWindows()) "xdyn_fmu.dll" else "xdyn_fmu.so";
    const install = b.addInstallFileWithDir(lib.getEmittedBin(), .prefix, b.fmt("fmu/{s}", .{name}));
    step.dependOn(&install.step);
}
