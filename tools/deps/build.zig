// Locating a dependency closure and checking it is the one assets.sha256 names: the recipes that
// build and fetch it are in this folder too.
const std = @import("std");
const root = @import("../../build.zig");

// Keyed on the full triple, since gnu and musl closures differ in exactly the libc. Order:
// -Ddeps, $XDYN_DEPS_<TRIPLE>, $XDYN_DEPS (an override for every target), ./libcxx-<triple>.
pub fn resolveRoot(b: *std.Build, target: std.Build.ResolvedTarget, codegen: *std.Build.Step) []const u8 {
    const t = target.result;
    const triple = b.fmt("{s}-{s}-{s}", .{ @tagName(t.cpu.arch), @tagName(t.os.tag), @tagName(t.abi) });

    const env_key = b.fmt("XDYN_DEPS_{s}", .{triple});
    for (env_key) |*c| c.* = switch (c.*) {
        '-' => '_',
        'a'...'z' => c.* - ('a' - 'A'),
        else => c.*,
    };

    const dir = b.option([]const u8, "deps", "libc++ dependency closure for the target (default: ./libcxx-<triple>)") orelse
        b.graph.environ_map.get(env_key) orelse
        b.graph.environ_map.get("XDYN_DEPS") orelse
        root.fromRoot(b, b.fmt("libcxx-{s}", .{triple}));

    // A warning, so `zig build --help` still works without a closure.
    if (std.Io.Dir.cwd().access(b.graph.io, b.pathJoin(&.{ dir, "libxdyndeps_core.a" }), .{})) |_|
        checkStamp(b, codegen, dir, triple, "install/lib")
    else |_|
        std.log.warn("no libxdyndeps_core.a under '{s}': the {s} closure is missing. Fetch it " ++
            "with `mise run deps:fetch {s}`, build it with `mise run deps:{s}`, or name one " ++
            "with -Ddeps=<path> or ${s}.", .{ dir, triple, triple, triple, env_key });
    return dir;
}

// fetch.sh leaves the asset's assets.sha256 line in the tree as asset.sha256. A failing step under
// `step`, which every artifact depends on, so `zig build --help` still works on a stale tree.
// Only a local build has `built_marker`: a tree without it or a stamp came from an older fetch.sh.
pub fn checkStamp(b: *std.Build, step: *std.Build.Step, dir: []const u8, flavor: []const u8, built_marker: ?[]const u8) void {
    const io = b.graph.io;
    const cwd = std.Io.Dir.cwd();
    const refetch = b.fmt("Move it aside and run `mise run deps:fetch {s}`.", .{flavor});
    const stamp = cwd.readFileAlloc(io, b.pathJoin(&.{ dir, "asset.sha256" }), b.allocator, .limited(4096)) catch {
        const built = if (built_marker) |marker|
            if (cwd.access(io, b.pathJoin(&.{ dir, marker }), .{})) |_| true else |_| false
        else
            true;
        if (built)
            std.log.warn("'{s}' has no asset.sha256: a local build, not checked against tools/deps/assets.sha256.", .{dir})
        else
            step.dependOn(&b.addFail(b.fmt("'{s}' was fetched before closures carried a stamp. {s}", .{ dir, refetch })).step);
        return;
    };
    const sums = cwd.readFileAlloc(io, root.fromRoot(b, "tools/deps/assets.sha256"), b.allocator, .limited(1 << 16)) catch |err|
        std.debug.panic("reading tools/deps/assets.sha256: {t}", .{err});
    const want = std.mem.trim(u8, stamp, " \r\n");
    var lines = std.mem.tokenizeAny(u8, sums, "\r\n");
    while (lines.next()) |line| if (std.mem.eql(u8, line, want)) return;
    step.dependOn(&b.addFail(b.fmt("'{s}' is not the {s} asset tools/deps/assets.sha256 names. {s}", .{ dir, flavor, refetch })).step);
}
