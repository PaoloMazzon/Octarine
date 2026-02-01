/// \brief Structs that are not visible to the engine user
#pragma once
#include <mimalloc.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <VK2D/Structs.h>
#include "oct/Common.h"
#include "oct/Constants.h"

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
    SDL_AtomicInt logicProcessTime; ///< Time spent processing logic frames - sleep time
    SDL_AtomicInt interpolatedTime; ///< Estimated time it should be in the logic frame cycle, for interpolation, normalized 0-1 (the frame just started would be 0, the frame is just about done is close to 1)
    uint64_t gameStartTime;         ///< Time the logic thread started for the user to query time

    struct {
        Oct_Command *commands; ///< Internal buffer
        SDL_AtomicInt head;    ///< The reading end of the buffer
        SDL_AtomicInt tail;    ///< The writing end of the buffer
        SDL_Mutex *tailLock;   ///< To allow for MPSC
    } RingBuffer;              ///< Ring buffer for commands
};

/// \brief A frame of a sprite's animation
typedef struct Oct_SpriteFrame_t {
    Oct_Vec2 position; ///< Position of this frame
    Oct_Vec2 size;     ///< Size of this frame
} Oct_SpriteFrame;

/// \brief Data needed to draw and manage a sprite
typedef struct Oct_SpriteData_t {
    Oct_Texture texture;     ///< Texture the sprite comes from
    int32_t frameCount;      ///< Number of frames in the animation
    Oct_SpriteFrame *frames; ///< Each individual animation frame
    double duration;         ///< Duration of each frame in seconds
} Oct_SpriteData;

/// \brief Data for audio
typedef struct Oct_AudioData_t {
    uint8_t *data; ///< Raw audio data in whatever format Audio.c gDeviceSpec says
    int32_t size;  ///< Size of the data in bytes
} Oct_AudioData;

/// \brief An individual glyph in a bitmap atlas
typedef struct Oct_FontGlyphData_t {
    Oct_Rectangle location; ///< Where in the atlas this specific glyph is
    int minBB[2];           ///< Minimum bounding box
    int maxBB[2];           ///< Maximum bounding box
    int advance;            ///< Horizontal advance
} Oct_FontGlyphData;

/// \brief An individual bitmap atlas
typedef struct Oct_FontAtlasData_t {
    VK2DTexture atlas;         ///< Texture atlas with all the characters
    VK2DImage img;             ///< Image backing the texture
    uint64_t unicodeStart;     ///< Start of the unicode range this atlas covers
    uint64_t unicodeEnd;       ///< End of the range this atlas covers
    Oct_FontGlyphData *glyphs; ///< Array of glyphs containing positional and offset data
} Oct_FontAtlasData;

/// \brief Data for bitmap fonts, may contain number of bitmap fonts and unicode ranges
typedef struct Oct_BitmapFontData_t {
    Oct_FontAtlasData *atlases; ///< Bitmap font atlas information
    int32_t atlasCount;         ///< How many atlases exist in this bitmap font
    float spaceSize;            ///< Size of a space
    float newLineSize;          ///< Size between newlines
    Oct_Font font;              ///< Font this comes from for kerning (or OCT_NO_ASSET)
} Oct_BitmapFontData;

/// \brief Info for fonts
typedef struct Oct_FontData_t {
    TTF_Font *font[OCT_FALLBACK_FONT_MAX]; ///< TTF fonts, plus extras if fallbacks are present
    void *buffers[OCT_FALLBACK_FONT_MAX];  ///< Copies of the buffers of each loaded fallback font
} Oct_FontData;

/// \brief Data for textures
typedef struct Oct_TextureData_t {
    VK2DTexture tex;
    SDL_AtomicInt width;
    SDL_AtomicInt height;
} Oct_TextureData;

typedef struct Oct_ShaderData_t {
    VK2DShader shader;
} Oct_ShaderData;

/// \brief An asset for the engine
struct Oct_AssetData_t {
    Oct_AssetType type;       ///< type of asset
    SDL_AtomicInt reserved;   ///< to allow the logic thread to find assets that still exist
    SDL_AtomicInt failed;     ///< This will be true if the load on this asset failed
    SDL_AtomicInt loaded;     ///< True when the asset is loaded
    SDL_AtomicInt generation; ///< Generation for unique ID purposes
    char name[OCT_ASSET_NAME_SIZE]; ///< Name of the asset for debugging
    union {
        Oct_TextureData texture;
        VK2DCameraIndex camera;
        Oct_SpriteData sprite;
        Oct_AudioData audio;
        Oct_FontData font;
        Oct_BitmapFontData fontAtlas; // bitmap fonts are collections of atlases
        Oct_ShaderData shader;
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

/// \brief Linked list for asset bundle hash map
typedef struct Oct_AssetLink_t {
    Oct_Asset asset;  ///< Asset itself (OCT_NO_ASSET) if spot is not occupied
    const char *name; ///< Name of the asset for collision checking
    int32_t next;     ///< Next asset in the linked list or -1 for end of it (index to last item)
} Oct_AssetLink;

/// \brief A bundle of loaded assets in a hashmap
struct Oct_AssetBundle_t {
    Oct_AssetLink *bucket;       ///< Bucket where all the assets are stored
    Oct_AssetLink *backupBucket; ///< Backup bucket (linked lists from the primary bucket point to an element of this list)
    int backupBucketSize;        ///< Size of the backupBucket list
    int backupBucketCount;       ///< Number of elements actually in use in the backupBucket
    SDL_AtomicInt bundleReady;   ///< Whether or not the bundle is ready to use
};

/// \brief A tilemap
struct Oct_Tilemap_t {
    Oct_Texture tex;   ///< Texture the tilemap uses
    int32_t *grid;     ///< The actual grid, each cell is an index starting at 1
    int width;         ///< Width of the grid in cells
    int height;        ///< Height of the grid in cells
    Oct_Vec2 cellSize; ///< Size of a cell in pixels
    Oct_Vec2 texSize;  ///< Size of the texture in pixels
};

#ifdef __cplusplus
};
#endif
