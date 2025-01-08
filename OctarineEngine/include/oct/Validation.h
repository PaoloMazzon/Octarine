/// \brief Functions to error check and validate
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Raises an error in engine
/// \param status Status to raise
/// \param fatal Whether or not its fatal
/// \param fmt Format for the printf-like effect
OCT_EXPORT void oct_Raise(Oct_Status status, Oct_Bool fatal, const char *fmt, ...);

/// \brief Returns true if there is a fatal error (ie stop what you're doing)
OCT_EXPORT Oct_Bool oct_IsFatal();

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
