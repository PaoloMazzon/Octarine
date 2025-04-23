/// \brief Functions relating to assets
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Adds a load command to the command queue and returns the asset
/// \param load Load command, refer to that Oct_LoadCommand's documentation
/// \return Returns a new asset ID
///
/// This will add a load command to the command queue. The asset is not immediately
/// loaded and will be loaded whenever the render thread receives the command. This is
/// guaranteed to be before the next draw command is executed unless the load fails for
/// any reason. What this means in practice is that this function does not immediately
/// load the asset but you may treat the asset as thought it were.
///
/// Assets in Octarine are just 64-bit integers. They internally index an asset array
/// but also their more significant 32 bits represent their generation which means that
/// no two assets will ever have the same asset value throughout the course of your game.
/// For that reason, if you delete an asset and allocate a new one that ends up in the
/// same spot in the asset array, something like oct_AssetLoaded will still say that the deleted
/// asset is not loaded because it knows the generation does not match.
OCTARINE_API Oct_Asset oct_Load(Oct_Context ctx, Oct_LoadCommand *load);

/// \brief Shorthand for oct_Load for loading textures
/// \param ctx Octarine context
/// \param filename Filename of the texture
/// \return Returns a new asset ID, see oct_Load for more info
OCTARINE_API Oct_Asset oct_LoadTexture(Oct_Context ctx, const char *filename);

/// \brief Shorthand for oct_Load for creating surfaces
/// \param ctx Octarine context
/// \param size Size of the new surface
/// \return Returns a new asset ID, see oct_Load for more info
OCTARINE_API Oct_Asset oct_CreateSurface(Oct_Context ctx, Oct_Vec2 size);

/// \brief Shorthand for oct_Load for creating cameras
/// \return Returns a new asset ID, see oct_Load for more info
OCTARINE_API Oct_Asset oct_CreateCamera(Oct_Context ctx);

/// \brief Shorthand for oct_Load for loading sprites
/// \param tex Texture for the sprite to use
/// \param frameCount Number of frames in the animation
/// \param fps FPS of the animation
/// \param startPos Where in the texture the animation starts
/// \param frameSize Size of each animation cell
/// \return Returns a new asset ID, see oct_Load for more info
OCTARINE_API Oct_Asset oct_LoadSprite(Oct_Context ctx, Oct_Texture tex, int32_t frameCount, double fps, Oct_Vec2 startPos, Oct_Vec2 frameSize);

// TODO - Duplicate sprite function

/// \brief Returns true if the asset was successfully loaded, false if its not loaded for any reason
OCTARINE_API Oct_Bool oct_AssetLoaded(Oct_Asset asset);

/// \brief Returns true if an asset was failed to load for any reason
/// \warning If this returns true that asset ID is invalidated
OCTARINE_API Oct_Bool oct_AssetLoadFailed(Oct_Asset asset);

/// \brief Returns an error message or null if there is none (or the allocation fails)
/// \param allocator Allocator to get memory from in order to copy the error and return it
/// \return Returns the error string or null, you must free that string yourself with the allocator provided.
/// \warning This will return any and all error strings should multiple errors happen in a short period
/// \warning This will clear the error message and load failed status
OCTARINE_API const char *oct_AssetErrorMessage(Oct_Allocator allocator);

/// \brief Same as oct_AssetErrorMessage but the result in your own provided buffer
/// \param size Pointer to an int that will be filled with the required size of the buffer
/// \param buffer Buffer of at least *size where the error message contents will be copied to (this may be null to get the size first)
/// \return Returns buffer, if buffer was provided
/// \warning This will clear the error message and load failed status
OCTARINE_API const char *oct_AssetGetErrorMessage(int *size, char *buffer);

/// \brief Frees any asset
///
/// You don't need to use this, the engine will free everything automatically
/// on exit. This is more for memory management in bigger projects.
OCTARINE_API void oct_FreeAsset(Oct_Context ctx, Oct_Asset asset);

/// \brief Returns true if any load as failed, can be used as an indication to get the error message
OCTARINE_API Oct_Bool oct_AssetLoadHasFailed();

#ifdef __cplusplus
};
#endif