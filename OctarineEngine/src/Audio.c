#include <SDL3/SDL.h>
#include "oct/Common.h"
#include "oct/Validation.h"
#include "oct/CommandBuffer.h"
#include "oct/Opaque.h"
#include "oct/Subsystems.h"

#define SOUND_INDEX(sound) (sound & UINT32_MAX)
#define SOUND_GENERATION(sound) ((uint32_t)(sound >> 32))

/////////////////////////// STRUCTS ///////////////////////////
/// \brief Info needed for the mixer to mix any particular sound
typedef struct Oct_PlayingSound_t {
    Oct_Audio sound;           ///< Actual sound asset being played
    int32_t pointer;           ///< Pointer (in samples) to where in the sound buffer the sound will be played from
    SDL_AtomicInt volumeLeft;  ///< Left volume from 0-AUDIO_VOLUME_NORMALIZED_FACTOR
    SDL_AtomicInt volumeRight; ///< Right volume from 0-AUDIO_VOLUME_NORMALIZED_FACTOR
    SDL_AtomicInt paused;      ///< Whether or not the sound is paused
    SDL_AtomicInt repeat;      ///< Whether or not the sound will repeat infinitely
    SDL_AtomicInt generation;  ///< Generation so the user gets a unique playing sound each time they play new sounds (least significant 32 bits are reserved for the index)
    SDL_AtomicInt alive;       ///< Whether or not this sound is currently valid
    SDL_AtomicInt reserved;    ///< Whether or not this sound is reserved from the logic thread
} Oct_PlayingSound;

/////////////////////////// GLOBALS ///////////////////////////
static const int AUDIO_FREQUENCY_HZ = 44100;
static const double AUDIO_REFRESH_RATE_HZ = 100;
static const double AUDIO_VOLUME_NORMALIZED_FACTOR = 10000;
static const int32_t AUDIO_SAMPLE_SIZE = 4;
static const int32_t AUDIO_CHANNELS = 2;
static SDL_Thread *gMixerThread;
static SDL_AudioSpec gDeviceSpec = {
        .channels = AUDIO_CHANNELS,
        .format = SDL_AUDIO_F32,
        .freq = AUDIO_FREQUENCY_HZ
};
#define MAX_PLAYING_SOUNDS 100
static Oct_PlayingSound gPlayingSounds[MAX_PLAYING_SOUNDS];

/////////////////////////// FUNCTIONS ///////////////////////////
// Reserves space in the playing sound list, returning OCT_SOUND_FAILED if it fails (too many concurrent noises)
Oct_Sound _oct_ReserveSound(Oct_Context ctx) {
    for (int i = 0; i < MAX_PLAYING_SOUNDS; i++)
        if (SDL_CompareAndSwapAtomicInt(&gPlayingSounds[i].reserved, 0, 1))
            return i + (((uint64_t)SDL_GetAtomicInt(&gPlayingSounds[i].generation)) << 32);
    return OCT_SOUND_FAILED;
}

static void _oct_KillSound(Oct_Sound snd) {
    SDL_SetAtomicInt(&gPlayingSounds[SOUND_INDEX(snd)].alive, 0);
    SDL_AddAtomicInt(&gPlayingSounds[SOUND_INDEX(snd)].generation, 1);
    SDL_SetAtomicInt(&gPlayingSounds[SOUND_INDEX(snd)].reserved, 0);
}

// Returns the number of seconds from start, which should be a SDL_GetPerformanceCounter call
inline static double _oct_GoofyTime(uint64_t start) {
    const double current = SDL_GetPerformanceCounter();
    const double freq = SDL_GetPerformanceFrequency();
    return (current - start) / freq;
}

