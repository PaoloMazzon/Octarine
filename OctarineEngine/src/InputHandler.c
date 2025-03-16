#include <SDL2/SDL.h>
#include "oct/InputHandler.h"
#include "oct/Subsystems.h"
#include "oct/Opaque.h"

// Globals
Oct_Bool gKeysCurrent[SDL_NUM_SCANCODES];
Oct_Bool gKeysPrevious[SDL_NUM_SCANCODES];
Oct_Bool gGamepadButtonsCurrent[4][OCT_GAMEPAD_BUTTON_MAX];
Oct_Bool gGamepadButtonsPrevious[4][OCT_GAMEPAD_BUTTON_MAX];
Oct_Bool gMouseButtonsCurrent[OCT_MOUSE_BUTTON_MAX];
Oct_Bool gMouseButtonsPrevious[OCT_MOUSE_BUTTON_MAX];
float gMouseX;
float gMouseY;
float gLeftAxisX[4];
float gLeftAxisY[4];
float gRightAxisX[4];
float gRightAxisY[4];

void _oct_InputInit(Oct_Context ctx) {
    // TODO: This
}

void _oct_InputUpdate(Oct_Context ctx) {
    // Copy previous buffers
    memcpy(gKeysPrevious, gKeysCurrent, SDL_NUM_SCANCODES * sizeof(Oct_Bool));
    memcpy(gGamepadButtonsPrevious[0], gGamepadButtonsCurrent[0], OCT_GAMEPAD_BUTTON_MAX * sizeof(Oct_Bool));
    memcpy(gGamepadButtonsPrevious[1], gGamepadButtonsCurrent[1], OCT_GAMEPAD_BUTTON_MAX * sizeof(Oct_Bool));
    memcpy(gGamepadButtonsPrevious[2], gGamepadButtonsCurrent[2], OCT_GAMEPAD_BUTTON_MAX * sizeof(Oct_Bool));
    memcpy(gGamepadButtonsPrevious[3], gGamepadButtonsCurrent[3], OCT_GAMEPAD_BUTTON_MAX * sizeof(Oct_Bool));
    memcpy(gMouseButtonsPrevious, gMouseButtonsCurrent, OCT_MOUSE_BUTTON_MAX * sizeof(Oct_Bool));

    // Process events
    Oct_WindowEvent event;
    while (_oct_WindowPopEvent(ctx, &event)) {
        // Keyboard event
        if (event.type == OCT_WINDOW_EVENT_TYPE_KEYBOARD) {
            if (event.keyboardEvent.type == SDL_KEYDOWN) {
                gKeysCurrent[event.keyboardEvent.keysym.scancode] = true;
            } else if (event.keyboardEvent.type == SDL_KEYUP) {
                gKeysCurrent[event.keyboardEvent.keysym.scancode] = false;
            }
        // Mouse movement
        } else if (event.type == OCT_WINDOW_EVENT_TYPE_MOUSE_MOTION) {
            gMouseX = (float)event.mouseMotionEvent.x;
            gMouseY = (float)event.mouseMotionEvent.y;
        }
        // TODO: The rest
    }
}

void _oct_InputEnd(Oct_Context ctx) {
    // TODO: This
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

OCTARINE_API Oct_Bool oct_GamepadButtonDown(int index, Oct_GamepadButton key) {
    return gGamepadButtonsCurrent[index][key];
}

OCTARINE_API Oct_Bool oct_GamepadButtonPressed(int index, Oct_GamepadButton key) {
    return gGamepadButtonsCurrent[index][key] && !gGamepadButtonsPrevious[index][key];
}

OCTARINE_API Oct_Bool oct_GamepadButtonReleased(int index, Oct_GamepadButton key) {
    return !gGamepadButtonsCurrent[index][key] && gGamepadButtonsPrevious[index][key];
}

OCTARINE_API float oct_GamepadLeftAxisX(int index) {
    return gLeftAxisX[index]; // TODO: Return 0 if the controller is disconnected
}

OCTARINE_API float oct_GamepadLeftAxisY(int index) {
    return gLeftAxisY[index]; // TODO: Return 0 if the controller is disconnected
}

OCTARINE_API float oct_GamepadRightAxisX(int index) {
    return gRightAxisX[index]; // TODO: Return 0 if the controller is disconnected
}

OCTARINE_API float oct_GamepadRightAxisY(int index) {
    return gRightAxisY[index]; // TODO: Return 0 if the controller is disconnected
}

OCTARINE_API Oct_Bool oct_GamepadConnected(int index) {
    return 0; // TODO: This
}
