#include <SDL2/SDL.h>
#include "oct/LogicalThread.h"
#include "oct/Opaque.h"

int oct_UserThread(void *ptr) {
    Oct_Context ctx = ptr;
    // TODO: This
    return 0;
}

void oct_Bootstrap(Oct_Context ctx) {
    ctx->logicThread = SDL_CreateThread(oct_UserThread, "Logic Thread", ctx);
}

void oct_UnstrapBoots(Oct_Context ctx) {
    SDL_WaitThread(ctx->logicThread, null);
}