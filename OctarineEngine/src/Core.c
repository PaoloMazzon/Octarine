#include <VK2D/VK2D.h>
#include <mimalloc.h>
#include <physfs.h>
#include "oct/Octarine.h"
#include "oct/LogicThread.h"
#include "oct/Opaque.h"
#include "oct/Subsystems.h"

static Oct_Context gInternalCtx;

Oct_Context _oct_GetCtx() {
    return gInternalCtx;
}

static inline void _oct_SetupInitInfo(Oct_InitInfo *initInfo) {
    Oct_Context ctx = _oct_GetCtx();
    if (initInfo->ringBufferSize == 0) {
        initInfo->ringBufferSize = 1000;
    }
    if (initInfo->logicHz == 0) {
        initInfo->logicHz = 30;
    }
    ctx->initInfo = initInfo;
}

// Returns the number of seconds from start, which should be a SDL_GetPerformanceCounter call
inline static double _oct_GoofyTime(uint64_t start) {
    const double current = SDL_GetPerformanceCounter();
    const double freq = SDL_GetPerformanceFrequency();
    return (current - start) / freq;
}

void _oct_DebugInit() {

}

void _oct_DebugUpdate() {
    Oct_Context ctx = _oct_GetCtx();
    if (!ctx->initInfo->debug) return;

    // Draw nuklear debug thing
    if (nk_begin(vk2dGuiContext(), "Performance", nk_rect(10, 10, 300, 200),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {

        // Host info
        nk_layout_row_dynamic(vk2dGuiContext(), 20, 1);

        // Performance metrics
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Render: %0.2ffps, %0.2fms", oct_GetRenderFPS(), (1.0 / oct_GetRenderFPS()) * 1000);
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Logic: %0.2fHz, %0.2fms", oct_GetLogicHz(), (1.0 / oct_GetLogicHz()) * 1000);
        float inUse, total;
        vk2dRendererGetVRAMUsage(&inUse, &total);
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "VRAM: %.2fmb/%.2fmb", inUse, total);
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Interpolations/frame: %0.2f", _oct_DrawingGetAverageInterpolationCalls());
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Interpolation time: %0.2fµs", _oct_DrawingGetAverageInterpolationTime() * 1000000);
    }
    nk_end(vk2dGuiContext());

    if (nk_begin(vk2dGuiContext(), "Assets", nk_rect(10, 220, 300, 350),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {

        // Assets
        nk_layout_row_dynamic(vk2dGuiContext(), 20, 3);
        nk_label(vk2dGuiContext(), "Index", NK_TEXT_CENTERED);
        nk_label(vk2dGuiContext(), "Gen.", NK_TEXT_CENTERED);
        nk_label(vk2dGuiContext(), "Type", NK_TEXT_CENTERED);

        for (int i = 0; i < OCT_MAX_ASSETS; i++) {
            if (_oct_AssetType(i) == OCT_ASSET_TYPE_NONE) continue;
            nk_labelf(vk2dGuiContext(), NK_TEXT_CENTERED, "%i", i);
            nk_labelf(vk2dGuiContext(), NK_TEXT_CENTERED, "%i", _oct_AssetGeneration(i));
            nk_labelf(vk2dGuiContext(), NK_TEXT_CENTERED, "%s", _oct_AssetTypeString(i));
        }
    }
    nk_end(vk2dGuiContext());


}

void _oct_DebugEnd() {

}

OCTARINE_API Oct_Status oct_Init(Oct_InitInfo *initInfo) {
    // Initialization
    Oct_Context ctx = mi_zalloc(sizeof(struct Oct_Context_t));
    gInternalCtx = ctx;
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO);
    PHYSFS_init(initInfo->argv[0]);
    _oct_SetupInitInfo(initInfo);
    _oct_ValidationInit();
    _oct_WindowInit();
    _oct_DrawingInit();
    _oct_AudioInit();
    _oct_CommandBufferInit();
    _oct_AssetsInit();
    _oct_DebugInit();

    // Debug settings
    if (ctx->initInfo->debug) {
        mi_option_enable(mi_option_show_stats);
        mi_option_enable(mi_option_show_errors);
    }

    // Bootstrap thread
    oct_Bootstrap();

    // Timekeeping
    float frameCount = 0;
    uint64_t refreshRateStartTime = SDL_GetPerformanceCounter();
    double iterations = 0;
    double totalTime = 0;

    // Main game loop
    while (SDL_GetAtomicInt(&ctx->quit) == 0) {
        // Timekeeping
        const uint64_t startTime = SDL_GetPerformanceCounter();

        // Start subsystems
        _oct_WindowUpdateBegin();
        _oct_AudioUpdateBegin();
        _oct_DrawingUpdateBegin();

        // Process command buffer
        _oct_CommandBufferDispatch();

        // Finish up subsystems for the frame
        _oct_WindowUpdateEnd();
        _oct_AudioUpdateEnd();
        _oct_DrawingUpdateEnd();
        _oct_DebugUpdate();

        // Timekeeping
        const int target = SDL_GetAtomicInt(&ctx->renderHz);
        const uint64_t currentTime = SDL_GetPerformanceCounter();
        totalTime += _oct_GoofyTime(startTime);
        iterations += 1;
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

    oct_Log("Average render tick: %.2fms", (totalTime / iterations) * 1000);

    // Cleanup
    vk2dRendererWait();
    _oct_DebugEnd();
    _oct_UnstrapBoots();
    _oct_AssetsEnd();
    _oct_CommandBufferEnd();
    _oct_AudioEnd();
    _oct_DrawingEnd();
    _oct_WindowEnd();
    _oct_ValidationEnd();
    PHYSFS_deinit();
    mi_free(ctx);
    return OCT_STATUS_SUCCESS;
}

OCTARINE_API double oct_GetRenderFPS() {
    Oct_Context ctx = _oct_GetCtx();
    int i = SDL_GetAtomicInt(&ctx->renderHzActual);
    return OCT_INT_TO_FLOAT(i);
}

OCTARINE_API double oct_GetLogicHz() {
    Oct_Context ctx = _oct_GetCtx();
    int i = SDL_GetAtomicInt(&ctx->logicHzActual);
    return OCT_INT_TO_FLOAT(i);
}
