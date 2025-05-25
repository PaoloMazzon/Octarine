/// \brief Core engine functionality
#pragma once
#include <stdint.h>
#include "oct/SDLConstants.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////// Defines //////////////////////

///< For opaque pointer types
#define OCT_OPAQUE_POINTER(type) typedef struct type##_t *type;

///< For user-modifiable and user-visible structures
#define OCT_USER_STRUCT(type) typedef struct type##_t type;

///< Grabs the structure type of a structure with an sType field (ie, `OCT_STRUCTURE_TYPE(&my_Oct_InitInfo)` -> `OCT_STRUCTURE_TYPE_INIT_INFO`)
#define OCT_STRUCTURE_TYPE(structure) (*((Oct_StructureType*)structure))

///< Turns an SDL_atomic_t into a float
#define OCT_INT_TO_FLOAT(i) (*((float*)&i))

///< Turns a float into an int
#define OCT_FLOAT_TO_INT(f) (*((int*)&f))

///< Max combined number of assets that can exist at once
#define OCT_MAX_ASSETS 3000

///< Tells texture rendering to render the whole width/height
#define OCT_WHOLE_TEXTURE -1

///< Tells origin to be the middle
#define OCT_ORIGIN_MIDDLE -9999

///< Tells origin to be on the right
#define OCT_ORIGIN_RIGHT -9998

///< Tells origin to be on the left (or just use 0)
#define OCT_ORIGIN_LEFT 0

///< How many possible fallback fonts a font can use
#define OCT_FALLBACK_FONT_MAX 5

///< For public definitions
/*#ifdef OctarineEngine_EXPORTS
#define OCTARINE_API __declspec(dllexport)
#else
#define OCTARINE_API __declspec(dllimport)
#endif*/
#define OCTARINE_API

///< In-house null
#define null (void*)0

#ifndef false
#define false (Oct_Bool)0
#endif
#ifndef true
#define true (Oct_Bool)1
#endif

////////////////////// Types //////////////////////
typedef uint64_t Oct_Status;     ///< Status code
typedef uint32_t Oct_Bool;       ///< Internal bool type
typedef uint64_t Oct_Asset;      ///< Any asset in the engine
typedef Oct_Asset Oct_Texture;   ///< A 2D texture in video memory
typedef Oct_Asset Oct_Audio;     ///< A loaded piece of audio
typedef Oct_Asset Oct_Sprite;    ///< 2D animation
typedef Oct_Asset Oct_Font;      ///< TrueType font (just the font itself)
typedef Oct_Asset Oct_FontAtlas; ///< A bitmap font
typedef Oct_Asset Oct_Camera;    ///< Camera that shows some portion of the game world
typedef uint64_t Oct_Sound;      ///< A sound that is currently playing (oct_Audio is the raw audio data, Oct_Sound is a currently playing piece of audio)
typedef float Oct_Vec4[4];       ///< Array of 4 floats
typedef float Oct_Vec3[3];       ///< Array of 3 floats
typedef float Oct_Vec2[2];       ///< Array of 2 floats
typedef void (*Oct_FileHandleCallback)(void*,uint32_t); ///< Callback for a file handle

////////////////////// Enums //////////////////////
/// \brief Structure types
typedef enum {
    OCT_STRUCTURE_TYPE_NONE = 0,
    OCT_STRUCTURE_TYPE_INIT_INFO = 1,
    OCT_STRUCTURE_TYPE_DRAW_COMMAND = 2,
    OCT_STRUCTURE_TYPE_WINDOW_COMMAND = 3,
    OCT_STRUCTURE_TYPE_LOAD_COMMAND = 4,
    OCT_STRUCTURE_TYPE_AUDIO_COMMAND = 5,
    OCT_STRUCTURE_TYPE_META_COMMAND = 6,
    OCT_STRUCTURE_TYPE_COMMAND = 7,
} Oct_StructureType;

