// `zig build tidy`: compile_commands.json for clang-tidy and clangd, from what the other steps compile.
const std = @import("std");
const root = @import("../../build.zig");
const LazyPath = std.Build.LazyPath;

// Vendored code and f2c's output, someone else's to lint.
const excluded = [_][]const u8{ "external/", "ssc/f2c/", "ssc/json/rapidjson/", "xdyn_wrapper_fmi/fmi2/" };

const Entry = struct { directory: []const u8, file: []const u8, arguments: []const []const u8 };

pub fn add(ctx: *const root.Ctx) void {
    const b = ctx.b;
    const io = b.graph.io;
    var db: Db = .{
        .ctx = ctx,
        .root_dir = std.Io.Dir.cwd().realPathFileAlloc(io, root.fromRoot(b, "."), b.allocator) catch @panic("cannot resolve the build root"),
        .cwd = std.Io.Dir.cwd().realPathFileAlloc(io, ".", b.allocator) catch @panic("cannot resolve the working directory"),
    };
    // Walked before the step exists, so the database never lists itself.
    for (b.top_level_steps.values()) |tls| db.walk(&tls.step);
    const step = b.step("tidy", "Write compile_commands.json, for clang-tidy and clangd");

    if (!db.has_python) step.dependOn(&b.addFail("the Python wrapper is not configured, so the database would leave it out: " ++
        "run `mise run python:setup`, or pass -Dpython=<python>.").step);

    const template = b.addWriteFiles().add("compile_commands.in.json", std.json.Stringify.valueAlloc(b.allocator, db.entries.items, .{}) catch @panic("OOM"));
    const triple = ctx.target.query.zigTriple(b.allocator) catch @panic("OOM");
    const probe = b.addSystemCommand(&.{ b.graph.zig_exe, "c++", "-target", triple, "-###", "-c", "-x", "c++", "/dev/null" });

    const filler = b.addExecutable(.{ .name = "compile_db", .root_module = b.createModule(.{
        .root_source_file = b.path("tools/tidy/compile_db.zig"),
        .target = b.graph.host,
    }) });
    const fill = b.addRunArtifact(filler);
    fill.addFileArg(template);
    fill.addFileArg(probe.captureStdErr(.{}));
    const out = fill.addOutputFileArg("compile_commands.json");
    for (db.generated.items) |dir| fill.addDirectoryArg(dir);

    const update = b.addUpdateSourceFiles();
    update.addCopyFileToSource(out, "compile_commands.json");
    step.dependOn(&update.step);
}

const Db = struct {
    ctx: *const root.Ctx,
    root_dir: []const u8,
    cwd: []const u8,
    seen_steps: std.AutoHashMapUnmanaged(*std.Build.Step, void) = .empty,
    seen_files: std.StringHashMapUnmanaged(void) = .empty,
    // The generated include directories, which compile_db.zig fills in as @gen<index>@.
    generated: std.ArrayList(LazyPath) = .empty,
    entries: std.ArrayList(Entry) = .empty,
    has_python: bool = false,

    fn walk(db: *Db, step: *std.Build.Step) void {
        const gpa = db.ctx.b.allocator;
        if ((db.seen_steps.getOrPut(gpa, step) catch @panic("OOM")).found_existing) return;
        if (step.cast(std.Build.Step.Compile)) |compile| db.addModule(compile.root_module);
        for (step.dependencies.items) |dep| db.walk(dep);
    }

    fn addModule(db: *Db, m: *std.Build.Module) void {
        const b = db.ctx.b;
        var common: std.ArrayList([]const u8) = .empty;
        common.appendSlice(b.allocator, &.{ "clang++", "@zig@" }) catch @panic("OOM");
        if (db.ctx.optimize != .debug) common.append(b.allocator, "-DNDEBUG") catch @panic("OOM");
        common.appendSlice(b.allocator, m.c_macros.items) catch @panic("OOM");
        for (m.include_dirs.items) |dir| {
            const flag, const path = switch (dir) {
                .path => |p| .{ "-I", p },
                .path_system => |p| .{ "-isystem", p },
                .path_after => |p| .{ "-idirafter", p },
                // linkLibrary's, which adds only the headers a library installs.
                .other_step => |lib| if (lib.installed_headers.items.len == 0) continue else @panic("tidy: a linked library installs headers"),
                else => std.debug.panic("tidy: unsupported include directory kind {t}", .{dir}),
            };
            common.appendSlice(b.allocator, &.{ flag, db.resolve(path) }) catch @panic("OOM");
        }
        for (m.link_objects.items) |object| switch (object) {
            .c_source_files => |set| for (set.files) |file| db.addSource(common.items, set.root.path(b, file), set.flags, set.language),
            .c_source_file => |source| db.addSource(common.items, source.file, source.flags, source.language),
            else => {},
        };
    }

    fn addSource(db: *Db, common: []const []const u8, file: LazyPath, flags: []const []const u8, language: ?std.Build.Module.CSourceLanguage) void {
        const b = db.ctx.b;
        // A generated source is protoc's.
        const sub_path = switch (file) {
            .src_path => |src| src.sub_path,
            else => return,
        };
        const cpp = if (language) |lang| lang == .cpp else std.mem.endsWith(u8, sub_path, ".cpp") or std.mem.endsWith(u8, sub_path, ".cc");
        if (!cpp) return;
        for (excluded) |prefix| if (std.mem.startsWith(u8, sub_path, prefix)) return;
        if ((db.seen_files.getOrPut(b.allocator, sub_path) catch @panic("OOM")).found_existing) return;
        if (std.mem.startsWith(u8, sub_path, "xdyn_wrapper_python/")) db.has_python = true;

        const absolute = b.pathJoin(&.{ db.root_dir, sub_path });
        const language_flags: []const []const u8 = if (language != null) &.{ "-x", "c++" } else &.{};
        const arguments = root.concat(b, root.concat(b, common, flags), root.concat(b, language_flags, &.{ "-c", absolute }));
        db.entries.append(b.allocator, .{ .directory = db.root_dir, .file = absolute, .arguments = arguments }) catch @panic("OOM");
    }

    fn resolve(db: *Db, path: LazyPath) []const u8 {
        const b = db.ctx.b;
        return switch (path) {
            .src_path => |src| std.fs.path.resolve(b.allocator, &.{ db.root_dir, src.sub_path }) catch @panic("OOM"),
            .cwd_relative => |p| std.fs.path.resolve(b.allocator, &.{ db.cwd, p }) catch @panic("OOM"),
            .generated => {
                for (db.generated.items, 0..) |known, i|
                    if (std.meta.eql(known.generated.index, path.generated.index) and known.generated.up == path.generated.up and
                        std.mem.eql(u8, known.generated.sub_path, path.generated.sub_path)) return b.fmt("@gen{d}@", .{i});
                db.generated.append(b.allocator, path) catch @panic("OOM");
                return b.fmt("@gen{d}@", .{db.generated.items.len - 1});
            },
            else => std.debug.panic("tidy: unsupported include path {f}", .{path}),
        };
    }
};
