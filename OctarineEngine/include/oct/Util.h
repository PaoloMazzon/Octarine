/// \brief Various game utilities
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Thread safe, decently random, unsafe random function
OCTARINE_API float oct_Random(float min, float max);

#ifdef __cplusplus
};
#endif