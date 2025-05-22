#include "Game.h"
#include <math.h>

Oct_Texture gTexMarble;
Oct_Texture gTexPaladinSheet;
Oct_Sprite gSprPaladinWalkRight;
Oct_Allocator gAllocator;
Oct_Font gPixelFont;
Oct_FontAtlas gPixelFontAtlas;
Oct_FontAtlas gBitmapFontAtlas;
Oct_AssetBundle gAssetBundle;

// Called at the start of the game after engine initialization, whatever you return is passed to update
void *startup() {
    gAllocator = oct_CreateHeapAllocator();
    gTexPaladinSheet = oct_LoadTexture("data/paladin.png");
    gSprPaladinWalkRight = oct_LoadSprite(gTexPaladinSheet, 4, 10, (Oct_Vec2){0, 0}, (Oct_Vec2){32, 32});
    gPixelFont = oct_LoadFont("data/Kingdom.ttf", 20);
    gPixelFontAtlas = oct_CreateFontAtlas(gPixelFont, OCT_NO_ASSET, 32, 128);
    oct_CreateFontAtlas(gPixelFont, gPixelFontAtlas, 0x400, 0x4ff);
    gBitmapFontAtlas = oct_LoadBitmapFont("data/monogram.png", (Oct_Vec2){6, 12}, 32, 160);
    // TODO: Test bitmap font
    // TODO: Test spritesheet xstop code

    gAssetBundle = oct_LoadAssetBundle("data");
    gTexMarble = oct_GetAsset(gAssetBundle, "marble.jpg");

    return null;
}

// Called each logical frame, whatever you return is passed to either the next update or shutdown
void *update(void *ptr) {
    oct_DrawRectangleIntColour(
            OCT_INTERPOLATE_ALL, 1,
            &(Oct_Colour){0, 0.6, 1, 1},
            &(Oct_Rectangle){
                .position = {320 + (cosf(oct_Time()) * 200), 240 + (sinf(oct_Time()) * 200)},
                .size = {40, 40}
            },
            true, 0
    );

    oct_DrawTextureIntExt(
            OCT_INTERPOLATE_ALL, 2,
            gTexMarble,
            (Oct_Vec2){oct_MouseX() + 100, oct_MouseY() + 100},
            (Oct_Vec2){1, 1},
            oct_Time(),
            (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE}
    );

    oct_DrawSpriteIntExt(
            OCT_INTERPOLATE_ALL, 3,
            gSprPaladinWalkRight,
            (Oct_Vec2){oct_MouseX(), oct_MouseY()},
            (Oct_Vec2){4, 4},
            oct_Time(),
            (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE}
    );

    oct_DrawDebugText(
            (Oct_Vec2){0, 0},
            1,
            "Render: %.2fFPS\nLogic: %.2fHz",
            oct_GetRenderFPS(),
            oct_GetLogicHz()
    );

    oct_DrawText(
            gPixelFontAtlas,
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
        oct_ToggleFullscreen();
    }

    return null;
}

// Called once when the engine is about to be deinitialized
void shutdown(void *ptr) {
    oct_FreeAssetBundle(gAssetBundle);
    oct_FreeAllocator(gAllocator);
}
