# Provenance

`ssc/` is Sirehna's Scientific Computing library (SSC), absorbed from
[`naval-group/scientific_computing`](https://github.com/naval-group/scientific_computing) at
`690f80f6a065dbea65d9423d03c96b3a50ebb1b5`. That repository is a fork of
[`sirehna/ssc`](https://github.com/sirehna/ssc) with no commits of its own, 1726 in all. It is
licensed under the EPL-2.0, in `LICENSE.md`; `Copyright.txt`, `copyright_notice.txt` and
`contributors` are its notices.

Only what xdyn compiles or includes is here, and the tests SSC's CMake built for those modules:
264 of SSC's 660 files, each as upstream had it. `build.zig` is new: it
replaces SSC's CMake and `generate_module_header.sh`, whose umbrella headers it writes into the build
cache.

The `.c` files in `integrate/` and `interpolation/` are f2c's translation of QUADPACK, `filon` and
de Boor's `cubspl`. They build without libf2c, SSC's f2c submodule: `fortran.h` is new and holds the
types and macros they took from its `f2c.h`, and `pow` replaces its `pow_dd`.

The history before absorption is in the repositories above, and every change since is in this
one: `git blame` stops at the commit that added them.
