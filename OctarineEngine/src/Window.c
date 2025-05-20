#include "oct/Window.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"
#include "oct/CommandBuffer.h"
#include "oct/Subsystems.h"

#define GAMEPAD_LIMIT 100

// Wraps an index, returns index + 1 unless index == len - 1 in which case it returns 0
static inline int32_t nextIndex(int32_t index, int32_t len) {
    return index == len - 1 ? 0 : index + 1;
}

// Globals for event ringbuffer
SDL_AtomicInt gRingHead; // Reading index
SDL_AtomicInt gRingTail; // Writing index
Oct_WindowEvent *gRingBuffer; // Event ring buffer
SDL_Gamepad *gControllers[GAMEPAD_LIMIT];

// Globals for communicating window things
SDL_AtomicInt gWindowWidth; // Current window width
SDL_AtomicInt gWindowHeight; // Current window height
SDL_AtomicInt gWindowFullscreen; // Whether or not the window is currently fullscreen

// Pushes an event to the event ringbuffer, might be blocking if the logic thread is falling behind
static void _oct_WindowPush(Oct_WindowEvent *event) {
    int tail = SDL_GetAtomicInt(&gRingTail);
    while (nextIndex(tail, OCT_RING_BUFFER_SIZE) == SDL_GetAtomicInt(&gRingHead)) {
        volatile int i;
    }

    // Insert new element
    memcpy(&gRingBuffer[tail], event, sizeof(struct Oct_WindowEvent_t));
    SDL_SetAtomicInt(&gRingTail, nextIndex(tail, OCT_RING_BUFFER_SIZE));
}

// Refreshes game controllers
static void _oct_RefreshControllers() {
    for (int i = 0; i < GAMEPAD_LIMIT; i++) {
        if (gControllers[i] != NULL) {
            SDL_CloseGamepad(gControllers[i]);
            gControllers[i] = NULL;
        }
    }

    int gamepad = 0;
    int numJoysticks;
    SDL_JoystickID *joys = SDL_GetJoysticks(&numJoysticks);
    for (int i = 0; i < numJoysticks && gamepad < GAMEPAD_LIMIT; i++) {
        if (SDL_IsGamepad(i)) {
            SDL_Gamepad *controller = SDL_OpenGamepad(joys[i]);
            if (controller == NULL) {
                oct_Raise(OCT_STATUS_SDL_ERROR, false, "Failed to open controller with joy index %i, SDL error: %s", i, SDL_GetError());
            } else {
                gControllers[gamepad] = controller;
                gamepad++;
            }
        }
    }
}

void _oct_WindowInit() {
    Oct_Context ctx = _oct_GetCtx();
    ctx->window = SDL_CreateWindow(
            ctx->initInfo->windowTitle,
            ctx->initInfo->windowWidth,
            ctx->initInfo->windowHeight,
            SDL_WINDOW_VULKAN
    );
    if (!ctx->window) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create window. SDL Error \"%s\"", SDL_GetError());
    }

    gRingBuffer = mi_malloc(sizeof(struct Oct_WindowEvent_t) * OCT_RING_BUFFER_SIZE);
    if (!gRingBuffer)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate events ring buffer.");

    _oct_RefreshControllers();

    oct_Log("Window system initialized.");
}

void _oct_WindowEnd() {
    Oct_Context ctx = _oct_GetCtx();
    mi_free(gRingBuffer);
    SDL_DestroyWindow(ctx->window);
}

