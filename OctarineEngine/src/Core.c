#include <VK2D/VK2D.h>
#include <mimalloc.h>
#include "oct/Octarine.h"
#include "oct/LogicalThread.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"
#include "oct/Subsystems.h"

OCTARINE_API Oct_Status oct_Init(Oct_InitInfo *initInfo) {
    // Initialization
    Oct_Context ctx = mi_malloc(sizeof(struct Oct_Context_t));
    ctx->initInfo = initInfo;
    _oct_ValidationInit(ctx);
    _oct_WindowInit(ctx);
    _oct_DrawingInit(ctx);

    // Bootstrap thread
    oct_Bootstrap(ctx);

    // Main game loop
    while (!ctx->quit) {
        // TODO: Swap around drawing queues

        _oct_WindowUpdate(ctx);
        _oct_DrawingUpdate(ctx);

        // TODO: Sleep if the user requests a target framerate
    }

    // Cleanup
    oct_UnstrapBoots(ctx);
    _oct_DrawingEnd(ctx);
    _oct_WindowEnd(ctx);
    _oct_ValidationEnd(ctx);
    mi_free(ctx);
    return OCT_STATUS_SUCCESS;
}
