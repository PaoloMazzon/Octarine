#include <SDL2/SDL.h>
#include "oct/Common.h"

/// \brief Types of assets stored in an Oct_Asset
typedef enum {
    OCT_ASSET_TYPE_NONE = 0,    ///< None
    OCT_ASSET_TYPE_TEXTURE = 1, ///< A texture
    OCT_ASSET_TYPE_MODEL = 2,   ///< Model
    OCT_ASSET_TYPE_FONT = 3,    ///< Font
    OCT_ASSET_TYPE_AUDIO = 4,   ///< Audio
    OCT_ASSET_TYPE_SPRITE = 5,  ///< Sprite
} Oct_AssetType;

// An asset for the engine
typedef struct Oct_AssetData_t {
    Oct_AssetType type; // type of asset
    Oct_Bool reserved; // to allow the logic thread to find assets that still exist
    SDL_atomic_t failed; // This will be true if the load on this asset failed
    SDL_atomic_t loaded; // True when the asset is loaded
    union {
        // TODO: These
    };
} Oct_AssetData;

// All assets
static Oct_AssetData gAssets[OCT_MAX_ASSETS];

// Error message in case an asset load fails
#define ERROR_BUFFER_SIZE 1024
static char gErrorMessage[ERROR_BUFFER_SIZE];
static SDL_mutex *gErrorMessageMutex;

void _oct_AssetsInit(Oct_Context ctx) {
    // TODO: This
}

void _oct_AssetsProcessCommand(Oct_Context ctx, Oct_Command *cmd) {
    // TODO: This
}

Oct_Asset *_oct_AssetGet(Oct_Context ctx, Oct_Asset asset) {
    return null; // TODO: This
}

void _oct_AssetsEnd(Oct_Context ctx) {
    // TODO: This
}

OCTARINE_API Oct_Bool oct_AssetLoaded(Oct_Asset asset) {
    return false; // TODO: This
}

OCTARINE_API Oct_Bool oct_AssetLoadFailed(Oct_Asset asset) {
    return false; // TODO: This
}

OCTARINE_API const char *oct_AssetErrorMessage(Oct_Allocator allocator) {
    return NULL; // TODO: This
}
