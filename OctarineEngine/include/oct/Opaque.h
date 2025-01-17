/// \brief Structs that are not visible to the engine user
#pragma once
#include <mimalloc.h>
#include <SDL2/SDL.h>
#include <VK2D/Structs.h>
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief General engine context
struct Oct_Context_t {
    SDL_Window *window;            ///< Game window
    Oct_InitInfo *initInfo;        ///< Parameters the engine started with
    SDL_Thread *logicThread;       ///< Logic thread
    SDL_Thread *clockThread;       ///< Clock thread
    SDL_atomic_t quit;             ///< True to quit the game
    SDL_atomic_t frameStart;       ///< Atomic for clock thread to tell logic thread when it can begin the frame
    SDL_atomic_t renderHz;         ///< Render refresh rate, 0 means unlocked
    SDL_atomic_t renderHzActual;   ///< True render refresh rate
    SDL_atomic_t logicHzActual;    ///< Actual refresh rate of the logic thread, use OCT_INT_TO_FLOAT to get the value
    SDL_atomic_t interpolatedTime; ///< Estimated time it should be in the logic frame cycle, for interpolation, normalized 0-1 (the frame just started would be 0, the frame is just about done is close to 1)
    uint64_t gameStartTime;        ///< Time the logic thread started for the user to query time

    struct {
        Oct_Command *commands; ///< Internal buffer
        SDL_atomic_t head;     ///< The reading end of the buffer
        SDL_atomic_t tail;     ///< The writing end of the buffer
    } RingBuffer;              ///< Ring buffer for commands
};

// An asset for the engine
struct Oct_AssetData_t {
    Oct_AssetType type;    // type of asset
    SDL_atomic_t reserved; // to allow the logic thread to find assets that still exist
    SDL_atomic_t failed;   // This will be true if the load on this asset failed
    SDL_atomic_t loaded;   // True when the asset is loaded
    union {
        VK2DTexture texture;
    };
};
typedef struct Oct_AssetData_t Oct_AssetData;

/// \brief Any type of allocator
struct Oct_Allocator_t {
    Oct_AllocatorType type;
    union {
        struct {
            uint8_t *buffer; ///< Internal memory buffer
            int32_t size;    ///< Size of this arena
            int32_t point;   ///< Where the last allocation ended in the arena
        } arenaAllocator;
        mi_heap_t *heapAllocator; ///< Internal mimalloc heap
    };
};

#ifdef __cplusplus
};
#endif