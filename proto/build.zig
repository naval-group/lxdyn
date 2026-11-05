// The gRPC stubs: proto/*.proto through a host protoc, compiled into whichever module serves them.
const std = @import("std");
const root = @import("../build.zig");

pub const Stubs = struct {
    step: *std.Build.Step,
    messages: std.Build.LazyPath,
    services: std.Build.LazyPath,
};

pub fn addStubs(b: *std.Build) Stubs {
    // A host protoc even when cross-compiling: the generated C++ is arch-independent.
    const host = b.option([]const u8, "deps-host", "Host tools providing protoc (default: ./host-tools)") orelse
        b.graph.environ_map.get("XDYN_DEPS_HOST") orelse
        root.fromRoot(b, "host-tools");
    const bin = b.pathJoin(&.{ host, "install", "bin" });
    const protoc = b.pathJoin(&.{ bin, "protoc" });
    const gen = b.addSystemCommand(&.{protoc});
    if (std.Io.Dir.cwd().access(b.graph.io, protoc, .{})) |_|
        root.checkStamp(b, &gen.step, host, "host-tools", null)
    else |_|
        std.log.warn("no protoc under '{s}': codegen will fail. Fetch it with `mise run deps:fetch " ++
            "host-tools`, or name one with -Ddeps-host=<path> or $XDYN_DEPS_HOST.", .{bin});

    gen.setCwd(b.path("."));
    gen.addFileInput(.{ .cwd_relative = protoc });
    gen.addPrefixedFileArg("--plugin=protoc-gen-grpc=", .{ .cwd_relative = b.pathJoin(&.{ bin, "grpc_cpp_plugin" }) });
    const messages = gen.addPrefixedOutputDirectoryArg("--cpp_out=", "proto");
    const services = gen.addPrefixedOutputDirectoryArg("--grpc_out=", "proto-grpc");
    gen.addPrefixedDirectoryArg("-I", b.path("proto"));
    for (root.sourcesIn(b, "proto", ".proto", &.{})) |file|
        gen.addFileArg(b.path(b.fmt("proto/{s}", .{file})));
    return .{ .step = &gen.step, .messages = messages, .services = services };
}

// `names` are .proto files without the extension: each module compiles only the services it uses.
pub fn addSources(ctx: *const root.Ctx, m: *std.Build.Module, names: []const []const u8) void {
    const b = ctx.b;
    // No -Werror: protoc's output is not ours to keep clean.
    const flags = ctx.withDebug(&.{ "-std=gnu++17", "-fPIC" });
    for (names) |name| {
        m.addCSourceFile(.{ .file = ctx.codegen.proto_messages.path(b, b.fmt("{s}.pb.cc", .{name})), .flags = flags });
        m.addCSourceFile(.{ .file = ctx.codegen.proto_services.path(b, b.fmt("{s}.grpc.pb.cc", .{name})), .flags = flags });
    }
}