/// \brief Types of draw commands
typedef enum {
    OCT_DRAW_COMMAND_TYPE_NONE       = 0,  ///< None
    OCT_DRAW_COMMAND_TYPE_TEXTURE    = 1,  ///< Texture rendering
    OCT_DRAW_COMMAND_TYPE_SPRITE     = 2,  ///< Sprite rendering
    OCT_DRAW_COMMAND_TYPE_DEBUG_TEXT = 3,  ///< Model rendering
    OCT_DRAW_COMMAND_TYPE_RECTANGLE  = 4,  ///< Square rendering
    OCT_DRAW_COMMAND_TYPE_CIRCLE     = 5,  ///< Circle rendering
    OCT_DRAW_COMMAND_TYPE_LINE       = 6,  ///< Line rendering
    OCT_DRAW_COMMAND_TYPE_POLYGON    = 7,  ///< Arbitrary polygon rendering
    OCT_DRAW_COMMAND_TYPE_CLEAR      = 8,  ///< Clear render target
    OCT_DRAW_COMMAND_TYPE_CAMERA     = 9,  ///< Some sort of camera update
    OCT_DRAW_COMMAND_TYPE_TARGET     = 10, ///< Changing render target
    OCT_DRAW_COMMAND_TYPE_FONT_ATLAS = 11, ///< Render bitmap fonts from an atlas
} Oct_DrawCommandType;

/// \brief Types of load commands
typedef enum {
    OCT_LOAD_COMMAND_TYPE_NONE = 0,               ///< None
    OCT_LOAD_COMMAND_TYPE_LOAD_TEXTURE = 1,       ///< Load a texture
    OCT_LOAD_COMMAND_TYPE_LOAD_SPRITE = 2,        ///< Load a sprite
    OCT_LOAD_COMMAND_TYPE_LOAD_FONT = 3,          ///< Load a font
    OCT_LOAD_COMMAND_TYPE_CREATE_FONT_ATLAS = 4,  ///< Load a font
    OCT_LOAD_COMMAND_TYPE_LOAD_BITMAP_FONT = 5,   ///< Load a font
    OCT_LOAD_COMMAND_TYPE_LOAD_AUDIO = 6,         ///< Loads audio
    OCT_LOAD_COMMAND_TYPE_CREATE_CAMERA = 7,      ///< Loads a model
    OCT_LOAD_COMMAND_TYPE_FREE = 8,               ///< Frees an asset
    OCT_LOAD_COMMAND_TYPE_CREATE_SURFACE = 9,     ///< Creates a surface
    OCT_LOAD_COMMAND_TYPE_CREATE_TEXT = 10,       ///< Creates a texture of properly formatted font TODO: This
    OCT_LOAD_COMMAND_TYPE_LOAD_ASSET_BUNDLE = 11, ///< Loads an asset bundle
} Oct_LoadCommandType;

/// \brief Types of window commands
typedef enum {
    OCT_WINDOW_COMMAND_TYPE_NONE = 0,              ///< None
    OCT_WINDOW_COMMAND_TYPE_RESIZE = 1,            ///< Resize the window
    OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_TOGGLE = 2, ///< Toggle fullscreen
    OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_EXIT = 3,   ///< Exit fullscreen (does nothing if windowed)
    OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_ENTER = 4,  ///< Enter fullscreen (does nothing if fullscreen)
} Oct_WindowCommandType;

/// \brief Types of audio commands
typedef enum {
    OCT_AUDIO_COMMAND_TYPE_NONE = 0,               ///< None
    OCT_AUDIO_COMMAND_TYPE_PLAY_SOUND = 1,         ///< Play a sound
    OCT_AUDIO_COMMAND_TYPE_UPDATE_SOUND = 2,       ///< Update a specific sound
    OCT_AUDIO_COMMAND_TYPE_PAUSE_ALL_SOUNDS = 3,   ///< Pause all currently playing sounds
    OCT_AUDIO_COMMAND_TYPE_UNPAUSE_ALL_SOUNDS = 4, ///< Unpause all paused sounds
    OCT_AUDIO_COMMAND_TYPE_STOP_ALL_SOUNDS = 5,    ///< Stop all currently playing sounds
} Oct_AudioCommandType;

