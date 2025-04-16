#include <SDL3/SDL.h>
#include "oct/InputHandler.h"
#include "oct/Subsystems.h"
#include "oct/Opaque.h"

#define GAMEPAD_COUNT 4
const SDL_JoystickID INVALID_GAMEPAD = -1;

// Globals
Oct_Bool gKeysCurrent[SDL_SCANCODE_COUNT];
Oct_Bool gKeysPrevious[SDL_SCANCODE_COUNT];
Oct_Bool gGamepadButtonsCurrent[GAMEPAD_COUNT][OCT_GAMEPAD_BUTTON_MAX];
Oct_Bool gGamepadButtonsPrevious[GAMEPAD_COUNT][OCT_GAMEPAD_BUTTON_MAX];
Oct_Bool gMouseButtonsCurrent[OCT_MOUSE_BUTTON_MAX];
Oct_Bool gMouseButtonsPrevious[OCT_MOUSE_BUTTON_MAX];
float gMouseX;
float gMouseY;
float gLeftAxisX[GAMEPAD_COUNT];
float gLeftAxisY[GAMEPAD_COUNT];
float gRightAxisX[GAMEPAD_COUNT];
float gRightAxisY[GAMEPAD_COUNT];
float gLeftTrigger[GAMEPAD_COUNT];
float gRightTrigger[GAMEPAD_COUNT];
float gAxisDeadzone = 0.05;
float gTriggerDeadzone = 0.05;
SDL_JoystickID gGamepadJoyIDMappings[GAMEPAD_COUNT];

static float _oct_AccountForAxisDeadzone(float val) {
    if (val < gAxisDeadzone && val > -gAxisDeadzone)
        return 0;
    return val;
}

static float _oct_AccountForTriggerDeadzone(float val) {
    if (val < gTriggerDeadzone && val > -gTriggerDeadzone)
        return 0;
    return val;
}

static int _oct_JoystickIDToArrayIndex(SDL_JoystickID id) {
    for (int i = 0; i < GAMEPAD_COUNT; i++)
        if (gGamepadJoyIDMappings[i] == id)
            return i;
    return INVALID_GAMEPAD;
}

void _oct_InputInit(Oct_Context ctx) {
    for (int i = 0; i < GAMEPAD_COUNT; i++)
        gGamepadJoyIDMappings[i] = INVALID_GAMEPAD;
}

