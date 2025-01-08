#include "oct/Validation.h"
#include "oct/Constants.h"

// Constants
#define BUFFER_SIZE 1024
static char gErrorBuffer[BUFFER_SIZE]; // Buffer for error string
static Oct_Bool gIsFatal = false;      // Fatal status
static Oct_Status gStatus;             // General status

OCT_EXPORT void oct_Raise(Oct_Status status, Oct_Bool fatal, const char *fmt, ...) {
    // TODO: This
}

OCT_EXPORT Oct_Bool oct_IsFatal() {
    return false; // TODO: This
}

OCT_EXPORT Oct_Status oct_GetStatus() {
    return OCT_STATUS_SUCCESS; // TODO: This
}

OCT_EXPORT const char *oct_GetError() {
    return ""; // TODO: This
}

OCT_EXPORT void oct_Log(const char *fmt, ...) {
    // TODO: This
}
