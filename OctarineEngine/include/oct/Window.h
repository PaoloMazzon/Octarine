/// \brief Handles windowing things
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Returns the width of the window
/// \param ctx Octarine context
/// \return Returns the width of the window
OCTARINE_API float oct_WindowWidth();

/// \brief Returns the height of the window
/// \param ctx Octarine context
/// \return Returns the height of the window
OCTARINE_API float oct_WindowHeight();

/// \brief Returns whether or not the window is currently fullscreen
/// \param ctx Octarine context
/// \return Returns whether or not the window is currently fullscreen
OCTARINE_API Oct_Bool oct_WindowIsFullscreen();

/// \brief Sets the window fullscreen
/// \param ctx Octarine context
/// \param fullscreen True for borderless fullscreen, false otherwise
OCTARINE_API void oct_SetFullscreen(Oct_Bool fullscreen);

/// \brief Toggles fullscreen mode
/// \param ctx Octarine context
OCTARINE_API void oct_ToggleFullscreen();

/// \brief Resizes the window
/// \param ctx Octarine context
/// \param width Width of the window
/// \param height Height of the window
OCTARINE_API void oct_ResizeWindow(float width, float height);

#ifdef __cplusplus
};
#endif