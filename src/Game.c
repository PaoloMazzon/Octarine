#include "Game.h"
#include <math.h>

Oct_Texture gTexMarble;
Oct_Texture gTexPaladinSheet;
Oct_Sprite gSprPaladinWalkRight;
Oct_Allocator gAllocator;
Oct_Font gPixelFont;
Oct_FontAtlas gPixelFontAtlas;
Oct_FontAtlas gBitmapFontAtlas;

// Called at the start of the game after engine initialization, whatever you return is passed to update
void *startup(Oct_Context ctx) {
    gAllocator = oct_CreateHeapAllocator();
    gTexMarble = oct_LoadTexture(ctx, "data/marble.jpg");
    gTexPaladinSheet = oct_LoadTexture(ctx, "data/paladin.png");
    gSprPaladinWalkRight = oct_LoadSprite(ctx, gTexPaladinSheet, 4, 10, (Oct_Vec2){0, 0}, (Oct_Vec2){32, 32});
    gPixelFont = oct_LoadFont(ctx, "data/Ubuntu-Regular.ttf", 16);
    gPixelFontAtlas = oct_CreateFontAtlas(ctx, gPixelFont, OCT_NO_ASSET, 32, 128);
    oct_CreateFontAtlas(ctx, gPixelFont, gPixelFontAtlas, 0x400, 0x4ff);
    gBitmapFontAtlas = oct_LoadBitmapFont(ctx, "data/monogram.png", (Oct_Vec2){6, 12}, 32, 160);
    // TODO: Test bitmap font
    // TODO: Test spritesheet xstop code

    return null;
}

// Called each logical frame, whatever you return is passed to either the next update or shutdown
void *update(Oct_Context ctx, void *ptr) {
    oct_DrawRectangleIntColour(
            ctx,
            OCT_INTERPOLATE_ALL, 1,
            &(Oct_Colour){0, 0.6, 1, 1},
            &(Oct_Rectangle){
                .position = {320 + (cosf(oct_Time(ctx)) * 200), 240 + (sinf(oct_Time(ctx)) * 200)},
                .size = {40, 40}
            },
            true, 0
    );

    oct_DrawTextureIntExt(
            ctx,
            OCT_INTERPOLATE_ALL, 2,
            gTexMarble,
            (Oct_Vec2){oct_MouseX() + 100, oct_MouseY() + 100},
            (Oct_Vec2){1, 1},
            oct_Time(ctx),
            (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE}
    );

    oct_DrawSpriteIntExt(
            ctx,
            OCT_INTERPOLATE_ALL, 3,
            gSprPaladinWalkRight,
            (Oct_Vec2){oct_MouseX(), oct_MouseY()},
            (Oct_Vec2){4, 4},
            oct_Time(ctx),
            (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE}
    );

    oct_DrawDebugText(
            ctx,
            (Oct_Vec2){0, 0},
            1,
            "Render: %.2fFPS\nLogic: %.2fHz",
            oct_GetRenderFPS(ctx),
            oct_GetLogicHz(ctx)
    );

    oct_DrawText(
            ctx,
            gBitmapFontAtlas,
            (Oct_Vec2){100, 100},
            1,
            "The quick brown fox jumps over the lazy dog.\n!@#$%^&*()_+-={}[]"
    );

    // Check for errors
    if (oct_AssetLoadHasFailed()) {
        const char *s = oct_AssetErrorMessage(gAllocator);
        oct_Raise(OCT_STATUS_ERROR, false, "%s", s);
        oct_Free(gAllocator, (void*)s);
    }

    // Allow toggling fullscreen
    if (oct_KeyPressed(OCT_KEY_F11)) {
        oct_ToggleFullscreen(ctx);
    }

    return null;
}

// Called once when the engine is about to be deinitialized
void shutdown(Oct_Context ctx, void *ptr) {
    oct_FreeAllocator(gAllocator);
}
