/// \brief Various internal subsystem functions (so subsystem functions aren't visible to the user)
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

void _oct_ValidationInit();
void _oct_ValidationEnd();
Oct_Context _oct_GetCtx();

// The command buffer subsystem is effectively a ring buffer that allows lockless command communication between
// the logic and render thread. The logic thread begins every frame with a "begin frame" command and ends that
// frame with an "end frame" command. Every command between is processed by its respective subsystem.
void _oct_CommandBufferInit();
void _oct_CommandBufferBeginFrame(); // This is for the logical thread
void _oct_CommandBufferEndFrame(); // This is for the logical thread
void _oct_CommandBufferEnd();
void _oct_CommandBufferDispatch(); // Handles all currently available commands in the buffer
void *_oct_CopyIntoFrameMemory(void *data, int32_t size);
void *_oct_GetFrameMemory(int32_t size);

// Drawing subsystem is responsible for all rendering, its basically a wrapper over VK2D. Drawing commands are
// different because they are not immediately processed, they are stored in a triple buffer by the drawing subsystem
// so it can put the current frame's commands in an intermediate buffer, and interpolate the previous two frames worth
// of drawing to screen. The other subsystems do not interpolate data like this and will just process commands as they
// are received.
void _oct_DrawingInit();
void _oct_DrawingUpdateBegin();
void _oct_DrawingUpdateEnd();
void _oct_DrawingProcessCommand(Oct_Command *cmd);
void _oct_DrawingEnd();

// Window subsystem manages window events like resizing and input
struct Oct_WindowEvent_t;
typedef struct Oct_WindowEvent_t Oct_WindowEvent;
void _oct_WindowInit();
void _oct_WindowUpdateBegin();
void _oct_WindowUpdateEnd();
void _oct_WindowProcessCommand(Oct_Command *cmd);
Oct_Bool _oct_WindowPopEvent(Oct_WindowEvent *event); // Used from the logic thread to pull key events, returns false if there are no more events (event is not valid in this case)
void _oct_WindowEnd();

// Audio handles audio as you might guess
void _oct_AudioInit();
void _oct_AudioUpdateBegin();
void _oct_AudioUpdateEnd();
void _oct_AudioProcessCommand(Oct_Command *cmd);
Oct_Sound _oct_ReserveSound(); // used from logic thread to reserve a space in the sound list
uint8_t *_oct_AudioConvertFormat(uint8_t *data, int32_t size, int32_t *newSize, SDL_AudioSpec *spec); // Converts audio to the necessary format and returns it, use SDL_free on the output
void _oct_AudioEnd();

// Handles loading/unloading assets
struct Oct_AssetData_t;
typedef struct Oct_AssetData_t Oct_AssetData;
void _oct_AssetsInit();
void _oct_AssetsProcessCommand(Oct_Command *cmd);
Oct_AssetType _oct_AssetType(Oct_Asset asset);
int _oct_AssetGeneration(Oct_Asset asset);
const char *_oct_AssetTypeString(Oct_Asset asset);
Oct_AssetData *_oct_AssetGet(Oct_Asset asset);
Oct_AssetData *_oct_AssetGetSafe(Oct_Asset asset, Oct_AssetType type); // returns null if the type is wrong, generation is wrong, or the asset isn't loaded yet
Oct_Asset _oct_AssetReserveSpace(); // used from logic thread to reserve a space in the asset list
void _oct_PlaceAssetInBucket(Oct_AssetBundle bundle, Oct_Asset asset, const char *name); // name will be copied
void _oct_AssetsEnd();

// Handles input processing on the logical thread
void _oct_InputInit();
void _oct_InputUpdate();
void _oct_InputEnd();

#ifdef __cplusplus
};
#endif