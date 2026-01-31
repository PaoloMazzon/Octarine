#include <VK2D/VK2D.h>
#include <mimalloc.h>
#include <physfs.h>
#include "oct/Octarine.h"
#include "oct/LogicThread.h"
#include "oct/Opaque.h"
#include "oct/Subsystems.h"

// This is from StackOverflow user Larry Gritz, https://stackoverflow.com/users/3832/larry-gritz
#ifdef __linux__
# include <sys/sysinfo.h>
#endif

#ifdef __APPLE__
# include <mach/task.h>
# include <mach/mach_init.h>
#endif

#ifdef __WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <windows.h>
#include <psapi.h>
#endif

/// The amount of memory currently being used by this process, in bytes.
/// By default, returns the full virtual arena, but if resident=true,
/// it will report just the resident set in RAM (if supported on that OS).
size_t memory_used(bool resident) {
#if defined(__linux__)
    // Ugh, getrusage doesn't work well on Linux.  Try grabbing info
    // directly from the /proc pseudo-filesystem.  Reading from
    // /proc/self/statm gives info on your own process, as one line of
    // numbers that are: virtual mem program size, resident set size,
    // shared pages, text/code, data/stack, library, dirty pages.  The
    // mem sizes should all be multiplied by the page size.
    size_t size = 0;
    FILE *file = fopen("/proc/self/statm", "r");
    if (file) {
        unsigned long vm = 0;
        fscanf (file, "%ul", &vm);  // Just need the first num: vm size
        fclose (file);
       size = (size_t)vm * getpagesize();
    }
    return size;

#elif defined(__APPLE__)
    // Inspired by:
    // http://miknight.blogspot.com/2005/11/resident-set-size-in-mac-os-x.html
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
    size_t size = (resident ? t_info.resident_size : t_info.virtual_size);
    return size;

#elif defined(_WIN32)
    // According to MSDN...
    PROCESS_MEMORY_COUNTERS counters;
    if (GetProcessMemoryInfo (GetCurrentProcess(), &counters, sizeof (counters)))
        return counters.PagefileUsage;
    else return 0;

#else
    // No idea what platform this is
    return 0;   // Punt
#endif
}

static Oct_Context gInternalCtx;

