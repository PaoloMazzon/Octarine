/// \brief User-visible audio functions
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Loads audio into memory and returns the handle
OCTARINE_API Oct_Audio oct_LoadAudio(Oct_Context ctx, const char *filename);

/// \brief Plays an audio sample
///
/// The returned sound handle is unique for each played sound and will never be reused (unless you play
/// and absolutely obscene amount of sounds). If you try to update a sound that has finished playing or
/// been stopped or anything like that, the audio subsystem will know the handle is invalid and ignore
/// the request.
OCTARINE_API Oct_Sound oct_PlaySound(Oct_Context ctx, Oct_Audio audio, Oct_Vec2 volume, Oct_Bool repeat);

/// \brief Returns true if a sound has stopped playing
OCTARINE_API Oct_Bool oct_SoundIsStopped(Oct_Context ctx, Oct_Sound sound);

/// \brief Stops a sound, if the sound has since stopped playing this does nothing
OCTARINE_API void oct_StopSound(Oct_Context ctx, Oct_Sound sound);

/// \brief Stops all sounds
OCTARINE_API void oct_StopAllSounds(Oct_Context ctx);

/// \brief Pauses all currently playing sounds
OCTARINE_API void oct_PauseAllSounds(Oct_Context ctx);

/// \brief Unpauses all sounds that are still alive and paused
OCTARINE_API void oct_UnpauseAllSounds(Oct_Context ctx);

/// \brief Updates an audio sample, if the sound has stopped playing nothing will happen
OCTARINE_API Oct_Sound oct_UpdateSound(Oct_Context ctx, Oct_Sound sound, Oct_Vec2 volume, Oct_Bool repeat, Oct_Bool paused);

#ifdef __cplusplus
};
#endif