// Options: -Dtarget -Ddeps -Ddeps-host -Deigen -Ddebug -Dgit-sha (`zig build --help` lists all).
// Each component's build lives beside its sources; this file resolves what they share.
const std = @import("std");
const builtin = @import("builtin");
const xdyn = @import("xdyn/build.zig");
const external = @import("external/build.zig");
const ssc = @import("ssc/build.zig");
const python = @import("xdyn_wrapper_python/build.zig");
const fmu = @import("xdyn_wrapper_fmi/build.zig");

// gnu at the 2.28 floor, so a native build links on the oldest glibc still in service.
const native_default: std.Target.Query = if (builtin.os.tag == .linux)
    .{ .abi = .gnu, .glibc_version = .{ .major = 2, .minor = 28, .patch = 0 } }
else
    .{};

const proto_files = [_][]const u8{
    "wave_types", "wave_grpc", "force", "controller", "cosimulation", "model_exchange",
};

pub fn build(b: *std.Build) void {
    // Configuration reads $XDYN_*, runs git, pkg-config and python: none of it is visible to the configure cache.
    b.graph.poisonCache();
    const target = b.standardTargetOptions(.{ .default_target = native_default });
    const codegen = addCodegen(b);
    var ctx: Ctx = .{
        .b = b,
        .target = target,
        .optimize = .ReleaseFast,
        .deps_root = resolveDepsRoot(b, target, codegen.step),
        .eigen_include = resolveEigen(b),
        .debug = b.option(bool, "debug", "Build xdyn -O0 -g for gdb (deps stay optimized)") orelse false,
        .codegen = codegen,
    };
    ctx.cpp_flags = xdyn.cppFlags(&ctx);
    // No -Werror: protoc's output is not ours to keep clean.
    ctx.proto_flags = ctx.withDebug(&.{ "-std=gnu++17", "-fPIC" });

    // libxdyn: one static archive of SSC, xdyn and the generated code.
    const lib = ctx.module(.{});
    ssc.addSources(&ctx, lib);
    xdyn.addSources(&ctx, lib);
    external.addThirdparty(&ctx, lib);
    const libxdyn = b.addLibrary(.{ .name = "xdyn", .root_module = lib, .linkage = .static });
    libxdyn.step.dependOn(ctx.codegen.step);
    b.installArtifact(libxdyn);

    xdyn.addExecutables(&ctx, libxdyn);
    ssc.addTests(&ctx, libxdyn, xdyn.addTests(&ctx, libxdyn));
    python.add(&ctx, libxdyn); // opt-in: zig build python
    fmu.add(&ctx, libxdyn); // opt-in: zig build fmu
}

