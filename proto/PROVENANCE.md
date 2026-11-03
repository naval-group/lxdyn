# Provenance

`proto/` is xdyn's gRPC API, absorbed from
[`naval-group/interfaces`](https://github.com/naval-group/interfaces) at
`5a3b79e3cc6c2a5c29f81d0316c0daa3d11f40d5`, a fork of Sirehna's repository with no commits of its
own, 368 in all. It is licensed under the EPL-2.0, like lxdyn. `build.zig` compiles it to C++, and
`mise run integration:setup` to the Python stubs `xdyngrpc` imports. The 6 files came in as
upstream had them.

The history before absorption is in the repository above, and every change since is in this one:
`git blame` stops at the commit that added them.
