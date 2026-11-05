// Options: -Dtarget -Ddeps -Ddeps-host -Deigen -Ddebug -Dcommit (`zig build --help` lists all).
// Each component's build lives beside its sources; this file resolves what they share.
const std = @import("std");
const builtin = @import("builtin");
const xdyn = @import("xdyn/build.zig");
const external = @import("external/build.zig");
const ssc = @import("ssc/build.zig");
const proto = @import("proto/build.zig");
const deps = @import("tools/deps/build.zig");
const python = @import("xdyn_wrapper_python/build.zig");
const fmu = @import("xdyn_wrapper_fmi/build.zig");

// gnu at the 2.28 floor, so a native build links on the oldest glibc still in service.
const native_default: std.Target.Query = if (builtin.os.tag == .linux)
    .{ .abi = .gnu, .glibc_version = .{ .major = 2, .minor = 28, .patch = 0 } }
else
    .{};

pub fn build(b: *std.Build) void {
    // Configuration reads $XDYN_*, runs pkg-config and python: none of it is visible to the configure cache.
    b.graph.poisonCache();
    const target = b.standardTargetOptions(.{ .default_target = native_default });
    const codegen = addCodegen(b);
    var ctx: Ctx = .{
        .b = b,
        .target = target,
        .optimize = .ReleaseFast,
        .deps_root = deps.resolveRoot(b, target, codegen.step),
        .eigen_include = resolveEigen(b),
        .debug = b.option(bool, "debug", "Build xdyn -O0 -g for gdb (deps stay optimized)") orelse false,
        .codegen = codegen,
    };
    ctx.cpp_flags = xdyn.cppFlags(&ctx);

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
};

// =============================================================================
// Codegen: SSC's umbrella headers and the gRPC stubs
// =============================================================================

pub const Codegen = struct {
    step: *std.Build.Step,
    proto_messages: std.Build.LazyPath,
    proto_services: std.Build.LazyPath,
    ssc_umbrellas: std.Build.LazyPath,
};

fn addCodegen(b: *std.Build) Codegen {
    const step = b.step("gen", "Run the code generators on their own");

    const umbrellas = ssc.addUmbrellas(b);
    step.dependOn(&umbrellas.step);

    const stubs = proto.addStubs(b);
    step.dependOn(stubs.step);

    return .{
        .step = step,
        .proto_messages = stubs.messages,
        .proto_services = stubs.services,
        .ssc_umbrellas = umbrellas.getDirectory(),
    };
}

// =============================================================================
// Locating Eigen: nothing absolute is baked into the build
// =============================================================================

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