// What every component shares: the target, the closure, the codegen and the flag sets.
pub const Ctx = struct {
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    deps_root: []const u8,
    eigen_include: ?[]const u8,
    debug: bool,
    codegen: Codegen,
    cpp_flags: []const []const u8 = &.{},
    proto_flags: []const []const u8 = &.{},

    pub fn isWindows(ctx: *const Ctx) bool {
        return ctx.target.result.os.tag == .windows;
    }

    // Appended last, since clang's last -O wins: xdyn at -O0, the closure still optimized.
    pub fn withDebug(ctx: *const Ctx, base: []const []const u8) []const []const u8 {
        if (!ctx.debug) return base;
        return concat(ctx.b, base, &.{ "-O0", "-g", "-fno-omit-frame-pointer" });
    }

    pub const ModuleOptions = struct { pic: ?bool = null, strip: ?bool = null };

    // A C++ module for the target, with every include path xdyn's sources expect.
    pub fn module(ctx: *const Ctx, opts: ModuleOptions) *std.Build.Module {
        const b = ctx.b;
        const m = b.createModule(.{
            .target = ctx.target,
            .optimize = ctx.optimize,
            .link_libcpp = true,
            .pic = opts.pic,
            .strip = opts.strip,
        });
        if (ctx.isWindows()) m.addCMacro("YAML_CPP_STATIC_DEFINE", "1");
        // -isystem for everything third-party: its diagnostics are not ours to fix.
        m.addSystemIncludePath(.{ .cwd_relative = b.fmt("{s}/install/include", .{ctx.deps_root}) });
        m.addIncludePath(b.path("."));
        m.addIncludePath(b.path("xdyn/cli"));
        m.addIncludePath(ctx.codegen.ssc_umbrellas);
        m.addIncludePath(b.path("external"));
        m.addIncludePath(ctx.codegen.proto_messages);
        m.addIncludePath(ctx.codegen.proto_services);
        m.addSystemIncludePath(b.path("external/thirdparty/eigen3-hdf5"));
        m.addSystemIncludePath(b.path("external/thirdparty"));
        m.addSystemIncludePath(b.path("external/stb"));
        if (ctx.eigen_include) |eigen| m.addSystemIncludePath(.{ .cwd_relative = eigen });
        return m;
    }

    // The closure's core archive, and on Windows the system libraries gRPC and Boost need.
    pub fn linkDeps(ctx: *const Ctx, m: *std.Build.Module) void {
        m.addObjectFile(.{ .cwd_relative = ctx.b.fmt("{s}/libxdyndeps_core.a", .{ctx.deps_root}) });
        if (!ctx.isWindows()) return;
        for ([_][]const u8{ "ws2_32", "mswsock", "crypt32", "bcrypt", "iphlpapi", "dbghelp", "advapi32" }) |lib|
            m.linkSystemLibrary(lib, .{});
    }

    pub fn addProtoSources(ctx: *const Ctx, m: *std.Build.Module, names: []const []const u8) void {
        const b = ctx.b;
        for (names) |name| {
            m.addCSourceFile(.{ .file = ctx.codegen.proto_messages.path(b, b.fmt("{s}.pb.cc", .{name})), .flags = ctx.proto_flags });
            m.addCSourceFile(.{ .file = ctx.codegen.proto_services.path(b, b.fmt("{s}.grpc.pb.cc", .{name})), .flags = ctx.proto_flags });
        }
    }
};

// =============================================================================
// Codegen: SSC's umbrella headers, the gRPC stubs, and the commit stamped into the binaries
// =============================================================================

pub const Codegen = struct {
    step: *std.Build.Step,
    proto_messages: std.Build.LazyPath,
    proto_services: std.Build.LazyPath,
    ssc_umbrellas: std.Build.LazyPath,
    git_sha_c: std.Build.LazyPath,
};

fn addCodegen(b: *std.Build) Codegen {
    const step = b.step("gen", "Run the code generators on their own");

    const umbrellas = ssc.addUmbrellas(b);
    step.dependOn(&umbrellas.step);

    // A host protoc even when cross-compiling: the generated C++ is arch-independent.
    const host = b.option([]const u8, "deps-host", "Host tools providing protoc (default: ./host-tools)") orelse
        b.graph.environ_map.get("XDYN_DEPS_HOST") orelse
        fromRoot(b, "host-tools");
    const bin = b.pathJoin(&.{ host, "install", "bin" });
    const protoc = b.pathJoin(&.{ bin, "protoc" });
    if (std.Io.Dir.cwd().access(b.graph.io, protoc, .{})) |_|
        checkStamp(b, step, host, "host-tools", null)
    else |_|
        std.log.warn("no protoc under '{s}': codegen will fail. Fetch it with `mise run deps:fetch " ++
            "host-tools`, or name one with -Ddeps-host=<path> or $XDYN_DEPS_HOST.", .{bin});

    const gen_proto = b.addSystemCommand(&.{protoc});
    gen_proto.setCwd(b.path("."));
    gen_proto.addFileInput(.{ .cwd_relative = protoc });
    gen_proto.addPrefixedFileArg("--plugin=protoc-gen-grpc=", .{ .cwd_relative = b.pathJoin(&.{ bin, "grpc_cpp_plugin" }) });
    const messages = gen_proto.addPrefixedOutputDirectoryArg("--cpp_out=", "proto");
    const services = gen_proto.addPrefixedOutputDirectoryArg("--grpc_out=", "proto-grpc");
    gen_proto.addPrefixedDirectoryArg("-I", b.path("proto"));
    for (proto_files) |name|
        gen_proto.addFileArg(b.path(b.fmt("proto/{s}.proto", .{name})));
    step.dependOn(&gen_proto.step);

    const sha = b.option([]const u8, "git-sha", "Commit stamped into the binaries (default: git rev-parse HEAD)") orelse
        headSha(b);
    const write_sha = b.addWriteFiles();
    const git_sha_c = write_sha.add("get_git_sha.c", b.fmt(
        \\#include "xdyn/get_git_sha/get_git_sha.h"
        \\const char* get_git_sha() {{ return "{s}"; }}
        \\
    , .{sha}));
    step.dependOn(&write_sha.step);

    return .{
        .step = step,
        .proto_messages = messages,
        .proto_services = services,
        .ssc_umbrellas = umbrellas.getDirectory(),
        .git_sha_c = git_sha_c,
    };
}

