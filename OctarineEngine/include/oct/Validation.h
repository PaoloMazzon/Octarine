/// \brief Functions to error check and validate
#pragma once
#include "oct/Common.h"
#include "oct/Constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Raises an error in engine
/// \param status Status to raise
/// \param fatal Whether or not its fatal (fatal means `abort()`)
/// \param fmt Format for the printf-like effect
/// If the crash is fatal, machine info and the error will be output to `octarinedump.log`.
OCTARINE_API void oct_Raise(Oct_Status status, Oct_Bool fatal, const char *fmt, ...);

/// \brief Returns the current engine status and clears it as well
OCTARINE_API Oct_Status oct_GetStatus();

/// \brief Returns any error message present and clears it
OCTARINE_API const char *oct_GetError();

/// \brief Thread-safe logging function
/// \param fmt Printf fmt
OCTARINE_API void oct_Log(const char *fmt, ...);

#ifdef __cplusplus
};
#endif
