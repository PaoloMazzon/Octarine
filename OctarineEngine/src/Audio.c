#include <math.h>
#include <SDL3/SDL.h>
#include "oct/Common.h"
#include "oct/Validation.h"
#include "oct/CommandBuffer.h"
#include "oct/Core.h"
#include "oct/Opaque.h"

#define SOUND_INDEX(sound) (sound & INT32_MAX)
#define SOUND_GENERATION(sound) ((uint32_t)(sound >> 32))

/////////////////////////// STRUCTS ///////////////////////////
/// \brief Info needed for the mixer to mix any particular sound
typedef struct Oct_PlayingSound_t {
    Oct_Audio sound;           ///< Actual sound asset being played
    int32_t pointer;           ///< Pointer (in bytes) to where in the sound buffer the sound will be played from
    SDL_AtomicInt volumeLeft;  ///< Left volume from 0-AUDIO_VOLUME_NORMALIZED_FACTOR
    SDL_AtomicInt volumeRight; ///< Right volume from 0-AUDIO_VOLUME_NORMALIZED_FACTOR
    SDL_AtomicInt paused;      ///< Whether or not the sound is paused
    SDL_AtomicInt repeat;      ///< Whether or not the sound will repeat infinitely
    SDL_AtomicU32 generation;  ///< Generation so the user gets a unique playing sound each time they play new sounds (least significant 32 bits are reserved for the index)
    SDL_AtomicInt alive;       ///< Whether or not this sound is currently valid
    SDL_AtomicInt reserved;    ///< Whether or not this sound is reserved from the logic thread
} Oct_PlayingSound;

/////////////////////////// GLOBALS ///////////////////////////
static SDL_AudioDeviceID gAudioDevice;
static SDL_AudioStream *gAudioStream;
static const int AUDIO_FREQUENCY_HZ = 44100;
static const double AUDIO_REFRESH_RATE_HZ = 100;
static const double AUDIO_VOLUME_NORMALIZED_FACTOR = 10000;
static SDL_Thread *gMixerThread;
static SDL_AudioSpec gDeviceSpec = {
        .channels = 2,
        .format = SDL_AUDIO_F32,
        .freq = AUDIO_FREQUENCY_HZ
};
#define MAX_PLAYING_SOUNDS 100
static Oct_PlayingSound gPlayingSounds[MAX_PLAYING_SOUNDS];

/////////////////////////// PLAYING SOUND STUFF ///////////////////////////
// Reserves space in the playing sound list, returning OCT_SOUND_FAILED if it fails (too many concurrent noises)
Oct_Sound _oct_ReserveSound(Oct_Context ctx) {
    Oct_Sound out = OCT_SOUND_FAILED;
    for (int i = 0; i < MAX_PLAYING_SOUNDS; i++) {
        if (!SDL_CompareAndSwapAtomicInt(&gPlayingSounds[i].reserved, 0, 1)) {
            out = i;
        }
    }
    return out;
}

/////////////////////////// FUNCTIONS ///////////////////////////
// Returns the number of seconds from start, which should be a SDL_GetPerformanceCounter call
inline static double _oct_GoofyTime(uint64_t start) {
    const double current = SDL_GetPerformanceCounter();
    const double freq = SDL_GetPerformanceFrequency();
    return (current - start) / freq;
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

    while (!SDL_GetAtomicInt(&ctx->quit)) {
        // TODO: Write mixer

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

    return 0;
}

void _oct_AudioInit(Oct_Context ctx) {
    // Create audio device and stream
    gAudioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &gDeviceSpec);
    gAudioStream = SDL_CreateAudioStream(&gDeviceSpec, &gDeviceSpec);
    if (gAudioDevice == 0 || gAudioStream == null || !SDL_BindAudioStream(gAudioDevice, gAudioStream)) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to initialize audio device, SDL error: %s", SDL_GetError());
    }

    // Create the mixer thread
    gMixerThread = SDL_CreateThread(_oct_MixerThread, "Audio Mixer", ctx);
    if (!gMixerThread) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create mixer thread, SDL error \"%s\"", SDL_GetError());
    }

    SDL_ResumeAudioDevice(gAudioDevice);
    oct_Log("Created audio device and thread using \"%s\"", SDL_GetAudioDeviceName(gAudioDevice));
}

void _oct_AudioUpdateBegin(Oct_Context ctx) {
    // TODO: This
}

void _oct_AudioUpdateEnd(Oct_Context ctx) {
    // TODO: This
}

void _oct_AudioProcessCommand(Oct_Context ctx, Oct_Command *cmd) {
    if (OCT_STRUCTURE_TYPE(&cmd->topOfUnion) == OCT_STRUCTURE_TYPE_META_COMMAND) {
        // Meta commands currently do not impact the audio subsystem
    } else {
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
            if (index < MAX_PLAYING_SOUNDS && SOUND_GENERATION(audio->Update.sound) == SDL_GetAtomicU32(&gPlayingSounds[index].generation)) {
                SDL_SetAtomicInt(&gPlayingSounds[index].repeat, audio->Update.repeat);
                SDL_SetAtomicInt(&gPlayingSounds[index].volumeLeft, audio->Update.volume[0] * AUDIO_VOLUME_NORMALIZED_FACTOR);
                SDL_SetAtomicInt(&gPlayingSounds[index].volumeRight, audio->Update.volume[1] * AUDIO_VOLUME_NORMALIZED_FACTOR);
            }
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
    SDL_DestroyAudioStream(gAudioStream);
    SDL_CloseAudioDevice(gAudioDevice);
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
        return SOUND_GENERATION(sound) == SDL_GetAtomicU32(&gPlayingSounds[SOUND_INDEX(sound)].generation);
    }
    return false;
}

OCTARINE_API Oct_Sound oct_UpdateSound(Oct_Context ctx, Oct_Sound sound, Oct_Vec2 volume, Oct_Bool repeat) {
    Oct_AudioCommand command = {
            .type = OCT_AUDIO_COMMAND_TYPE_UPDATE_SOUND,
            .Update = {
                    .sound = sound,
                    .repeat = repeat,
                    .volume = {volume[0], volume[1]}
            }
    };
    return oct_AudioUpdate(ctx, &command);
}