// Adds a certain number of samples from a playing sound to an audio sample, dealing with the playing sound should
// it run out of new audio
inline static void _oct_AddPlayingSound(Oct_Context ctx, float *buffer, int32_t samples, int32_t playingSound) {
    Oct_PlayingSound *snd = &gPlayingSounds[playingSound];
    Oct_AssetData *data = _oct_AssetGet(ctx, snd->sound);
    float *soundBuffer = (void*)data->audio.data; // the audio that is queued
    Oct_Vec2 vol = {
            (float)SDL_GetAtomicInt(&snd->volumeLeft) / AUDIO_VOLUME_NORMALIZED_FACTOR,
            (float)SDL_GetAtomicInt(&snd->volumeRight) / AUDIO_VOLUME_NORMALIZED_FACTOR
    };
    for (int i = 0; i < samples; i++) {
        // Add sample to the mix
        buffer[i] += soundBuffer[snd->pointer] * vol[i % 2];

        // Remove this sound if the audio sample is done
        if (++snd->pointer >= data->audio.size / (AUDIO_SAMPLE_SIZE)) {
            if (!SDL_GetAtomicInt(&snd->repeat)) {
                _oct_KillSound(playingSound);
                break;
            } else {
                snd->pointer = 0;
            }
        }
    }
}

/*
 * The mixer works by first checking if the audio stream has less data available than would be needed by the next
 * cycle. So if each audio cycle is 10ms and only 5ms of audio data is present, it will load an additional 10ms of
 * audio data into the stream. To find audio data, it iterates each sound in the gPlayingSounds list, and any sounds
 * that are currently unpaused and alive will be mixed together.
 */
static int _oct_MixerThread(void *data) {
    Oct_Context ctx = data;
    const uint64_t start = SDL_GetPerformanceCounter();
    double lastTime = _oct_GoofyTime(start);
    double startTime = lastTime;
    double iterations = 0;

    // Create audio device and stream
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &gDeviceSpec);
    SDL_AudioStream *audioStream = SDL_CreateAudioStream(&gDeviceSpec, &gDeviceSpec);
    if (audioDevice == 0 || audioStream == null || !SDL_BindAudioStream(audioDevice, audioStream)) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to initialize audio device, SDL error: %s", SDL_GetError());
    }
    SDL_ResumeAudioDevice(audioDevice);
    oct_Log("Created audio device and thread using \"%s\"", SDL_GetAudioDeviceName(audioDevice));

    // Amount of samples that would be played during 1 tick of this update thing
    const int32_t UPDATE_SAMPLES = ((AUDIO_FREQUENCY_HZ / AUDIO_REFRESH_RATE_HZ) + 1) * AUDIO_CHANNELS;

    // Buffer we will mix sound into, must be 1 refresh rate worth of sound long
    float *writeBuffer = mi_malloc(UPDATE_SAMPLES * AUDIO_SAMPLE_SIZE);

    while (!SDL_GetAtomicInt(&ctx->quit)) {
        // Mixer
        if (SDL_GetAudioStreamQueued(audioStream) < UPDATE_SAMPLES * AUDIO_SAMPLE_SIZE) {
            // Zero the current mix
            memset(writeBuffer, 0, UPDATE_SAMPLES * AUDIO_SAMPLE_SIZE);

            // Loop through each sound and add the ones that are currently playing
            for (int i = 0; i < MAX_PLAYING_SOUNDS; i++) {
                if (SDL_GetAtomicInt(&gPlayingSounds[i].alive) && !SDL_GetAtomicInt(&gPlayingSounds[i].paused)) {
                    _oct_AddPlayingSound(ctx, writeBuffer, UPDATE_SAMPLES, i);
                }
            }

            // Add the new mix to the audio stream
            SDL_PutAudioStreamData(audioStream, writeBuffer, UPDATE_SAMPLES * AUDIO_SAMPLE_SIZE);
        }

        // Wait to make sure we refresh at a certain frequency
        const double current = _oct_GoofyTime(start);
        if (current - lastTime < 1.0 / AUDIO_REFRESH_RATE_HZ) {
            SDL_DelayPrecise((uint64_t)(((1.0 / AUDIO_REFRESH_RATE_HZ) * 10e8) - ((current - lastTime) * 10e8)));
        }
        iterations += 1;
        lastTime = _oct_GoofyTime(start);
    }

    // Print out average audio refresh rate for debug purposes
    if (ctx->initInfo->debug) {
        oct_Log("Average audio refresh rate: %.2fHz", iterations / (_oct_GoofyTime(start) - startTime));
    }

    // Cleanup
    mi_free(writeBuffer);
    SDL_DestroyAudioStream(audioStream);
    SDL_CloseAudioDevice(audioDevice);

    return 0;
}

