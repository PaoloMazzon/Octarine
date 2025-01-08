#include <mimalloc.h>
#include "oct/Opaque.h"
#include "oct/Allocators.h"

Oct_HeapAllocator oct_CreateHeapAllocator() {
    Oct_HeapAllocator gpa = mi_malloc(sizeof(struct Oct_HeapAllocator_t));
    if (gpa) {
        gpa->heap = mi_heap_new();
    }
    return gpa;
}

void oct_FreeHeapAllocator(Oct_HeapAllocator allocator) {
    if (allocator)
        mi_heap_delete(allocator->heap);
}

void *oct_HeapAllocatorMalloc(Oct_HeapAllocator allocator, int32_t size) {
    return mi_heap_malloc(allocator->heap, size);
}

void *oct_HeapAllocatorRealloc(Oct_HeapAllocator allocator, void *memory, int32_t size) {
    return mi_heap_realloc(allocator->heap, memory, size);
}

void *oct_HeapAllocatorZalloc(Oct_HeapAllocator allocator, int32_t size) {
    return mi_heap_zalloc(allocator->heap, size);
}

void oct_HeapAllocatorFree(Oct_HeapAllocator allocator, void *memory) {
    mi_free(memory); // not confident this works
}

Oct_ArenaAllocator oct_CreateArenaAllocator(int32_t size) {
    Oct_ArenaAllocator arena = mi_malloc(sizeof(struct Oct_ArenaAllocator_t));
    if (arena) {
        arena->buffer = mi_malloc(size);
        if (arena->buffer) {
            arena->size = size;
            arena->point = 0;
        } else {
            mi_free(arena);
            arena = null;
        }
    }
    return arena;
}

void oct_FreeArenaAllocator(Oct_ArenaAllocator allocator) {
    if (allocator) {
        mi_free(allocator->buffer);
        mi_free(allocator);
    }
}

void *oct_ArenaAllocatorMalloc(Oct_ArenaAllocator allocator, int32_t size) {
    void *out = null;
    if (allocator->point + size <= allocator->size) {
        out = &allocator->buffer[allocator->point];
        allocator->point += size;
    }
    return out;
}

void *oct_ArenaAllocatorZalloc(Oct_ArenaAllocator allocator, int32_t size) {
    uint8_t *out = null;
    if (allocator->point + size <= allocator->size) {
        out = &allocator->buffer[allocator->point];
        allocator->point += size;
        for (int i = 0; i < size; i++) {
            out[i] = 0;
        }
    }
    return out;
}
