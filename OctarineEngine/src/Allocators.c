#include <mimalloc.h>
#include "oct/Opaque.h"
#include "oct/Allocators.h"

OCTARINE_API Oct_Allocator oct_CreateHeapAllocator() {
    Oct_Allocator gpa = mi_malloc(sizeof(struct Oct_Allocator_t));
    if (gpa) {
        gpa->type = OCT_ALLOCATOR_TYPE_HEAP;
        gpa->heapAllocator = mi_heap_new();
    }
    return gpa;
}

OCTARINE_API Oct_Allocator oct_CreateArenaAllocator(int32_t size) {
    Oct_Allocator arena = mi_malloc(sizeof(struct Oct_Allocator_t));
    if (arena) {
        arena->type = OCT_ALLOCATOR_TYPE_ARENA;
        arena->arenaAllocator.buffer = mi_malloc(size);
        if (arena->arenaAllocator.buffer) {
            arena->arenaAllocator.size = size;
            arena->arenaAllocator.point = 0;
        } else {
            mi_free(arena);
            arena = null;
        }
    }
    return arena;
}

OCTARINE_API Oct_AllocatorType oct_GetAllocatorType(Oct_Allocator allocator) {
    return allocator->type;
}

OCTARINE_API void oct_FreeAllocator(Oct_Allocator allocator) {
    if (allocator) {
        if (allocator->type == OCT_ALLOCATOR_TYPE_HEAP)
            mi_heap_delete(allocator->heapAllocator);
        else if (allocator->type == OCT_ALLOCATOR_TYPE_ARENA)
            mi_free(allocator->arenaAllocator.buffer);
        mi_free(allocator);
    }
}

OCTARINE_API void *oct_Malloc(Oct_Allocator allocator, int32_t size) {
    if (allocator->type == OCT_ALLOCATOR_TYPE_HEAP) {
        return mi_heap_malloc(allocator->heapAllocator, size);
    } else if (allocator->type == OCT_ALLOCATOR_TYPE_ARENA) {
        if (allocator->arenaAllocator.point + size <= allocator->arenaAllocator.size) {
            void *out = &allocator->arenaAllocator.buffer[allocator->arenaAllocator.point];
            allocator->arenaAllocator.point += size;
            return out;
        }
    }
    return null;
}

OCTARINE_API void *oct_Realloc(Oct_Allocator allocator, void *memory, int32_t size) {
    if (allocator->type == OCT_ALLOCATOR_TYPE_HEAP)
        return mi_heap_realloc(allocator->heapAllocator, memory, size);
    return NULL;
}

OCTARINE_API void *oct_Zalloc(Oct_Allocator allocator, int32_t size) {
    if (allocator->type == OCT_ALLOCATOR_TYPE_HEAP) {
        return mi_heap_zalloc(allocator->heapAllocator, size);
    } else if (allocator->type == OCT_ALLOCATOR_TYPE_ARENA) {
        if (allocator->arenaAllocator.point + size <= allocator->arenaAllocator.size) {
            uint8_t *out = &allocator->arenaAllocator.buffer[allocator->arenaAllocator.point];
            allocator->arenaAllocator.point += size;
            for (int i = 0; i < size; i++) {
                out[i] = 0;
            }
            return out;
        }
    }
}

OCTARINE_API void oct_Free(Oct_Allocator allocator, void *memory) {
    if (allocator->type == OCT_ALLOCATOR_TYPE_HEAP)
        mi_free(memory); // not confident this works
}
