/// \brief Core engine functionality
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Initializes the engine
/// \param initInfo Info needed to initialize
OCTARINE_API Oct_Status oct_Init(Oct_InitInfo *initInfo);

#ifdef __cplusplus
};
#endif