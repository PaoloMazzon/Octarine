#include <SDL2/SDL.h>
#include "oct/LogicThread.h"
#include "oct/Opaque.h"

int oct_UserThread(void *ptr) {
    Oct_Context ctx = ptr;

    void *userData = ctx->initInfo->startup(ctx);

    // User-end game loop
    while (SDL_AtomicGet(&ctx->quit) == 0) {
        userData = ctx->initInfo->update(ctx, userData);
        // TODO: Wait until the render thread tell us to start a new frame
    }

    ctx->initInfo->shutdown(ctx, userData);

    return 0;
}

void oct_Bootstrap(Oct_Context ctx) {
    ctx->logicThread = SDL_CreateThread(oct_UserThread, "Logic Thread", ctx);
}

void _oct_UnstrapBoots(Oct_Context ctx) {
    SDL_WaitThread(ctx->logicThread, null);
}