/// \brief Types of meta commands
typedef enum {
    OCT_META_COMMAND_TYPE_NONE = 0,        ///< None
    OCT_META_COMMAND_TYPE_START_FRAME = 1, ///< Resize the window
    OCT_META_COMMAND_TYPE_END_FRAME = 2,   ///< Toggle fullscreen
} Oct_MetaCommandType;

/// \brief Types of file handles
typedef enum {
    OCT_FILE_HANDLE_TYPE_NONE = 0,        ///< This is not a file
    OCT_FILE_HANDLE_TYPE_FILENAME = 1,    ///< The file is a filename on disk
    OCT_FILE_HANDLE_TYPE_FILE_BUFFER = 2, ///< The file is available from a binary buffer
} Oct_FileHandleType;

/// \brief Types of allocators
typedef enum {
    OCT_ALLOCATOR_TYPE_NONE = 0,         ///< None
    OCT_ALLOCATOR_TYPE_HEAP = 1,         ///< General-purpose heap allocator
    OCT_ALLOCATOR_TYPE_ARENA = 2,        ///< Arena allocator
    OCT_ALLOCATOR_TYPE_VIRTUAL_PAGE = 3, ///< Virtual page allocator
} Oct_AllocatorType;

/// \brief Types of assets stored in an Oct_Asset
typedef enum {
    OCT_ASSET_TYPE_NONE = 0,       ///< None
    OCT_ASSET_TYPE_TEXTURE = 1,    ///< A texture
    OCT_ASSET_TYPE_FONT = 2,       ///< Font
    OCT_ASSET_TYPE_FONT_ATLAS = 3, ///< Font
    OCT_ASSET_TYPE_AUDIO = 4,      ///< Audio
    OCT_ASSET_TYPE_SPRITE = 5,     ///< Sprite
    OCT_ASSET_TYPE_CAMERA = 6,     ///< Camera
    OCT_ASSET_TYPE_MAX = 7,        ///< For iteration
    OCT_ASSET_TYPE_ANY = 100       ///< Any type
} Oct_AssetType;

/// \brief Things you can interpolate
typedef enum {
    OCT_INTERPOLATE_NONE = 0,         ///< Don't interpolate
    OCT_INTERPOLATE_POSITION = 1<<1,  ///< Interpolate position
    OCT_INTERPOLATE_ROTATION = 1<<2,  ///< Interpolate rotation
    OCT_INTERPOLATE_SCALE_X = 1<<3,   ///< Interpolate horizontal scale
    OCT_INTERPOLATE_SCALE_Y = 1<<4,   ///< Interpolate vertical scale
    OCT_INTERPOLATE_WIDTH = 1<<5,     ///< Interpolate width
    OCT_INTERPOLATE_HEIGHT = 1<<6,    ///< Interpolate height
    OCT_INTERPOLATE_RADIUS = 1<<7,    ///< Interpolate circle radius
    OCT_INTERPOLATE_ALL = INT32_MAX,  ///< Interpolate all of the above (where available)
} Oct_InterpolationType;

/// \brief Different ways a camera update can play out, can be bitwise or'd
typedef enum {
    OCT_CAMERA_UPDATE_TYPE_UPDATE_CAMERA = 1<<1,   ///< Update the associated camera with the associated camera update
    OCT_CAMERA_UPDATE_TYPE_LOCK_CAMERA = 1<<2,     ///< Lock rendering to exclusively this camera
    OCT_CAMERA_UPDATE_TYPE_UNLOCK_CAMERA = 1<<3,   ///< Unlock rendering (render to every camera)
    OCT_CAMERA_UPDATE_TYPE_ENABLE_TEX_CAM = 1<<4,  ///< Enables the use of cameras on texture targets
    OCT_CAMERA_UPDATE_TYPE_DISABLE_TEX_CAM = 1<<5, ///< Disables the use of cameras on texture targets
} Oct_CameraUpdateType;

