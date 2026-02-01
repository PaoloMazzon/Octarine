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

// Thing in the draw command hash bucket
typedef struct DrawCommandLink_t DrawCommandLink;
struct DrawCommandLink_t {
    Oct_DrawCommand *command; // Pointer to a command in the main draw command link in the frame command buffer thing
    uint64_t id;              // Id for collision checking
    uint64_t frame;           // Frame this is associated with so we don't need to clear the bucket each frame
    int32_t next;             // Next in the chain for collision checking (this is an index to the backup bucket)
};

const int BUCKET_SIZE = 10000;

// Commands are tied to the frame they came from for interpolation and triple buffering
typedef struct FrameCommandBuffer_t {
    Oct_DrawCommand *commands; ///< Internal command list
    int size;                  ///< Size of the internal buffer
    int count;                 ///< Number of commands
    Oct_Bool singleBuffer;     ///< If this command buffer is only meant to be ran once
    Oct_Bool executed;         ///< If this is a single buffer and it has already been run

    // Draw command hash bucket for interpolated draws
    DrawCommandLink *bucket;
    DrawCommandLink *backupBucket;
    int backupBucketSize;
    int backupBucketCount; // Set to 0 each time this starts working to make room for new links
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
static uint64_t gFrame = 2; // Total number of frames, in general

// For keeping track of average interpolation time
static double gTotalInterpolationTime; // Total time spent finding interpolated draw commands
static double gTotalInterpolationCalls; // Total amount of draws that were interpolated
static double gTotalFrames;
static double gLastInterpolationStatsUpdate; // Last time the interpolation stats were updated
static double gAverageInterpolationTime;
static double gAverageInterpolationCalls;

///////////////////// Hash bucket functions /////////////////////
uint64_t hash(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

// Puts a command into the hash bucket of a given frame
static void addCommandToBucket(int index) {
    Oct_DrawCommand *cmd = &gFrameBuffers[gCurrentFrame].commands[index];
    uint64_t bucketLocation = hash(cmd->id) % BUCKET_SIZE;

    if (gFrameBuffers[gCurrentFrame].bucket[bucketLocation].frame != gFrame) {
        // This spot is empty
        gFrameBuffers[gCurrentFrame].bucket[bucketLocation].command = cmd;
        gFrameBuffers[gCurrentFrame].bucket[bucketLocation].id = cmd->id;
        gFrameBuffers[gCurrentFrame].bucket[bucketLocation].frame = gFrame;
        gFrameBuffers[gCurrentFrame].bucket[bucketLocation].next = -1;
    } else {
        // This spot is taken, find the end of the linked list
        DrawCommandLink *current = &gFrameBuffers[gCurrentFrame].bucket[bucketLocation];
        while (current->next != -1)
            current = &gFrameBuffers[gCurrentFrame].backupBucket[current->next];

        // Find a spot in the extended bucket list
        if (gFrameBuffers[gCurrentFrame].backupBucketCount == gFrameBuffers[gCurrentFrame].backupBucketSize) {
            void *temp = mi_realloc(gFrameBuffers[gCurrentFrame].backupBucket,
                                    sizeof(struct DrawCommandLink_t) * (gFrameBuffers[gCurrentFrame].backupBucketSize + 10));
            if (!temp)
                oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to reallocate backup bucket.");
            gFrameBuffers[gCurrentFrame].backupBucket = temp;
            gFrameBuffers[gCurrentFrame].backupBucketSize += 10;
        }
        const int32_t extendedBucketSpot = gFrameBuffers[gCurrentFrame].backupBucketCount++;

        current->next = extendedBucketSpot;
        gFrameBuffers[gCurrentFrame].backupBucket[extendedBucketSpot].command = cmd;
        gFrameBuffers[gCurrentFrame].backupBucket[extendedBucketSpot].id = cmd->id;
        gFrameBuffers[gCurrentFrame].backupBucket[extendedBucketSpot].frame = gFrame;
        gFrameBuffers[gCurrentFrame].backupBucket[extendedBucketSpot].next = -1;
    }
}

// Pulls a command from the previous frame's hash bucket or null if there is no match
static Oct_DrawCommand *getCommandFromBucket(uint64_t id) {
    // Get expected location
    uint64_t bucketLocation = hash(id) % BUCKET_SIZE;

    // Traverse linked list till we find it
    DrawCommandLink *link = &gFrameBuffers[PREVIOUS_DRAW_FRAME].bucket[bucketLocation];
    while (link) {
        if (link->frame == gFrame - 1 && link->id == id)
            return link->command;
        else if (link->frame != gFrame - 1)
            break;
        if (link->next != -1)
            link = &gFrameBuffers[PREVIOUS_DRAW_FRAME].backupBucket[link->next];
        else
            link = null;
    }

    return null;
}

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
    if (cmd->interpolate != 0)
        addCommandToBucket(gFrameBuffers[gCurrentFrame].count - 1);
}

///////////////////// Subsystem /////////////////////
void _oct_DrawingInit() {
    Oct_Context ctx = _oct_GetCtx();

    VK2DRendererConfig config = {
            .msaa = VK2D_MSAA_32X,
            .filterMode = VK2D_FILTER_TYPE_NEAREST,
            .screenMode = VK2D_SCREEN_MODE_TRIPLE_BUFFER
    };
    VK2DStartupOptions options = {
            .errorFile = "octarinedump.log",
            .quitOnError = true,
            .stdoutLogging = false,
            .enableDebug = false,
            .enableNuklear = ctx->initInfo->debug
    };
    VK2DResult result = vk2dRendererInit(ctx->window, config, &options);

    if (result == VK2D_ERROR)
        oct_Raise(OCT_STATUS_VK2D_ERROR, true, "Failed to create renderer. VK2D error: %s", vk2dStatusMessage());

    // Allocate frame buffers
    for (int i = 0; i < 3; i++) {
        gFrameBuffers[i].size = ctx->initInfo->ringBufferSize;
        gFrameBuffers[i].commands = mi_malloc(gFrameBuffers[i].size * sizeof(struct Oct_DrawCommand_t));
        gFrameBuffers[i].bucket = mi_zalloc(sizeof(struct DrawCommandLink_t) * BUCKET_SIZE);
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

void _oct_DrawingEnd() {
    vk2dTextureFree(gDebugFont);

    // Free frame buffers
    for (int i = 0; i < 3; i++) {
        mi_free(gFrameBuffers[i].commands);
        mi_free(gFrameBuffers[i].bucket);
        mi_free(gFrameBuffers[i].backupBucket);
    }

    vk2dRendererQuit();
}

double _oct_DrawingGetAverageInterpolationCalls() {
    return gAverageInterpolationCalls;
}

double _oct_DrawingGetAverageInterpolationTime() {
    return gAverageInterpolationTime;
}

void _oct_DrawingUpdateBegin() { }

void _oct_DrawingProcessCommand(Oct_Command *cmd) {
    if (OCT_STRUCTURE_TYPE(&cmd->topOfUnion) == OCT_STRUCTURE_TYPE_META_COMMAND) {
        if (cmd->metaCommand.type == OCT_META_COMMAND_TYPE_END_FRAME || cmd->metaCommand.type == OCT_META_COMMAND_TYPE_END_SINGLE_FRAME) {
            gCurrentFrame = NEXT_INDEX(gCurrentFrame);
            gFrameBuffers[gCurrentFrame].count = 0;
            gFrameBuffers[gCurrentFrame].backupBucketCount = 0;
            gFrameBuffers[gCurrentFrame].executed = false;
            gFrameBuffers[gCurrentFrame].singleBuffer = false;

            if (cmd->metaCommand.type == OCT_META_COMMAND_TYPE_END_SINGLE_FRAME)
                gFrameBuffers[gCurrentFrame].singleBuffer = true;
        } else if (cmd->metaCommand.type == OCT_META_COMMAND_TYPE_START_FRAME || cmd->metaCommand.type == OCT_META_COMMAND_TYPE_START_SINGLE_FRAME) {
            gFrame++;
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

static void _oct_DrawRectangle(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
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

static void _oct_UpdateCamera(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    // Update the camera if the user wishes to
    if (cmd->Camera.updateType & OCT_CAMERA_UPDATE_TYPE_UPDATE_CAMERA) {
        Oct_AssetData *data = _oct_AssetGetSafe(cmd->Camera.camera, OCT_ASSET_TYPE_CAMERA);
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
        Oct_AssetData *data = _oct_AssetGetSafe(cmd->Camera.camera, OCT_ASSET_TYPE_CAMERA);
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

static void _oct_DrawCircle(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
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

static void _oct_ClearTarget(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    vk2dRendererClear();
}

static void _oct_DrawTexture(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    Oct_AssetData *asset = _oct_AssetGetSafe(cmd->Texture.texture, OCT_ASSET_TYPE_TEXTURE);
    if (!asset)
        return;
    VK2DTexture tex = asset->texture.tex;

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

static void _oct_DrawShader(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    Oct_AssetData *texture = _oct_AssetGetSafe(cmd->Shader.texture, OCT_ASSET_TYPE_TEXTURE);
    Oct_AssetData *shader = _oct_AssetGetSafe(cmd->Shader.shader, OCT_ASSET_TYPE_SHADER);
    if (!texture || !shader)
        return;
    VK2DTexture tex = texture->texture.tex;

    // Process interpolation
    Oct_Vec2 position;
    Oct_Vec2 scale;
    Oct_Vec2 origin = {0, 0};
    float rotation;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Shader.position[0], cmd->Shader.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->Shader.position[1], cmd->Shader.position[1]);
    scale[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_X, prevCmd, interpolatedTime, prevCmd->Shader.scale[0], cmd->Shader.scale[0]);
    scale[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_Y, prevCmd, interpolatedTime, prevCmd->Shader.scale[1], cmd->Shader.scale[1]);
    rotation = interpolate(cmd->interpolate & OCT_INTERPOLATE_ROTATION, prevCmd, interpolatedTime, prevCmd->Shader.rotation, cmd->Shader.rotation);

    // Find viewport
    const float w = cmd->Shader.viewport.size[0] == OCT_WHOLE_TEXTURE ? vk2dTextureWidth(tex) : cmd->Shader.viewport.size[0];
    const float h = cmd->Shader.viewport.size[1] == OCT_WHOLE_TEXTURE ? vk2dTextureHeight(tex) : cmd->Shader.viewport.size[1];

    // Process origin
    _oct_ProcessOrigin(cmd->Shader.origin, origin, w, h);

    // Draw texture
    vk2dRendererDrawShader(
            shader->shader.shader,
            cmd->Shader.uniformData,
            tex,
            position[0] - (origin[0] * cmd->Shader.scale[0]),
            position[1] - (origin[1] * cmd->Shader.scale[1]),
            scale[0],
            scale[1],
            rotation,
            origin[0],
            origin[1],
            cmd->Shader.viewport.position[0],
            cmd->Shader.viewport.position[1],
            w,
            h
    );
}

static void _oct_DrawSprite(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    Oct_AssetData *asset = _oct_AssetGetSafe(cmd->Sprite.sprite, OCT_ASSET_TYPE_SPRITE);
    if (!asset)
        return;
    Oct_SpriteData *spr = &asset->sprite;

    // Make sure the sprite's texture still exists
    VK2DTexture tex = null;
    Oct_AssetData *texData = _oct_AssetGetSafe(spr->texture, OCT_ASSET_TYPE_TEXTURE);
    if (texData) {
        tex = texData->texture.tex;
    } else {
        oct_Raise(OCT_STATUS_BAD_PARAMETER, true, "Sprite ID %" PRIu64 " uses a texture that does not exist (" PRIu64 ").", cmd->Sprite.sprite, spr->texture);
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

    // Locate frame in the texture
    int32_t frame = cmd->Sprite.frame;
    if (frame == OCT_SPRITE_LAST_FRAME)
        frame = spr->frameCount - 1;
    else
        frame = frame % spr->frameCount;
    float x = spr->frames[frame].position[0];
    float y = spr->frames[frame].position[1];
    float w = spr->frames[frame].size[0];
    float h = spr->frames[frame].size[1];

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
}

static void _oct_DrawDebugFont(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
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

static void _oct_DrawFontAtlas(Oct_DrawCommand *cmd, Oct_DrawCommand *prevCmd, float interpolatedTime) {
    // Process interpolation
    Oct_Vec2 position;
    float scale;
    position[0] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->FontAtlas.position[0], cmd->FontAtlas.position[0]);
    position[1] = interpolate(cmd->interpolate & OCT_INTERPOLATE_POSITION, prevCmd, interpolatedTime, prevCmd->FontAtlas.position[1], cmd->FontAtlas.position[1]);
    scale = interpolate(cmd->interpolate & OCT_INTERPOLATE_SCALE_X || cmd->interpolate & OCT_INTERPOLATE_SCALE_Y, prevCmd, interpolatedTime, prevCmd->FontAtlas.scale, cmd->FontAtlas.scale);

    // Find atlas
    Oct_AssetData *asset = _oct_AssetGetSafe(cmd->FontAtlas.atlas, OCT_ASSET_TYPE_FONT_ATLAS);
    if (!asset)
        return;
    Oct_BitmapFontData *atlas = &asset->fontAtlas;

    // Find the font for kerning
    TTF_Font *font = null;
    Oct_AssetData *fontAsset = atlas->font != OCT_NO_ASSET ? _oct_AssetGetSafe(atlas->font, OCT_ASSET_TYPE_FONT) : null;
    if (fontAsset)
        font = fontAsset->font.font[0];

    // Iterate each utf-8 codepoint in the string
    const char *t = cmd->FontAtlas.text;
    uint32_t codePoint = SDL_StepUTF8(&t, null);
    float x = position[0];
    float y = position[1];
    uint32_t previousCodePoint = UINT32_MAX;
    while (codePoint) {
        if (codePoint == '\n') {
            x = position[0];
            y += atlas->newLineSize * cmd->FontAtlas.scale;
            codePoint = SDL_StepUTF8(&t, null);
            previousCodePoint = UINT32_MAX;
            continue;
        } else if (codePoint == ' ') {
            x += atlas->spaceSize * cmd->FontAtlas.scale;
            codePoint = SDL_StepUTF8(&t, null);
            previousCodePoint = UINT32_MAX;
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

            // Find additional kerning
            if (previousCodePoint != UINT32_MAX && font) {
                int kern;
                TTF_GetGlyphKerning(font, previousCodePoint, codePoint, &kern);
                x += kern;
            }
            previousCodePoint = codePoint;
        }

        codePoint = SDL_StepUTF8(&t, null);
    }
}

static void _oct_SwitchTarget(Oct_DrawCommand *cmd) {
    Oct_AssetData *tex = _oct_AssetGetSafe(cmd->Target.texture, OCT_ASSET_TYPE_TEXTURE);
    if (cmd->Target.texture != OCT_TARGET_SWAPCHAIN && !tex)
        return;
    VK2DTexture target = cmd->Target.texture != OCT_TARGET_SWAPCHAIN ? tex->texture.tex : null;

    vk2dRendererSetTarget(target);
}

void _oct_DrawingUpdateEnd() {
    Oct_Context ctx = _oct_GetCtx();

    // Handle single frames
    if (gFrameBuffers[CURRENT_DRAW_FRAME].singleBuffer && gFrameBuffers[CURRENT_DRAW_FRAME].executed) {
        return;
    }
    gFrameBuffers[CURRENT_DRAW_FRAME].executed = true;

    // Get interpolated time
    int atomic = SDL_GetAtomicInt(&ctx->interpolatedTime);
    float interpolatedTime = OCT_INT_TO_FLOAT(atomic);

    // Interpolate/draw current frame's commands
    for (int i = 0; i < gFrameBuffers[CURRENT_DRAW_FRAME].count; i++) {
        Oct_DrawCommand *cmd = &gFrameBuffers[CURRENT_DRAW_FRAME].commands[i];
        vk2dRendererSetColourMod((float*)&cmd->colour);

        // If its interpolated, find the corresponding command
        Oct_DrawCommand *prevCmd = null;
        if (cmd->interpolate) {
            // Performance metrics
            gTotalInterpolationCalls += 1;
            const double startTime = oct_Time();

            // Find the interpolated command
            prevCmd = getCommandFromBucket(cmd->id);

            // Performance metrics
            gTotalInterpolationTime += oct_Time() - startTime;
        }

        if (cmd->type == OCT_DRAW_COMMAND_TYPE_RECTANGLE) {
            _oct_DrawRectangle(cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_TEXTURE) {
            _oct_DrawTexture(cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_SPRITE) {
            _oct_DrawSprite(cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_CIRCLE) {
            _oct_DrawCircle(cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_TARGET) {
            _oct_SwitchTarget(cmd);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_CAMERA) {
            _oct_UpdateCamera(cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_DEBUG_TEXT) {
            _oct_DrawDebugFont(cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_FONT_ATLAS) {
            _oct_DrawFontAtlas(cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_CLEAR) {
            _oct_ClearTarget(cmd, prevCmd, interpolatedTime);
        } else if (cmd->type == OCT_DRAW_COMMAND_TYPE_SHADER) {
            _oct_DrawShader(cmd, prevCmd, interpolatedTime);
        }
    }

    vk2dRendererPresent();

    // Deal with interpolation times
    gTotalFrames += 1;
    if (oct_Time() - gLastInterpolationStatsUpdate >= 1) {
        gAverageInterpolationTime = gTotalInterpolationTime / gTotalInterpolationCalls;
        gAverageInterpolationCalls = gTotalInterpolationCalls / gTotalFrames;
        gTotalInterpolationTime = 0;
        gTotalInterpolationCalls = 0;
        gTotalFrames = 0;
        gLastInterpolationStatsUpdate = oct_Time();
    }
}
