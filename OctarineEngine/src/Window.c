#include "oct/Window.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"

void _oct_WindowInit(Oct_Context ctx) {
    ctx->window = SDL_CreateWindow(
            ctx->initInfo->windowTitle,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            ctx->initInfo->windowWidth,
            ctx->initInfo->windowHeight,
            SDL_WINDOW_VULKAN
    );
    if (!ctx->window) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create window. SDL Error \"%s\"", SDL_GetError());
    }
}

void _oct_WindowEnd(Oct_Context ctx) {
    SDL_DestroyWindow(ctx->window);
}

void _oct_WindowUpdate(Oct_Context ctx) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            SDL_AtomicSet(&ctx->quit, 1);
    }

    // TODO: Input polling
}
