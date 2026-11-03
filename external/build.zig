// The vendored sources compiled into libxdyn from external/: thirdparty/.
const std = @import("std");
const root = @import("../build.zig");

pub fn addThirdparty(ctx: *const root.Ctx, lib: *std.Build.Module) void {
    lib.addCSourceFiles(.{
        .root = ctx.b.path("external/thirdparty/boost_program_options_descriptions"),
        .files = &.{ "CustomOptionDescription.cpp", "OptionPrinter.cpp" },
        .flags = ctx.cpp_flags,
    });
}
