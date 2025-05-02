#include <stdarg.h>
#include <VK2D/VK2D.h>
#include "oct/CommandBuffer.h"
#include "oct/Drawing.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"
#include "oct/Subsystems.h"
#include "oct/Core.h"
#include "oct/Assets.h"
#include "oct/Blobs.h"

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
static VK2DTexture gDebugFont; // Bitmap font of the debug texture

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
    VK2DResult result = vk2dRendererInit(ctx->window, config, &options);

    if (result == VK2D_ERROR)
        oct_Raise(OCT_STATUS_VK2D_ERROR, true, "Failed to create renderer. VK2D error: %s", vk2dStatusMessage());

    // Allocate frame buffers
    for (int i = 0; i < 3; i++) {
        gFrameBuffers[i].size = ctx->initInfo->ringBufferSize;
        gFrameBuffers[i].commands = mi_malloc(gFrameBuffers[i].size * sizeof(struct Oct_DrawCommand_t));
    }

    // Allocate debug font
    gDebugFont = vk2dTextureFrom((void*)FONT_PNG, sizeof(FONT_PNG));

    // Format host info nicely
    char copy[1024] = {0};
    strncpy(copy, vk2dHostInformation(), 1023);
    for (int i = 0; i < strlen(copy); i++) {
        if (copy[i] == '\n')
            copy[i] = ' ';
    }

    oct_Log("Drawing system initialized on \"%s\".", copy);
}

