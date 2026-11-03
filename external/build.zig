// The vendored sources compiled into libxdyn: SSC with its f2c and websocket, and thirdparty/.
// SSC moves to ssc/build.zig when it is absorbed (lxdyn#13).
const std = @import("std");
const root = @import("../build.zig");

const ssc_modules = [_][]const u8{
    "external/ssc/ssc/csv_file_reader", "external/ssc/ssc/data_source",
    "external/ssc/ssc/decode_unit",     "external/ssc/ssc/exception_handling",
    "external/ssc/ssc/integrate",       "external/ssc/ssc/interpolation",
    "external/ssc/ssc/json",            "external/ssc/ssc/kinematics",
    "external/ssc/ssc/numeric",         "external/ssc/ssc/random_data_generator",
    "external/ssc/ssc/solver",          "external/ssc/ssc/text_file_reader",
};

const ws_dir = "external/ssc/ssc/websocket/src";

const f2c_dir = "external/ssc/ssc/f2c";
const f2c_excluded = [_][]const u8{
    "main.c", "arithchk.c", "arithchk_ori.c", // carry main()
    "uninit.c", "signbit.c", // need a generated arith.h
    "pow_qq.c", "qbitbits.c", "qbitshft.c", "ftell64_.c", // need longint
    "dtime_.c", "etime_.c", // POSIX-only: break the windows target
    "getarg_.c", "iargc_.c", "s_paus.c", "i_ceiling.c", "i_len_trim.c", // unused
};

pub fn addSsc(ctx: *const root.Ctx, lib: *std.Build.Module) void {
    const b = ctx.b;
    // SSC's 11 misleading-indentation sites are all in .cpp files, so suppressing them here stays out of xdyn's headers.
    const ssc_flags = root.concat(b, ctx.cpp_flags, &.{"-Wno-misleading-indentation"});
    // No -Werror: f2c's Fortran-to-C output is not ours to keep clean.
    const f2c_flags = ctx.withDebug(&.{ "-std=gnu11", "-Wall", "-Wextra", "-fno-common", "-fPIC", "-Iexternal/ssc/ssc/f2c" });
    const sir_flags = ctx.withDebug(&.{ "-std=gnu++17", "-Wall", "-Werror", "-fPIC" });
    const ws_flags = ctx.withDebug(&.{
        "-std=gnu++17",                 "-Wall",                            "-Wextra",                          "-Werror",                            "-fPIC",
        "-D_WEBSOCKETPP_CPP11_STL_",    "-D_WEBSOCKETPP_CPP11_THREAD_",     "-D_WEBSOCKETPP_CPP11_FUNCTIONAL_", "-D_WEBSOCKETPP_CPP11_SYSTEM_ERROR_", "-D_WEBSOCKETPP_CPP11_RANDOM_DEVICE_",
        "-D_WEBSOCKETPP_CPP11_MEMORY_", "-Iexternal/ssc/ssc/websocket/inc", "-isystem",                         "external/websocketpp",
    });

    // Non-recursive, which skips SSC's own <module>/unit_tests/.
    for (ssc_modules) |dir|
        lib.addCSourceFiles(.{ .root = b.path(dir), .files = root.sourcesIn(b, dir, ".cpp", &.{}), .flags = ssc_flags });
    // The f2c-derived C in these two.
    for ([_][]const u8{ "external/ssc/ssc/integrate", "external/ssc/ssc/interpolation" }) |dir|
        lib.addCSourceFiles(.{ .root = b.path(dir), .files = root.sourcesIn(b, dir, ".c", &.{}), .flags = f2c_flags });
    lib.addCSourceFiles(.{ .root = b.path(f2c_dir), .files = root.sourcesIn(b, f2c_dir, ".c", &f2c_excluded), .flags = f2c_flags });
    // C++ function-style casts in a .c file.
    lib.addCSourceFiles(.{ .root = b.path("external/ssc/ssc/random_data_generator"), .files = &.{"sir_rand.c"}, .flags = sir_flags, .language = .cpp });
    lib.addCSourceFiles(.{ .root = b.path(ws_dir), .files = root.sourcesIn(b, ws_dir, ".cpp", &.{}), .flags = ws_flags });
}

pub fn addThirdparty(ctx: *const root.Ctx, lib: *std.Build.Module) void {
    lib.addCSourceFiles(.{
        .root = ctx.b.path("external/thirdparty/boost_program_options_descriptions"),
        .files = &.{ "CustomOptionDescription.cpp", "OptionPrinter.cpp" },
        .flags = ctx.cpp_flags,
    });
}
