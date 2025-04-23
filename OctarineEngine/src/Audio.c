#include <math.h>
#include <SDL3/SDL.h>
#include "oct/Common.h"
#include "oct/Validation.h"
#include "oct/Core.h"

static SDL_AudioDeviceID gAudioDevice;
static SDL_AudioStream *gAudioStream;
static const int gFrequency = 44100;

void _oct_AudioInit(Oct_Context ctx) {
    // TODO: This
    SDL_AudioSpec spec = {
            .channels = 2,
            .format = SDL_AUDIO_F32,
            .freq = gFrequency
    };
    gAudioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    gAudioStream = SDL_CreateAudioStream(&spec, &spec);
    if (gAudioDevice == 0 || gAudioStream == null || !SDL_BindAudioStream(gAudioDevice, gAudioStream)) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to initialize audio device, SDL error: %s", SDL_GetError());
    }
    SDL_ResumeAudioDevice(gAudioDevice);
    oct_Log("Created audio device using \"%s\"", SDL_GetAudioDeviceName(gAudioDevice));
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
    SDL_DestroyAudioStream(gAudioStream);
    SDL_CloseAudioDevice(gAudioDevice);
}
