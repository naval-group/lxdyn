// Locating the dependency closures: the recipes that build them are in this folder too.
const std = @import("std");

// Keyed on the full triple, since gnu and musl closures differ in exactly the libc.
pub fn resolveRoot(b: *std.Build, target: std.Build.ResolvedTarget, codegen: *std.Build.Step) []const u8 {
    const t = target.result;
    const triple = b.fmt("{s}-{s}-{s}", .{ @tagName(t.cpu.arch), @tagName(t.os.tag), @tagName(t.abi) });

    const dir = b.option([]const u8, "deps", "libc++ dependency closure for the target (default: $XDYN_CLOSURES/<triple>)") orelse
        fromClosures(b, triple) orelse {
        fail(b, codegen, "no closures: enter `nix develop`, or pass -Ddeps=<path>");
        return "";
    };
    std.Io.Dir.cwd().access(b.graph.io, b.pathJoin(&.{ dir, "libxdyndeps_core.a" }), .{}) catch
        fail(b, codegen, b.fmt("no {s} closure under '{s}': `nix develop .#cross` has every target's", .{ triple, dir }));
    return dir;
}

// protoc and grpc_cpp_plugin, for the machine running the build whatever the target.
pub fn resolveHost(b: *std.Build) ?[]const u8 {
    return b.option([]const u8, "deps-host", "Host tools providing protoc (default: $XDYN_CLOSURES/host-tools)") orelse
        fromClosures(b, "host-tools");
}

// `nix develop` sets $XDYN_CLOSURES to one folder per triple, and host-tools.
fn fromClosures(b: *std.Build, name: []const u8) ?[]const u8 {
    const closures = b.graph.environ_map.get("XDYN_CLOSURES") orelse return null;
    return b.pathJoin(&.{ closures, name });
}

// A failing step rather than a panic, so `zig build --help` still works without a closure.
pub fn fail(b: *std.Build, step: *std.Build.Step, message: []const u8) void {
    step.dependOn(&b.addFail(message).step);
}
