#include "oct/Constants.h"

// Statuses
Oct_Status OCT_STATUS_SUCCESS             = 1<<1;
Oct_Status OCT_STATUS_ERROR               = 1<<2;
Oct_Status OCT_STATUS_SDL_ERROR           = 1<<3;
Oct_Status OCT_STATUS_VULKAN_ERROR        = 1<<4;
Oct_Status OCT_STATUS_VK2D_ERROR          = 1<<5;
Oct_Status OCT_STATUS_FILE_DOES_NOT_EXIST = 1<<6;
Oct_Status OCT_STATUS_BAD_PARAMETER       = 1<<7;
Oct_Status OCT_STATUS_OUT_OF_MEMORY       = 1<<8;

// Memory-related
int32_t OCT_RING_BUFFER_SIZE = 1000;
int32_t OCT_STANDARD_PAGE_SIZE = 1024 * 50; // 50kb
int32_t OCT_PAGE_SCALE_FACTOR = 3;

// Drawing
int32_t OCT_SPRITE_LAST_FRAME = -1;
int32_t OCT_SPRITE_FIRST_FRAME = 1;
int32_t OCT_SPRITE_CURRENT_FRAME = 0;

// Various
Oct_Asset OCT_TARGET_SWAPCHAIN = UINT64_MAX;
Oct_Asset OCT_NO_ASSET = UINT64_MAX;
Oct_Sound OCT_SOUND_FAILED = UINT64_MAX;