////////////////////// Hidden structs //////////////////////
OCT_OPAQUE_POINTER(Oct_Context)
OCT_OPAQUE_POINTER(Oct_Allocator)
OCT_OPAQUE_POINTER(Oct_AssetBundle)

////////////////////// Structs //////////////////////

/// \brief Parameters for oct_Init
struct Oct_InitInfo_t {
    Oct_StructureType sType;                     ///< Structure type
    const char *windowTitle;                     ///< Title of the window
    int windowWidth;                             ///< Window width
    int windowHeight;                            ///< Window height
    int32_t ringBufferSize;                      ///< Size of the draw command ring buffer, if 0 this will be 1000
    int logicHz;                                 ///< Refresh rate of the logic thread, 0 will set this to 30
    Oct_Bool debug;                              ///< Enables debug features
    int argc;                                    ///< Command line parameters
    const char **argv;                           ///< Command line parameters
    void *(*startup)();                          ///< Function pointer to the startup function
    void *(*update)(void *ptr);                  ///< Function pointer to the update function
    void(*shutdown)(void *ptr);                  ///< Function pointer to the shutdown function
    void *pNext;                                 ///< For future use
};

/// \brief Window command to queue window events
struct Oct_WindowCommand_t {
    Oct_StructureType sType;    ///< Structure type
    Oct_WindowCommandType type; ///< Type of window command this is
    union {
        struct {
            Oct_Vec2 size; ///< Size of the window in pixels
        } Resize;          ///< Info needed for a resize
    };
    void *pNext; ///< For future use
};

/// \brief Way of representing a file across loads
struct Oct_FileHandle_t {
    Oct_FileHandleType type; ///< Type of file handle this is
    union {
        const char *filename; ///< Filename of the file on disk
        struct {
            uint8_t *buffer;                 ///< Binary buffer containing the file
            uint32_t size;                   ///< Size of the binary buffer in bytes
            const char *name;                ///< For debug purposes, internal use
            Oct_FileHandleCallback callback; ///< Callback called when the buffer is done being worked with, may be null
        };
    };
};
OCT_USER_STRUCT(Oct_FileHandle)

/// \brief Load command to load or free anything (just use oct_FreeAsset, don't use this manually for freeing)
/// \note Cameras don't require anything, just pass a load command with the type OCT_LOAD_COMMAND_TYPE_CREATE_CAMERA
struct Oct_LoadCommand_t {
    Oct_StructureType sType;  ///< Structure type
    Oct_LoadCommandType type; ///< Type of load command this is
    Oct_Asset _assetID;       ///< Internal use
    union {
        struct {
            Oct_FileHandle fileHandle; ///< File handle of the texture to load (png, jpg, bmp)
        } Texture;                     ///< Information needed to load a texture
        struct {
            Oct_Vec2 dimensions; ///< Dimensions of the new surface
        } Surface;               ///< Information needed to create a surface
        struct {
            Oct_Texture texture; ///< Texture the sprite comes from
            int32_t frameCount;  ///< Number of frames in the animation
            Oct_Bool repeat;     ///< Whether or not the animation repeats
            double fps;          ///< Number of seconds between each frame
            Oct_Vec2 startPos;   ///< Starting position in the texture of the animation
            Oct_Vec2 frameSize;  ///< Size (in pixels) of each cell of the animation
            Oct_Vec2 padding;    ///< Horizontal and vertical padding (in pixels) between each animation frame
            float xStop;         ///< Horizontal stop for consecutive animation cells to start from (like if the animation is only in the right half of the image)
        } Sprite;                ///< Information needed to load a sprite
        struct {
            Oct_FileHandle fileHandle; ///< File handle of the audio file
        } Audio;                       ///< Information needed to create an audio sample
        struct {
            Oct_FileHandle fileHandles[OCT_FALLBACK_FONT_MAX]; ///< File handles of the ttf
        } Font;                                                ///< Information needed to load a font
        struct {
            Oct_Font font;         ///< Font to generate the atlas from
            Oct_FontAtlas atlas;   ///< If this is not OCT_NO_ASSET, the new atlas will be appended to this one
            float size;            ///< Size of the font to make the atlas for
            uint64_t unicodeStart; ///< Unicode start range
            uint64_t unicodeEnd;   ///< Unicode end range
        } FontAtlas;               ///< Info needed to create or extend a font atlas
        struct {
            Oct_FileHandle fileHandle; ///< File handle of the bitmap font (an image)
            uint64_t unicodeStart;     ///< Unicode start range (inclusive)
            uint64_t unicodeEnd;       ///< Unicode end range (exclusive)
            Oct_Vec2 cellSize;         ///< Size (in pixels) of each font glyph
        } BitmapFont;
        struct {
            const char *filename;   ///< Filename of the bundle
            Oct_AssetBundle bundle; ///< Bundle that will be loaded into
        } AssetBundle;              ///< Info to load a bundle
    };
    // TODO: This
    void *pNext; ///< For future use
};

