/// \brief
#pragma once
#include <oct/Octarine.h>

#ifdef __cplusplus
extern "C" {
#endif

void *startup();
void *update(void *ptr);
void shutdown(void *ptr);

#ifdef __cplusplus
};
#endif