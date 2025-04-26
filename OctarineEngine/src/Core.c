#include <VK2D/VK2D.h>
#include <mimalloc.h>
#include "oct/Octarine.h"
#include "oct/LogicThread.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"
#include "oct/Subsystems.h"

static inline void _oct_SetupInitInfo(Oct_Context ctx, Oct_InitInfo *initInfo) {
    if (initInfo->ringBufferSize == 0) {
        initInfo->ringBufferSize = 1000;
    }
    if (initInfo->logicHz == 0) {
        initInfo->logicHz = 30;
    }
    ctx->initInfo = initInfo;
}

OCTARINE_API Oct_Status oct_Init(Oct_InitInfo *initInfo) {
    // Initialization
    Oct_Context ctx = mi_zalloc(sizeof(struct Oct_Context_t));
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO);
    _oct_SetupInitInfo(ctx, initInfo);
    _oct_ValidationInit(ctx);
    _oct_WindowInit(ctx);
    _oct_DrawingInit(ctx);
    _oct_AudioInit(ctx);
    _oct_CommandBufferInit(ctx);
    _oct_AssetsInit(ctx);

    // Debug settings
    if (ctx->initInfo->debug) {
        mi_option_enable(mi_option_show_stats);
        mi_option_enable(mi_option_show_errors);
    }

    // Bootstrap thread
    oct_Bootstrap(ctx);

    // Timekeeping
    float frameCount = 0;
    uint64_t refreshRateStartTime = SDL_GetPerformanceCounter();

    // Main game loop
    while (SDL_GetAtomicInt(&ctx->quit) == 0) {
        // Timekeeping
        const uint64_t startTime = SDL_GetPerformanceCounter();

        // Start subsystems
        _oct_WindowUpdateBegin(ctx);
        _oct_AudioUpdateBegin(ctx);
        _oct_DrawingUpdateBegin(ctx);

        // Process command buffer
        _oct_CommandBufferDispatch(ctx);

        // Finish up subsystems for the frame
        _oct_WindowUpdateEnd(ctx);
        _oct_AudioUpdateEnd(ctx);
        _oct_DrawingUpdateEnd(ctx);

        // Timekeeping
        const int target = SDL_GetAtomicInt(&ctx->renderHz);
        const uint64_t currentTime = SDL_GetPerformanceCounter();
        if (target > 0) {
            const double between = (double)(currentTime - startTime) / SDL_GetPerformanceFrequency();
            vk2dSleep((1.0 / target) - between);
        }

        // Recalculate refresh rate every second
        frameCount++;
        if (currentTime - refreshRateStartTime >= SDL_GetPerformanceFrequency()) {
            const float refreshRate = frameCount / ((float)(currentTime - refreshRateStartTime) / SDL_GetPerformanceFrequency());
            SDL_SetAtomicInt(&ctx->renderHzActual, OCT_FLOAT_TO_INT(refreshRate));
            refreshRateStartTime = SDL_GetPerformanceCounter();
            frameCount = 0;
        }
    }

    // Cleanup
    vk2dRendererWait();
    _oct_AssetsEnd(ctx);
    _oct_CommandBufferEnd(ctx);
    _oct_UnstrapBoots(ctx);
    _oct_AudioEnd(ctx);
    _oct_DrawingEnd(ctx);
    _oct_WindowEnd(ctx);
    _oct_ValidationEnd(ctx);
    mi_free(ctx);
    return OCT_STATUS_SUCCESS;
}

OCTARINE_API double oct_GetRenderFPS(Oct_Context ctx) {
    int i = SDL_GetAtomicInt(&ctx->renderHzActual);
    return OCT_INT_TO_FLOAT(i);
}

OCTARINE_API double oct_GetLogicHz(Oct_Context ctx) {
    int i = SDL_GetAtomicInt(&ctx->logicHzActual);
    return OCT_INT_TO_FLOAT(i);
}