/// \brief Audio command to draw anything from any thread
struct Oct_AudioCommand_t {
    Oct_StructureType sType;   ///< Structure type
    Oct_AudioCommandType type; ///< Type of load command this is
    union {
        struct {
            Oct_Sound _soundID; ///< Internal use
            Oct_Audio audio;    ///< Audio asset to play
            Oct_Bool repeat;    ///< Whether or not the sound will repeat
            Oct_Vec2 volume;    ///< L/R normalized volume
        } Play;                 ///< Things needed to play a sound
        struct {
            Oct_Sound sound; ///< Sound ID of a sound currently playing
            Oct_Bool repeat; ///< Whether or not the sound will repeat
            Oct_Vec2 volume; ///< L/R normalized volume
            Oct_Bool paused; ///< Whether or not the sound is paused
            Oct_Bool stop;   ///< Whether or not to stop the sound
        } Update;            ///< Things needed to update a sound
    };
    void *pNext;               ///< For future use
};

/// \brief Meta commands, probably only internal use
struct Oct_MetaCommand_t {
    Oct_StructureType sType;  ///< Structure type
    Oct_MetaCommandType type; ///< Type of meta command this is
    void *pNext;              ///< For future use
};

/// \brief A camera update
struct Oct_CameraUpdate_t {
    Oct_Vec2 position;       ///< Position in the game world of the camera
    Oct_Vec2 size;           ///< Virtual size of the camera
    float rotation;          ///< Camera's rotation (in radians)
    Oct_Vec2 screenPosition; ///< Position in the window of the camera
    Oct_Vec2 screenSize;     ///< Size of the camera in the window
};

/// \brief Rectangle
struct Oct_Rectangle_t {
    Oct_Vec2 position; ///< X/Y position
    Oct_Vec2 size;     ///< Width/height in pixels
};

/// \brief Circle
struct Oct_Circle_t {
    Oct_Vec2 position; ///< X/Y position
    float radius;      ///< Radius in pixels
};

/// \brief Normalized colour
struct Oct_Colour_t {
    float r; ///< Red component (0 - 1)
    float g; ///< Green component (0 - 1)
    float b; ///< Blue component (0 - 1)
    float a; ///< Alpha component (0 - 1)
};

////////////////////// User structs //////////////////////
OCT_USER_STRUCT(Oct_InitInfo)
OCT_USER_STRUCT(Oct_DrawCommand)
OCT_USER_STRUCT(Oct_Command)
OCT_USER_STRUCT(Oct_DrawCommand)
OCT_USER_STRUCT(Oct_WindowCommand)
OCT_USER_STRUCT(Oct_LoadCommand)
OCT_USER_STRUCT(Oct_AudioCommand)
OCT_USER_STRUCT(Oct_MetaCommand)
OCT_USER_STRUCT(Oct_CameraUpdate)
OCT_USER_STRUCT(Oct_Rectangle)
OCT_USER_STRUCT(Oct_Circle)
OCT_USER_STRUCT(Oct_Colour)

