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
typedef uint64_t Oct_Status;
typedef uint32_t Oct_Bool;

////////////////////// Enums //////////////////////
/// \brief Structure types
typedef enum {
    OCT_STRUCTURE_TYPE_NONE = 0,
    OCT_STRUCTURE_TYPE_INIT_INFO = 1,
} OctStructureType;

////////////////////// Hidden structs //////////////////////
OCT_OPAQUE_POINTER(Oct_Context)
OCT_OPAQUE_POINTER(Oct_ArenaAllocator)
OCT_OPAQUE_POINTER(Oct_HeapAllocator)

////////////////////// Function parameters //////////////////////

/// \brief Parameters for oct_Init
struct Oct_InitInfo_t {
    OctStructureType sType;                      ///< Structure type
    const char *windowTitle;                     ///< Title of the window
    int windowWidth;                             ///< Window width
    int windowHeight;                            ///< Window height
    void *(*startup)(Oct_Context ctx);           ///< Function pointer to the startup function
    void *(*update)(Oct_Context ctx, void *ptr); ///< Function pointer to the update function
    void(*shutdown)(Oct_Context ctx, void *ptr); ///< Function pointer to the shutdown function
    void *pNext;                                 ///< For future use
};

////////////////////// User structs //////////////////////
OCT_USER_STRUCT(Oct_InitInfo)

#ifdef __cplusplus
};
#endif