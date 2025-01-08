/// \brief
#pragma once
#include <oct/Octarine.h>

#ifdef __cplusplus
extern "C" {
#endif

void *startup(Oct_Context ctx);
void *update(Oct_Context ctx, void *ptr);
void shutdown(Oct_Context ctx, void *ptr);

#ifdef __cplusplus
};
#endif