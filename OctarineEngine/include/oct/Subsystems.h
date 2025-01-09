/// \brief Various internal subsystem functions (so subsystem functions aren't visible to the user)
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void _oct_ValidationInit(Oct_Context ctx);
void _oct_ValidationEnd(Oct_Context ctx);

void _oct_DrawingInit(Oct_Context ctx);
void _oct_DrawingUpdate(Oct_Context ctx);
void _oct_DrawingEnd(Oct_Context ctx);

void _oct_WindowInit(Oct_Context ctx);
void _oct_WindowUpdate(Oct_Context ctx);
void _oct_WindowEnd(Oct_Context ctx);

void _oct_CommandBufferInit(Oct_Context ctx);
void _oct_CommandBufferBeginFrame(Oct_Context ctx);
void _oct_CommandBufferEndFrame(Oct_Context ctx);
void _oct_CommandBufferEnd(Oct_Context ctx);

#ifdef __cplusplus
};
#endif