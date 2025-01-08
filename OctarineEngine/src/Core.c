#include <VK2D/VK2D.h>
#include <mimalloc.h>
#include "oct/Octarine.h"
#include "oct/LogicalThread.h"
#include "oct/Opaque.h"

OCT_EXPORT Oct_Status oct_Init(Oct_InitInfo *initInfo) {
    Oct_Context ctx = mi_malloc(sizeof(struct Oct_Context_t));

    // TODO: Initialize SDL window, VK2D, and logical thread

    mi_free(ctx);
    return OCT_STATUS_SUCCESS;
}
