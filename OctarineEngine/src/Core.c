#include <VK2D/VK2D.h>
#include <mimalloc.h>
#include "oct/Octarine.h"
#include "oct/LogicalThread.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"

OCT_EXPORT Oct_Status oct_Init(Oct_InitInfo *initInfo) {
    Oct_Context ctx = mi_malloc(sizeof(struct Oct_Context_t));
    _oct_ValidationInit();

    // TODO: Initialize SDL window, VK2D, and logical thread

    _oct_ValidationEnd();
    mi_free(ctx);
    return OCT_STATUS_SUCCESS;
}
