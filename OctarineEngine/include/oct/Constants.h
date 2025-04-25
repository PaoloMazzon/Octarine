/// \brief Constant declarations
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Status codes
extern Oct_Status OCT_STATUS_SUCCESS;             ///< Success
extern Oct_Status OCT_STATUS_ERROR;               ///< Catch-all failure
extern Oct_Status OCT_STATUS_OUT_OF_MEMORY;       ///< A memory allocation failed
extern Oct_Status OCT_STATUS_SDL_ERROR;           ///< Something went wrong in SDL
extern Oct_Status OCT_STATUS_VULKAN_ERROR;        ///< Something went wrong in Vulkan
extern Oct_Status OCT_STATUS_VK2D_ERROR;          ///< Something went wrong in VK2D
extern Oct_Status OCT_STATUS_FILE_DOES_NOT_EXIST; ///< A file does not exist
extern Oct_Status OCT_STATUS_BAD_PARAMETER;       ///< A parameter given was not allowed

// Memory
extern int32_t OCT_RING_BUFFER_SIZE;   ///< Size of the command/event ring buffer
extern int32_t OCT_STANDARD_PAGE_SIZE; ///< Size of a page in a virtual page allocator
extern int32_t OCT_PAGE_SCALE_FACTOR;  ///< How much bigger a page should be to accommodate large memory

// Drawing
extern int32_t OCT_SPRITE_LAST_FRAME;    ///< Draw the last frame of the animation
extern int32_t OCT_SPRITE_FIRST_FRAME;   ///< Draw the first frame of the animation
extern int32_t OCT_SPRITE_CURRENT_FRAME; ///< Draw the current frame of the animation

// Various
extern Oct_Asset OCT_TARGET_SWAPCHAIN; ///< Target the swapchain (window)
extern Oct_Asset OCT_NO_ASSET;         ///< For things like specifying a new texture in sprite creation
extern Oct_Sound OCT_SOUND_FAILED;     ///< Sound was failed to be played

#ifdef __cplusplus
};
#endif
