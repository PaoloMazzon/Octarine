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

/// \brief Queues an asset load -- todo: make this per asset type and return the proper type
OCTARINE_API Oct_Asset oct_Load(Oct_Context ctx, Oct_LoadCommand *load);

#ifdef __cplusplus
};
#endif