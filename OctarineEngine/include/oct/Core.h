/// \brief Core engine functionality
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Initializes the engine
/// \param initInfo Info needed to initialize
OCTARINE_API Oct_Status oct_Init(Oct_InitInfo *initInfo);

/// \brief Returns the framerate of the render thread
OCTARINE_API double oct_GetRenderFPS(Oct_Context ctx);

/// \brief Returns the average refresh rate of the logic thread
OCTARINE_API double oct_GetLogicHz(Oct_Context ctx);

/// \brief Returns the time since the game started in seconds
OCTARINE_API double oct_Time(Oct_Context ctx);

#ifdef __cplusplus
};
#endif