void _oct_DrawingEnd(Oct_Context ctx) {
    vk2dTextureFree(gDebugFont);

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

static void _oct_UpdateCamera(Oct_Context ctx, Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    // Update the camera if the user wishes to
    if (cmd->Camera.updateType & OCT_CAMERA_UPDATE_TYPE_UPDATE_CAMERA) {
        Oct_AssetData *data = _oct_AssetGetSafe(ctx, cmd->Camera.camera, OCT_ASSET_TYPE_CAMERA);
        if (!data)
            return;
        VK2DCameraIndex cam = data->camera;

        Oct_Vec2 position;
        Oct_Vec2 size;
        float rotation;
        position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime,
                                  prevCmd->Camera.cameraUpdate.position[0], cmd->Camera.cameraUpdate.position[0]);
        position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime,
                                  prevCmd->Camera.cameraUpdate.position[1], cmd->Camera.cameraUpdate.position[1]);
        size[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_WIDTH, prevCmd, interpolatedTime,
                              prevCmd->Camera.cameraUpdate.size[0], cmd->Camera.cameraUpdate.size[0]);
        size[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_HEIGHT, prevCmd, interpolatedTime,
                              prevCmd->Camera.cameraUpdate.size[1], cmd->Camera.cameraUpdate.size[1]);
        rotation = interpolate(cmd->interpolate & OCT_INTERPOLATE_ROTATION, prevCmd, interpolatedTime,
                               prevCmd->Camera.cameraUpdate.rotation, cmd->Camera.cameraUpdate.rotation);
        VK2DCameraSpec spec = {
                .type = VK2D_CAMERA_TYPE_DEFAULT,
                .x = position[0],
                .y = position[1],
                .w = size[0],
                .h = size[1],
                .xOnScreen = cmd->Camera.cameraUpdate.screenPosition[0],
                .yOnScreen = cmd->Camera.cameraUpdate.screenPosition[1],
                .wOnScreen = cmd->Camera.cameraUpdate.screenSize[0],
                .hOnScreen = cmd->Camera.cameraUpdate.screenSize[1],
                .rot = rotation,
                .zoom = 1
        };
        vk2dCameraUpdate(cam, spec);
    }

    // Lock/Unlock the camera
    if (cmd->Camera.updateType & OCT_CAMERA_UPDATE_TYPE_LOCK_CAMERA) {
        Oct_AssetData *data = _oct_AssetGetSafe(ctx, cmd->Camera.camera, OCT_ASSET_TYPE_CAMERA);
        if (!data)
            return;
        VK2DCameraIndex cam = data->camera;
        vk2dRendererLockCameras(cam);
    } else if (cmd->Camera.updateType & OCT_CAMERA_UPDATE_TYPE_UNLOCK_CAMERA) {
        vk2dRendererUnlockCameras();
    }

    if (cmd->Camera.updateType & OCT_CAMERA_UPDATE_TYPE_ENABLE_TEX_CAM) {
        vk2dRendererSetTextureCamera(true);
    } else if (cmd->Camera.updateType & OCT_CAMERA_UPDATE_TYPE_DISABLE_TEX_CAM) {
        vk2dRendererSetTextureCamera(false);
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
    Oct_AssetData *asset = _oct_AssetGetSafe(ctx, cmd->Texture.texture, OCT_ASSET_TYPE_TEXTURE);
    if (!asset)
        return;
    VK2DTexture tex = asset->texture;

    // Process interpolation
    Oct_Vec2 position;
    Oct_Vec2 scale;
    Oct_Vec2 origin = {0, 0};
    float rotation;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Texture.position[0], cmd->Texture.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Texture.position[1], cmd->Texture.position[1]);
    scale[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_X, prevCmd, interpolatedTime, prevCmd->Texture.scale[0], cmd->Texture.scale[0]);
    scale[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_Y, prevCmd, interpolatedTime, prevCmd->Texture.scale[1], cmd->Texture.scale[1]);
    rotation = interpolate(cmd->interpolate & OCT_INTERPOLATE_ROTATION, prevCmd, interpolatedTime, prevCmd->Texture.rotation, cmd->Texture.rotation);

    // Find viewport
    const float w = cmd->Texture.viewport.size[0] == OCT_WHOLE_TEXTURE ? vk2dTextureWidth(tex) : cmd->Texture.viewport.size[0];
    const float h = cmd->Texture.viewport.size[1] == OCT_WHOLE_TEXTURE ? vk2dTextureHeight(tex) : cmd->Texture.viewport.size[1];

    // Process origin
    _oct_ProcessOrigin(cmd->Texture.origin, origin, w, h);

    // Draw texture
    vk2dRendererDrawTexture(
            tex,
            position[0] - (origin[0] * cmd->Texture.scale[0]),
            position[1] - (origin[1] * cmd->Texture.scale[1]),
            scale[0],
            scale[1],
            rotation,
            origin[0],
            origin[1],
            cmd->Texture.viewport.position[0],
            cmd->Texture.viewport.position[1],
            w,
            h
    );
}

