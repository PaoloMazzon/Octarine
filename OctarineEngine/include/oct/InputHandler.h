/// \brief Declares input handling things for the user to use
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Returns true if a key is currently held down
OCTARINE_API Oct_Bool oct_KeyDown(Oct_Key key);

/// \brief Returns true if a key was just pressed this frame
OCTARINE_API Oct_Bool oct_KeyPressed(Oct_Key key);

/// \brief Returns true if a key was just released this frame
OCTARINE_API Oct_Bool oct_KeyReleased(Oct_Key key);

/// \brief Returns true if a mouse button is currently held down
OCTARINE_API Oct_Bool oct_MouseButtonDown(Oct_MouseButton key);

/// \brief Returns true if a mouse button was just pressed this frame
OCTARINE_API Oct_Bool oct_MouseButtonPressed(Oct_MouseButton key);

/// \brief Returns true if a mouse button was just released this frame
OCTARINE_API Oct_Bool oct_MouseButtonReleased(Oct_MouseButton key);

/// \brief Returns the current mouse X position relative to the window
OCTARINE_API float oct_MouseX();

/// \brief Returns the current mouse Y position relative to the window
OCTARINE_API float oct_MouseY();

/// \brief Sets the gamepad axis deadzone (if an axis registers below this, 0 will be returned)
OCTARINE_API void oct_GamepadSetAxisDeadzone(float deadzone);

/// \brief Gets the gamepad axis deadzone (if an axis registers below this, 0 will be returned)
OCTARINE_API float oct_GamepadGetAxisDeadzone();

/// \brief Sets the gamepad trigger deadzone (if an axis registers below this, 0 will be returned)
OCTARINE_API void oct_GamepadSetTriggerDeadzone(float deadzone);

/// \brief Gets the gamepad trigger deadzone (if an axis registers below this, 0 will be returned)
OCTARINE_API float oct_GamepadGetTriggerDeadzone();

/// \brief Returns true if a gamepad button is currently held down (returns false if the gamepad isn't connected)
OCTARINE_API Oct_Bool oct_GamepadButtonDown(int index, Oct_GamepadButton key);

/// \brief Returns true if a gamepad button was just pressed this frame (returns false if the gamepad isn't connected)
OCTARINE_API Oct_Bool oct_GamepadButtonPressed(int index, Oct_GamepadButton key);

/// \brief Returns true if a gamepad button was just released this frame (returns false if the gamepad isn't connected)
OCTARINE_API Oct_Bool oct_GamepadButtonReleased(int index, Oct_GamepadButton key);

/// \brief Returns the gamepad's left axis X as a normalized value from -1 to 1 (returns 0 if the gamepad isn't connected)
OCTARINE_API float oct_GamepadLeftAxisX(int index);

/// \brief Returns the gamepad's left axis Y as a normalized value from -1 to 1 (returns 0 if the gamepad isn't connected)
OCTARINE_API float oct_GamepadLeftAxisY(int index);

/// \brief Returns the gamepad's right axis X as a normalized value from -1 to 1 (returns 0 if the gamepad isn't connected)
OCTARINE_API float oct_GamepadRightAxisX(int index);

/// \brief Returns the gamepad's right axis Y as a normalized value from -1 to 1 (returns 0 if the gamepad isn't connected)
OCTARINE_API float oct_GamepadRightAxisY(int index);

/// \brief Returns the gamepad's right trigger as a normalized value from 0 to 1
OCTARINE_API float oct_GamepadRightTrigger(int index);

/// \brief Returns the gamepad's left trigger as a normalized value from 0 to 1
OCTARINE_API float oct_GamepadLeftTrigger(int index);

/// \brief Returns true if a gamepad index is connected
OCTARINE_API Oct_Bool oct_GamepadConnected(int index);

#ifdef __cplusplus
};
#endif