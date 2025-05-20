/// \brief A circular buffer for lockless command communication between threads
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Queues a draw command
OCTARINE_API void oct_Draw(Oct_DrawCommand *draw);

/// \brief Queues a window update
OCTARINE_API void oct_WindowUpdate(Oct_WindowCommand *windowUpdate);

/// \brief Queues an audio command and returns the handle of the new playing sound or whatever sound was updated
OCTARINE_API Oct_Sound oct_AudioUpdate(Oct_AudioCommand *audioCommand);

/// \brief Queues an asset load and returns the asset -- todo: make this per asset type and return the proper type
OCTARINE_API Oct_Asset oct_Load(Oct_LoadCommand *load);

/// \brief Copies memory into volatile per-frame memory and returns it
///
/// Some commands require variable amounts of memory, like sending strings to the render thread, that may not
/// exist in permanent memory on the user's side. For example, if you were to pass a string in a command that you
/// expect to be destroyed very soon. In that case, you can copy that string into this function and use the pointer
/// returned by this instead. This memory is guaranteed to exist until the render thread has finished processing.
/// The shorthand functions, like oct_LoadTexture will use this automatically under the hood, and you'd only ever
/// need to use this if you were creating your own commands.
OCTARINE_API void *oct_CopyFrameData(void *data, int32_t size);

#ifdef __cplusplus
};
#endif