void _oct_AudioInit(Oct_Context ctx) {
    // Create the mixer thread, the mixer thread does all the actual work
    gMixerThread = SDL_CreateThread(_oct_MixerThread, "Audio Mixer", ctx);
    if (!gMixerThread) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create mixer thread, SDL error \"%s\"", SDL_GetError());
    }
}

void _oct_AudioUpdateBegin(Oct_Context ctx) {
    // All the work is done in the mixer thread
}

void _oct_AudioUpdateEnd(Oct_Context ctx) {
    // All the work is done in the mixer thread
}

void _oct_AudioProcessCommand(Oct_Context ctx, Oct_Command *cmd) {
    if (OCT_STRUCTURE_TYPE(&cmd->topOfUnion) == OCT_STRUCTURE_TYPE_META_COMMAND) {
        // Meta commands currently do not impact the audio subsystem
    } else {
        // TODO: Move this all to the logic thread cuz this can cause a weird race condition
        //       where the mixer thread can theoretically end a sound then the logic thread
        //       reserve that slot while the render thread updates the details of a sound that
        //       no longer exists.
        Oct_AudioCommand *audio = &cmd->audioCommand;
        if (audio->type == OCT_AUDIO_COMMAND_TYPE_PLAY_SOUND) {
            int32_t index = SOUND_INDEX(audio->Play._soundID);
            if (index < MAX_PLAYING_SOUNDS) {
                gPlayingSounds[index].sound = audio->Play.audio;
                gPlayingSounds[index].pointer = 0;
                SDL_SetAtomicInt(&gPlayingSounds[index].repeat, audio->Play.repeat);
                SDL_SetAtomicInt(&gPlayingSounds[index].volumeLeft, audio->Play.volume[0] * AUDIO_VOLUME_NORMALIZED_FACTOR);
                SDL_SetAtomicInt(&gPlayingSounds[index].volumeRight, audio->Play.volume[1] * AUDIO_VOLUME_NORMALIZED_FACTOR);
                SDL_SetAtomicInt(&gPlayingSounds[index].paused, 0);
                SDL_SetAtomicInt(&gPlayingSounds[index].alive, 1);
            }
        } else if (audio->type == OCT_AUDIO_COMMAND_TYPE_UPDATE_SOUND) {
            int32_t index = SOUND_INDEX(audio->Update.sound);
            if (index < MAX_PLAYING_SOUNDS && SOUND_GENERATION(audio->Update.sound) == SDL_GetAtomicInt(&gPlayingSounds[index].generation)) {
                if (!audio->Update.stop) {
                    SDL_SetAtomicInt(&gPlayingSounds[index].repeat, audio->Update.repeat);
                    SDL_SetAtomicInt(&gPlayingSounds[index].volumeLeft,audio->Update.volume[0] * AUDIO_VOLUME_NORMALIZED_FACTOR);
                    SDL_SetAtomicInt(&gPlayingSounds[index].volumeRight,audio->Update.volume[1] * AUDIO_VOLUME_NORMALIZED_FACTOR);
                } else {
                    _oct_KillSound(index);
                }
            }
        } else if (audio->type == OCT_AUDIO_COMMAND_TYPE_STOP_ALL_SOUNDS) {
            for (int i = 0; i < MAX_PLAYING_SOUNDS; i++) {
                if (SDL_GetAtomicInt(&gPlayingSounds[i].alive)) {
                    _oct_KillSound(i);
                }
            }
        } else if (audio->type == OCT_AUDIO_COMMAND_TYPE_PAUSE_ALL_SOUNDS) {
            for (int i = 0; i < MAX_PLAYING_SOUNDS; i++)
                SDL_SetAtomicInt(&gPlayingSounds[i].paused, 1);
        } else if (audio->type == OCT_AUDIO_COMMAND_TYPE_UNPAUSE_ALL_SOUNDS) {
            for (int i = 0; i < MAX_PLAYING_SOUNDS; i++)
                SDL_SetAtomicInt(&gPlayingSounds[i].paused, 0);
        }
    }
}

