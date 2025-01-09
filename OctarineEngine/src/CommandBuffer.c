#include <mimalloc.h>
#include "oct/Validation.h"
#include "oct/CommandBuffer.h"
#include "oct/Opaque.h"
#include "oct/Constants.h"

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

}

void _oct_CommandBufferEndFrame(Oct_Context ctx) {

}

void _oct_CommandBufferEnd(Oct_Context ctx) {
    mi_free(ctx->RingBuffer.commands);
}

OCTARINE_API void oct_Draw(Oct_DrawCommand *draw) {

}

OCTARINE_API void oct_WindowUpdate(Oct_WindowCommand *windowUpdate) {

}

OCTARINE_API Oct_Asset oct_Load(Oct_LoadCommand *load) {

}
