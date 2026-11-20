/*
 * stb_image_implementation.cpp
 *
 * stb ships its implementation inside the header, guarded by a macro. This is the single
 * translation unit that defines it: every other file includes <stb_image.h> for the declarations
 * alone, so the loader is linked exactly once.
 */

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
