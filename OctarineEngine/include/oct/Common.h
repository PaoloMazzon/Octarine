/// \brief Core engine functionality
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////// Defines //////////////////////

// For opaque pointer types
#define OCT_OPAQUE_POINTER(type) typedef struct type##_t *type;

// For user-modifiable and user-visible structures
#define OCT_USER_STRUCT(type) typedef struct type##_t type;

// Grabs the structure type of a structure with an sType field (ie, `OCT_STRUCTURE_TYPE(&my_Oct_InitInfo)` -> `OCT_STRUCTURE_TYPE_INIT_INFO`)
#define OCT_STRUCTURE_TYPE(structure) (*((Oct_StructureType*)structure))

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
typedef uint64_t Oct_Status; ///< Status code
typedef uint32_t Oct_Bool;   ///< Internal bool type
typedef uint64_t Oct_Asset;  ///< Any asset in the engine

////////////////////// Enums //////////////////////
/// \brief Structure types
typedef enum {
    OCT_STRUCTURE_TYPE_NONE = 0,
    OCT_STRUCTURE_TYPE_INIT_INFO = 1,
    OCT_STRUCTURE_TYPE_DRAW_COMMAND = 2,
    OCT_STRUCTURE_TYPE_WINDOW_COMMAND = 3,
    OCT_STRUCTURE_TYPE_LOAD_COMMAND = 4,
    OCT_STRUCTURE_TYPE_COMMAND = 5,
} Oct_StructureType;

/// \brief Types of draw commands
typedef enum {
    OCT_DRAW_COMMAND_TYPE_NONE    = 0, ///< None
    OCT_DRAW_COMMAND_TYPE_TEXTURE = 1, ///< Texture rendering
    OCT_DRAW_COMMAND_TYPE_SPRITE  = 2, ///< Sprite rendering
    OCT_DRAW_COMMAND_TYPE_MODEL   = 3, ///< Model rendering
    OCT_DRAW_COMMAND_TYPE_SQUARE  = 4, ///< Square rendering
    OCT_DRAW_COMMAND_TYPE_CIRCLE  = 5, ///< Circle rendering
    OCT_DRAW_COMMAND_TYPE_LINE    = 6, ///< Line rendering
    OCT_DRAW_COMMAND_TYPE_POLYGON = 7, ///< Arbitrary polygon rendering
    OCT_DRAW_COMMAND_TYPE_FONT    = 8, ///< Font rendering
    OCT_DRAW_COMMAND_TYPE_CAMERA  = 9, ///< Some sort of camera update
} Oct_DrawCommandType;

/// \brief Types of load commands
typedef enum {
    OCT_LOAD_COMMAND_TYPE_NONE = 0,    ///< None
    OCT_LOAD_COMMAND_TYPE_TEXTURE = 1, ///< Load a texture
    OCT_LOAD_COMMAND_TYPE_SPRITE = 2,  ///< Load a sprite
    OCT_LOAD_COMMAND_TYPE_FONT = 3,    ///< Load a font
    OCT_LOAD_COMMAND_TYPE_MODEL = 4,   ///< Loads a model
} Oct_LoadCommandType;

/// \brief Types of window commands
typedef enum {
    OCT_WINDOW_COMMAND_TYPE_NONE = 0,              ///< None
    OCT_WINDOW_COMMAND_TYPE_RESIZE = 1,            ///< Resize the window
    OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_TOGGLE = 2, ///< Toggle fullscreen
    OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_EXIT = 3,   ///< Exit fullscreen (does nothing if windowed)
    OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_ENTER = 4,  ///< Enter fullscreen (does nothing if fullscreen)
} Oct_WindowCommandType;

/// \brief Types of meta commands
typedef enum {
    OCT_META_COMMAND_TYPE_NONE = 0,        ///< None
    OCT_META_COMMAND_TYPE_START_FRAME = 1, ///< Resize the window
    OCT_META_COMMAND_TYPE_END_FRAME = 2,   ///< Toggle fullscreen
} Oct_MetaCommandType;

////////////////////// Hidden structs //////////////////////
OCT_OPAQUE_POINTER(Oct_Context)
OCT_OPAQUE_POINTER(Oct_ArenaAllocator)
OCT_OPAQUE_POINTER(Oct_HeapAllocator)

////////////////////// Function parameters //////////////////////

/// \brief Parameters for oct_Init
struct Oct_InitInfo_t {
    Oct_StructureType sType;                     ///< Structure type
    const char *windowTitle;                     ///< Title of the window
    int windowWidth;                             ///< Window width
    int windowHeight;                            ///< Window height
    void *(*startup)(Oct_Context ctx);           ///< Function pointer to the startup function
    void *(*update)(Oct_Context ctx, void *ptr); ///< Function pointer to the update function
    void(*shutdown)(Oct_Context ctx, void *ptr); ///< Function pointer to the shutdown function
    void *pNext;                                 ///< For future use
};

////////////////////// Structs //////////////////////

/// \brief Draw command to draw anything from any thread
struct Oct_DrawCommand_t {
    Oct_StructureType sType;  ///< Structure type
    Oct_DrawCommandType type; ///< Type of draw command this is
    // TODO: This
    void *pNext;              ///< For future use
};

/// \brief Draw command to draw anything from any thread
struct Oct_WindowCommand_t {
    Oct_StructureType sType;    ///< Structure type
    Oct_WindowCommandType type; ///< Type of window command this is
    // TODO: This
    void *pNext;                ///< For future use
};

/// \brief Draw command to draw anything from any thread
struct Oct_LoadCommand_t {
    Oct_StructureType sType;  ///< Structure type
    Oct_LoadCommandType type; ///< Type of load command this is
    // TODO: This
    void *pNext;              ///< For future use
};

/// \brief Meta commands
struct Oct_MetaCommand_t {
    Oct_StructureType sType;  ///< Structure type
    Oct_MetaCommandType type; ///< Type of meta command this is
    void *pNext;              ///< For future use
};

////////////////////// User structs //////////////////////
OCT_USER_STRUCT(Oct_InitInfo)
OCT_USER_STRUCT(Oct_DrawCommand)
OCT_USER_STRUCT(Oct_Command)
OCT_USER_STRUCT(Oct_DrawCommand)
OCT_USER_STRUCT(Oct_WindowCommand)
OCT_USER_STRUCT(Oct_LoadCommand)
OCT_USER_STRUCT(Oct_MetaCommand)

/// \brief Any type of command the logic thread sends to the render thread
struct Oct_Command_t {
    Oct_StructureType sType;             ///< Structure type
    union {
        Oct_DrawCommand drawCommand;     ///< Draw command
        Oct_WindowCommand windowCommand; ///< Window command
        Oct_LoadCommand loadCommand;     ///< Load command
        Oct_MetaCommand metaCommand;     ///< Meta command
    } command;                           ///< The internal command
    void *pNext;                         ///< For future use
};

#ifdef __cplusplus
};
#endif