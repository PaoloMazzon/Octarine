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
OCTARINE_API Oct_Asset oct_Load(Oct_LoadCommand *load);

/// \brief Shorthand for oct_Load for loading textures
/// \param ctx Octarine context
/// \param filename Filename of the texture
/// \return Returns a new asset ID, see oct_Load for more info
OCTARINE_API Oct_Texture oct_LoadTexture(const char *filename);

/// \brief Shorthand for oct_Load for loading fonts
/// \param ctx Octarine context
/// \param filename Filename of the font
/// \return Returns a new asset ID, see oct_Load for more info
///
/// This function only allows for one font file to be associated with the font, if you
/// want to set fallbacks you'll have to create the load command manually.
OCTARINE_API Oct_Font oct_LoadFont(const char *filename);

/// \brief Shorthand for oct_Load for creating surfaces
/// \param ctx Octarine context
/// \param size Size of the new surface
/// \return Returns a new asset ID, see oct_Load for more info
OCTARINE_API Oct_Texture oct_CreateSurface(Oct_Vec2 size);

/// \brief Shorthand for oct_Load for creating cameras
/// \return Returns a new asset ID, see oct_Load for more info
OCTARINE_API Oct_Camera oct_CreateCamera();

/// \brief Shorthand for oct_Load for loading sprites
/// \param ctx Context
/// \param tex Texture for the sprite to use
/// \param frameCount Number of frames in the animation
/// \param fps FPS of the animation
/// \param startPos Where in the texture the animation starts
/// \param frameSize Size of each animation cell
/// \return Returns a new asset ID, see oct_Load for more info
OCTARINE_API Oct_Sprite oct_LoadSprite(Oct_Texture tex, int32_t frameCount, double fps, Oct_Vec2 startPos, Oct_Vec2 frameSize);

// TODO - Duplicate sprite function

/// \brief Creates or extends an existing font atlas
/// \param ctx Context
/// \param font Font to create the atlas from
/// \param atlas Atlas to extend, if you use OCT_NO_ASSET a new atlas will be created
/// \param size Size of the font to use when creating the atlas
/// \param unicodeStart First unicode code point to put in the atlas (inclusive)
/// \param unicodeEnd Last unicode code point to put in the atlas (exclusive)
/// \return Returns either the handle to a new font atlas, or the atlas passed in if one was specified
OCTARINE_API Oct_FontAtlas oct_CreateFontAtlas(Oct_Font font, Oct_FontAtlas atlas, float size, uint32_t unicodeStart, uint32_t unicodeEnd);

/// \brief Loads a font atlas from a bitmap font
/// \param ctx Context
/// \param filename Filename of the bitmap to load
/// \param cellSize Size of each cell in pixels
/// \param unicodeStart First unicode code point to put in the atlas (inclusive)
/// \param unicodeEnd Last unicode code point to put in the atlas (exclusive)
/// \return Returns a new font atlas that can be used like any other
OCTARINE_API Oct_FontAtlas oct_LoadBitmapFont(const char *filename, Oct_Vec2 cellSize, uint32_t unicodeStart, uint32_t unicodeEnd);

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
OCTARINE_API void oct_FreeAsset(Oct_Asset asset);

/// \brief Returns true if any load as failed, can be used as an indication to get the error message
OCTARINE_API Oct_Bool oct_AssetLoadHasFailed();

/// \brief Loads an asset bundle from a file
/// \param filename Name of the asset bundle
/// \return Returns a new asset bundle or null, if it fails
/// \warning This function queues an asset bundle load like any other asset. The difference is that unlike asset loading
///          functions, the assets are not guaranteed to be ready by the time you call oct_GetAsset -- as this function
///          is not blocking. For this reason, calling oct_GetAsset will be blocking if the asset bundle is not done
///          loading. Use oct_IsAssetBundleReady to check if oct_GetAsset will be blocking or not.
///
/// An asset bundle is either a directory or archive with a manifest.json in the root of that directory/archive.
/// This function internally uses PhysFS, which is to say you can use any archive supported by that library (basically
/// anything).
///
/// This will automatically load spritesheets exported from Aseprite that have their .json sprite data -- as long as
/// you export metadata in that json and its exported as "Array," NOT "Hash." Sprites loaded this way will be available
/// under the name of the json.
OCTARINE_API Oct_AssetBundle oct_LoadAssetBundle(const char *filename);

/// \brief Destroys an asset bundle and all the assets in it
OCTARINE_API void oct_FreeAssetBundle(Oct_AssetBundle bundle);

/// \brief Checks if an asset bundle is loaded yet
OCTARINE_API Oct_Bool oct_IsAssetBundleReady(Oct_AssetBundle bundle);

/// \brief Gets an asset from a bundle, or OCT_NO_ASSET if no such asset exists
/// \warning If the asset bundle is not yet loaded completely, this will be blocking
OCTARINE_API Oct_Asset oct_GetAsset(Oct_AssetBundle bundle, const char *name);

/// \brief Returns true if a given asset exists
/// \warning If the asset bundle is not yet loaded completely, this will be blocking
OCTARINE_API Oct_Bool oct_AssetExists(Oct_AssetBundle bundle, const char *name);

#ifdef __cplusplus
};
#endif