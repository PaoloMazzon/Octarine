#include <VK2D/VK2D.h>
#include "oct/Drawing.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"
#include "oct/Subsystems.h"

// Commands are tied to the frame they came from for interpolation and triple buffering
typedef struct FrameCommandBuffer_t {
    Oct_DrawCommand *commands; ///< Internal command list
    int size;                  ///< Size of the internal buffer
    int count;                 ///< Number of commands
} FrameCommandBuffer;

// Wraps an index (0 = 1, 1 = 2, 2 = 0)
#define NEXT_INDEX(i) (i == 2 ? 0 : i + 1)
#define PREVIOUS_INDEX(i) (i == 0 ? 2 : i - 1)
#define PREVIOUS_DRAW_FRAME (gCurrentFrame == 2 ? 0 : gCurrentFrame + 1)
#define CURRENT_DRAW_FRAME (gCurrentFrame == 0 ? 2 : gCurrentFrame - 1)

// Globals
static FrameCommandBuffer gFrameBuffers[3]; // Triple buffer
static int gCurrentFrame; // Current frame is the one not being interpolated

///////////////////// Internal functions /////////////////////
// Adds a command to the current frame buffer, expanding if necessary
void addCommand(Oct_DrawCommand *cmd) {
    // Need more buffer space
    if (gFrameBuffers[gCurrentFrame].size == gFrameBuffers[gCurrentFrame].count) {
        gFrameBuffers[gCurrentFrame].size *= 2;
        gFrameBuffers[gCurrentFrame].commands = mi_realloc(gFrameBuffers[gCurrentFrame].commands, gFrameBuffers[gCurrentFrame].size * sizeof(struct Oct_DrawCommand_t));
        if (!gFrameBuffers[gCurrentFrame].commands) {
            oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to expand frame buffer.");
        }
    }
    memcpy(&gFrameBuffers[gCurrentFrame].commands[gFrameBuffers[gCurrentFrame].count++], cmd, sizeof(struct Oct_DrawCommand_t));
}

///////////////////// Subsystem /////////////////////
void _oct_DrawingInit(Oct_Context ctx) {
    VK2DRendererConfig config = {
            .msaa = VK2D_MSAA_32X,
            .filterMode = VK2D_FILTER_TYPE_NEAREST,
            .screenMode = VK2D_SCREEN_MODE_TRIPLE_BUFFER
    };
    VK2DStartupOptions options = {
            .errorFile = "octarinedump.log",
            .quitOnError = true,
            .stdoutLogging = false,
            .enableDebug = false
    };
    vk2dRendererInit(ctx->window, config, &options);

    // Allocate frame buffers
    for (int i = 0; i < 3; i++) {
        gFrameBuffers[i].size = ctx->initInfo->ringBufferSize;
        gFrameBuffers[i].commands = mi_malloc(gFrameBuffers[i].size * sizeof(struct Oct_DrawCommand_t));
    }
}

void _oct_DrawingEnd(Oct_Context ctx) {
    // Free frame buffers
    for (int i = 0; i < 3; i++) {
        mi_free(gFrameBuffers[i].commands);
    }

    vk2dRendererQuit();
}

void _oct_DrawingUpdateBegin(Oct_Context ctx) {
    vk2dRendererStartFrame(VK2D_BLACK);
}

void _oct_DrawingProcessCommand(Oct_Context ctx, Oct_Command *cmd) {
    if (OCT_STRUCTURE_TYPE(&cmd->topOfUnion) == OCT_STRUCTURE_TYPE_META_COMMAND) {
        if (cmd->metaCommand.type == OCT_META_COMMAND_TYPE_END_FRAME) {
            gCurrentFrame = NEXT_INDEX(gCurrentFrame);
            gFrameBuffers[gCurrentFrame].count = 0;
        }
    } else {
        addCommand(&cmd->drawCommand);
    }
}

static inline float lerp(float x, float min, float max) {
    return (x * (max - min)) + min;
}

/////////////////////////////// DRAWING COMMANDS ///////////////////////////////
#define interpolate(interpolateFlags, prevCmd, time, min, max) (prevCmd) && (interpolateFlags) ? lerp(time, min, max) : (max)

static void _oct_ProcessOrigin(Oct_Vec2 origin, Oct_Vec2 out, float width, float height) {
    if (origin[0] == OCT_ORIGIN_MIDDLE) {
        out[0] = width / 2;
    } else if (origin[0] == OCT_ORIGIN_RIGHT) {
        origin[0] = width;
    }
    if (origin[1] == OCT_ORIGIN_MIDDLE) {
        out[1] = height / 2;
    } else if (origin[1] == OCT_ORIGIN_RIGHT) {
        out[1] = height;
    }
}

