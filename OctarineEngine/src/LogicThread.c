#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>
#include "oct/LogicThread.h"
#include "oct/Validation.h"
#include "oct/Opaque.h"
#include "oct/Subsystems.h"

// Returns the number of seconds from start, which should be a SDL_GetPerformanceCounter call
inline static double _oct_GoofyTime(uint64_t start) {
    const double current = SDL_GetPerformanceCounter();
    const double freq = SDL_GetPerformanceFrequency();
    return (current - start) / freq;
}

int oct_UserThread(void *ptr) {
    Oct_Context ctx = ptr;

    _oct_InputInit();
    void *userData = null;
    Oct_Bool firstLoop = true;

    // Timekeeping
    uint64_t startTime = SDL_GetPerformanceCounter();
    uint64_t between = 0;
    uint64_t frameCount = 0;
    float averageHz = 0;
    double totalTime = 0; // These are for the whole duration of the program
    double iterations = 0;
    double totalTimeSmaller = 0; // These are updated each second
    double iterationsSmaller = 0;
    double averageProcessTime = 0;


    _oct_CommandBufferBeginSingleFrame();
    userData = ctx->initInfo->startup();
    _oct_CommandBufferEndSingleFrame();

    // User-end game loop
    while (SDL_GetAtomicInt(&ctx->quit) == 0) {
        // Process input
        _oct_InputUpdate();

        // Process user frame
        _oct_CommandBufferBeginFrame();
        userData = ctx->initInfo->update(userData);
        _oct_CommandBufferEndFrame();

        // Keep track of average ticks
        iterations += 1;
        totalTime += _oct_GoofyTime(startTime);
        iterationsSmaller += 1;
        totalTimeSmaller += _oct_GoofyTime(startTime);

        // Wait until clock thread says we may begin a new frame
        while (SDL_GetAtomicInt(&ctx->frameStart) == 0) {
            volatile int i;
        }
        SDL_SetAtomicInt(&ctx->frameStart, 0);

        // Timekeeping
        between += SDL_GetPerformanceCounter() - startTime;
        frameCount++;
        if (between >= SDL_GetPerformanceFrequency()) {
            averageHz = (float)((double)frameCount / ((double)between / (double)SDL_GetPerformanceFrequency()));
            averageProcessTime = totalTimeSmaller / iterationsSmaller;
            totalTimeSmaller = 0;
            iterationsSmaller = 0;
            between = 0;
            frameCount = 0;
            float avg = averageProcessTime;
            SDL_SetAtomicInt(&ctx->logicProcessTime, OCT_FLOAT_TO_INT(avg));
            SDL_SetAtomicInt(&ctx->logicHzActual, OCT_FLOAT_TO_INT(averageHz));
        }
        startTime = SDL_GetPerformanceCounter();
    }

    ctx->initInfo->shutdown(userData);
    oct_Log("Average logic tick: %.2fms", (totalTime / iterations) * 1000);
    _oct_InputEnd();

    return 0;
}

int oct_ClockThread(void *ptr) {
    Oct_Context ctx = ptr;

    // Keep track of time
    while (SDL_GetAtomicInt(&ctx->quit) == 0) {
        // Calculate estimated frame time for interpolation
        const uint64_t start = SDL_GetPerformanceCounter();
        const uint64_t target = SDL_GetPerformanceCounter() + (SDL_GetPerformanceFrequency() / ctx->initInfo->logicHz);
        uint64_t current;
        while ((current = SDL_GetPerformanceCounter()) < target) {
            float estimated = (float)(current - start) / (float)(SDL_GetPerformanceFrequency() / ctx->initInfo->logicHz);
            SDL_SetAtomicInt(&ctx->interpolatedTime, OCT_FLOAT_TO_INT(estimated));
        }

        // Tell the logic thread it may begin
        SDL_SetAtomicInt(&ctx->frameStart, 1);

        // Busy loop until the logic thread has acknowledged a new frame
        while (SDL_GetAtomicInt(&ctx->frameStart) == 1) {
            volatile int i;
        }
    }

    return 0;
}

void oct_Bootstrap() {
    Oct_Context ctx = _oct_GetCtx();

    ctx->logicThread = SDL_CreateThread(oct_UserThread, "Logic Thread", ctx);
    ctx->clockThread = SDL_CreateThread(oct_ClockThread, "Clock Thread", ctx);
    if (ctx->logicThread == null || ctx->clockThread == null) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create one or more threads, SDL error: %s", SDL_GetError());
    }
    oct_Log("Created logic and clock thread.");
}

void _oct_UnstrapBoots() {
    Oct_Context ctx = _oct_GetCtx();
    SDL_WaitThread(ctx->logicThread, null);
    SDL_WaitThread(ctx->clockThread, null);
}

OCTARINE_API double oct_Time() {
    Oct_Context ctx = _oct_GetCtx();
    return (double)((double)SDL_GetPerformanceCounter() - ctx->gameStartTime) / (double)SDL_GetPerformanceFrequency();
}
