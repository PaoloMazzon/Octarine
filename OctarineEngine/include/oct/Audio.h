/// \brief User-visible audio functions
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Loads audio into memory and returns the handle
OCTARINE_API Oct_Audio oct_LoadAudio(Oct_Context ctx, const char *filename);

/// \brief Plays an audio sample
OCTARINE_API Oct_Sound oct_PlaySound(Oct_Context ctx, Oct_Audio audio, Oct_Vec2 volume, Oct_Bool repeat);

/// \brief Returns true if a sound has stopped playing
OCTARINE_API Oct_Bool oct_SoundIsStopped(Oct_Context ctx, Oct_Sound sound);

/// \brief Updates an audio sample, if the sound has stopped playing nothing will happen
OCTARINE_API Oct_Sound oct_UpdateSound(Oct_Context ctx, Oct_Sound sound, Oct_Vec2 volume, Oct_Bool repeat);

#ifdef __cplusplus
};
#endif