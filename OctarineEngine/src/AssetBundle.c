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

OCTARINE_API Oct_AssetBundle oct_LoadAssetBundle(const char *filename) {
    // TODO: This
    return null;
}

OCTARINE_API void oct_FreeAssetBundle(Oct_AssetBundle bundle) {
    // TODO: This
}

OCTARINE_API Oct_Asset oct_GetAsset(Oct_AssetBundle bundle, const char *name) {
    // TODO: This
    return OCT_NO_ASSET;
}
