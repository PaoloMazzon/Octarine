/// \brief Heap allocators available to the user
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Creates a general-purpose heap allocator
/// \return Returns a new heap, or NULL if it fails
/// \warning You may malloc from this heap on the thread that created it
OCTARINE_API Oct_Allocator oct_CreateHeapAllocator();

/// \brief Creates a new arena allocator, a fast allocator that may only be destroyed all at once
/// \param size Size of the arena
/// \return Returns the new arena allocator, or null if it fails
/// \warning Arena allocations may be made from any thread but it is not thread safe
/// to do so from multiple different threads concurrently.
OCTARINE_API Oct_Allocator oct_CreateArenaAllocator(int32_t size);

/// \brief Returns the type of allocator this is
/// \param allocator Allocator to check
/// \return Returns one of the OCT_ALLOCATOR_TYPE_* enums
OCTARINE_API Oct_AllocatorType oct_GetAllocatorType(Oct_Allocator allocator);

/// \brief Destroys a heap allocator and everything in it
/// \param allocator Allocator to destroy
OCTARINE_API void oct_FreeAllocator(Oct_Allocator allocator);

/// \brief Allocator equivalent of malloc
/// \param allocator Allocator to use
/// \param size Size of the allocation
/// \return Returns the new allocation, or null if it fails
///
/// This is supported by all allocator types.
OCTARINE_API void *oct_Malloc(Oct_Allocator allocator, int32_t size);

/// \brief Allocator equivalent of realloc
/// \param allocator Allocator to use
/// \param memory Original allocation
/// \param size Size of the new allocation
/// \return Returns the new allocation, or null if it fails
/// \warning Only heap allocators may use this.
OCTARINE_API void *oct_Realloc(Oct_Allocator allocator, void *memory, int32_t size);

/// \brief Same as oct_HeapAllocatorMalloc, but zeros the data
/// \param allocator Allocator to allocate from
/// \param size Size of the allocation
/// \return Returns the new allocation, or null if it fails
///
/// This is supported by all allocator types.
OCTARINE_API void *oct_Zalloc(Oct_Allocator allocator, int32_t size);

/// \brief Frees an allocation out of a heap allocator
/// \param allocator Allocator the memory came from
/// \param memory Memory to free
/// \warning Only heap allocators may free individual allocations.
OCTARINE_API void oct_Free(Oct_Allocator allocator, void *memory);

#ifdef __cplusplus
};
#endif
