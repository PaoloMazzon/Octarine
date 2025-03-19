#include "oct/Window.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"

#define GAMEPAD_LIMIT 100

// Wraps an index, returns index + 1 unless index == len - 1 in which case it returns 0
static inline int32_t nextIndex(int32_t index, int32_t len) {
    return index == len - 1 ? 0 : index + 1;
}

// Globals for event ringbuffer
SDL_atomic_t gRingHead; // Reading index
SDL_atomic_t gRingTail; // Writing index
Oct_WindowEvent *gRingBuffer; // Event ring buffer
SDL_GameController *gControllers[GAMEPAD_LIMIT];

// Pushes an event to the event ringbuffer, might be blocking if the logic thread is falling behind
static void _oct_WindowPush(Oct_WindowEvent *event) {
    int tail = SDL_AtomicGet(&gRingTail);
    while (nextIndex(tail, OCT_RING_BUFFER_SIZE) == SDL_AtomicGet(&gRingHead)) {
        volatile int i;
    }

    // Insert new element
    memcpy(&gRingBuffer[tail], event, sizeof(struct Oct_WindowEvent_t));
    SDL_AtomicSet(&gRingTail, nextIndex(tail, OCT_RING_BUFFER_SIZE));
}

// Refreshes game controllers
static void _oct_RefreshControllers(Oct_Context ctx) {
    for (int i = 0; i < GAMEPAD_LIMIT; i++) {
        if (gControllers[i] != NULL) {
            SDL_GameControllerClose(gControllers[i]);
            gControllers[i] = NULL;
        }
    }

    int gamepad = 0;
    for (int i = 0; i < SDL_NumJoysticks() && gamepad < GAMEPAD_LIMIT; i++) {
        if (SDL_IsGameController(i)) {
            SDL_GameController *controller = SDL_GameControllerOpen(i);
            if (controller == NULL) {
                oct_Raise(OCT_STATUS_SDL_ERROR, false, "Failed to open controller with joy index %i, SDL error: %s", i, SDL_GetError());
            } else {
                gControllers[gamepad] = controller;
                gamepad++;
            }
        }
    }
}

void _oct_WindowInit(Oct_Context ctx) {
    ctx->window = SDL_CreateWindow(
            ctx->initInfo->windowTitle,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
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

    _oct_RefreshControllers(ctx);
}

void _oct_WindowEnd(Oct_Context ctx) {
    mi_free(gRingBuffer);
    SDL_DestroyWindow(ctx->window);
}

void _oct_WindowUpdateBegin(Oct_Context ctx) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            SDL_AtomicSet(&ctx->quit, 1);
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_KEYBOARD,
                    .keyboardEvent = e.key
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_MOUSE_BUTTON,
                    .mouseButtonEvent = e.button
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_MOUSEWHEEL) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_MOUSE_WHEEL,
                    .mouseWheelEvent = e.wheel
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_MOUSEMOTION) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_MOUSE_MOTION,
                    .mouseMotionEvent = e.motion
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_GAMEPAD_BUTTON,
                    .gamepadButtonEvent = e.cbutton
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_CONTROLLERDEVICEADDED || e.type == SDL_CONTROLLERDEVICEREMOVED) {
            _oct_RefreshControllers(ctx);
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_GAMEPAD_EVENT,
                    .gamepadDeviceEvent = e.cdevice
            };
            _oct_WindowPush(&event);
        } else if (e.type == SDL_CONTROLLERAXISMOTION) {
            Oct_WindowEvent event = {
                    .type = OCT_WINDOW_EVENT_TYPE_GAMEPAD_AXIS,
                    .gamepadAxisEvent = e.caxis
            };
            _oct_WindowPush(&event);
        }
    }
}

void _oct_WindowUpdateEnd(Oct_Context ctx) {
    // TODO: This
}

void _oct_WindowProcessCommand(Oct_Context ctx, Oct_Command *cmd) {
    // TODO: This
}

Oct_Bool _oct_WindowPopEvent(Oct_Context ctx, Oct_WindowEvent *event) {
    int head = SDL_AtomicGet(&gRingHead);

    // There are no new events in the ring buffer
    if (head == SDL_AtomicGet(&gRingTail))
        return false;

    // Pull out element from the ring buffer
    memcpy(event, &gRingBuffer[head], sizeof(struct Oct_WindowEvent_t));
    SDL_AtomicSet(&gRingHead, nextIndex(head, OCT_RING_BUFFER_SIZE));
    return true;
}