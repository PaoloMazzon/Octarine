#include <SDL2/SDL.h>
#include "oct/InputHandler.h"

// Globals
Oct_Bool gKeysCurrent[SDL_NUM_SCANCODES];
Oct_Bool gKeysPrevious[SDL_NUM_SCANCODES];
Oct_Bool gGamepadButtonsCurrent[OCT_GAMEPAD_BUTTON_MAX];
Oct_Bool gGamepadButtonsPrevious[OCT_GAMEPAD_BUTTON_MAX];
Oct_Bool gMouseButtonsCurrent[OCT_MOUSE_BUTTON_MAX];
Oct_Bool gMouseButtonsPrevious[OCT_MOUSE_BUTTON_MAX];

void _oct_InputInit(Oct_Context ctx) {
    // TODO: This
}

void _oct_InputUpdate(Oct_Context ctx) {
    // TODO: This
}

void _oct_InputEnd(Oct_Context ctx) {
    // TODO: This
}

OCTARINE_API Oct_Bool oct_KeyDown(Oct_Key key) {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_KeyPressed(Oct_Key key) {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_KeyReleased(Oct_Key key) {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_MouseButtonDown(Oct_MouseButton key) {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_MouseButtonPressed(Oct_MouseButton key) {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_MouseButtonReleased(Oct_MouseButton key) {
    return 0; // TODO: This
}

OCTARINE_API float oct_MouseX() {
    return 0; // TODO: This
}

OCTARINE_API float oct_MouseY() {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_GamepadButtonDown(int index, Oct_GamepadButton key) {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_GamepadButtonPressed(int index, Oct_GamepadButton key) {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_GamepadButtonReleased(int index, Oct_GamepadButton key) {
    return 0; // TODO: This
}

OCTARINE_API float oct_GamepadLeftAxisX(int index) {
    return 0; // TODO: This
}

OCTARINE_API float oct_GamepadLeftAxisY(int index) {
    return 0; // TODO: This
}

OCTARINE_API float oct_GamepadRightAxisX(int index) {
    return 0; // TODO: This
}

OCTARINE_API float oct_GamepadRightAxisY(int index) {
    return 0; // TODO: This
}

OCTARINE_API Oct_Bool oct_GamepadConnected(int index) {
    return 0; // TODO: This
}
