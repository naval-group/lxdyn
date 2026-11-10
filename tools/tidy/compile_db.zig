// Fills in what only the make phase knows: zig's own clang flags, and where the generators wrote.
// Usage: compile_db <template> <zig c++ -### stderr> <out> [generated dir]...
const std = @import("std");

pub fn main(init: std.process.Init) !void {
    const io = init.io;
    const arena = init.arena.allocator();
    const args = try init.minimal.args.toSlice(arena);
    const cwd = std.Io.Dir.cwd();

    const template = try cwd.readFileAlloc(io, args[1], arena, .limited(64 << 20));
    const driver = try cwd.readFileAlloc(io, args[2], arena, .limited(1 << 20));

    var db = try std.mem.replaceOwned(u8, arena, template, "\"@zig@\"", try zigFlags(arena, driver));
    for (args[4..], 0..) |dir, i| {
        const absolute = try cwd.realPathFileAlloc(io, dir, arena);
        db = try std.mem.replaceOwned(u8, arena, db, try std.fmt.allocPrint(arena, "@gen{d}@", .{i}), try jsonChars(arena, absolute));
    }
    try cwd.writeFile(io, .{ .sub_path = args[3], .data = db });
}

// zig's target, headers and -D set, from its cc1 line; -nostdinc keeps every other header out.
fn zigFlags(arena: std.mem.Allocator, driver: []const u8) ![]const u8 {
    var lines = std.mem.tokenizeScalar(u8, driver, '\n');
    const cc1 = while (lines.next()) |line| {
        if (std.mem.indexOf(u8, line, "\"-cc1\"") != null) break line;
    } else return error.NoCc1Line;

    // Every token is quoted, so the odd fields of a split on '"' are the tokens.
    var tokens: std.ArrayList([]const u8) = .empty;
    var fields = std.mem.splitScalar(u8, cc1, '"');
    var odd = false;
    while (fields.next()) |field| : (odd = !odd) if (odd) try tokens.append(arena, field);

    var flags: std.ArrayList([]const u8) = .empty;
    try flags.appendSlice(arena, &.{ "-nostdinc", "-nostdinc++" });
    var i: usize = 0;
    while (i + 1 < tokens.items.len) : (i += 1) {
        const value = tokens.items[i + 1];
        if (std.mem.eql(u8, tokens.items[i], "-triple")) {
            try flags.append(arena, try std.fmt.allocPrint(arena, "--target={s}", .{value}));
        } else if (std.mem.eql(u8, tokens.items[i], "-isystem")) {
            try flags.appendSlice(arena, &.{ "-isystem", value });
        } else if (std.mem.eql(u8, tokens.items[i], "-D")) {
            try flags.append(arena, try std.fmt.allocPrint(arena, "-D{s}", .{value}));
        } else continue;
        i += 1;
    }
    const list = try std.json.Stringify.valueAlloc(arena, flags.items, .{});
    return list[1 .. list.len - 1];
}

fn jsonChars(arena: std.mem.Allocator, s: []const u8) ![]const u8 {
    const quoted = try std.json.Stringify.valueAlloc(arena, s, .{});
    return quoted[1 .. quoted.len - 1];
}
