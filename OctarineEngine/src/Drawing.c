#include <VK2D/VK2D.h>
#include "oct/Drawing.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"

void _oct_DrawingInit(Oct_Context ctx) {
    VK2DRendererConfig config = {
            .msaa = VK2D_MSAA_32X,
            .filterMode = VK2D_FILTER_TYPE_NEAREST,
            .screenMode = VK2D_SCREEN_MODE_TRIPLE_BUFFER
    };
    VK2DStartupOptions options = {
            .errorFile = "octarinedump.log",
            .quitOnError = true,
            .stdoutLogging = false,
            .enableDebug = false
    };
    vk2dRendererInit(ctx->window, config, &options);
}

void _oct_DrawingEnd(Oct_Context ctx) {
    vk2dRendererQuit();
}

void _oct_DrawingUpdate(Oct_Context ctx) {
    vk2dRendererStartFrame(VK2D_BLACK);

    // TODO: Parse future draw queue

    vk2dRendererEndFrame();
}
