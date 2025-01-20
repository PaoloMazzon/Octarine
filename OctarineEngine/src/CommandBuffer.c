#include <mimalloc.h>
#include "oct/Validation.h"
#include "oct/CommandBuffer.h"
#include "oct/Opaque.h"
#include "oct/Constants.h"
#include "oct/Subsystems.h"

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
            .metaCommand = {
                    .sType = OCT_STRUCTURE_TYPE_META_COMMAND,
                    .type = OCT_META_COMMAND_TYPE_START_FRAME
            }
    };
    pushCommand(ctx, &cmd);
}

void _oct_CommandBufferEndFrame(Oct_Context ctx) {
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .metaCommand = {
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
    draw->pNext = null;
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .drawCommand = *draw
    };
    pushCommand(ctx, &cmd);
}

OCTARINE_API void oct_WindowUpdate(Oct_Context ctx, Oct_WindowCommand *windowUpdate) {
    windowUpdate->sType = OCT_STRUCTURE_TYPE_WINDOW_COMMAND;
    windowUpdate->pNext = null;
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .windowCommand = *windowUpdate
    };
    pushCommand(ctx, &cmd);
}

OCTARINE_API Oct_Asset oct_Load(Oct_Context ctx, Oct_LoadCommand *load) {
    load->sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND;
    load->pNext = null;
    load->_assetID = _oct_AssetReserveSpace(ctx);
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = *load
    };
    pushCommand(ctx, &cmd);
    return load->_assetID;
}

OCTARINE_API void oct_FreeAsset(Oct_Context ctx, Oct_Asset asset) {
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    ._assetID = asset,
                    .type = OCT_LOAD_COMMAND_TYPE_FREE
            }
    };
    pushCommand(ctx, &cmd);
}

void _oct_CommandBufferDispatch(Oct_Context ctx) {
    Oct_Command cmd = {0};
    while (_oct_CommandBufferPop(ctx, &cmd)) {
        // Find out the kind of command this is
        const Oct_StructureType sType = OCT_STRUCTURE_TYPE(&cmd.topOfUnion);

        // Dispatch to the proper subsystem
        if (sType == OCT_STRUCTURE_TYPE_META_COMMAND) {
            // Meta commands are dispatched everywhere
            _oct_AudioProcessCommand(ctx, &cmd);
            _oct_DrawingProcessCommand(ctx, &cmd);
            _oct_WindowProcessCommand(ctx, &cmd);
        } else if (sType == OCT_STRUCTURE_TYPE_DRAW_COMMAND) {
            _oct_DrawingProcessCommand(ctx, &cmd);
        } else if (sType == OCT_STRUCTURE_TYPE_WINDOW_COMMAND) {
            _oct_WindowProcessCommand(ctx, &cmd);
        } else if (sType == OCT_STRUCTURE_TYPE_AUDIO_COMMAND) {
            _oct_AudioProcessCommand(ctx, &cmd);
        } else if (sType == OCT_STRUCTURE_TYPE_LOAD_COMMAND) {
            _oct_AssetsProcessCommand(ctx, &cmd);
        }
    }
}