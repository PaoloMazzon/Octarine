/// \brief Shorthands for drawing.
///
/// These functions don't do everything its possible to do with the rendering system in Octarine, they are just
/// quick shorthands for common things you'll need to do. To do more advanced things, you'll need to make a
/// Oct_DrawCommand and queue the draw manually with oct_Draw.
///
/// In terms of convention, `oct_Draw*Int` represents functions that allow interpolation. `The oct_Draw*` functions
/// do not allow interpolation. `oct_Draw*Colour` allow you to specify a colour modifier, and `ext` at the end
/// means there are more possible drawing options (usually rotation among other things). For example,
/// `oct_DrawRectangleIntColourExt` means draw an interpolated rectangle with a colour parameter and extra parameters,
/// in this case for rotation.
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Draws a rectangle
OCTARINE_API void oct_DrawRectangleInt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth);

/// \brief Draws a rectangle
OCTARINE_API void oct_DrawRectangleIntExt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth, float rotation, Oct_Vec2 origin);

/// \brief Draws a rectangle
OCTARINE_API void oct_DrawRectangleIntColour(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Colour *colour, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth);

/// \brief Draws a rectangle
OCTARINE_API void oct_DrawRectangleIntColourExt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Colour *colour, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth, float rotation, Oct_Vec2 origin);

/// \brief Draws a rectangle
OCTARINE_API void oct_DrawRectangle(Oct_Context ctx, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth);

/// \brief Draws a rectangle
OCTARINE_API void oct_DrawRectangleExt(Oct_Context ctx, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth, float rotation, Oct_Vec2 origin);

/// \brief Draws a rectangle
OCTARINE_API void oct_DrawRectangleColour(Oct_Context ctx, Oct_Colour *colour, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth);

/// \brief Draws a rectangle
OCTARINE_API void oct_DrawRectangleColourExt(Oct_Context ctx, Oct_Colour *colour, Oct_Rectangle *rectangle, Oct_Bool filled, float lineWidth, float rotation, Oct_Vec2 origin);

/// \brief Draws a circle
OCTARINE_API void oct_DrawCircleInt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Circle *circle, Oct_Bool filled, float lineWidth);

/// \brief Draws a circle
OCTARINE_API void oct_DrawCircleIntColour(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Circle *circle, Oct_Colour *colour, Oct_Bool filled, float lineWidth);

/// \brief Draws a circle
OCTARINE_API void oct_DrawCircle(Oct_Context ctx, Oct_Circle *circle, Oct_Bool filled, float lineWidth);

/// \brief Draws a circle
OCTARINE_API void oct_DrawCircleColour(Oct_Context ctx, Oct_Circle *circle, Oct_Colour *colour, Oct_Bool filled, float lineWidth);

/// \brief Draws a texture
OCTARINE_API void oct_DrawTextureInt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Texture texture, Oct_Vec2 position);

/// \brief Draws a texture
OCTARINE_API void oct_DrawTextureIntColour(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position);

/// \brief Draws a texture
OCTARINE_API void oct_DrawTextureIntExt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Texture texture, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draws a texture
OCTARINE_API void oct_DrawTextureIntColourExt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draws a texture
OCTARINE_API void oct_DrawTexture(Oct_Context ctx, Oct_Texture texture, Oct_Vec2 position);

/// \brief Draws a texture
OCTARINE_API void oct_DrawTextureColour(Oct_Context ctx, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position);

