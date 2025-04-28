// This file is basically an extension of Assets.c, but because fonts are sufficiently
// complicated they get their own file to deal with creating/destroying them.

#include <stdarg.h>
#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>
#include <stdio.h>

#include "oct/Common.h"
#include "oct/Allocators.h"
#include "oct/Opaque.h"
#include "oct/Core.h"
#include "oct/Validation.h"
#include "oct/Subsystems.h"

void _oct_AssetCreateFont(Oct_Context ctx, Oct_LoadCommand *load) {

}

void _oct_AssetCreateFontAtlas(Oct_Context ctx, Oct_LoadCommand *load) {

}

// Bitmap fonts are just font atlases
void _oct_AssetCreateBitmapFont(Oct_Context ctx, Oct_LoadCommand *load) {

}

void _oct_AssetDestroyFont(Oct_Context ctx, Oct_Asset asset) {

}

void _oct_AssetDestroyFontAtlas(Oct_Context ctx, Oct_Asset asset) {

}

