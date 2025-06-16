/// \brief Various game utilities
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Thread safe, decently random, unsafe random function
OCTARINE_API float oct_Random(float min, float max);

/// \brief Checks for a collision between two rectangles
OCTARINE_API Oct_Bool oct_AABB(Oct_Rectangle *r1, Oct_Rectangle *r2);

/// \brief Returns the distance between two points
OCTARINE_API float oct_PointDistance(Oct_Vec2 p1, Oct_Vec2 p2);

/// \brief Returns the angle between two points (in radians)
OCTARINE_API float oct_PointAngle(Oct_Vec2 p1, Oct_Vec2 p2);

/// \brief Linear interpolation
OCTARINE_API float oct_Lerp(float min, float max, float val);

/// \brief Linear interpolation, but over a sin function
OCTARINE_API float oct_Sirp(float min, float max, float val);

/// \brief Checks if a file exists
OCTARINE_API Oct_Bool oct_FileExists(const char *filename);

/// \brief Returns a binary buffer containing a specified filename, or null if it fails
OCTARINE_API void *oct_ReadFile(const char *filename, Oct_Allocator allocator, uint32_t *size);

/// \brief Dumps arbitrary data to a file, returns false if it fails
OCTARINE_API Oct_Bool oct_WriteFile(const char *filename, void *data, uint32_t size);

#ifdef __cplusplus
};
#endif