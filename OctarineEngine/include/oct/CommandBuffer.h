/// \brief A circular buffer for lockless command communication between threads
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Queues a draw command
OCTARINE_API void oct_Draw(Oct_Context ctx, Oct_DrawCommand *draw);

/// \brief Queues a window update
OCTARINE_API void oct_WindowUpdate(Oct_Context ctx, Oct_WindowCommand *windowUpdate);

/// \brief Queues an asset load and returns the asset -- todo: make this per asset type and return the proper type
OCTARINE_API Oct_Asset oct_Load(Oct_Context ctx, Oct_LoadCommand *load);

/// \brief Frees any asset
///
/// You don't need to use this, the engine will free everything automatically
/// on exit. This is more for memory management in bigger projects.
OCTARINE_API void oct_FreeAsset(Oct_Context ctx, Oct_Asset asset);

#ifdef __cplusplus
};
#endif