void _oct_InputUpdate(Oct_Context ctx) {
    // Copy previous buffers
    memcpy(gKeysPrevious, gKeysCurrent, SDL_SCANCODE_COUNT * sizeof(Oct_Bool));
    memcpy(gMouseButtonsPrevious, gMouseButtonsCurrent, OCT_MOUSE_BUTTON_MAX * sizeof(Oct_Bool));
    for (int i = 0; i < GAMEPAD_COUNT; i++)
        memcpy(gGamepadButtonsPrevious[i], gGamepadButtonsCurrent[i], OCT_GAMEPAD_BUTTON_MAX * sizeof(Oct_Bool));

    // Process events
    Oct_WindowEvent event;
    while (_oct_WindowPopEvent(ctx, &event)) {
        // Keyboard event
        if (event.type == OCT_WINDOW_EVENT_TYPE_KEYBOARD) {
            if (event.keyboardEvent.type == SDL_EVENT_KEY_DOWN) {
                gKeysCurrent[event.keyboardEvent.scancode] = true;
            } else if (event.keyboardEvent.type == SDL_EVENT_KEY_UP) {
                gKeysCurrent[event.keyboardEvent.scancode] = false;
            }
        // Mouse movement
        } else if (event.type == OCT_WINDOW_EVENT_TYPE_MOUSE_MOTION) {
            gMouseX = (float)event.mouseMotionEvent.x;
            gMouseY = (float)event.mouseMotionEvent.y;
        // Mouse button
        } else if (event.type == OCT_WINDOW_EVENT_TYPE_MOUSE_BUTTON) {
            if (event.mouseButtonEvent.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                gMouseButtonsCurrent[event.mouseButtonEvent.button] = false;
            } else if (event.mouseButtonEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                gMouseButtonsCurrent[event.mouseButtonEvent.button] = true;
            }
        // Gamepad buttons
        } else if (event.type == OCT_WINDOW_EVENT_TYPE_GAMEPAD_BUTTON) {
            if (_oct_JoystickIDToArrayIndex(event.gamepadButtonEvent.which) == INVALID_GAMEPAD) continue;
            if (event.gamepadButtonEvent.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
                gGamepadButtonsCurrent[_oct_JoystickIDToArrayIndex(event.gamepadButtonEvent.which)][event.gamepadButtonEvent.button] = false;
            } else if (event.gamepadButtonEvent.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                gGamepadButtonsCurrent[_oct_JoystickIDToArrayIndex(event.gamepadButtonEvent.which)][event.gamepadButtonEvent.button] = true;
            }
        // Gamepad axis -- TODO: Triggers are fucked
        } else if (event.type == OCT_WINDOW_EVENT_TYPE_GAMEPAD_AXIS) {
            if (_oct_JoystickIDToArrayIndex(event.gamepadAxisEvent.which) == INVALID_GAMEPAD) continue;
            if (event.gamepadAxisEvent.axis == SDL_GAMEPAD_AXIS_LEFTX)
                gLeftAxisX[_oct_JoystickIDToArrayIndex(event.gamepadAxisEvent.which)] = (float)event.gamepadAxisEvent.value / (float)SDL_MAX_SINT16;
            else if (event.gamepadAxisEvent.axis == SDL_GAMEPAD_AXIS_LEFTY)
                gLeftAxisY[_oct_JoystickIDToArrayIndex(event.gamepadAxisEvent.which)] = (float)event.gamepadAxisEvent.value / (float)SDL_MAX_SINT16;
            else if (event.gamepadAxisEvent.axis == SDL_GAMEPAD_AXIS_RIGHTX)
                gRightAxisX[_oct_JoystickIDToArrayIndex(event.gamepadAxisEvent.which)] = (float)event.gamepadAxisEvent.value / (float)SDL_MAX_SINT16;
            else if (event.gamepadAxisEvent.axis == SDL_GAMEPAD_AXIS_RIGHTY)
                gRightAxisY[_oct_JoystickIDToArrayIndex(event.gamepadAxisEvent.which)] = (float)event.gamepadAxisEvent.value / (float)SDL_MAX_SINT16;
            else if (event.gamepadAxisEvent.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER)
                gLeftTrigger[_oct_JoystickIDToArrayIndex(event.gamepadAxisEvent.which)] = (float)event.gamepadAxisEvent.value / (float)SDL_MAX_SINT16;
            else if (event.gamepadAxisEvent.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
                gRightTrigger[_oct_JoystickIDToArrayIndex(event.gamepadAxisEvent.which)] = (float)event.gamepadAxisEvent.value / (float)SDL_MAX_SINT16;
        // Add/remove gamepads
        } else if (event.type == OCT_WINDOW_EVENT_TYPE_GAMEPAD_EVENT) {
            if (event.gamepadDeviceEvent.type == SDL_EVENT_GAMEPAD_ADDED) {
                for (int i = 0; i < GAMEPAD_COUNT; i++) {
                    if (gGamepadJoyIDMappings[i] == INVALID_GAMEPAD) {
                        gGamepadJoyIDMappings[i] = event.gamepadDeviceEvent.which;
                        break;
                    }
                }
            } else if (event.gamepadDeviceEvent.type == SDL_EVENT_GAMEPAD_REMOVED) {
                for (int i = 0; i < GAMEPAD_COUNT; i++) {
                    if (gGamepadJoyIDMappings[i] == event.gamepadDeviceEvent.which) {
                        gGamepadJoyIDMappings[i] = INVALID_GAMEPAD;
                        break;
                    }
                }
            }
        }
    }
}

void _oct_InputEnd(Oct_Context ctx) {
    // Nothing needed here yet
}

OCTARINE_API Oct_Bool oct_KeyDown(Oct_Key key) {
    return gKeysCurrent[key];
}

OCTARINE_API Oct_Bool oct_KeyPressed(Oct_Key key) {
    return gKeysCurrent[key] && !gKeysPrevious[key];
}

OCTARINE_API Oct_Bool oct_KeyReleased(Oct_Key key) {
    return !gKeysCurrent[key] && gKeysPrevious[key];
}

OCTARINE_API Oct_Bool oct_MouseButtonDown(Oct_MouseButton key) {
    return gMouseButtonsCurrent[key];
}

OCTARINE_API Oct_Bool oct_MouseButtonPressed(Oct_MouseButton key) {
    return gMouseButtonsCurrent[key] && !gMouseButtonsPrevious[key];
}

OCTARINE_API Oct_Bool oct_MouseButtonReleased(Oct_MouseButton key) {
    return !gMouseButtonsCurrent[key] && gMouseButtonsPrevious[key];
}

OCTARINE_API float oct_MouseX() {
    return gMouseX;
}

OCTARINE_API float oct_MouseY() {
    return gMouseY;
}

OCTARINE_API void oct_GamepadSetAxisDeadzone(float deadzone) {
    gAxisDeadzone = deadzone;
}

OCTARINE_API float oct_GamepadGetAxisDeadzone() {
    return gAxisDeadzone;
}

OCTARINE_API void oct_GamepadSetTriggerDeadzone(float deadzone) {
    gTriggerDeadzone = deadzone;
}

OCTARINE_API float oct_GamepadGetTriggerDeadzone() {
    return gTriggerDeadzone;
}

OCTARINE_API Oct_Bool oct_GamepadButtonDown(int index, Oct_GamepadButton key) {
    return gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? gGamepadButtonsCurrent[index][key] : 0;
}

OCTARINE_API Oct_Bool oct_GamepadButtonPressed(int index, Oct_GamepadButton key) {
    return gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? gGamepadButtonsCurrent[index][key] && !gGamepadButtonsPrevious[index][key] : 0;
}

OCTARINE_API Oct_Bool oct_GamepadButtonReleased(int index, Oct_GamepadButton key) {
    return gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? !gGamepadButtonsCurrent[index][key] && gGamepadButtonsPrevious[index][key] : 0;
}

OCTARINE_API float oct_GamepadLeftAxisX(int index) {
    return _oct_AccountForAxisDeadzone(gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? gLeftAxisX[index] : 0);
}

OCTARINE_API float oct_GamepadLeftAxisY(int index) {
    return _oct_AccountForAxisDeadzone(gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? gLeftAxisY[index] : 0);
}

OCTARINE_API float oct_GamepadRightAxisX(int index) {
    return _oct_AccountForAxisDeadzone(gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? gRightAxisX[index] : 0);
}

OCTARINE_API float oct_GamepadRightAxisY(int index) {
    return _oct_AccountForAxisDeadzone(gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? gRightAxisY[index] : 0);
}

OCTARINE_API float oct_GamepadRightTrigger(int index) {
    return _oct_AccountForTriggerDeadzone(gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? gRightTrigger[index] : 0);
}

OCTARINE_API float oct_GamepadLeftTrigger(int index) {
    return _oct_AccountForTriggerDeadzone(gGamepadJoyIDMappings[index] != INVALID_GAMEPAD ? gLeftTrigger[index] : 0);
}

OCTARINE_API Oct_Bool oct_GamepadConnected(int index) {
    return gGamepadJoyIDMappings[index] != INVALID_GAMEPAD;
}