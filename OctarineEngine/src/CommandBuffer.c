#include <mimalloc.h>
#include "oct/Validation.h"
#include "oct/CommandBuffer.h"
#include "oct/Opaque.h"
#include "oct/Constants.h"

// Wraps an index, returns index + 1 unless index == len - 1 in which case it returns 0
static inline int32_t nextIndex(int32_t index, int32_t len) {
    return index == len - 1 ? 0 : index + 1;
}

// Places a command into the ring buffer at tail, stalling if its full
static inline void pushCommand(Oct_Context ctx, Oct_Command *command) {
    // Busy loop if the buffer is full
    int tail = SDL_AtomicGet(&ctx->RingBuffer.tail);
    while (nextIndex(tail, ctx->initInfo->ringBufferSize) == SDL_AtomicGet(&ctx->RingBuffer.head)) {
        volatile int i;
    }

    // Insert new element
    memcpy(&ctx->RingBuffer.commands[tail], command, sizeof(struct Oct_Command_t));
    SDL_AtomicSet(&ctx->RingBuffer.tail, nextIndex(tail, ctx->initInfo->ringBufferSize));
}

void _oct_CommandBufferInit(Oct_Context ctx) {
    ctx->RingBuffer.commands = mi_malloc(sizeof(struct Oct_Command_t) * OCT_RING_BUFFER_SIZE);
    if (ctx->RingBuffer.commands) {
        SDL_AtomicSet(&ctx->RingBuffer.head, 0);
        SDL_AtomicSet(&ctx->RingBuffer.tail, 0);
    } else {
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate ringbuffer.");
    }
}

void _oct_CommandBufferBeginFrame(Oct_Context ctx) {
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .command.metaCommand = {
                    .sType = OCT_STRUCTURE_TYPE_META_COMMAND,
                    .type = OCT_META_COMMAND_TYPE_START_FRAME
            }
    };
    pushCommand(ctx, &cmd);
}

void _oct_CommandBufferEndFrame(Oct_Context ctx) {
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .command.metaCommand = {
                    .sType = OCT_STRUCTURE_TYPE_META_COMMAND,
                    .type = OCT_META_COMMAND_TYPE_END_FRAME
            }
    };
    pushCommand(ctx, &cmd);
}

bool _oct_CommandBufferPop(Oct_Context ctx, Oct_Command *out) {
    if (SDL_AtomicGet(&ctx->RingBuffer.head) != SDL_AtomicGet(&ctx->RingBuffer.tail)) {
        memcpy(out, &ctx->RingBuffer.commands[SDL_AtomicGet(&ctx->RingBuffer.head)], sizeof(struct Oct_Command_t));
        SDL_AtomicSet(&ctx->RingBuffer.head, nextIndex(SDL_AtomicGet(&ctx->RingBuffer.head), ctx->initInfo->ringBufferSize));
        return true;
    }
    return false;
}

void _oct_CommandBufferEnd(Oct_Context ctx) {
    mi_free(ctx->RingBuffer.commands);
}

OCTARINE_API void oct_Draw(Oct_Context ctx, Oct_DrawCommand *draw) {
    draw->sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND;
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .command.drawCommand = *draw
    };
    pushCommand(ctx, &cmd);
}

OCTARINE_API void oct_WindowUpdate(Oct_Context ctx, Oct_WindowCommand *windowUpdate) {
    windowUpdate->sType = OCT_STRUCTURE_TYPE_WINDOW_COMMAND;
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .command.windowCommand = *windowUpdate
    };
    pushCommand(ctx, &cmd);
}

OCTARINE_API Oct_Asset oct_Load(Oct_Context ctx, Oct_LoadCommand *load) {
    load->sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND;
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .command.loadCommand = *load
    };
    pushCommand(ctx, &cmd);
}