double _oct_GetLogicProcessTime() {
    Oct_Context ctx = _oct_GetCtx();
    int i = SDL_GetAtomicInt(&ctx->logicProcessTime);
    return OCT_INT_TO_FLOAT(i);
}

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
    static nk_bool showFullAssets = true;
    static const char *typeStrings[OCT_ASSET_TYPE_MAX] = {
            "Texture",
            "Font",
            "Font Atlas",
            "Audio",
            "Sprite",
            "Camera",
    };

    // Draw nuklear debug thing
    if (nk_begin(vk2dGuiContext(), "Performance", nk_rect(10, 10, 300, 220),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {

        // Host info
        nk_layout_row_dynamic(vk2dGuiContext(), 20, 1);

        // Performance metrics
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "=======Octarine=======");
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Render: %0.2ffps, %0.2fms", oct_GetRenderFPS(), (1.0 / oct_GetRenderFPS()) * 1000);
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Logic: %0.2fHz, %0.2fms", oct_GetLogicHz(), _oct_GetLogicProcessTime() * 1000);
        float inUse, total;
        vk2dRendererGetVRAMUsage(&inUse, &total);
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "VRAM: %.2fmb/%.2fmb", inUse, total);
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "RAM: %.2fmb/%.2fgb", (double)memory_used(false) / 1024 / 1024, (double)SDL_GetSystemRAM() / 1024);
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Interpolations/frame: %0.2f", _oct_DrawingGetAverageInterpolationCalls());
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Interpolation time: %0.2fµs", _oct_DrawingGetAverageInterpolationTime() * 1000000);
    }
    nk_end(vk2dGuiContext());

    if (nk_begin(vk2dGuiContext(), "Assets", nk_rect(10, 240, 300, 350),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {

        nk_layout_row_dynamic(vk2dGuiContext(), 20, 1);
        nk_checkbox_label(vk2dGuiContext(), "Show Asset Detail", &showFullAssets);

        // Assets
        if (showFullAssets) {
            nk_layout_row_template_begin(vk2dGuiContext(), 20);
            nk_layout_row_template_push_static(vk2dGuiContext(), 80);
            nk_layout_row_template_push_static(vk2dGuiContext(), 100);
            nk_layout_row_template_push_static(vk2dGuiContext(), 600);
            nk_layout_row_template_end(vk2dGuiContext());
            nk_label(vk2dGuiContext(), "Index, Gen", NK_TEXT_CENTERED);
            nk_label(vk2dGuiContext(), "Type", NK_TEXT_CENTERED);
            nk_label(vk2dGuiContext(), "Info", NK_TEXT_LEFT);

            for (int i = 0; i < OCT_MAX_ASSETS; i++) {
                if (_oct_AssetType(i) == OCT_ASSET_TYPE_NONE) continue;
                nk_labelf(vk2dGuiContext(), NK_TEXT_CENTERED, "%i, %i", i, _oct_AssetGeneration(i));
                nk_labelf(vk2dGuiContext(), NK_TEXT_CENTERED, "%s", _oct_AssetTypeString(i));
                nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "%s", _oct_AssetName(i));
            }
        } else {
            int assetTypeCounts[OCT_ASSET_TYPE_MAX] = {0};
            for (int i = 0; i < OCT_MAX_ASSETS; i++) {
                if (_oct_AssetType(i) < OCT_ASSET_TYPE_MAX && _oct_AssetType(i) >= 0)
                    assetTypeCounts[_oct_AssetType(i)] += 1;
            }
            nk_layout_row_dynamic(vk2dGuiContext(), 20, 1);
            nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Type: Count");
            for (int i = 1; i < OCT_ASSET_TYPE_MAX; i++) {
                nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "%s: %i", typeStrings[i - 1], assetTypeCounts[i]);
            }
        }
    }
    nk_end(vk2dGuiContext());

    // Audio interface
    if (nk_begin(vk2dGuiContext(), "Audio", nk_rect(320, 10, 330, 220),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
        // Top-level audio information/settings
        nk_layout_row_begin(vk2dGuiContext(), NK_STATIC, 20, 2);
        nk_layout_row_push(vk2dGuiContext(), 75);
        nk_label(vk2dGuiContext(), "Volume: ", NK_TEXT_LEFT);
        nk_layout_row_push(vk2dGuiContext(), 220);
        _oct_SetGlobalVolume(nk_slide_float(vk2dGuiContext(), 0, _oct_GetGlobalVolume(), 1, 0.01));
        nk_layout_row_push(vk2dGuiContext(), 95);
        nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "Sounds: %i", _oct_CountPlayingSounds());
        nk_layout_row_push(vk2dGuiContext(), 200);
        if (nk_button_label(vk2dGuiContext(), "Stop all"))
            oct_StopAllSounds();

        // Each loaded audio sample
        nk_layout_row_template_begin(vk2dGuiContext(), 20);
        nk_layout_row_template_push_static(vk2dGuiContext(), 80);
        nk_layout_row_template_push_static(vk2dGuiContext(), 100);
        nk_layout_row_template_push_static(vk2dGuiContext(), 600);
        nk_layout_row_template_end(vk2dGuiContext());
        nk_label(vk2dGuiContext(), "Index, Gen", NK_TEXT_CENTERED);
        nk_label(vk2dGuiContext(), "Play", NK_TEXT_CENTERED);
        nk_label(vk2dGuiContext(), "Info", NK_TEXT_LEFT);

        for (int i = 0; i < OCT_MAX_ASSETS; i++) {
            if (_oct_AssetType(i) != OCT_ASSET_TYPE_AUDIO) continue;
            nk_labelf(vk2dGuiContext(), NK_TEXT_CENTERED, "%i, %i", i, _oct_AssetGeneration(i));
            if (nk_button_label(vk2dGuiContext(), "Play"))
                _oct_PlaySoundInternal(((uint64_t)_oct_AssetGeneration(i) << 32) + i);
            nk_labelf(vk2dGuiContext(), NK_TEXT_LEFT, "%s", _oct_AssetName(i));
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
    ctx->gameStartTime = SDL_GetPerformanceCounter();
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
    _oct_JobsInit();

    // Debug settings
    if (ctx->initInfo->debug) {
        mi_option_enable(mi_option_show_stats);
        mi_option_enable(mi_option_show_errors);
        mi_option_enable(mi_option_show_stats);
        mi_option_enable(mi_option_verbose);
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
        _oct_JobsUpdate();

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
    _oct_UnstrapBoots();
    _oct_DebugEnd();
    _oct_JobsEnd();
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

