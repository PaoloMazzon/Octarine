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

// All assets
static Oct_AssetData gAssets[OCT_MAX_ASSETS];

// Returns the actual index in the asset array given an asset id
#define ASSET_INDEX(asset) (asset & INT32_MAX)

// Error message in case an asset load fails
#define ERROR_BUFFER_SIZE 1024
static char gErrorMessage[ERROR_BUFFER_SIZE];
static SDL_Mutex *gErrorMessageMutex;
static SDL_AtomicInt gErrorHasOccurred;

///////////////////////////////// ASSET CREATION HELP /////////////////////////////////
static void _oct_LogError(const char *fmt, ...) {
    va_list l;
    va_start(l, fmt);
    const int len = strlen(gErrorMessage);
    vsnprintf(&gErrorMessage[len], ERROR_BUFFER_SIZE - len - 1, fmt, l);
    va_end(l);
}

// Destroys metadata for an asset when its being freed
static void _oct_DestroyAssetMetadata(Oct_Context ctx, Oct_Asset asset) {
    SDL_SetAtomicInt(&gAssets[asset].loaded, 0);
    SDL_SetAtomicInt(&gAssets[asset].reserved, 0);
    SDL_AddAtomicInt(&gAssets[ASSET_INDEX(asset)].generation, 1);
}

static void _oct_FailLoad(Oct_Context ctx, Oct_Asset asset) {
    SDL_SetAtomicInt(&gAssets[asset].failed, 1);
    SDL_SetAtomicInt(&gAssets[asset].reserved, 1);
    SDL_SetAtomicInt(&gErrorHasOccurred, 1);
}

///////////////////////////////// ASSET CREATION /////////////////////////////////
static void _oct_AssetCreateTexture(Oct_Context ctx, Oct_LoadCommand *load) {
    VK2DTexture tex = vk2dTextureLoad(load->Texture.filename);
    if (tex) {
        gAssets[ASSET_INDEX(load->_assetID)].texture = tex;
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_TEXTURE;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
    } else {
        _oct_FailLoad(ctx, load->_assetID);
        _oct_LogError("Failed to load texture \"%s\"\n", load->Texture.filename);
    }
}

static void _oct_AssetCreateSurface(Oct_Context ctx, Oct_LoadCommand *load) {
    VK2DTexture tex = vk2dTextureCreate(load->Surface.dimensions[0], load->Surface.dimensions[1]);
    if (tex) {
        gAssets[ASSET_INDEX(load->_assetID)].texture = tex;
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_TEXTURE;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
    } else {
        _oct_FailLoad(ctx, load->_assetID);
        _oct_LogError("Failed to create surface of dimensions %.2f/%.2f\n", load->Surface.dimensions[0], load->Surface.dimensions[1]);
    }
}

static void _oct_AssetCreateCamera(Oct_Context ctx, Oct_LoadCommand *load) {
    VK2DCameraIndex camIndex = vk2dCameraCreate(vk2dCameraGetSpec(VK2D_DEFAULT_CAMERA));
    if (camIndex != VK2D_INVALID_CAMERA) {
        gAssets[ASSET_INDEX(load->_assetID)].camera = camIndex;
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_CAMERA;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
    } else {
        _oct_FailLoad(ctx, load->_assetID);
        _oct_LogError("Failed to create camera\n");
    }
}

static void _oct_AssetCreateAudio(Oct_Context ctx, Oct_LoadCommand *load) {
    // Find file extension
    const char *extension = strrchr(load->Audio.filename, '.');
    extension = extension == null ? "" : extension;
    uint8_t *data = null;
    uint32_t dataSize = 0;
    SDL_AudioSpec spec;

    if (strcmp(extension, ".ogg") == 0) {
        // TODO: Implement ogg loading
        _oct_FailLoad(ctx, load->_assetID);
        _oct_LogError("Failed to load audio sample \"%s\", ogg is not yet supported.\n", load->Audio.filename);
    } else if (strcmp(extension, ".wav") == 0) {
        // Use SDL to load wavs
        if (!SDL_LoadWAV(load->Audio.filename, &spec, &data, &dataSize)) {
            _oct_FailLoad(ctx, load->_assetID);
            _oct_LogError("Failed to load audio sample \"%s\", SDL Error: %s.\n", load->Audio.filename, SDL_GetError());
        }
    } else if (strcmp(extension, ".mp3") == 0) {
        // TODO: Implement mp3 loading
        _oct_FailLoad(ctx, load->_assetID);
        _oct_LogError("Failed to load audio sample \"%s\", mp3 is not yet supported.\n", load->Audio.filename);
    } else {
        _oct_FailLoad(ctx, load->_assetID);
        _oct_LogError("Failed to load audio sample \"%s\", unrecognized extension.\n", load->Audio.filename);
    }

    // Convert format if we found good data
    if (data) {
        int32_t newSize;
        uint8_t *newSamples = _oct_AudioConvertFormat(data, dataSize, &newSize, &spec);
        if (newSamples) {
            gAssets[ASSET_INDEX(load->_assetID)].audio.size = newSize;
            gAssets[ASSET_INDEX(load->_assetID)].audio.data = newSamples;
        } else {
            _oct_FailLoad(ctx, load->_assetID);
            _oct_LogError("Failed to load audio sample \"%s\", failed to convert format, SDL Error %s.\n", load->Audio.filename, SDL_GetError());
        }
        SDL_free(data);
    }
}

