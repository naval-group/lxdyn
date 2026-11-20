// The gRPC stubs: proto/*.proto through a host protoc, compiled into whichever module serves them.
const std = @import("std");
const root = @import("../build.zig");
const deps = @import("../tools/deps/build.zig");

pub const Stubs = struct {
    step: *std.Build.Step,
    messages: std.Build.LazyPath,
    services: std.Build.LazyPath,
};

pub fn addStubs(b: *std.Build) Stubs {
    // A host protoc even when cross-compiling: the generated C++ is arch-independent.
    const host = deps.resolveHost(b);
    const bin = b.pathJoin(&.{ host orelse "", "install", "bin" });
    const protoc = b.pathJoin(&.{ bin, "protoc" });
    const gen = b.addSystemCommand(&.{protoc});
    if (host == null) {
        deps.fail(b, &gen.step, "no host tools: enter `nix develop`, or pass -Ddeps-host=<path>");
    } else std.Io.Dir.cwd().access(b.graph.io, protoc, .{}) catch
        deps.fail(b, &gen.step, b.fmt("no protoc under '{s}'", .{bin}));

    gen.setCwd(b.path("."));
    gen.addFileInput(.{ .cwd_relative = protoc });
    gen.addPrefixedFileArg("--plugin=protoc-gen-grpc=", .{ .cwd_relative = b.pathJoin(&.{ bin, "grpc_cpp_plugin" }) });
    const messages = gen.addPrefixedOutputDirectoryArg("--cpp_out=", "proto");
    const services = gen.addPrefixedOutputDirectoryArg("--grpc_out=", "proto-grpc");
    gen.addPrefixedDirectoryArg("-I", b.path("proto"));
    for (root.sourcesIn(b, "proto", ".proto")) |file|
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