fn headSha(b: *std.Build) []const u8 {
    var code: u8 = 0;
    const out = b.runAllowFail(&.{ "git", "-C", fromRoot(b, "."), "rev-parse", "HEAD" }, &code, .ignore) catch return "unknown";
    if (code != 0) return "unknown";
    return b.dupe(std.mem.trim(u8, out, " \t\r\n"));
}

// =============================================================================
// Locating the closure and Eigen: nothing absolute is baked into the build
// =============================================================================

// Keyed on the full triple, since gnu and musl closures differ in exactly the libc. Order:
// -Ddeps, $XDYN_DEPS_<TRIPLE>, $XDYN_DEPS (an override for every target), ./libcxx-<triple>.
fn resolveDepsRoot(b: *std.Build, target: std.Build.ResolvedTarget, codegen: *std.Build.Step) []const u8 {
    const t = target.result;
    const triple = b.fmt("{s}-{s}-{s}", .{ @tagName(t.cpu.arch), @tagName(t.os.tag), @tagName(t.abi) });

    const env_key = b.fmt("XDYN_DEPS_{s}", .{triple});
    for (env_key) |*c| c.* = switch (c.*) {
        '-' => '_',
        'a'...'z' => c.* - ('a' - 'A'),
        else => c.*,
    };

    const root = b.option([]const u8, "deps", "libc++ dependency closure for the target (default: ./libcxx-<triple>)") orelse
        b.graph.environ_map.get(env_key) orelse
        b.graph.environ_map.get("XDYN_DEPS") orelse
        fromRoot(b, b.fmt("libcxx-{s}", .{triple}));

    // A warning, so `zig build --help` still works without a closure.
    if (std.Io.Dir.cwd().access(b.graph.io, b.pathJoin(&.{ root, "libxdyndeps_core.a" }), .{})) |_|
        checkStamp(b, codegen, root, triple, "install/lib")
    else |_|
        std.log.warn("no libxdyndeps_core.a under '{s}': the {s} closure is missing. Fetch it " ++
            "with `mise run deps:fetch {s}`, build it with `mise run deps:{s}`, or name one " ++
            "with -Ddeps=<path> or ${s}.", .{ root, triple, triple, triple, env_key });
    return root;
}

// fetch.sh leaves the asset's assets.sha256 line in the tree as asset.sha256. A failing step under
// `gen`, which every artifact depends on, so `zig build --help` still works on a stale tree.
// Only a local build has `built_marker`: a tree without it or a stamp came from an older fetch.sh.
fn checkStamp(b: *std.Build, codegen: *std.Build.Step, dir: []const u8, flavor: []const u8, built_marker: ?[]const u8) void {
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
            codegen.dependOn(&b.addFail(b.fmt("'{s}' was fetched before closures carried a stamp. {s}", .{ dir, refetch })).step);
        return;
    };
    const sums = cwd.readFileAlloc(io, fromRoot(b, "tools/deps/assets.sha256"), b.allocator, .limited(1 << 16)) catch |err|
        std.debug.panic("reading tools/deps/assets.sha256: {t}", .{err});
    const want = std.mem.trim(u8, stamp, " \r\n");
    var lines = std.mem.tokenizeAny(u8, sums, "\r\n");
    while (lines.next()) |line| if (std.mem.eql(u8, line, want)) return;
    codegen.dependOn(&b.addFail(b.fmt("'{s}' is not the {s} asset tools/deps/assets.sha256 names. {s}", .{ dir, flavor, refetch })).step);
}