void _oct_WindowUpdateBegin() {
    Oct_Context ctx = _oct_GetCtx();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            SDL_SetAtomicInt(&ctx->quit, 1);
        } else if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_KEYBOARD,
                    .keyboardEvent = e.key
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_MOUSE_BUTTON,
                    .mouseButtonEvent = e.button
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_EVENT_MOUSE_WHEEL) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_MOUSE_WHEEL,
                    .mouseWheelEvent = e.wheel
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_EVENT_MOUSE_MOTION) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_MOUSE_MOTION,
                    .mouseMotionEvent = e.motion
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || e.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_GAMEPAD_BUTTON,
                    .gamepadButtonEvent = e.gbutton
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_EVENT_GAMEPAD_ADDED || e.type == SDL_EVENT_GAMEPAD_REMOVED) {
            _oct_RefreshControllers();
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_GAMEPAD_EVENT,
                    .gamepadDeviceEvent = e.gdevice
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_GAMEPAD_AXIS,
                    .gamepadAxisEvent = e.gaxis
            };
            _oct_WindowPush(&event);
        }
    }

    // Update window variables
    int w, h;
    SDL_GetWindowSize(ctx->window, &w, &h);
    SDL_SetAtomicInt(&gWindowWidth, w);
    SDL_SetAtomicInt(&gWindowHeight, h);
    SDL_SetAtomicInt(&gWindowFullscreen, (SDL_GetWindowFlags(ctx->window) & SDL_WINDOW_FULLSCREEN) != 0);
}

void _oct_WindowUpdateEnd() {
    // TODO: This
}

void _oct_WindowProcessCommand(Oct_Command *cmd) {
    Oct_Context ctx = _oct_GetCtx();

    if (OCT_STRUCTURE_TYPE(&cmd->topOfUnion) == OCT_STRUCTURE_TYPE_META_COMMAND) {
        // Meta commands are currently of no interest to the windowing subsystem
    } else {
        if (cmd->windowCommand.type == OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_ENTER) {
            SDL_SetWindowFullscreen(ctx->window, true);
        } else if (cmd->windowCommand.type == OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_EXIT) {
            SDL_SetWindowFullscreen(ctx->window, false);
        } else if (cmd->windowCommand.type == OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_TOGGLE) {
            Oct_Bool fs = SDL_GetWindowFlags(ctx->window) & SDL_WINDOW_FULLSCREEN;
            SDL_SetWindowFullscreen(ctx->window, !fs);
        } else if (cmd->windowCommand.type == OCT_WINDOW_COMMAND_TYPE_RESIZE) {
            SDL_SetWindowSize(ctx->window, cmd->windowCommand.Resize.size[0], cmd->windowCommand.Resize.size[1]);
        }
    }
}

Oct_Bool _oct_WindowPopEvent(Oct_WindowEvent *event) {
    int head = SDL_GetAtomicInt(&gRingHead);

    // There are no new events in the ring buffer
    if (head == SDL_GetAtomicInt(&gRingTail))
        return false;

    // Pull out element from the ring buffer
    memcpy(event, &gRingBuffer[head], sizeof(struct Oct_WindowEvent_t));
    SDL_SetAtomicInt(&gRingHead, nextIndex(head, OCT_RING_BUFFER_SIZE));
    return true;
}

OCTARINE_API float oct_WindowWidth() {
    return (float)SDL_GetAtomicInt(&gWindowWidth);
}

OCTARINE_API float oct_WindowHeight() {
    return (float)SDL_GetAtomicInt(&gWindowHeight);
}

OCTARINE_API Oct_Bool oct_WindowIsFullscreen() {
    return (Oct_Bool)SDL_GetAtomicInt(&gWindowFullscreen);
}

OCTARINE_API void oct_SetFullscreen(Oct_Bool fullscreen) {
    Oct_WindowCommand c = {
            .type = fullscreen ? OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_ENTER : OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_EXIT,
    };
    oct_WindowUpdate(&c);
}

OCTARINE_API void oct_ToggleFullscreen() {
    Oct_WindowCommand c = {
            .type = OCT_WINDOW_COMMAND_TYPE_FULLSCREEN_TOGGLE,
    };
    oct_WindowUpdate(&c);
}

OCTARINE_API void oct_ResizeWindow(float width, float height) {
    Oct_WindowCommand c = {
            .type = OCT_WINDOW_COMMAND_TYPE_RESIZE,
            .Resize.size = {width, height}
    };
    oct_WindowUpdate(&c);
}