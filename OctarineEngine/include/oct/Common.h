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
typedef uint64_t Oct_Status;   ///< Status code
typedef uint32_t Oct_Bool;     ///< Internal bool type
typedef uint64_t Oct_Asset;    ///< Any asset in the engine
typedef Oct_Asset Oct_Texture; ///< Any asset in the engine
typedef Oct_Asset Oct_Audio;   ///< Any asset in the engine
typedef Oct_Asset Oct_Model;   ///< Any asset in the engine
typedef Oct_Asset Oct_Sprite;  ///< Any asset in the engine
typedef Oct_Asset Oct_Font;    ///< Any asset in the engine
typedef float Oct_Vec4[4];     ///< Array of 4 floats
typedef float Oct_Vec3[3];     ///< Array of 3 floats
typedef float Oct_Vec2[2];     ///< Array of 2 floats

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
    OCT_DRAW_COMMAND_TYPE_NONE    = 0,    ///< None
    OCT_DRAW_COMMAND_TYPE_TEXTURE = 1,    ///< Texture rendering
    OCT_DRAW_COMMAND_TYPE_SPRITE  = 2,    ///< Sprite rendering
    OCT_DRAW_COMMAND_TYPE_MODEL   = 3,    ///< Model rendering
    OCT_DRAW_COMMAND_TYPE_RECTANGLE  = 4, ///< Square rendering
    OCT_DRAW_COMMAND_TYPE_CIRCLE  = 5,    ///< Circle rendering
    OCT_DRAW_COMMAND_TYPE_LINE    = 6,    ///< Line rendering
    OCT_DRAW_COMMAND_TYPE_POLYGON = 7,    ///< Arbitrary polygon rendering
    OCT_DRAW_COMMAND_TYPE_FONT    = 8,    ///< Font rendering
    OCT_DRAW_COMMAND_TYPE_CAMERA  = 9,    ///< Some sort of camera update
    OCT_DRAW_COMMAND_TYPE_TARGET  = 10,   ///< Changing render target
} Oct_DrawCommandType;

/// \brief Types of load commands
typedef enum {
    OCT_LOAD_COMMAND_TYPE_NONE = 0,           ///< None
    OCT_LOAD_COMMAND_TYPE_LOAD_TEXTURE = 1,   ///< Load a texture
    OCT_LOAD_COMMAND_TYPE_LOAD_SPRITE = 2,    ///< Load a sprite
    OCT_LOAD_COMMAND_TYPE_LOAD_FONT = 3,      ///< Load a font
    OCT_LOAD_COMMAND_TYPE_LOAD_MODEL = 4,     ///< Loads a model
    OCT_LOAD_COMMAND_TYPE_FREE = 5,           ///< Frees an asset
    OCT_LOAD_COMMAND_TYPE_CREATE_SURFACE = 6, ///< Creates a surface
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
    OCT_AUDIO_COMMAND_TYPE_NONE = 0,         ///< None
    OCT_AUDIO_COMMAND_TYPE_PLAY_SOUND = 1,   ///< Resize the window
    OCT_AUDIO_COMMAND_TYPE_UPDATE_SOUND = 2, ///< Toggle fullscreen
} Oct_AudioCommandType;

/// \brief Types of meta commands
typedef enum {
    OCT_META_COMMAND_TYPE_NONE = 0,        ///< None
    OCT_META_COMMAND_TYPE_START_FRAME = 1, ///< Resize the window
    OCT_META_COMMAND_TYPE_END_FRAME = 2,   ///< Toggle fullscreen
} Oct_MetaCommandType;

/// \brief Types of allocators
typedef enum {
    OCT_ALLOCATOR_TYPE_NONE = 0,         ///< None
    OCT_ALLOCATOR_TYPE_HEAP = 1,         ///< General-purpose heap allocator
    OCT_ALLOCATOR_TYPE_ARENA = 2,        ///< Arena allocator
    OCT_ALLOCATOR_TYPE_VIRTUAL_PAGE = 3, ///< Virtual page allocator
} Oct_AllocatorType;

