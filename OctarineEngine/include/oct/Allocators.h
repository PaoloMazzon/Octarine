/// \brief Heap allocators available to the user
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Creates a general-purpose heap allocator
/// \return Returns a new heap, or NULL if it fails
Oct_HeapAllocator oct_CreateHeapAllocator();

/// \brief Destroys a heap allocator and everything in it
/// \param allocator Allocator to destroy
void oct_FreeHeapAllocator(Oct_HeapAllocator *allocator);

/// \brief Mallocs out of a heap allocator
/// \param allocator Allocator to use
/// \param size Size of the allocation
/// \return Returns the new allocation, or null if it fails
void *oct_HeapAllocatorMalloc(Oct_HeapAllocator *allocator, int32_t size);

/// \brief Reallocs out of a heap allocator
/// \param allocator Allocator to use
/// \param memory Original allocation
/// \param size Size of the new allocation
/// \return Returns the new allocation, or null if it fails
void *oct_HeapAllocatorRealloc(Oct_HeapAllocator *allocator, void *memory, int32_t size);

/// \brief Same as oct_HeapAllocatorMalloc, but zeros the data
/// \param allocator Allocator to allocate from
/// \param size Size of the allocation
/// \return Returns the new allocation, or null if it fails
void *oct_HeapAllocatorZalloc(Oct_HeapAllocator *allocator, int32_t size);

/// \brief Frees an allocation out of a heap allocator
/// \param allocator Allocator the memory came from
/// \param memory Memory to free
void oct_HeapAllocatorFree(Oct_HeapAllocator *allocator, void *memory);

/// \brief Creates a new arena allocator, a fast allocator that may only be destroyed all at once
/// \param size Size of the arena
/// \return Returns the new arena allocator, or null if it fails
Oct_ArenaAllocator oct_CreateArenaAllocator(int32_t size);

/// \brief Destroys an arena allocator and everything in it
/// \param allocator Arena to destroy
void oct_FreeArenaAllocator(Oct_ArenaAllocator allocator);

/// \brief Mallocs out of an arena
/// \param allocator Arena to allocate from
/// \param size Size to allocate
/// \return Returns the allocation, or null if there is not enough memory left in the arena
void *oct_ArenaAllocatorMalloc(Oct_ArenaAllocator *allocator, int32_t size);

/// \brief Same as oct_ArenaAllocatorMalloc, but zeroes the data
/// \param allocator Arena to allocate from
/// \param size Size to allocate
/// \return Returns the allocation, or null if there is not enough memory left in the arena
void *oct_ArenaAllocatorZalloc(Oct_ArenaAllocator *allocator, int32_t size);

#ifdef __cplusplus
};
#endif