/// \brief The logical thread used to process the user's functions
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Launches thread
void oct_Bootstrap();

// Quits the thread
void _oct_UnstrapBoots();

#ifdef __cplusplus
};
#endif