/// \brief Types of assets stored in an Oct_Asset - mostly internal use
typedef enum {
    OCT_ASSET_TYPE_NONE = 0,    ///< None
    OCT_ASSET_TYPE_TEXTURE = 1, ///< A texture
    OCT_ASSET_TYPE_MODEL = 2,   ///< Model
    OCT_ASSET_TYPE_FONT = 3,    ///< Font
    OCT_ASSET_TYPE_AUDIO = 4,   ///< Audio
    OCT_ASSET_TYPE_SPRITE = 5,  ///< Sprite
} Oct_AssetType;

/// \brief Things you can interpolate
typedef enum {
    OCT_INTERPOLATE_NONE = 0,         ///< Don't interpolate
    OCT_INTERPOLATE_POSITION = 1<<1,  ///< Interpolate position
    OCT_INTERPOLATE_ROTATION = 1<<2,  ///< Interpolate rotation
    OCT_INTERPOLATE_SCALE_X = 1<<3,   ///< Interpolate horizontal scale
    OCT_INTERPOLATE_SCALE_Y = 1<<4,   ///< Interpolate vertical scale
    OCT_INTERPOLATE_RADIUS = 1<<4,    ///< Interpolate circle radius
    OCT_INTERPOLATE_ALL = 0xFFFFFFFF, ///< Interpolate all of the above (where available)
} Oct_InterpolationType;

////////////////////// Hidden structs //////////////////////
OCT_OPAQUE_POINTER(Oct_Context)
OCT_OPAQUE_POINTER(Oct_Allocator)

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
    void *(*startup)(Oct_Context ctx);           ///< Function pointer to the startup function
    void *(*update)(Oct_Context ctx, void *ptr); ///< Function pointer to the update function
    void(*shutdown)(Oct_Context ctx, void *ptr); ///< Function pointer to the shutdown function
    void *pNext;                                 ///< For future use
};

/// \brief Window command to queue window events
struct Oct_WindowCommand_t {
    Oct_StructureType sType;    ///< Structure type
    Oct_WindowCommandType type; ///< Type of window command this is
    // TODO: This
    void *pNext;                ///< For future use
};

/// \brief Load command to load or free anything (just use oct_FreeAsset, don't use this manually for freeing)
struct Oct_LoadCommand_t {
    Oct_StructureType sType;  ///< Structure type
    Oct_LoadCommandType type; ///< Type of load command this is
    Oct_Asset _assetID;       ///< Internal use
    union {
        struct {
            const char *filename; ///< Filename of the texture to load (png, jpg, bmp)
            // TODO: Loading from binary
        } Texture;                ///< Information needed to load a texture
        struct {
            Oct_Vec2 dimensions; ///< Dimensions of the new surface
        } Surface;               ///< Information needed to create a surface
    };
    // TODO: This
    void *pNext; ///< For future use
};

/// \brief Audio command to draw anything from any thread
struct Oct_AudioCommand_t {
    Oct_StructureType sType;   ///< Structure type
    Oct_AudioCommandType type; ///< Type of load command this is
    // TODO: This
    void *pNext;               ///< For future use
};

/// \brief Meta commands, probably only internal use
struct Oct_MetaCommand_t {
    Oct_StructureType sType;  ///< Structure type
    Oct_MetaCommandType type; ///< Type of meta command this is
    void *pNext;              ///< For future use
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
    float r; ///< Red component
    float g; ///< Green component
    float b; ///< Blue component
    float a; ///< Alpha component
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
            Oct_Asset texture; ///< Texture that the render target will switch to (or OCT_TARGET_SWAPCHAIN)
        } Target;              ///< For changing render targets
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
        Oct_MetaCommand metaCommand;     ///< Meta command
    };
    void *pNext;                         ///< For future use
};

#ifdef __cplusplus
};
#endif