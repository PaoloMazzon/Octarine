#include <SDL2/SDL.h>
#include "oct/Common.h"
#include "oct/Allocators.h"
#include "oct/Opaque.h"

// All assets
static Oct_AssetData gAssets[OCT_MAX_ASSETS];

// Error message in case an asset load fails
#define ERROR_BUFFER_SIZE 1024
static char gErrorMessage[ERROR_BUFFER_SIZE];
static SDL_mutex *gErrorMessageMutex;

void _oct_AssetsInit(Oct_Context ctx) {
    gErrorMessageMutex = SDL_CreateMutex();
}

void _oct_AssetsProcessCommand(Oct_Context ctx, Oct_Command *cmd) {
    //TODO: This
}

Oct_AssetType _oct_AssetType(Oct_Context ctx, Oct_Asset asset) {
    return gAssets[asset].type;
}

Oct_AssetData *_oct_AssetGet(Oct_Context ctx, Oct_Asset asset) {
    return &gAssets[asset];
}

void _oct_AssetsEnd(Oct_Context ctx) {
    SDL_DestroyMutex(gErrorMessageMutex);
}

Oct_Asset _oct_AssetReserveSpace(Oct_Context ctx) {
    for (int i = 0; i < OCT_MAX_ASSETS; i++) {
        if (!SDL_AtomicGet(&gAssets->reserved)) {
            SDL_AtomicSet(&gAssets->reserved, 1);
            return i;
        }
    }
}

OCTARINE_API Oct_Bool oct_AssetLoaded(Oct_Asset asset) {
    return SDL_AtomicGet(&gAssets[asset].loaded);
}

OCTARINE_API Oct_Bool oct_AssetLoadFailed(Oct_Asset asset) {
    return SDL_AtomicGet(&gAssets[asset].failed);
}

OCTARINE_API const char *oct_AssetErrorMessage(Oct_Allocator allocator) {
    char *out = null;
    SDL_LockMutex(gErrorMessageMutex);
    const int slen = strlen(gErrorMessage);
    const int len = slen >= ERROR_BUFFER_SIZE ? ERROR_BUFFER_SIZE - 1 : slen;

    // Allocate new string and copy it
    out = oct_Malloc(allocator, len + 1);
    if (out) {
        strncpy(out, gErrorMessage, len);
        out[len] = 0;
    }

    SDL_UnlockMutex(gErrorMessageMutex);
    return out;
}
