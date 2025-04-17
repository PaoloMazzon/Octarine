#include <mimalloc.h>
#include "oct/Opaque.h"
#include "oct/Allocators.h"
#include "oct/Constants.h"

// Returns null if it fails, a new arena allocator otherwise
static Oct_Allocator _oct_AddExtraPage(Oct_Allocator allocator, int32_t size) {
    void *new = mi_realloc(
            allocator->virtualPageAllocator.pages,
            sizeof(Oct_Allocator) * allocator->virtualPageAllocator.count + 1);
    if (!new)
        return null;
    allocator->virtualPageAllocator.pages = new;
    allocator->virtualPageAllocator.pages[allocator->virtualPageAllocator.count] = oct_CreateArenaAllocator(size);
    allocator->virtualPageAllocator.count += 1;
    return allocator->virtualPageAllocator.pages[allocator->virtualPageAllocator.count - 1];
}

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

OCTARINE_API Oct_Allocator oct_CreateVirtualPageAllocator() {
    Oct_Allocator arena = mi_malloc(sizeof(struct Oct_Allocator_t));
    if (arena) {
        arena->type = OCT_ALLOCATOR_TYPE_VIRTUAL_PAGE;
        arena->virtualPageAllocator.count = 0;
        arena->virtualPageAllocator.pages = null;
    }
    return arena;
}

OCTARINE_API Oct_AllocatorType oct_GetAllocatorType(Oct_Allocator allocator) {
    return allocator->type;
}

OCTARINE_API void oct_ResetAllocator(Oct_Allocator allocator) {
    if (allocator) {
        if (allocator->type == OCT_ALLOCATOR_TYPE_HEAP) {
            mi_heap_delete(allocator->heapAllocator);
            allocator->heapAllocator = mi_heap_new();
        } else if (allocator->type == OCT_ALLOCATOR_TYPE_ARENA) {
            allocator->arenaAllocator.point = 0;
        } else if (allocator->type == OCT_ALLOCATOR_TYPE_VIRTUAL_PAGE) {
            for (int i = 0; i < allocator->virtualPageAllocator.count; i++)
                oct_ResetAllocator(allocator->virtualPageAllocator.pages[i]);
        }
    }
}

OCTARINE_API void oct_FreeAllocator(Oct_Allocator allocator) {
    if (allocator) {
        if (allocator->type == OCT_ALLOCATOR_TYPE_HEAP) {
            mi_heap_delete(allocator->heapAllocator);
        } else if (allocator->type == OCT_ALLOCATOR_TYPE_ARENA) {
            mi_free(allocator->arenaAllocator.buffer);
        } else if (allocator->type == OCT_ALLOCATOR_TYPE_VIRTUAL_PAGE) {
            for (int i = 0; i < allocator->virtualPageAllocator.count; i++)
                oct_FreeAllocator(allocator->virtualPageAllocator.pages[i]);
            mi_free(allocator->virtualPageAllocator.pages);
        }
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
    } else if (allocator->type == OCT_ALLOCATOR_TYPE_VIRTUAL_PAGE) {
        // Find a page with enough room
        void *mem = null;
        for (int i = 0; i < allocator->virtualPageAllocator.count; i++) {
            mem = oct_Malloc(allocator->virtualPageAllocator.pages[i], size);
            if (mem)
                return mem;
        }

        // There are no available pages
        Oct_Allocator alloc = _oct_AddExtraPage(
                allocator,
                size < OCT_STANDARD_PAGE_SIZE ? OCT_STANDARD_PAGE_SIZE : size * OCT_PAGE_SCALE_FACTOR);
        if (alloc)
            return oct_Malloc(alloc, size);
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
    } else if (allocator->type == OCT_ALLOCATOR_TYPE_VIRTUAL_PAGE) {
        // Find a page with enough room
        void *mem = null;
        for (int i = 0; i < allocator->virtualPageAllocator.count; i++) {
            mem = oct_Zalloc(allocator->virtualPageAllocator.pages[i], size);
            if (mem)
                return mem;
        }

        // There are no available pages
        Oct_Allocator alloc = _oct_AddExtraPage(
                allocator,
                size < OCT_STANDARD_PAGE_SIZE ? OCT_STANDARD_PAGE_SIZE : size * OCT_PAGE_SCALE_FACTOR);
        if (alloc)
            return oct_Zalloc(alloc, size);
    }
    return null;
}

OCTARINE_API void oct_Free(Oct_Allocator allocator, void *memory) {
    if (allocator->type == OCT_ALLOCATOR_TYPE_HEAP)
        mi_free(memory); // not confident this works
}