/// \brief Draws a texture
OCTARINE_API void oct_DrawTextureExt(Oct_Context ctx, Oct_Texture texture, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draws a texture
OCTARINE_API void oct_DrawTextureColourExt(Oct_Context ctx, Oct_Texture texture, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draws a sprite
OCTARINE_API void oct_DrawSpriteInt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, Oct_Vec2 position);

/// \brief Draws a sprite
OCTARINE_API void oct_DrawSpriteIntColour(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, Oct_Colour *colour, Oct_Vec2 position);

/// \brief Draws a sprite
OCTARINE_API void oct_DrawSpriteIntExt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draws a sprite
OCTARINE_API void oct_DrawSpriteIntColourExt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draws a sprite
OCTARINE_API void oct_DrawSprite(Oct_Context ctx, Oct_Sprite sprite, Oct_Vec2 position);

/// \brief Draws a sprite
OCTARINE_API void oct_DrawSpriteColour(Oct_Context ctx, Oct_Sprite sprite, Oct_Colour *colour, Oct_Vec2 position);

/// \brief Draws a sprite
OCTARINE_API void oct_DrawSpriteExt(Oct_Context ctx, Oct_Sprite sprite, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draws a sprite
OCTARINE_API void oct_DrawSpriteColourExt(Oct_Context ctx, Oct_Sprite sprite, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draw a specific sprite frame (use SPRITE_*_FRAME, otherwise index from 1)
OCTARINE_API void oct_DrawSpriteFrameInt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, int32_t frame, Oct_Vec2 position);

/// \brief Draw a specific sprite frame (use SPRITE_*_FRAME, otherwise index from 1)
OCTARINE_API void oct_DrawSpriteFrameIntColour(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, int32_t frame, Oct_Colour *colour, Oct_Vec2 position);

/// \brief Draw a specific sprite frame (use SPRITE_*_FRAME, otherwise index from 1)
OCTARINE_API void oct_DrawSpriteFrameIntExt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, int32_t frame, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draw a specific sprite frame (use SPRITE_*_FRAME, otherwise index from 1)
OCTARINE_API void oct_DrawSpriteFrameIntColourExt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Sprite sprite, int32_t frame, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draw a specific sprite frame (use SPRITE_*_FRAME, otherwise index from 1)
OCTARINE_API void oct_DrawSpriteFrame(Oct_Context ctx, Oct_Sprite sprite, int32_t frame, Oct_Vec2 position);

/// \brief Draw a specific sprite frame (use SPRITE_*_FRAME, otherwise index from 1)
OCTARINE_API void oct_DrawSpriteFrameColour(Oct_Context ctx, Oct_Sprite sprite, int32_t frame, Oct_Colour *colour, Oct_Vec2 position);

/// \brief Draw a specific sprite frame (use SPRITE_*_FRAME, otherwise index from 1)
OCTARINE_API void oct_DrawSpriteFrameExt(Oct_Context ctx, Oct_Sprite sprite, int32_t frame, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Draw a specific sprite frame (use SPRITE_*_FRAME, otherwise index from 1)
OCTARINE_API void oct_DrawSpriteFrameColourExt(Oct_Context ctx, Oct_Sprite sprite, int32_t frame, Oct_Colour *colour, Oct_Vec2 position, Oct_Vec2 scale, float rotation, Oct_Vec2 origin);

/// \brief Interpolates a camera update
OCTARINE_API void oct_UpdateCameraInt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Camera camera, Oct_CameraUpdate *update);

/// \brief Updates a camera
OCTARINE_API void oct_UpdateCamera(Oct_Context ctx, Oct_Camera camera, Oct_CameraUpdate *update);

/// \brief Locks rendering to a single camera (only 1 camera will be rendered to)
OCTARINE_API void oct_LockCameras(Oct_Context ctx, Oct_Camera camera);

/// \brief Unlocks rendering such that all cameras will be rendered to
OCTARINE_API void oct_UnlockCameras(Oct_Context ctx);

/// \brief If enabled, cameras will be used on textures instead of rendering relative to their top-left
OCTARINE_API void oct_SetTextureCamerasEnabled(Oct_Context ctx, Oct_Bool enabled);

/// \brief Sets the render target
OCTARINE_API void oct_SetDrawTarget(Oct_Context ctx, Oct_Texture target);

/// \brief Draws debug text on screen with interpolation, format like you would printf
OCTARINE_API void oct_DrawDebugTextInt(Oct_Context ctx, Oct_InterpolationType interp, uint64_t id, Oct_Vec2 position, float scale, const char *fmt, ...);

/// \brief Draws debug text on screen, format like you would printf
OCTARINE_API void oct_DrawDebugText(Oct_Context ctx, Oct_Vec2 position, float scale, const char *fmt, ...);

#ifdef __cplusplus
};
#endif