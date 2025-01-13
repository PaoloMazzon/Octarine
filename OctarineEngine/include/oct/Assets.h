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
OCTARINE_API Oct_Asset oct_Load(Oct_LoadCommand *load);

/// \brief Returns true if the asset was successfully loaded, false if its not loaded for any reason
OCTARINE_API Oct_Bool oct_AssetLoaded(Oct_Asset asset);

/// \brief Returns true if an asset was failed to load for any reason
OCTARINE_API Oct_Bool oct_AssetLoadFailed(Oct_Asset asset);

/// \brief Returns an error message or null if there is none (or the allocation fails)
/// \param allocator Allocator to get memory from in order to copy the error and return it
/// \return Returns the error string or null, you must free that string yourself with the allocator provided.
/// \warning This will return any and all error strings should multiple errors happen in a short period
OCTARINE_API const char *oct_AssetErrorMessage(Oct_Allocator allocator);

#ifdef __cplusplus
};
#endif