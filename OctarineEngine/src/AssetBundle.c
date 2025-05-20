#include <SDL3/SDL.h>

#include "oct/Common.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"

void _oct_PlaceAssetInBucket(Oct_Context ctx, Oct_AssetBundle bundle, Oct_Asset asset, const char *name) {
    // TODO: This
}

OCTARINE_API void oct_FreeAssetBundle(Oct_Context ctx, Oct_AssetBundle bundle) {
    // TODO: This
}

OCTARINE_API Oct_Bool oct_IsAssetBundleReady(Oct_Context ctx, Oct_AssetBundle bundle) {
    return SDL_GetAtomicInt(&bundle->bundleReady);
}

OCTARINE_API Oct_Asset oct_GetAsset(Oct_Context ctx, Oct_AssetBundle bundle, const char *name) {
    return OCT_NO_ASSET; // TODO: This
}

OCTARINE_API Oct_Bool oct_AssetExists(Oct_Context ctx, Oct_AssetBundle bundle, const char *name, Oct_AssetType type) {
    return true; // TODO: This
}