// Header-only and target-independent, so the host's copy serves cross builds too. Order:
// -Deigen, $XDYN_EIGEN, pkg-config (the devShell's route), then the usual system prefixes.
fn resolveEigen(b: *std.Build) ?[]const u8 {
    if (b.option([]const u8, "eigen", "Eigen 3 include directory (default: probe the host)") orelse
        b.graph.environ_map.get("XDYN_EIGEN")) |explicit| return explicit;

    if (probeEigenPkgConfig(b)) |probed| return probed;

    for ([_][]const u8{ "/usr/include/eigen3", "/usr/local/include/eigen3", "/opt/homebrew/include/eigen3" }) |candidate|
        if (hasEigen(b, candidate)) return candidate;

    std.log.warn("Eigen not found (tried pkg-config and the usual prefixes). " ++
        "Point at it with -Deigen=<dir> or $XDYN_EIGEN — the dir that contains Eigen/Core.", .{});
    return null;
}

fn probeEigenPkgConfig(b: *std.Build) ?[]const u8 {
    var code: u8 = 0;
    const out = b.runAllowFail(&.{ "pkg-config", "--cflags-only-I", "eigen3" }, &code, .ignore) catch return null;
    if (code != 0) return null;
    var it = std.mem.tokenizeAny(u8, out, " \t\r\n");
    while (it.next()) |token| {
        if (!std.mem.startsWith(u8, token, "-I")) continue;
        if (hasEigen(b, token[2..])) return b.dupe(token[2..]);
    }
    return null;
}

fn hasEigen(b: *std.Build, dir: []const u8) bool {
    var buf: [std.fs.max_path_bytes]u8 = undefined;
    const probe = std.fmt.bufPrint(&buf, "{s}/Eigen/Core", .{dir}) catch return false;
    std.Io.Dir.cwd().access(b.graph.io, probe, .{}) catch return false;
    return true;
}

// =============================================================================
// Helpers the component files share
// =============================================================================

pub fn fromRoot(b: *std.Build, sub_path: []const u8) []const u8 {
    return b.root.joinString(b.allocator, sub_path) catch @panic("OOM");
}

pub fn concat(b: *std.Build, base: []const []const u8, extra: []const []const u8) []const []const u8 {
    const out = b.allocator.alloc([]const u8, base.len + extra.len) catch @panic("OOM");
    @memcpy(out[0..base.len], base);
    @memcpy(out[base.len..], extra);
    return out;
}

// Every `ext` file directly in `dir`, minus `excluded`, sorted.
pub fn sourcesIn(b: *std.Build, dir: []const u8, ext: []const u8, excluded: []const []const u8) []const []const u8 {
    var handle = b.root.openDir(b.graph.io, dir, .{ .iterate = true }) catch
        std.debug.panic("cannot open {s} to collect sources", .{dir});
    defer handle.close(b.graph.io);

    var found: std.ArrayList([]const u8) = .empty;
    var it = handle.iterate();
    while (it.next(b.graph.io) catch std.debug.panic("cannot read {s}", .{dir})) |entry| {
        if (entry.kind != .file) continue;
        if (!std.mem.endsWith(u8, entry.name, ext)) continue;
        for (excluded) |skip| {
            if (std.mem.eql(u8, entry.name, skip)) break;
        } else found.append(b.allocator, b.dupe(entry.name)) catch @panic("OOM");
    }
    return sorted(b, &found);
}

pub fn sorted(b: *std.Build, found: *std.ArrayList([]const u8)) []const []const u8 {
    std.mem.sort([]const u8, found.items, {}, struct {
        fn lessThan(_: void, a: []const u8, c: []const u8) bool {
            return std.mem.order(u8, a, c) == .lt;
        }
    }.lessThan);
    return found.toOwnedSlice(b.allocator) catch @panic("OOM");
}
