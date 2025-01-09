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
extern int32_t OCT_RING_BUFFER_SIZE; ///< Size of the command ring buffer

#ifdef __cplusplus
};
#endif
