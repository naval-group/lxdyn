# xdyn gdb helpers for the zig/libc++ lane.
#
#   mise run zig:debug       ->  build/<arch>-<os>-<abi>-debug/bin/
#   mise run gdb run_all_tests --gtest_filter='Foo*'
#   sh tools/gdb.sh  run_all_tests --gtest_filter='Foo*'     (same thing, without mise)
#
# or by hand:
#   gdb -q -x .gdbinit --args build/x86_64-linux-gnu-debug/bin/run_all_tests
#
# The explicit -x matters, and is the whole reason tools/gdb.sh exists: gdb does NOT
# auto-load a repository .gdbinit unless the directory is on its auto-load safe-path.
# It declines with a warning — which it still prints even when -x loads the file anyway,
# so do not read that warning as "catchthrow is missing". Check with `help catchthrow`.
# To make the automatic load work too, add to ~/.config/gdb/gdbinit:
#   add-auto-load-safe-path /path/to/xdyn

set print pretty on
set print object on

# ---------------------------------------------------------------------------
# catchthrow — the replacement for `catch throw <regexp>`, which DOES NOT WORK here.
#
# gdb implements "stop only on exceptions of this type" by reading SystemTap SDT probes
# (libstdcxx:throw) that GNU libstdc++ compiles into its exception runtime. zig ships
# LLVM's libc++abi, which has none. gdb 17.2 says so —
#     did not find exception probe (does libstdcxx have SDT probes?)
# — and then ignores the regex and stops on EVERY throw anyway. Verified: with
# `catch throw out_of_range`, the first stop is a YAML::InvalidNode. yaml-cpp throws
# constantly while parsing, so the filter is exactly what you needed.
#
# So write the condition by hand:
#     __cxa_throw(void *thrown_object, std::type_info *tinfo, void (*dest)(void *))
# In a std::type_info the vptr sits at [0] and the mangled type name at [1], hence
# ((char**)tinfo)[1]. Match against the MANGLED name: out_of_range is
# St12out_of_range, YAML::InvalidNode is N4YAML11InvalidNodeE. Substring regexes are
# the practical way in.
#
# zig's libc++abi carries debug info, so `tinfo` resolves by name and this is
# architecture-independent — no register variant needed. Use catchthrow-raw if you ever
# meet a build where it does not.
#
# Requires a gdb built with Python ($_regex). Verified on gdb 17.2.
#
# Usage:  catchthrow out_of_range
#         catchthrow .*InvalidNode.*
# ---------------------------------------------------------------------------
define catchthrow
  break __cxa_throw if $_regex(((char**)tinfo)[1], "$arg0")
end
document catchthrow
Break only on C++ throws whose mangled type name matches a regex.
Replacement for `catch throw <regexp>`, which ignores its regex against
zig's libc++abi (no SystemTap probes) and stops on every throw.
Usage: catchthrow out_of_range
end

# Fallback for a libc++abi without debug info, where `tinfo` will not resolve. tinfo is
# argument 2: %rsi under x86-64 SysV, x1 under aarch64 AAPCS — edit to taste. Breaking at
# *__cxa_throw (the exact entry address, no prologue skip) is what keeps the argument
# registers valid.
define catchthrow-raw
  break *__cxa_throw if $_regex(((char**)$rsi)[1], "$arg0")
end
document catchthrow-raw
Register-based catchthrow for a stripped libc++abi (x86-64: tinfo in %rsi).
end

# At a __cxa_throw stop, print the mangled type name of the exception in flight.
define throwtype
  printf "%s\n", ((char**)tinfo)[1]
end
document throwtype
At a __cxa_throw breakpoint, print the mangled type name of the exception in flight.
end
