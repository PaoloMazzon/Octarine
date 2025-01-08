#include "oct/Allocators.h"

Oct_HeapAllocator oct_CreateHeapAllocator() {
    return null; // TODO: This
}

void oct_FreeHeapAllocator(Oct_HeapAllocator *allocator) {
    // TODO: This
}

void *oct_HeapAllocatorMalloc(Oct_HeapAllocator *allocator, int32_t size) {
    return null; // TODO: This
}

void *oct_HeapAllocatorRealloc(Oct_HeapAllocator *allocator, void *memory, int32_t size) {
    return null; // TODO: This
}

void *oct_HeapAllocatorZalloc(Oct_HeapAllocator *allocator, int32_t size) {
    return null; // TODO: This
}

void oct_HeapAllocatorFree(Oct_HeapAllocator *allocator, void *memory) {
    // TODO: This
}

Oct_ArenaAllocator oct_CreateArenaAllocator(int32_t size) {
    return null; // TODO: This
}

void oct_FreeArenaAllocator(Oct_ArenaAllocator allocator) {
    // TODO: This
}

void *oct_ArenaAllocatorMalloc(Oct_ArenaAllocator *allocator, int32_t size) {
    return null; // TODO: This
}

void *oct_ArenaAllocatorZalloc(Oct_ArenaAllocator *allocator, int32_t size) {
    return null; // TODO: This
}
