/// \brief Structs that are not visible to the engine user
#pragma once
#include <mimalloc.h>
#include <SDL3/SDL.h>
#include <VK2D/Structs.h>
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief General engine context
struct Oct_Context_t {
    SDL_Window *window;             ///< Game window
    Oct_InitInfo *initInfo;         ///< Parameters the engine started with
    SDL_Thread *logicThread;        ///< Logic thread
    SDL_Thread *clockThread;        ///< Clock thread
    SDL_AtomicInt quit;             ///< True to quit the game
    SDL_AtomicInt frameStart;       ///< Atomic for clock thread to tell logic thread when it can begin the frame
    SDL_AtomicInt renderHz;         ///< Render refresh rate, 0 means unlocked
    SDL_AtomicInt renderHzActual;   ///< True render refresh rate
    SDL_AtomicInt logicHzActual;    ///< Actual refresh rate of the logic thread, use OCT_INT_TO_FLOAT to get the value
    SDL_AtomicInt interpolatedTime; ///< Estimated time it should be in the logic frame cycle, for interpolation, normalized 0-1 (the frame just started would be 0, the frame is just about done is close to 1)
    uint64_t gameStartTime;         ///< Time the logic thread started for the user to query time

    struct {
        Oct_Command *commands; ///< Internal buffer
        SDL_AtomicInt head;    ///< The reading end of the buffer
        SDL_AtomicInt tail;    ///< The writing end of the buffer
    } RingBuffer;              ///< Ring buffer for commands
};

/// \brief Data needed to draw and manage a sprite
typedef struct Oct_SpriteData_t {
    Oct_Texture texture;  ///< Texture the sprite comes from
    Oct_Bool ownsTexture; ///< Whether or not this specific sprite owns the texture (will delete it on its own deletion)
    int32_t frameCount;   ///< Number of frames in the animation
    int32_t frame;        ///< Current frame
    Oct_Bool repeat;      ///< Whether or not the animation repeats
    Oct_Bool pause;       ///< Whether or not the animation is currently paused
    double delay;         ///< Number of seconds between each frame
    double lastTime;      ///< The last time a frame was changed
    double accumulator;   ///< Time accumulator for more accurate animation timings
    Oct_Vec2 startPos;    ///< Starting position in the texture of the animation
    Oct_Vec2 frameSize;   ///< Size (in pixels) of each cell of the animation
    Oct_Vec2 padding;     ///< Horizontal and vertical padding (in pixels) between each animation frame
    float xStop;          ///< Horizontal stop for consecutive animation cells to start from (like if the animation is only in the right half of the image)
} Oct_SpriteData;

/// \brief Data for audio
typedef struct Oct_AudioData_t {
    uint8_t *data; ///< Raw audio data in whatever format Audio.c gDeviceSpec says
    int32_t size;  ///< Size of the data in bytes
} Oct_AudioData;

/// \brief An asset for the engine
struct Oct_AssetData_t {
    Oct_AssetType type;       ///< type of asset
    SDL_AtomicInt reserved;   ///< to allow the logic thread to find assets that still exist
    SDL_AtomicInt failed;     ///< This will be true if the load on this asset failed
    SDL_AtomicInt loaded;     ///< True when the asset is loaded
    SDL_AtomicInt generation; ///< Generation for unique ID purposes
    union {
        VK2DTexture texture;
        VK2DCameraIndex camera;
        Oct_SpriteData sprite;
        Oct_AudioData audio;
    };
};
typedef struct Oct_AssetData_t Oct_AssetData;

/// \brief Any type of allocator
struct Oct_Allocator_t {
    Oct_AllocatorType type; ///< Type of allocator this is
    union {
        struct {
            uint8_t *buffer; ///< Internal memory buffer
            int32_t size;    ///< Size of this arena
            int32_t point;   ///< Where the last allocation ended in the arena
        } arenaAllocator;
        mi_heap_t *heapAllocator; ///< Internal mimalloc heap
        struct {
            Oct_Allocator *pages; ///< Arenas (pages)
            int32_t count;        ///< Number of arenas
        }virtualPageAllocator;
    };
};

/// \brief Types of window eventsw
typedef enum {
    OCT_WINDOW_EVENT_TYPE_NONE = 0,           ///< None
    OCT_WINDOW_EVENT_TYPE_KEYBOARD = 1,       ///< Keyboard
    OCT_WINDOW_EVENT_TYPE_GAMEPAD_AXIS = 2,   ///< Gamepad axis
    OCT_WINDOW_EVENT_TYPE_GAMEPAD_BUTTON = 3, ///< Gamepad button
    OCT_WINDOW_EVENT_TYPE_GAMEPAD_EVENT = 4,  ///< Gamepad being connected/disconnected
    OCT_WINDOW_EVENT_TYPE_MOUSE_MOTION = 5,   ///< Mouse motion
    OCT_WINDOW_EVENT_TYPE_MOUSE_BUTTON = 6,   ///< Mouse buttons
    OCT_WINDOW_EVENT_TYPE_MOUSE_WHEEL = 7,    ///< Mouse wheel
} Oct_WindowEventType;

/// \brief A window event (typically input) that will get put in a ringbuffer to communicate from render thread -> logic thread
struct Oct_WindowEvent_t {
    Oct_WindowEventType type; ///< Type of event this is

    union {
        SDL_KeyboardEvent keyboardEvent;           ///< Keyboard event
        SDL_GamepadAxisEvent gamepadAxisEvent;     ///< Gamepad events
        SDL_GamepadButtonEvent gamepadButtonEvent; ///< Gamepad events
        SDL_GamepadDeviceEvent gamepadDeviceEvent; ///< Gamepad events
        SDL_MouseMotionEvent mouseMotionEvent;     ///< Mouse motion event
        SDL_MouseWheelEvent mouseWheelEvent;       ///< Mouse wheel event
        SDL_MouseButtonEvent mouseButtonEvent;     ///< Mouse button event
    };
};

typedef struct Oct_WindowEvent_t Oct_WindowEvent;

#ifdef __cplusplus
};
#endif