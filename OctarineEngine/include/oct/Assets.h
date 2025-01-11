/// \brief Functions relating to assets
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

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