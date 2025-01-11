/// \brief Various internal subsystem functions (so subsystem functions aren't visible to the user)
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void _oct_ValidationInit(Oct_Context ctx);
void _oct_ValidationEnd(Oct_Context ctx);

// The command buffer subsystem is effectively a ring buffer that allows lockless command communication between
// the logic and render thread. The logic thread begins every frame with a "begin frame" command and ends that
// frame with an "end frame" command. Every command between is processed by its respective subsystem.
void _oct_CommandBufferInit(Oct_Context ctx);
void _oct_CommandBufferBeginFrame(Oct_Context ctx); // This is for the logical thread
void _oct_CommandBufferEndFrame(Oct_Context ctx); // This is for the logical thread
void _oct_CommandBufferEnd(Oct_Context ctx);
void _oct_CommandBufferDispatch(Oct_Context ctx); // Handles all currently available commands in the buffer

// Drawing subsystem is responsible for all rendering, its basically a wrapper over VK2D. Drawing commands are
// different because they are not immediately processed, they are stored in a triple buffer by the drawing subsystem
// so it can put the current frame's commands in an intermediate buffer, and interpolate the previous two frames worth
// of drawing to screen. The other subsystems do not interpolate data like this and will just process commands as they
// are received.
void _oct_DrawingInit(Oct_Context ctx);
void _oct_DrawingUpdateBegin(Oct_Context ctx);
void _oct_DrawingUpdateEnd(Oct_Context ctx);
void _oct_DrawingProcessCommand(Oct_Context ctx, Oct_Command *cmd);
void _oct_DrawingEnd(Oct_Context ctx);

// Window subsystem manages window events like resizing and input
void _oct_WindowInit(Oct_Context ctx);
void _oct_WindowUpdateBegin(Oct_Context ctx);
void _oct_WindowUpdateEnd(Oct_Context ctx);
void _oct_WindowProcessCommand(Oct_Context ctx, Oct_Command *cmd);
void _oct_WindowEnd(Oct_Context ctx);

// Audio handles audio as you might guess
void _oct_AudioInit(Oct_Context ctx);
void _oct_AudioUpdateBegin(Oct_Context ctx);
void _oct_AudioUpdateEnd(Oct_Context ctx);
void _oct_AudioProcessCommand(Oct_Context ctx, Oct_Command *cmd);
void _oct_AudioEnd(Oct_Context ctx);

// Handles loading/unloading assets
void _oct_AssetsInit(Oct_Context ctx);
void _oct_AssetsProcessCommand(Oct_Context ctx, Oct_Command *cmd);
Oct_Asset *_oct_AssetGet(Oct_Context ctx, Oct_Asset asset);
void _oct_AssetsEnd(Oct_Context ctx);

#ifdef __cplusplus
};
#endif