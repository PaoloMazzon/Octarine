#include <SDL2/SDL.h>
#include <VK2D/VK2D.h>
#include "oct/LogicThread.h"
#include "oct/Validation.h"
#include "oct/Subsystems.h"
#include "oct/Opaque.h"

int oct_UserThread(void *ptr) {
    Oct_Context ctx = ptr;

    ctx->gameStartTime = SDL_GetPerformanceCounter();
    void *userData = ctx->initInfo->startup(ctx);

    // Timekeeping
    uint64_t startTime = SDL_GetPerformanceCounter();
    uint64_t between = 0;
    uint64_t frameCount = 0;
    float averageHz = 0;

    // User-end game loop
    while (SDL_AtomicGet(&ctx->quit) == 0) {
        // Process user frame
        _oct_CommandBufferBeginFrame(ctx);
        userData = ctx->initInfo->update(ctx, userData);
        _oct_CommandBufferEndFrame(ctx); // TODO - This may need to be moved to after the wait

        // Wait until clock thread says we may begin a new frame
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
        // Calculate estimated frame time for interpolation
        const uint64_t start = SDL_GetPerformanceCounter();
        const uint64_t target = SDL_GetPerformanceCounter() + (SDL_GetPerformanceFrequency() / ctx->initInfo->logicHz);
        uint64_t current;
        while ((current = SDL_GetPerformanceCounter()) < target) {
            float estimated = (float)(current - start) / (float)(SDL_GetPerformanceFrequency() / ctx->initInfo->logicHz);
            SDL_AtomicSet(&ctx->interpolatedTime, OCT_FLOAT_TO_INT(estimated));
        }

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

OCTARINE_API double oct_Time(Oct_Context ctx) {
    return (double)((double)SDL_GetPerformanceCounter() - ctx->gameStartTime) / (double)SDL_GetPerformanceFrequency();
}