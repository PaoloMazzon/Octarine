/// \brief Functions for handling drawing
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Draws debug text on screen with interpolation, format like you would printf
OCTARINE_API void oct_DrawDebugTextInt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Vec2 position, float scale, const char *fmt, ...);

/// \brief Draws debug text on screen, format like you would printf
OCTARINE_API void oct_DrawDebugText(Oct_Context ctx, Oct_Vec2 position, float scale, const char *fmt, ...);

#ifdef __cplusplus
};
#endif