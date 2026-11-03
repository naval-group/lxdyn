# Provenance

`interfaces/` is xdyn's gRPC interface, absorbed from
[`naval-group/interfaces`](https://github.com/naval-group/interfaces) at
`5a3b79e3cc6c2a5c29f81d0316c0daa3d11f40d5`, a fork of Sirehna's repository with no commits of its
own, 368 in all. It is licensed under the EPL-2.0, in `LICENSE.md`.

Only what the build, the integration tests and the server image read is here: `proto/`, the
`xdyngrpc` package, its `requirements.txt` and the server image recipe, 33 of the 93 files. `Containerfile.python` is
upstream's `Dockerfile.python`, renamed; every file came in as upstream had it.

The history before absorption is in the repository above, and every change since is in this one:
`git blame` stops at the commit that added them.