static void _oct_DrawRectangle(Oct_Context ctx, Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    // Process interpolation
    Oct_Vec2 position;
    Oct_Vec2 origin;
    float rotation;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Rectangle.rectangle.position[0], cmd->Rectangle.rectangle.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Rectangle.rectangle.position[1], cmd->Rectangle.rectangle.position[1]);
    rotation = interpolate(cmd->interpolate & OCT_INTERPOLATE_ROTATION, prevCmd, interpolatedTime, prevCmd->Rectangle.rotation, cmd->Rectangle.rotation);

    _oct_ProcessOrigin(cmd->Rectangle.origin, origin, cmd->Rectangle.rectangle.size[0], cmd->Rectangle.rectangle.size[1]);

    if (cmd->Rectangle.filled) {
        vk2dRendererDrawRectangle(
                position[0] - origin[0],
                position[1] - origin[1],
                cmd->Rectangle.rectangle.size[0],
                cmd->Rectangle.rectangle.size[1],
                rotation,
                origin[0],
                origin[1]
        );
    } else {
        vk2dRendererDrawRectangleOutline(
                position[0] - origin[0],
                position[1] - origin[1],
                cmd->Rectangle.rectangle.size[0],
                cmd->Rectangle.rectangle.size[1],
                rotation,
                origin[0],
                origin[1],
                cmd->Rectangle.lineSize
        );
    }
}

static void _oct_DrawCircle(Oct_Context ctx, Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    // Process interpolation
    Oct_Vec2 position;
    float radius;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Circle.circle.position[0], cmd->Circle.circle.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Circle.circle.position[1], cmd->Circle.circle.position[1]);
    radius = interpolate(cmd->interpolate & OCT_INTERPOLATE_ROTATION, prevCmd, interpolatedTime, prevCmd->Circle.circle.radius, cmd->Circle.circle.radius);

    if (cmd->Circle.filled) {
        vk2dRendererDrawCircle(
                position[0],
                position[1],
                radius
        );
    } else {
        vk2dRendererDrawCircleOutline(
                position[0],
                position[1],
                radius,
                cmd->Circle.lineSize
        );
    }
}

static void _oct_DrawTexture(Oct_Context ctx, Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    if (_oct_AssetType(ctx, cmd->Texture.texture) != OCT_ASSET_TYPE_TEXTURE)
        return;
    VK2DTexture tex = _oct_AssetGet(ctx, cmd->Texture.texture)->texture;

    // Process interpolation
    Oct_Vec2 position;
    Oct_Vec2 scale;
    Oct_Vec2 origin;
    float rotation;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Texture.position[0], cmd->Texture.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Texture.position[1], cmd->Texture.position[1]);
    scale[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_X, prevCmd, interpolatedTime, prevCmd->Texture.scale[0], cmd->Texture.scale[0]);
    scale[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_Y, prevCmd, interpolatedTime, prevCmd->Texture.scale[1], cmd->Texture.scale[1]);
    rotation = interpolate(cmd->interpolate & OCT_INTERPOLATE_ROTATION, prevCmd, interpolatedTime, prevCmd->Texture.rotation, cmd->Texture.rotation);

    // Process origin
    _oct_ProcessOrigin(cmd->Texture.origin, origin, vk2dTextureWidth(tex), vk2dTextureHeight(tex));

        // Draw texture
    vk2dRendererDrawTexture(
            tex,
            position[0] - origin[0],
            position[1] - origin[1],
            scale[0],
            scale[1],
            rotation,
            origin[0],
            origin[1],
            cmd->Texture.viewport.position[0],
            cmd->Texture.viewport.position[1],
            cmd->Texture.viewport.size[0] == OCT_WHOLE_TEXTURE ? vk2dTextureWidth(tex) : cmd->Texture.viewport.size[0],
            cmd->Texture.viewport.size[1] == OCT_WHOLE_TEXTURE ? vk2dTextureHeight(tex) : cmd->Texture.viewport.size[1]
    );
}

void _oct_DrawingUpdateEnd(Oct_Context ctx) {
    int atomic = SDL_GetAtomicInt(&ctx->interpolatedTime);
    float interpolatedTime = OCT_INT_TO_FLOAT(atomic);

    // Interpolate/draw current frame's commands
    for (int i = 0; i < gFrameBuffers[CURRENT_DRAW_FRAME].count; i++) {
        Oct_DrawCommand *cmd = &gFrameBuffers[CURRENT_DRAW_FRAME].commands[i];
        vk2dRendererSetColourMod((float*)&cmd->colour);

        // If its interpolated, find the corresponding command
        Oct_DrawCommand *prevCmd = null;
        if (cmd->interpolate) {
            for (int j = 0; prevCmd == null && j < gFrameBuffers[PREVIOUS_DRAW_FRAME].count; j++)
                if (gFrameBuffers[PREVIOUS_DRAW_FRAME].commands[j].id == cmd->id && cmd->type == gFrameBuffers[PREVIOUS_DRAW_FRAME].commands[j].type)
                    prevCmd = &gFrameBuffers[PREVIOUS_DRAW_FRAME].commands[j];
        }

        if (cmd->type == OCT_DRAW_COMMAND_TYPE_RECTANGLE) {
            _oct_DrawRectangle(ctx, cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_TEXTURE) {
            _oct_DrawTexture(ctx, cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_CIRCLE) {
            _oct_DrawCircle(ctx, cmd, prevCmd, interpolatedTime);
        } // TODO: Implement other command types
    }

    vk2dRendererEndFrame();
}
