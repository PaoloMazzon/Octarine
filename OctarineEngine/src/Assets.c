#include <stdarg.h>
#include <SDL2/SDL.h>
#include <VK2D/VK2D.h>
#include "oct/Common.h"
#include "oct/Allocators.h"
#include "oct/Opaque.h"

// All assets
static Oct_AssetData gAssets[OCT_MAX_ASSETS];

// Error message in case an asset load fails
#define ERROR_BUFFER_SIZE 1024
static char gErrorMessage[ERROR_BUFFER_SIZE];
static SDL_mutex *gErrorMessageMutex;
static SDL_atomic_t gErrorHasOccurred;

void _oct_AssetsInit(Oct_Context ctx) {
    gErrorMessageMutex = SDL_CreateMutex();
}

static void _oct_LogError(const char *fmt, ...) {
    va_list l;
    va_start(l, fmt);
    const int len = strlen(gErrorMessage);
    vsnprintf(&gErrorMessage[len], ERROR_BUFFER_SIZE - len - 1, fmt, l);
    va_end(l);
}

// Destroys metadata for an asset when its being freed
static void _oct_DestroyAssetMetadata(Oct_Context ctx, Oct_Asset asset) {
    SDL_AtomicSet(&gAssets[asset].loaded, 0);
    SDL_AtomicSet(&gAssets[asset].reserved, 0);
}

static void _oct_FailLoad(Oct_Context ctx, Oct_Asset asset) {
    SDL_AtomicSet(&gAssets[asset].failed, 1);
    SDL_AtomicSet(&gAssets[asset].reserved, 1);
    SDL_AtomicSet(&gErrorHasOccurred, 1);
}

void _oct_AssetsProcessCommand(Oct_Context ctx, Oct_Command *cmd) {
    Oct_LoadCommand *load = &cmd->loadCommand;
    if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_TEXTURE) {
        VK2DTexture tex = vk2dTextureLoad(load->Texture.filename);
        if (tex) {
            gAssets[load->_assetID].texture = tex;
            gAssets[load->_assetID].type = OCT_ASSET_TYPE_TEXTURE;
            SDL_AtomicSet(&gAssets[load->_assetID].loaded, 1);
        } else {
            _oct_FailLoad(ctx, load->_assetID);
            _oct_LogError("Failed to load texture \"%s\"\n", load->Texture.filename);
        } // TODO: The other types
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_FREE) {
        if (gAssets[load->_assetID].type == OCT_ASSET_TYPE_TEXTURE) {
            vk2dRendererWait();
            vk2dTextureFree(gAssets[load->_assetID].texture);
            _oct_DestroyAssetMetadata(ctx, load->_assetID);
        } // TODO: The other types
    }
}

Oct_AssetType _oct_AssetType(Oct_Context ctx, Oct_Asset asset) {
    return SDL_AtomicGet(&gAssets[asset].loaded) ? gAssets[asset].type : OCT_ASSET_TYPE_NONE;
}

Oct_AssetData *_oct_AssetGet(Oct_Context ctx, Oct_Asset asset) {
    return &gAssets[asset];
}

void _oct_AssetsEnd(Oct_Context ctx) {
    // Delete all the assets still loaded
    for (int i = 0; i < OCT_MAX_ASSETS; i++) {
        if (SDL_AtomicGet(&gAssets[i].loaded)) {
            Oct_AssetData *data = &gAssets[i];
            if (data->type == OCT_ASSET_TYPE_TEXTURE) {
                vk2dRendererWait();
                vk2dTextureFree(data->texture);
            } // TODO: The other types
            _oct_DestroyAssetMetadata(ctx, i);
        }
    }

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
    bool failed = SDL_AtomicGet(&gAssets[asset].failed);
    if (failed) {
        SDL_AtomicSet(&gAssets[asset].failed, 0);
        SDL_AtomicSet(&gAssets[asset].reserved, 0);
    }
    return failed;
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
        SDL_AtomicSet(&gErrorHasOccurred, 0);
        gErrorMessage[0] = 0;
    }

    SDL_UnlockMutex(gErrorMessageMutex);
    return out;
}

OCTARINE_API const char *oct_AssetGetErrorMessage(int *size, char *buffer) {
    if (buffer) {
        const int slen = strlen(gErrorMessage);
        const int len = slen >= ERROR_BUFFER_SIZE ? ERROR_BUFFER_SIZE - 1 : slen;

        // Allocate new string and copy it
        strncpy(buffer, gErrorMessage, len);
        buffer[len] = 0;

        gErrorMessage[0] = 0;
        SDL_AtomicSet(&gErrorHasOccurred, 0);
        SDL_UnlockMutex(gErrorMessageMutex);
    } else if (size) {
        SDL_LockMutex(gErrorMessageMutex);
        const int slen = strlen(gErrorMessage);
        const int len = slen >= ERROR_BUFFER_SIZE ? ERROR_BUFFER_SIZE - 1 : slen;
        *size = len + 1;
    }
    return buffer;
}

OCTARINE_API Oct_Bool oct_AssetLoadHasFailed() {
    return SDL_AtomicGet(&gErrorHasOccurred);
}