static void _oct_AssetCreateSprite(Oct_Context ctx, Oct_LoadCommand *load) {
    Oct_SpriteData *data = &gAssets[ASSET_INDEX(load->_assetID)].sprite;
    gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_SPRITE;
    data->texture = load->Sprite.texture;
    data->ownsTexture = false;
    data->frameCount = load->Sprite.frameCount;
    data->frame = 0;
    data->repeat = load->Sprite.repeat;
    data->pause = false;
    data->delay = 1.0 / load->Sprite.fps;
    data->lastTime = oct_Time(ctx);
    data->accumulator = 0;
    data->startPos[0] = load->Sprite.startPos[0];
    data->startPos[1] = load->Sprite.startPos[1];
    data->frameSize[0] = load->Sprite.frameSize[0];
    data->frameSize[1] = load->Sprite.frameSize[1];
    data->padding[0] = load->Sprite.padding[0];
    data->padding[1] = load->Sprite.padding[1];
    data->xStop = load->Sprite.xStop;
    SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
}

///////////////////////////////// ASSET DESTRUCTION /////////////////////////////////
static void _oct_AssetDestroyTexture(Oct_Context ctx, Oct_Asset asset) {
    vk2dRendererWait();
    vk2dTextureFree(gAssets[ASSET_INDEX(asset)].texture);
    _oct_DestroyAssetMetadata(ctx, asset);
}

static void _oct_AssetDestroyCamera(Oct_Context ctx, Oct_Asset asset) {
    vk2dCameraSetState(gAssets[ASSET_INDEX(asset)].camera, VK2D_CAMERA_STATE_DELETED);
    _oct_DestroyAssetMetadata(ctx, asset);
}

static void _oct_AssetDestroySprite(Oct_Context ctx, Oct_Asset asset) {
    // TODO: This
    _oct_DestroyAssetMetadata(ctx, asset);
}

static void _oct_AssetDestroyAudio(Oct_Context ctx, Oct_Asset asset) {
    // TODO: Destroy audio
    _oct_DestroyAssetMetadata(ctx, asset);
}

static void _oct_AssetDestroy(Oct_Context ctx, Oct_Asset asset) {
    if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_TEXTURE) {
        _oct_AssetDestroyTexture(ctx, asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_CAMERA) {
        _oct_AssetDestroyCamera(ctx, asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_SPRITE) {
        _oct_AssetDestroySprite(ctx, asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_AUDIO) {
        _oct_AssetDestroyAudio(ctx, asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_FONT) {
        _oct_AssetDestroyFont(ctx, asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_FONT_ATLAS) {
        _oct_AssetDestroyFont(ctx, asset);
    }
}

///////////////////////////////// INTERNAL /////////////////////////////////
void _oct_AssetsInit(Oct_Context ctx) {
    gErrorMessageMutex = SDL_CreateMutex();
    oct_Log("Asset system initialized.");
}

void _oct_AssetsProcessCommand(Oct_Context ctx, Oct_Command *cmd) {
    Oct_LoadCommand *load = &cmd->loadCommand;
    if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_TEXTURE) {
        _oct_AssetCreateTexture(ctx, load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_CREATE_SURFACE) {
        _oct_AssetCreateSurface(ctx, load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_CREATE_CAMERA) {
        _oct_AssetCreateCamera(ctx, load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_SPRITE) {
        _oct_AssetCreateSprite(ctx, load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_AUDIO) {
        _oct_AssetCreateAudio(ctx, load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_FONT) {
        _oct_AssetCreateFont(ctx, load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_CREATE_FONT_ATLAS) {
        _oct_AssetCreateFontAtlas(ctx, load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_BITMAP_FONT) {
        _oct_AssetCreateBitmapFont(ctx, load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_FREE) {
        if (SDL_GetAtomicInt(&gAssets[load->_assetID].loaded))
            _oct_AssetDestroy(ctx, load->_assetID);
    }
}

Oct_AssetType _oct_AssetType(Oct_Context ctx, Oct_Asset asset) {
    return SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded) ? gAssets[ASSET_INDEX(asset)].type : OCT_ASSET_TYPE_NONE;
}

Oct_AssetData *_oct_AssetGet(Oct_Context ctx, Oct_Asset asset) {
    return &gAssets[ASSET_INDEX(asset)];
}

void _oct_AssetsEnd(Oct_Context ctx) {
    // Delete all the assets still loaded
    for (int i = 0; i < OCT_MAX_ASSETS; i++)
        if (SDL_GetAtomicInt(&gAssets[i].loaded))
            _oct_AssetDestroy(ctx, i);

    SDL_DestroyMutex(gErrorMessageMutex);
}

Oct_Asset _oct_AssetReserveSpace(Oct_Context ctx) {
    for (int i = 0; i < OCT_MAX_ASSETS; i++) {
        if (!SDL_GetAtomicInt(&gAssets[i].reserved)) {
            SDL_SetAtomicInt(&gAssets[i].reserved, 1);
            const int64_t gen = SDL_GetAtomicInt(&gAssets[i].generation);
            return i + ((gen) << 32);
        }
    }
}

///////////////////////////////// EXTERNAL /////////////////////////////////
OCTARINE_API Oct_Bool oct_AssetLoaded(Oct_Asset asset) {
    const Oct_Bool loaded = SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded);
    const int64_t gen = SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].generation);
    const Oct_Bool matchesGeneration = gen == asset >> 32;
    return loaded && matchesGeneration;
}

OCTARINE_API Oct_Bool oct_AssetLoadFailed(Oct_Asset asset) {
    bool failed = SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].failed);
    if (failed) {
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(asset)].failed, 0);
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(asset)].reserved, 0);
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
        SDL_SetAtomicInt(&gErrorHasOccurred, 0);
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
        SDL_SetAtomicInt(&gErrorHasOccurred, 0);
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
    return SDL_GetAtomicInt(&gErrorHasOccurred);
}