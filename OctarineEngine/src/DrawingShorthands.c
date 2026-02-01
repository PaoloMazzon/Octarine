#include <SDL3/SDL.h>
#include "oct/Core.h"
#include "oct/CommandBuffer.h"
#include "oct/Subsystems.h"
#include "oct/Drawing.h"
#include "oct/Constants.h"
#include "oct/Opaque.h"

Oct_Colour _OCT_WHITE = {1, 1, 1, 1};
Oct_Vec2 _OCT_ONE2 = {1, 1};
Oct_Vec3 _OCT_ONE3 = {1, 1, 1};
Oct_Vec4 _OCT_ONE4 = {1, 1, 1, 1};
Oct_Vec2 _OCT_ZERO2 = {0, 0};
Oct_Vec3 _OCT_ZERO3 = {0, 0, 0};
Oct_Vec4 _OCT_ZERO4 = {0, 0, 0, 0};

OCTARINE_API void oct_DrawClear(Oct_Colour *colour) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_CLEAR,
            .colour = *colour
    };
    oct_Draw(&cmd);
}

/////////////////////////////////////// RECTANGLE ///////////////////////////////////////
OCTARINE_API void oct_DrawRectangleInt(Oct_InterpolationType interp, uint64_t id, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth) {
    oct_DrawRectangleIntColourExt(interp, id, &_OCT_WHITE, rectangle, filled, lineWidth, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawRectangleIntExt(Oct_InterpolationType interp, uint64_t id, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth, float rotation, Oct_Vec2 origin) {
    oct_DrawRectangleIntColourExt(interp, id, &_OCT_WHITE, rectangle, filled, lineWidth, rotation, origin);
}

OCTARINE_API void oct_DrawRectangleIntColour(Oct_InterpolationType interp, uint64_t id, Oct_Colour *colour, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth) {
    oct_DrawRectangleIntColourExt(interp, id, colour, rectangle, filled, lineWidth, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawRectangleIntColourExt(Oct_InterpolationType interp, uint64_t id, Oct_Colour *colour, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth, float rotation, Oct_Vec2 origin) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_RECTANGLE,
            .colour = *colour,
            .interpolate = interp,
            .id = id,
            .Rectangle = {
                    .rectangle = {
                            .position = {rectangle->position[0], rectangle->position[1]},
                            .size = {rectangle->size[0], rectangle->size[1]},
                            },
                    .filled = filled,
                    .lineSize = lineWidth,
                    .rotation = rotation,
                    .origin = {origin[0], origin[1]},
            }
    };
    oct_Draw(&cmd);
}

OCTARINE_API void oct_DrawRectangle(Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth) {
    oct_DrawRectangleIntColourExt(0, 0, &_OCT_WHITE, rectangle, filled, lineWidth, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawRectangleExt(Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth, float rotation, Oct_Vec2 origin) {
    oct_DrawRectangleIntColourExt(0, 0, &_OCT_WHITE, rectangle, filled, lineWidth, rotation, origin);
}

OCTARINE_API void oct_DrawRectangleColour(Oct_Colour *colour, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth) {
    oct_DrawRectangleIntColourExt(0, 0, colour, rectangle, filled, lineWidth, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawRectangleColourExt(Oct_Colour *colour, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth, float rotation, Oct_Vec2 origin) {
    oct_DrawRectangleIntColourExt(0, 0, colour, rectangle, filled, lineWidth, rotation, origin);
}

/////////////////////////////////////// CIRCLE ///////////////////////////////////////
OCTARINE_API void oct_DrawCircleInt(Oct_InterpolationType interp, uint64_t id, Oct_Circle *circle, Oct_Bool filled, float lineWidth) {
    oct_DrawCircleIntColour(interp, id, circle, &_OCT_WHITE, filled, lineWidth);
}

OCTARINE_API void oct_DrawCircleIntColour(Oct_InterpolationType interp, uint64_t id, Oct_Circle *circle, Oct_Colour *colour, Oct_Bool filled, float lineWidth) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_CIRCLE,
            .colour = *colour,
            .interpolate = interp,
            .id = id,
            .Circle = {
                    .circle = {
                            .position = {circle->position[0], circle->position[1]},
                            .radius = circle->radius
                            },
                    .filled = filled,
                    .lineSize = lineWidth,
            }
    };
    oct_Draw(&cmd);
}

OCTARINE_API void oct_DrawCircle(Oct_Circle *circle, Oct_Bool filled, float lineWidth) {
    oct_DrawCircleIntColour(0, 0, circle, &_OCT_WHITE, filled, lineWidth);
}

OCTARINE_API void oct_DrawCircleColour(Oct_Circle *circle, Oct_Colour *colour, Oct_Bool filled, float lineWidth) {
    oct_DrawCircleIntColour(0, 0, circle, colour, filled, lineWidth);
}

/////////////////////////////////////// TEXTURE ///////////////////////////////////////
OCTARINE_API void oct_DrawTextureInt(Oct_InterpolationType interp, uint64_t id, Oct_Texture texture, Oct_Vec2 position) {
    oct_DrawTextureIntColourExt(interp, id, texture, &_OCT_WHITE, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawTextureIntColour(Oct_InterpolationType interp, uint64_t id, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position) {
    oct_DrawTextureIntColourExt(interp, id, texture, colour, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawTextureIntExt(Oct_InterpolationType interp, uint64_t id, Oct_Texture texture, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawTextureIntColourExt(interp, id, texture, &_OCT_WHITE, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawTextureIntColourExt(Oct_InterpolationType interp, uint64_t id, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_TEXTURE,
            .colour = *colour,
            .interpolate = interp,
            .id = id,
            .Texture = {
                    .texture = texture,
                    .viewport = {0, 0, OCT_WHOLE_TEXTURE, OCT_WHOLE_TEXTURE},
                    .position = {position[0], position[1]},
                    .scale = {scale[0], scale[1]},
                    .origin = {origin[0], origin[1]},
                    .rotation = rotation,
            }
    };
    oct_Draw(&cmd);
}

OCTARINE_API void oct_DrawTexture(Oct_Texture texture, Oct_Vec2 position) {
    oct_DrawTextureIntColourExt(0, 0, texture, &_OCT_WHITE, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawTextureColour(Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position) {
    oct_DrawTextureIntColourExt(0, 0, texture, colour, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawTextureExt(Oct_Texture texture, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawTextureIntColourExt(0, 0, texture, &_OCT_WHITE, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawTextureColourExt(Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawTextureIntColourExt(0, 0, texture, colour, position, scale, rotation, origin);
}

/////////////////////////////////////// SHADER ///////////////////////////////////////
OCTARINE_API void oct_DrawShaderInt(Oct_InterpolationType interp, uint64_t id, Oct_Shader shader, void *data, uint32_t size, Oct_Texture texture, Oct_Vec2 position) {
    oct_DrawShaderIntColourExt(interp, id, shader, data, size, texture, &_OCT_WHITE, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawShaderIntColour(Oct_InterpolationType interp, uint64_t id, Oct_Shader shader, void *data, uint32_t size, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position) {
    oct_DrawShaderIntColourExt(interp, id, shader, data, size, texture, colour, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawShaderIntExt(Oct_InterpolationType interp, uint64_t id, Oct_Shader shader, void *data, uint32_t size, Oct_Texture texture, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawShaderIntColourExt(interp, id, shader, data, size, texture, &_OCT_WHITE, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawShaderIntColourExt(Oct_InterpolationType interp, uint64_t id, Oct_Shader shader, void *data, uint32_t size, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    void *shaderData = _oct_GetFrameMemory(size);
    memcpy(shaderData, data, size);
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_SHADER,
            .colour = *colour,
            .interpolate = interp,
            .id = id,
            .Shader = {
                    .shader = shader,
                    .uniformData = shaderData,
                    .texture = texture,
                    .viewport = {0, 0, OCT_WHOLE_TEXTURE, OCT_WHOLE_TEXTURE},
                    .position = {position[0], position[1]},
                    .scale = {scale[0], scale[1]},
                    .origin = {origin[0], origin[1]},
                    .rotation = rotation,
            }
    };
    oct_Draw(&cmd);
}

OCTARINE_API void oct_DrawShader(Oct_Shader shader, void *data, uint32_t size, Oct_Texture texture, Oct_Vec2 position) {
    oct_DrawShaderIntColourExt(0, 0, shader, data, size, texture, &_OCT_WHITE, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawShaderColour(Oct_Shader shader, void *data, uint32_t size, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position) {
    oct_DrawShaderIntColourExt(0, 0, shader, data, size, texture, colour, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawShaderExt(Oct_Shader shader, void *data, uint32_t size, Oct_Texture texture, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawShaderIntColourExt(0, 0, shader, data, size, texture, &_OCT_WHITE, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawShaderColourExt(Oct_Shader shader, void *data, uint32_t size, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawShaderIntColourExt(0, 0, shader, data, size, texture, colour, position, scale, rotation, origin);
}

/////////////////////////////////////// SPRITE ///////////////////////////////////////
// Returns the frame to send to the command
static int _oct_ProcessSpriteUpdate(Oct_SpriteInstance *instance, Oct_Sprite sprite) {
    // Its possible for the sprite to not be loaded yet, so the first frame will just be displayed until its loaded
    int32_t frameCount = 0;
    Oct_AssetData *spr = _oct_AssetGetSafe(sprite, OCT_ASSET_TYPE_SPRITE);
    double duration = 0;
    if (spr) {
        frameCount = spr->sprite.frameCount;
        duration = spr->sprite.duration;
    }

    // Process frame update if unpaused and the animation is not ended (if repeat is disabled)
    if (frameCount > 0 && !instance->pause && (instance->repeat || (instance->repeat && instance->frame % frameCount < frameCount - 1))) {
        instance->accumulator += oct_Time() - instance->lastTime;
        if (instance->accumulator > duration) {
            instance->frame += 1;
            instance->accumulator -= duration;
        }
        instance->lastTime = oct_Time();
    }
    return instance->frame;
}

OCTARINE_API void oct_InitSpriteInstance(Oct_SpriteInstance *instance, Oct_Sprite sprite, Oct_Bool repeat) {
    instance->spr = sprite;
    instance->frame = 0;
    instance->repeat = repeat;
    instance->pause = false;
    instance->lastTime = oct_Time();
    instance->accumulator = 0;
}

OCTARINE_API Oct_Bool oct_SpriteIsDone(Oct_SpriteInstance *instance) {
    int32_t frameCount = 0;
    Oct_AssetData *spr = _oct_AssetGetSafe(instance->spr, OCT_ASSET_TYPE_SPRITE);
    if (spr)
        frameCount = spr->sprite.frameCount;
    return spr && instance->repeat && (instance->frame % frameCount) == frameCount - 1;
}

OCTARINE_API void oct_DrawSpriteInt(Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, Oct_SpriteInstance *instance, Oct_Vec2 position) {
    int frame = _oct_ProcessSpriteUpdate(instance, sprite);
    oct_DrawSpriteFrameIntColourExt(interp, id, sprite, frame, &_OCT_WHITE, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawSpriteIntColour(Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, Oct_SpriteInstance *instance, Oct_Colour *colour, Oct_Vec2 position) {
    int frame = _oct_ProcessSpriteUpdate(instance, sprite);
    oct_DrawSpriteFrameIntColourExt(interp, id, sprite, frame, colour, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawSpriteIntExt(Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, Oct_SpriteInstance *instance, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    int frame = _oct_ProcessSpriteUpdate(instance, sprite);
    oct_DrawSpriteFrameIntColourExt(interp, id, sprite, frame, &_OCT_WHITE, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawSpriteIntColourExt(Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, Oct_SpriteInstance *instance, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    int frame = _oct_ProcessSpriteUpdate(instance, sprite);
    oct_DrawSpriteFrameIntColourExt(interp, id, sprite, frame, colour, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawSprite(Oct_Sprite sprite, Oct_SpriteInstance *instance, Oct_Vec2 position) {
    int frame = _oct_ProcessSpriteUpdate(instance, sprite);
    oct_DrawSpriteFrameIntColourExt(0, 0, sprite, frame, &_OCT_WHITE, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawSpriteColour(Oct_Sprite sprite, Oct_SpriteInstance *instance, Oct_Colour *colour, Oct_Vec2 position) {
    int frame = _oct_ProcessSpriteUpdate(instance, sprite);
    oct_DrawSpriteFrameIntColourExt(0, 0, sprite, frame, colour, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawSpriteExt(Oct_Sprite sprite, Oct_SpriteInstance *instance, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    int frame = _oct_ProcessSpriteUpdate(instance, sprite);
    oct_DrawSpriteFrameIntColourExt(0, 0, sprite, frame, &_OCT_WHITE, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawSpriteColourExt(Oct_Sprite sprite, Oct_SpriteInstance *instance, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    int frame = _oct_ProcessSpriteUpdate(instance, sprite);
    oct_DrawSpriteFrameIntColourExt(0, 0, sprite, frame, colour, position, scale, rotation, origin);
}

/////////////////////////////////////// SPRITE FRAME ///////////////////////////////////////
OCTARINE_API void oct_DrawSpriteFrameInt(Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, int32_t frame, Oct_Vec2 position) {
    oct_DrawSpriteFrameIntColourExt(interp, id, sprite, frame, &_OCT_WHITE, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawSpriteFrameIntColour(Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, int32_t frame, Oct_Colour *colour, Oct_Vec2 position) {
    oct_DrawSpriteFrameIntColourExt(interp, id, sprite, frame, colour, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawSpriteFrameIntExt(Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, int32_t frame, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawSpriteFrameIntColourExt(interp, id, sprite, frame, &_OCT_WHITE, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawSpriteFrameIntColourExt(Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, int32_t frame, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_SPRITE,
            .colour = *colour,
            .interpolate = interp,
            .id = id,
            .Sprite = {
                    .sprite = sprite,
                    .viewport = {0, 0, OCT_WHOLE_TEXTURE, OCT_WHOLE_TEXTURE},
                    .position = {position[0], position[1]},
                    .scale = {scale[0], scale[1]},
                    .origin = {origin[0], origin[1]},
                    .frame = frame,
                    .rotation = rotation,
            }
    };
    oct_Draw(&cmd);
}

OCTARINE_API void oct_DrawSpriteFrame(Oct_Sprite sprite, int32_t frame, Oct_Vec2 position) {
    oct_DrawSpriteFrameIntColourExt(0, 0, sprite, frame, &_OCT_WHITE, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawSpriteFrameColour(Oct_Sprite sprite, int32_t frame, Oct_Colour *colour, Oct_Vec2 position) {
    oct_DrawSpriteFrameIntColourExt(0, 0, sprite, frame, colour, position, _OCT_ONE2, 0, _OCT_ZERO2);
}

OCTARINE_API void oct_DrawSpriteFrameExt(Oct_Sprite sprite, int32_t frame, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawSpriteFrameIntColourExt(0, 0, sprite, frame, &_OCT_WHITE, position, scale, rotation, origin);
}

OCTARINE_API void oct_DrawSpriteFrameColourExt(Oct_Sprite sprite, int32_t frame, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin) {
    oct_DrawSpriteFrameIntColourExt(0, 0, sprite, frame, colour, position, scale, rotation, origin);
}

/////////////////////////////////////// CAMERA ///////////////////////////////////////
OCTARINE_API void oct_UpdateCameraInt(Oct_InterpolationType interp, uint64_t id, Oct_Camera camera, Oct_CameraUpdate *update) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_CAMERA,
            .interpolate = interp,
            .id = id,
            .Camera = {
                    .cameraUpdate = {
                            .position = {update->position[0], update->position[1]},
                            .size = {update->size[0], update->size[1]},
                            .rotation = update->rotation,
                            .screenPosition = {update->screenPosition[0], update->screenPosition[1]},
                            .screenSize = {update->screenSize[0], update->screenSize[1]},
                            },
                    .updateType = OCT_CAMERA_UPDATE_TYPE_UPDATE_CAMERA,
                    .camera = camera,
            }
    };
    oct_Draw(&cmd);
}

OCTARINE_API void oct_UpdateCamera(Oct_Camera camera, Oct_CameraUpdate *update) {
    oct_UpdateCameraInt(0, 0, camera, update);
}

OCTARINE_API void oct_LockCameras(Oct_Camera camera) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_CAMERA,
            .Camera = {
                    .updateType = OCT_CAMERA_UPDATE_TYPE_LOCK_CAMERA,
                    .camera = camera,
            }
    };
    oct_Draw(&cmd);
}

OCTARINE_API void oct_UnlockCameras() {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_CAMERA,
            .Camera = {
                    .updateType = OCT_CAMERA_UPDATE_TYPE_UNLOCK_CAMERA,
            }
    };
    oct_Draw(&cmd);
}

OCTARINE_API void oct_SetTextureCamerasEnabled(Oct_Bool enabled) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_CAMERA,
            .Camera = {
                    .updateType = enabled ? OCT_CAMERA_UPDATE_TYPE_ENABLE_TEX_CAM : OCT_CAMERA_UPDATE_TYPE_DISABLE_TEX_CAM,
            }
    };
    oct_Draw(&cmd);
}

/////////////////////////////////////// RENDER TARGET ///////////////////////////////////////
OCTARINE_API void oct_SetDrawTarget(Oct_Texture target) {
    Oct_DrawCommand cmd = {
            .sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND,
            .type = OCT_DRAW_COMMAND_TYPE_TARGET,
            .Target.texture = target
    };
    oct_Draw(&cmd);
}

/////////////////////////////////////// DEBUG TEXT ///////////////////////////////////////
OCTARINE_API void oct_DrawDebugText(Oct_Vec2 position, float scale, const char *fmt, ...) {
    char *tempBuffer = _oct_GetFrameMemory(1024);
    if (tempBuffer) {
        tempBuffer[1023] = 0;
        va_list l;
        va_start(l, fmt);
        SDL_vsnprintf(tempBuffer, 1023, fmt, l);
        va_end(l);

        Oct_DrawCommand cmd = {
                .type = OCT_DRAW_COMMAND_TYPE_DEBUG_TEXT,
                .colour = {1, 1, 1, 1},
                .DebugText = {
                        .scale = scale,
                        .position = {position[0], position[1]},
                        .text = tempBuffer
                }
        };
        oct_Draw(&cmd);
    }
}

OCTARINE_API void oct_DrawDebugTextInt(Oct_InterpolationType interp, uint64_t id, Oct_Vec2 position, float scale, const char *fmt, ...) {
    char *tempBuffer = _oct_GetFrameMemory(1024);
    if (tempBuffer) {
        tempBuffer[1023] = 0;
        va_list l;
        va_start(l, fmt);
        SDL_vsnprintf(tempBuffer, 1023, fmt, l);
        va_end(l);

        Oct_DrawCommand cmd = {
                .type = OCT_DRAW_COMMAND_TYPE_DEBUG_TEXT,
                .colour = {1, 1, 1, 1},
                .interpolate = interp,
                .id = id,
                .DebugText = {
                        .scale = scale,
                        .position = {position[0], position[1]},
                        .text = tempBuffer
                }
        };
        oct_Draw(&cmd);
    }
}

/////////////////////////////////////// ATLAS TEXT ///////////////////////////////////////
OCTARINE_API void oct_DrawText(Oct_FontAtlas atlas, Oct_Vec2 position, float scale, const char *fmt, ...) {
    char *tempBuffer = _oct_GetFrameMemory(1024);
    if (tempBuffer) {
        tempBuffer[1023] = 0;
        va_list l;
        va_start(l, fmt);
        SDL_vsnprintf(tempBuffer, 1023, fmt, l);
        va_end(l);

        Oct_DrawCommand cmd = {
                .type = OCT_DRAW_COMMAND_TYPE_FONT_ATLAS,
                .colour = {1, 1, 1, 1},
                .FontAtlas = {
                        .atlas = atlas,
                        .scale = scale,
                        .position = {position[0], position[1]},
                        .text = tempBuffer
                }
        };
        oct_Draw(&cmd);
    }
}

OCTARINE_API void oct_DrawTextColour(Oct_FontAtlas atlas, Oct_Vec2 position, Oct_Colour *colour, float scale, const char *fmt, ...) {
    char *tempBuffer = _oct_GetFrameMemory(1024);
    if (tempBuffer) {
        tempBuffer[1023] = 0;
        va_list l;
        va_start(l, fmt);
        SDL_vsnprintf(tempBuffer, 1023, fmt, l);
        va_end(l);

        Oct_DrawCommand cmd = {
                .type = OCT_DRAW_COMMAND_TYPE_FONT_ATLAS,
                .colour = {colour->r, colour->g, colour->b, colour->a},
                .FontAtlas = {
                        .atlas = atlas,
                        .scale = scale,
                        .position = {position[0], position[1]},
                        .text = tempBuffer
                }
        };
        oct_Draw(&cmd);
    }
}

OCTARINE_API void oct_DrawTextInt(Oct_InterpolationType interp, uint64_t id, Oct_FontAtlas atlas, Oct_Vec2 position, float scale, const char *fmt, ...) {
    char *tempBuffer = _oct_GetFrameMemory(1024);
    if (tempBuffer) {
        tempBuffer[1023] = 0;
        va_list l;
        va_start(l, fmt);
        SDL_vsnprintf(tempBuffer, 1023, fmt, l);
        va_end(l);

        Oct_DrawCommand cmd = {
                .type = OCT_DRAW_COMMAND_TYPE_FONT_ATLAS,
                .colour = {1, 1, 1, 1},
                .interpolate = interp,
                .id = id,
                .FontAtlas = {
                        .atlas = atlas,
                        .scale = scale,
                        .position = {position[0], position[1]},
                        .text = tempBuffer
                }
        };
        oct_Draw(&cmd);
    }
}

OCTARINE_API void oct_DrawTextIntColour(Oct_InterpolationType interp, uint64_t id, Oct_FontAtlas atlas, Oct_Vec2 position, Oct_Colour *colour, float scale, const char *fmt, ...) {
    char *tempBuffer = _oct_GetFrameMemory(1024);
    if (tempBuffer) {
        tempBuffer[1023] = 0;
        va_list l;
        va_start(l, fmt);
        SDL_vsnprintf(tempBuffer, 1023, fmt, l);
        va_end(l);

        Oct_DrawCommand cmd = {
                .type = OCT_DRAW_COMMAND_TYPE_FONT_ATLAS,
                .colour = {colour->r, colour->g, colour->b, colour->a},
                .interpolate = interp,
                .id = id,
                .FontAtlas = {
                        .atlas = atlas,
                        .scale = scale,
                        .position = {position[0], position[1]},
                        .text = tempBuffer
                }
        };
        oct_Draw(&cmd);
    }
}