/// \brief Draw command to draw anything
struct Oct_DrawCommand_t {
    Oct_StructureType sType;           ///< Structure type
    Oct_DrawCommandType type;          ///< Type of draw command this is
    Oct_Colour colour;                 ///< Colour modifier
    Oct_InterpolationType interpolate; ///< See Oct_InterpolationType, bitwise OR them together
    uint64_t id;                       ///< ID of this command to match it with a previous command for interpolation
    union {
        struct {
            Oct_Rectangle rectangle; ///< Rectangle
            Oct_Bool filled;         ///< Weather or not its filled
            float lineSize;          ///< Size of the lines if its not filled
            float rotation;          ///< Rotation in radians
            Oct_Vec2 origin;         ///< Origin of rotation, x/y
        } Rectangle;                 ///< Information to draw a rectangle
        struct {
            Oct_Circle circle; ///< Rectangle
            Oct_Bool filled;   ///< Weather or not its filled
            float lineSize;    ///< Size of the lines if its not filled
        } Circle;              ///< Information to draw a circle
        struct {
            Oct_Texture texture;    ///< Texture to draw
            Oct_Rectangle viewport; ///< Where in the texture to draw (use OCT_WHOLE_TEXTURE)
            Oct_Vec2 position;      ///< Where on the game world to draw it
            Oct_Vec2 scale;         ///< Scale of the texture, {1, 1} being normal
            Oct_Vec2 origin;        ///< Origin of rotation and offset
            float rotation;         ///< Rotation in radians
        } Texture;
        struct {
            Oct_Sprite sprite;      ///< Sprite to draw
            Oct_Rectangle viewport; ///< Where in the animation frame to draw (use OCT_WHOLE_TEXTURE)
            Oct_Vec2 position;      ///< Where on the game world to draw it
            Oct_Vec2 scale;         ///< Scale of the texture, {1, 1} being normal
            Oct_Vec2 origin;        ///< Origin of rotation and offset
            int32_t frame;          ///< Animation frame to render (use OCT_SPRITE_*_FRAME, typically OCT_SPRITE_CURRENT_FRAME, frames are indexed starting at 1)
            float rotation;         ///< Rotation in radians
        } Sprite;
        struct {
            Oct_Asset texture; ///< Texture that the render target will switch to (or OCT_TARGET_SWAPCHAIN)
        } Target;              ///< For changing render targets
        struct {
            Oct_CameraUpdate cameraUpdate;   ///< New camera information
            Oct_CameraUpdateType updateType; ///< Type of update
            Oct_Camera camera;               ///< Which camera to update
        } Camera;                            ///< A camera update
        struct {
            const char *text;  ///< Text to render
            Oct_Vec2 position; ///< Position in the game world of the text
            float scale;       ///< Scale of the text (1 for default)
        } DebugText;           ///< Info needed to draw a simple debug string on screen (ASCII only)
        struct {
            Oct_Vec2 position;   ///< Position of the text
            const char *text;    ///< The text itself
            float scale;         ///< Scale of the text
            Oct_FontAtlas atlas; ///< Atlas to use
        } FontAtlas;             ///< Info needed to render from a font atlas
    };
    void *pNext; ///< For future use
};

/// \brief Any type of command the logic thread sends to the render thread - mostly internal use
struct Oct_Command_t {
    Oct_StructureType sType;             ///< Structure type
    union {
        int topOfUnion;                  ///< For OCT_STRUCTURE_TYPE
        Oct_DrawCommand drawCommand;     ///< Draw command
        Oct_WindowCommand windowCommand; ///< Window command
        Oct_LoadCommand loadCommand;     ///< Load command
        Oct_AudioCommand audioCommand;   ///< Audio command
        Oct_MetaCommand metaCommand;     ///< Meta command
    };
    void *pNext;                         ///< For future use
};

#ifdef __cplusplus
};
#endif