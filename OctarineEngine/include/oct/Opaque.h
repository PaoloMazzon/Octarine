/// \brief Structs that are not visible to the engine user
#pragma once
#include <mimalloc.h>
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief General engine context
struct Oct_Context_t {
    int padding;
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