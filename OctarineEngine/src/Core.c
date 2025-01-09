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
    ctx->initInfo = initInfo;
}

OCTARINE_API Oct_Status oct_Init(Oct_InitInfo *initInfo) {
    // Initialization
    Oct_Context ctx = mi_malloc(sizeof(struct Oct_Context_t));
    _oct_SetupInitInfo(ctx, initInfo);
    _oct_ValidationInit(ctx);
    _oct_WindowInit(ctx);
    _oct_DrawingInit(ctx);
    _oct_CommandBufferInit(ctx);

    // Bootstrap thread
    oct_Bootstrap(ctx);

    // Main game loop
    while (SDL_AtomicGet(&ctx->quit) == 0) {
        _oct_WindowUpdate(ctx);
        _oct_DrawingUpdate(ctx);

        // TODO: Sleep if the user requests a target framerate
    }

    // Cleanup
    _oct_CommandBufferEnd(ctx);
    _oct_UnstrapBoots(ctx);
    _oct_DrawingEnd(ctx);
    _oct_WindowEnd(ctx);
    _oct_ValidationEnd(ctx);
    mi_free(ctx);
    return OCT_STATUS_SUCCESS;
}
