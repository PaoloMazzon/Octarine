#include <SDL2/SDL.h>
#include <VK2D/VK2D.h>
#include "oct/LogicThread.h"
#include "oct/Validation.h"
#include "oct/Opaque.h"

int oct_UserThread(void *ptr) {
    Oct_Context ctx = ptr;

    void *userData = ctx->initInfo->startup(ctx);

    // Timekeeping
    uint64_t startTime = SDL_GetPerformanceCounter();
    uint64_t between = 0;
    uint64_t frameCount = 0;
    float averageHz = 0;

    // User-end game loop
    while (SDL_AtomicGet(&ctx->quit) == 0) {
        // Process user frame
        userData = ctx->initInfo->update(ctx, userData);

        // Wait until clock thread we may begin a new frame
        while (SDL_AtomicGet(&ctx->frameStart) == 0) {
            volatile int i;
        }
        SDL_AtomicSet(&ctx->frameStart, 0);

        // Timekeeping
        between += SDL_GetPerformanceCounter() - startTime;
        frameCount++;
        if (between >= SDL_GetPerformanceFrequency()) {
            averageHz = (float)((double)frameCount / ((double)between / (double)SDL_GetPerformanceFrequency()));
            between = 0;
            frameCount = 0;
            SDL_AtomicSet(&ctx->logicHzActual, OCT_FLOAT_TO_INT(averageHz));
        }
        startTime = SDL_GetPerformanceCounter();
    }

    ctx->initInfo->shutdown(ctx, userData);

    return 0;
}

int oct_ClockThread(void *ptr) {
    Oct_Context ctx = ptr;

    // Keep track of time
    while (SDL_AtomicGet(&ctx->quit) == 0) {
        // Wait 1 frame worth of time TODO - Change this out with a busy loop that changes an estimation variable
        vk2dSleep(1.0 / ctx->initInfo->logicHz);

        // Tell the logic thread it may begin
        SDL_AtomicSet(&ctx->frameStart, 1);

        // Busy loop until the logic thread has acknowledged a new frame
        while (SDL_AtomicGet(&ctx->frameStart) == 1) {
            volatile int i;
        }
    }

    return 0;
}

void oct_Bootstrap(Oct_Context ctx) {
    ctx->logicThread = SDL_CreateThread(oct_UserThread, "Logic Thread", ctx);
    ctx->clockThread = SDL_CreateThread(oct_ClockThread, "Clock Thread", ctx);
}

void _oct_UnstrapBoots(Oct_Context ctx) {
    SDL_WaitThread(ctx->logicThread, null);
    SDL_WaitThread(ctx->clockThread, null);
}