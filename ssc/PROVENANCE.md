# Provenance

`ssc/` is Sirehna's Scientific Computing library (SSC), absorbed from
[`naval-group/scientific_computing`](https://github.com/naval-group/scientific_computing) at
`690f80f6a065dbea65d9423d03c96b3a50ebb1b5`. That repository is a fork of
[`sirehna/ssc`](https://github.com/sirehna/ssc) with no commits of its own, 1726 in all. It is
licensed under the EPL-2.0, in `LICENSE.md`; `Copyright.txt`, `copyright_notice.txt` and
`contributors` are its notices.

`ssc/f2c/` comes from SSC's f2c submodule,
[`sirehna_naval_group/ThirdParty/f2c`](https://gitlab.com/sirehna_naval_group/ThirdParty/f2c) at
`844703f7eff4c533bb147931bffe914b3345ef4d` (`v1.0.0`), under the licence in `f2c/Notice`.

Only what xdyn compiles or includes is here, and the tests SSC's CMake built for those modules:
300 of SSC's 660 files and 157 of f2c's 183, each as upstream had it. `build.zig` is new: it
replaces SSC's CMake and `generate_module_header.sh`, whose umbrella headers it writes into the build
cache.

`build.zig` compiles every `.c` in `ssc/f2c/`, so a re-import has to leave out these 16 of f2c's
sources: `main.c`, `arithchk.c` and `arithchk_ori.c` define `main()`; `uninit.c` and `signbit.c`
need a generated `arith.h`; `pow_qq.c`, `qbitbits.c`, `qbitshft.c` and `ftell64_.c` need `longint`;
`dtime_.c` and `etime_.c` are POSIX-only and break the Windows target; `getarg_.c`, `iargc_.c`,
`s_paus.c`, `i_ceiling.c` and `i_len_trim.c` are unused. The other 10 are f2c's own build files.

The history before absorption is in the repositories above, and every change since is in this
one: `git blame` stops at the commit that added them.