uint8_t *_oct_AudioConvertFormat(uint8_t *data, int32_t size, int32_t *newSize, SDL_AudioSpec *spec) {
    uint8_t *newData = null;
    if (!SDL_ConvertAudioSamples(spec, data, size, &gDeviceSpec, &newData, newSize)) {
        newData = null;
        *newSize = 0;
    }
    return newData;
}

void _oct_AudioEnd(Oct_Context ctx) {
    SDL_WaitThread(gMixerThread, null);
}

OCTARINE_API Oct_Audio oct_LoadAudio(Oct_Context ctx, const char *filename) {
    Oct_LoadCommand command = {
            .type = OCT_LOAD_COMMAND_TYPE_LOAD_AUDIO,
            .Audio.filename = filename
    };
    return oct_Load(ctx, &command);
}

OCTARINE_API Oct_Sound oct_PlaySound(Oct_Context ctx, Oct_Audio audio, Oct_Vec2 volume, Oct_Bool repeat) {
    Oct_AudioCommand command = {
            .type = OCT_AUDIO_COMMAND_TYPE_PLAY_SOUND,
            .Play = {
                    .audio = audio,
                    .repeat = repeat,
                    .volume = {volume[0], volume[1]}
            }
    };
    return oct_AudioUpdate(ctx, &command);
}

// Generation is up-ticked each time a sound finishes playing in the mixer, so if the sound is a valid handle
// and the ID's generation matches the generation in that slot, it must still be playing.
OCTARINE_API Oct_Bool oct_SoundIsStopped(Oct_Context ctx, Oct_Sound sound) {
    if (sound < MAX_PLAYING_SOUNDS) {
        return SOUND_GENERATION(sound) == SDL_GetAtomicInt(&gPlayingSounds[SOUND_INDEX(sound)].generation);
    }
    return false;
}

OCTARINE_API Oct_Sound oct_UpdateSound(Oct_Context ctx, Oct_Sound sound, Oct_Vec2 volume, Oct_Bool repeat, Oct_Bool paused) {
    Oct_AudioCommand command = {
            .type = OCT_AUDIO_COMMAND_TYPE_UPDATE_SOUND,
            .Update = {
                    .sound = sound,
                    .repeat = repeat,
                    .volume = {volume[0], volume[1]},
                    .paused = paused
            }
    };
    return oct_AudioUpdate(ctx, &command);
}

OCTARINE_API void oct_StopSound(Oct_Context ctx, Oct_Sound sound) {
    Oct_AudioCommand command = {
            .type = OCT_AUDIO_COMMAND_TYPE_UPDATE_SOUND,
            .Update = {
                    .sound = sound,
                    .stop = true
            }
    };
    oct_AudioUpdate(ctx, &command);
}

OCTARINE_API void oct_StopAllSounds(Oct_Context ctx) {
    Oct_AudioCommand command = {
            .type = OCT_AUDIO_COMMAND_TYPE_STOP_ALL_SOUNDS,
    };
    oct_AudioUpdate(ctx, &command);
}

OCTARINE_API void oct_PauseAllSounds(Oct_Context ctx) {
    Oct_AudioCommand command = {
            .type = OCT_AUDIO_COMMAND_TYPE_PAUSE_ALL_SOUNDS,
    };
    oct_AudioUpdate(ctx, &command);
}

OCTARINE_API void oct_UnpauseAllSounds(Oct_Context ctx) {
    Oct_AudioCommand command = {
            .type = OCT_AUDIO_COMMAND_TYPE_UNPAUSE_ALL_SOUNDS,
    };
    oct_AudioUpdate(ctx, &command);
}