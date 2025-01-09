/// \brief Structs that are not visible to the engine user
#pragma once
#include <mimalloc.h>
#include <SDL2/SDL.h>
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief General engine context
struct Oct_Context_t {
    SDL_Window *window;         ///< Game window
    Oct_InitInfo *initInfo;     ///< Parameters the engine started with
    SDL_atomic_t quit;          ///< True to quit the game
    SDL_Thread *logicThread;    ///< Logic thread
    SDL_Thread *clockThread;    ///< Clock thread
    SDL_atomic_t frameStart;    ///< Atomic for clock thread to tell logic thread when it can begin the frame
    SDL_atomic_t renderHz;      ///< Render refresh rate, 0 means unlocked
    SDL_atomic_t logicHzActual; ///< Actual refresh rate of the logic thread, use OCT_INT_TO_FLOAT to get the value

    struct {
        Oct_Command *commands; ///< Internal buffer
        SDL_atomic_t head;     ///< The reading end of the buffer
        SDL_atomic_t tail;     ///< The writing end of the buffer
    } RingBuffer;              ///< Ring buffer for commands
};

/// \brief General purpose allocator
struct Oct_HeapAllocator_t {
    mi_heap_t *heap; ///< Internal mimalloc heap
};

/// \brief Arena allocator
struct Oct_ArenaAllocator_t {
    uint8_t *buffer; ///< Internal memory buffer
    int32_t size;    ///< Size of this arena
    int32_t point;   ///< Where the last allocation ended in the arena
};

#ifdef __cplusplus
};
#endif