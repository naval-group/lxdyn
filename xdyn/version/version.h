#ifndef XDYN_VERSION_H
#define XDYN_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

// The release, from build.zig.zon: "26.11".
const char* xdyn_version(void);
// The commit Nix built, or "dev" for a local build.
const char* xdyn_commit(void);

#ifdef __cplusplus
}
#endif

#endif
