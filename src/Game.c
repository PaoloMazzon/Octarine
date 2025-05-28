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

typedef struct Warrior_t {
    Oct_Vec2 position;
    Oct_Vec2 velocity;
    uint64_t id;
    Oct_SpriteInstance sprite;
} Warrior;
Warrior *warriorList;
const int WARRIOR_COUNT = 1000;
const Oct_Rectangle roomBounds = {
        .position = {0, 0},
        .size = {1280, 720}
};

// Called at the start of the game after engine initialization, whatever you return is passed to update
void *startup() {
    gAllocator = oct_CreateHeapAllocator();
    gAssetBundle = oct_LoadAssetBundle("data");
    gTexMarble = oct_GetAsset(gAssetBundle, "marble.jpg");
    gSprPaladinWalkRight = oct_GetAsset(gAssetBundle, "sprites/paladin.json");
    gPixelFont = oct_GetAsset(gAssetBundle, "ubuntu");
    gPixelFontAtlas = oct_CreateFontAtlas(gPixelFont, OCT_NO_ASSET, 20, 32, 128);
    oct_CreateFontAtlas(gPixelFont, gPixelFontAtlas, 20, 0x400, 0x4ff);

    // Create some test characters
    warriorList = oct_Malloc(gAllocator, sizeof(struct Warrior_t) * WARRIOR_COUNT);
    for (int i = 0; i < WARRIOR_COUNT; i++) {
        warriorList[i].id = 1000 + i;
        warriorList[i].position[0] = oct_Random(roomBounds.position[0], roomBounds.size[0]);
        warriorList[i].position[1] = oct_Random(roomBounds.position[1], roomBounds.size[1]);
        warriorList[i].velocity[0] = oct_Random(-3, 3);
        warriorList[i].velocity[1] = oct_Random(-3, 3);
        oct_InitSpriteInstance(&warriorList[i].sprite, gSprPaladinWalkRight, true);
        warriorList[i].sprite.frame = i;
    }

    return null;
}

// Called each logical frame, whatever you return is passed to either the next update or shutdown
void *update(void *ptr) {
    oct_DrawClear(&(Oct_Colour){0, 0.6, 1, 1});

    // Update and draw warriors
    for (int i = 0; i < WARRIOR_COUNT; i++) {
        // Update warrior
        warriorList[i].position[0] += warriorList[i].velocity[0];
        warriorList[i].position[1] += warriorList[i].velocity[1];
        if (warriorList[i].position[0] < roomBounds.position[0] || warriorList[i].position[0] > roomBounds.position[0] + roomBounds.size[0])
            warriorList[i].velocity[0] *= -1;
        if (warriorList[i].position[1] < roomBounds.position[1] || warriorList[i].position[1] > roomBounds.position[1] + roomBounds.size[1])
            warriorList[i].velocity[1] *= -1;

        // Draw warrior
        oct_DrawSpriteInt(OCT_INTERPOLATE_ALL, warriorList[i].id, gSprPaladinWalkRight, &warriorList[i].sprite, warriorList[i].position);
    }

    oct_DrawText(
            gPixelFontAtlas,
            (Oct_Vec2){320, 100},
            1,
            "The quick brown fox jumps over the lazy dog.\n!@#$%^&*()_+-={}[]"
    );

    // Allow toggling fullscreen
    if (oct_KeyPressed(OCT_KEY_F11)) {
        oct_ToggleFullscreen();
    }
    if (oct_KeyPressed(OCT_KEY_SPACE)) {
        oct_PlaySound(oct_GetAsset(gAssetBundle, "test.wav"), (Oct_Vec2){1, 1}, false);
    }

    return null;
}

// Called once when the engine is about to be deinitialized
void shutdown(void *ptr) {
    oct_FreeAssetBundle(gAssetBundle);
    oct_FreeAllocator(gAllocator);
}
