/// \brief The logical thread used to process the user's functions
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Launches thread
void oct_Bootstrap(Oct_Context ctx);

// Quits the thread
void _oct_UnstrapBoots(Oct_Context ctx);

#ifdef __cplusplus
};
#endif