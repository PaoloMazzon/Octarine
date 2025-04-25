#include <math.h>
#include <SDL3/SDL.h>
#include "oct/Common.h"
#include "oct/Validation.h"
#include "oct/Core.h"
#include "oct/Opaque.h"

/////////////////////////// STRUCTS ///////////////////////////
/// \brief Info needed for the mixer to mix any particular sound
typedef struct Oct_PlayingSound_t {
    Oct_Audio sound;           ///< Actual sound asset being played
    int32_t pointer;           ///< Pointer (in bytes) to where in the sound buffer the sound will be played from
    SDL_AtomicInt volumeLeft;  ///< Left volume from 0-10,000
    SDL_AtomicInt volumeRight; ///< Right volume from 0-10,000
    SDL_AtomicInt paused;      ///< Whether or not the sound is paused
    SDL_AtomicInt repeat;      ///< Whether or not the sound will repeat infinitely
    SDL_AtomicInt generation;  ///< Generation so the user gets a unique playing sound each time they play new sounds (least significant 16 bits are reserved for the index)
    SDL_AtomicInt alive;       ///< Whether or not this sound is currently valid
    SDL_AtomicInt reserved;    ///< Whether or not this sound is reserved from the logic thread
} Oct_PlayingSound;

/////////////////////////// GLOBALS ///////////////////////////
static SDL_AudioDeviceID gAudioDevice;
static SDL_AudioStream *gAudioStream;
static const int AUDIO_FREQUENCY_HZ = 44100;
static const double AUDIO_REFRESH_RATE_HZ = 100;
static SDL_Thread *gMixerThread;
static SDL_AudioSpec gDeviceSpec = {
        .channels = 2,
        .format = SDL_AUDIO_F32,
        .freq = AUDIO_FREQUENCY_HZ
};
static uint8_t *gTestAudio;
static int32_t gTestAudioSize;
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

    // Load test audio
    SDL_AudioSpec outSpec;
    uint8_t *tempBytes;
    uint32_t tempSize;
    if (!SDL_LoadWAV("data/test.wav", &outSpec, &tempBytes, &tempSize) ||
        !SDL_ConvertAudioSamples(&outSpec, tempBytes, tempSize, &gDeviceSpec, &gTestAudio, &gTestAudioSize)) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create test audio, SDL error \"%s\"", SDL_GetError());
    }
    SDL_free(tempBytes);

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
    // TODO: This
}


void _oct_AudioEnd(Oct_Context ctx) {
    SDL_free(gTestAudio);
    SDL_WaitThread(gMixerThread, null);
    SDL_DestroyAudioStream(gAudioStream);
    SDL_CloseAudioDevice(gAudioDevice);
}
