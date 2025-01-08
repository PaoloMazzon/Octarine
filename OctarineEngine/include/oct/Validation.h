/// \brief Functions to error check and validate
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Initializes validation
void _oct_ValidationInit();

/// \brief Cleans up validation
void _oct_ValidationEnd();

/// \brief Raises an error in engine
/// \param status Status to raise
/// \param fatal Whether or not its fatal (fatal means `abort()`)
/// \param fmt Format for the printf-like effect
/// If the crash is fatal, machine info and the error will be output to `octarinedump.log`.
OCT_EXPORT void oct_Raise(Oct_Status status, Oct_Bool fatal, const char *fmt, ...);

/// \brief Returns the current engine status and clears it as well
OCT_EXPORT Oct_Status oct_GetStatus();

/// \brief Returns any error message present and clears it
OCT_EXPORT const char *oct_GetError();

/// \brief Thread-safe logging function
/// \param fmt Printf fmt
OCT_EXPORT void oct_Log(const char *fmt, ...);

#ifdef __cplusplus
};
#endif