static void _oct_DrawSprite(Oct_Context ctx, Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    Oct_AssetData *asset = _oct_AssetGetSafe(ctx, cmd->Sprite.sprite, OCT_ASSET_TYPE_SPRITE);
    if (!asset)
        return;
    Oct_SpriteData *spr = &asset->sprite;

    // Make sure the sprite's texture still exists
    VK2DTexture tex = null;
    Oct_AssetData *texData = _oct_AssetGetSafe(ctx, spr->texture, OCT_ASSET_TYPE_TEXTURE);
    if (oct_AssetLoaded(spr->texture)) {
        tex = texData->texture;
    } else {
        oct_Raise(OCT_STATUS_BAD_PARAMETER, true, "Sprite ID %" PRIu64 " uses a texture that no longer exists.");
        return;
    }

    // Process interpolation
    Oct_Vec2 position;
    Oct_Vec2 scale;
    Oct_Vec2 origin = {0, 0};
    float rotation;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Sprite.position[0], cmd->Sprite.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Sprite.position[1], cmd->Sprite.position[1]);
    scale[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_X, prevCmd, interpolatedTime, prevCmd->Sprite.scale[0], cmd->Sprite.scale[0]);
    scale[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_Y, prevCmd, interpolatedTime, prevCmd->Sprite.scale[1], cmd->Sprite.scale[1]);
    rotation = interpolate(cmd->interpolate & OCT_INTERPOLATE_ROTATION, prevCmd, interpolatedTime, prevCmd->Sprite.rotation, cmd->Sprite.rotation);

    // Find current frame
    int32_t frame = cmd->Sprite.frame;
    if (frame == OCT_SPRITE_CURRENT_FRAME) {
        frame = spr->frame;
    } else if (frame == OCT_SPRITE_LAST_FRAME) {
        frame = spr->frameCount - 1;
    } else {
        frame -= 1;
    }

    // Locate frame in the texture
    const int totalHorizontal = frame * (spr->frameSize[0] + spr->padding[0]);
    const int lineBreaks = (int)(spr->startPos[0] + totalHorizontal) / (int)(vk2dTextureWidth(tex) - spr->xStop);
    float x;
    if (lineBreaks == 0)
        x = (float)((int)(spr->startPos[0] + (totalHorizontal - (spr->padding[0] * lineBreaks))) % (int)(vk2dTextureWidth(tex) - spr->xStop));
    else
        x = (float)(spr->xStop + ((int)(spr->startPos[0] + (totalHorizontal - (spr->padding[0] * lineBreaks))) % (int)(vk2dTextureWidth(tex) - spr->xStop)));
    float y = lineBreaks * spr->frameSize[1];
    float w = spr->frameSize[0];
    float h = spr->frameSize[1];

    // Process the viewport for the sprite
    if (cmd->Sprite.viewport.size[0] != OCT_WHOLE_TEXTURE) {
        w = cmd->Sprite.viewport.size[0];
    }
    if (cmd->Sprite.viewport.size[1] != OCT_WHOLE_TEXTURE) {
        w = cmd->Sprite.viewport.size[1];
    }
    x += cmd->Sprite.viewport.position[0];
    y += cmd->Sprite.viewport.position[1];

    // Process origin
    _oct_ProcessOrigin(cmd->Sprite.origin, origin, w, h);

    // Draw sprite
    vk2dRendererDrawTexture(
            tex,
            position[0] - (origin[0] * cmd->Sprite.scale[0]),
            position[1] - (origin[1] * cmd->Sprite.scale[1]),
            scale[0],
            scale[1],
            rotation,
            origin[0],
            origin[1],
            x,
            y,
            w,
            h
    );

    // Process frame update
    if (!spr->pause && cmd->Sprite.frame == OCT_SPRITE_CURRENT_FRAME) {
        spr->accumulator += oct_Time(ctx) - spr->lastTime;
        if (spr->accumulator > spr->delay) {
            if (spr->frame < spr->frameCount - 1) {
                spr->frame += 1;
            } else if (spr->frame == spr->frameCount - 1 && spr->repeat) {
                spr->frame = 0;
            }
            spr->accumulator -= spr->delay;
        }
        spr->lastTime = oct_Time(ctx);
    }
}

static void _oct_DrawDebugFont(Oct_Context ctx, Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    // Process interpolation
    Oct_Vec2 position;
    float scale;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->DebugText.position[0], cmd->DebugText.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->DebugText.position[1], cmd->DebugText.position[1]);
    scale = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_X || cmd->interpolate & OCT_INTERPOLATE_SCALE_Y, prevCmd, interpolatedTime, prevCmd->DebugText.scale, cmd->DebugText.scale);

    // Render each character
    float x = position[0];
    const float width = 21;
    const float height = 24;
    for (int i = 0; i < strlen(cmd->DebugText.text); i++) {
        const float c = (float)cmd->DebugText.text[i] - 32;
        if (cmd->DebugText.text[i] == '\n') {
            position[0] = x;
            position[1] += height;
            continue;
        }
        vk2dRendererDrawTexture(
                gDebugFont,
                position[0],
                position[1],
                scale,
                scale,
                0,
                0,
                0,
                (int)(c * width) % 336,
                ((int)(c * width) / 336) * height,
                width,
                height
        );
        position[0] += width;
    }
}

