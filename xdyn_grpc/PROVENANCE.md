# Provenance

`xdyn_grpc/` is the Python package for writing xdyn's external models — force models, wave models
and controllers — and their clients, absorbed from
[`naval-group/interfaces`](https://github.com/naval-group/interfaces) at
`5a3b79e3cc6c2a5c29f81d0316c0daa3d11f40d5`, a fork of Sirehna's repository with no commits of its
own, 368 in all. It is licensed under the EPL-2.0, in `LICENSE.md`.

The package is `xdyn_grpc/xdyngrpc/`, imported from here with `PYTHONPATH` rather than installed. Its
protobuf stubs are generated from `proto/`. Of upstream's 93 files these are 27, and `proto/` holds
6 more; every file came in as upstream had it.

The history before absorption is in the repository above, and every change since is in this one:
`git blame` stops at the commit that added them.
