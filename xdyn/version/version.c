// The only file built with these defines, so a new commit recompiles this one file.
#include "xdyn/version/version.h"

const char* xdyn_version(void) { return XDYN_VERSION; }
const char* xdyn_commit(void) { return XDYN_COMMIT; }