static void _oct_DrawFontAtlas(Oct_Context ctx, Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    // Process interpolation
    Oct_Vec2 position;
    float scale;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->FontAtlas.position[0], cmd->FontAtlas.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->FontAtlas.position[1], cmd->FontAtlas.position[1]);
    scale = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_X || cmd->interpolate & OCT_INTERPOLATE_SCALE_Y, prevCmd, interpolatedTime, prevCmd->FontAtlas.scale, cmd->FontAtlas.scale);

    // Find atlas
    Oct_AssetData *asset = _oct_AssetGetSafe(ctx, cmd->FontAtlas.atlas, OCT_ASSET_TYPE_FONT_ATLAS);
    if (!asset)
        return;
    Oct_BitmapFontData *atlas = &asset->fontAtlas;

    // Iterate each utf-8 codepoint in the string
    const char *t = cmd->FontAtlas.text;
    uint32_t codePoint = SDL_StepUTF8(&t, null);
    float x = position[0];
    float y = position[1];
    while (codePoint) {
        if (codePoint == '\n') {
            x = position[0];
            y += atlas->newLineSize * cmd->FontAtlas.scale;
            codePoint = SDL_StepUTF8(&t, null);
            continue;
        } else if (codePoint == ' ') {
            x += atlas->spaceSize * cmd->FontAtlas.scale;
            codePoint = SDL_StepUTF8(&t, null);
            continue;
        }

        // For each character, check each layer in the atlas until we either run out
        // of layers, or find an atlas that contains the given character
        int layer = -1;
        for (int i = 0; i < atlas->atlasCount; i++) {
            if (codePoint >= atlas->atlases[i].unicodeStart && codePoint < atlas->atlases[i].unicodeEnd) {
                layer = i;
                break;
            }
        }

        if (layer != -1) {
            vk2dRendererDrawTexture(
                    atlas->atlases[layer].atlas,
                    x, y,
                    scale, scale,
                    0, 0, 0,
                    atlas->atlases[layer].glyphs[codePoint - atlas->atlases[layer].unicodeStart].location.position[0],
                    atlas->atlases[layer].glyphs[codePoint - atlas->atlases[layer].unicodeStart].location.position[1],
                    atlas->atlases[layer].glyphs[codePoint - atlas->atlases[layer].unicodeStart].location.size[0],
                    atlas->atlases[layer].glyphs[codePoint - atlas->atlases[layer].unicodeStart].location.size[1]
            );
            x += atlas->atlases[layer].glyphs[codePoint - atlas->atlases[layer].unicodeStart].advance * scale;
        }

        codePoint = SDL_StepUTF8(&t, null);
    }
}

static void _oct_SwitchTarget(Oct_Context ctx, Oct_DrawCommand *cmd) {
    Oct_AssetData *tex = _oct_AssetGetSafe(ctx, cmd->Target.texture, OCT_ASSET_TYPE_TEXTURE);
    if (cmd->Target.texture != OCT_TARGET_SWAPCHAIN && !tex)
        return;
    VK2DTexture target = cmd->Target.texture != OCT_TARGET_SWAPCHAIN ? tex->texture : null;

    vk2dRendererSetTarget(target);
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
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_SPRITE) {
            _oct_DrawSprite(ctx, cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_CIRCLE) {
            _oct_DrawCircle(ctx, cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_TARGET) {
            _oct_SwitchTarget(ctx, cmd);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_CAMERA) {
            _oct_UpdateCamera(ctx, cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_DEBUG_TEXT) {
            _oct_DrawDebugFont(ctx, cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_FONT_ATLAS) {
            _oct_DrawFontAtlas(ctx, cmd, prevCmd, interpolatedTime);
        } // TODO: Implement other command types
    }

    vk2dRendererEndFrame();
}
