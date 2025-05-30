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

// Little warrior dude
typedef struct Warrior_t {
    Oct_Vec2 position;
    Oct_Vec2 velocity;
    uint64_t id;
    Oct_SpriteInstance sprite;
} Warrior;

// Warriors in the room
const int WARRIOR_COUNT = 5000;
Warrior *gWarriorList;

// Room size
const Oct_Rectangle ROOM_BOUNDS = {
        .position = {0, 0},
        .size = {1280, 720}
};

// Info needed for each warrior job
typedef struct WarriorJobInfo_t {
    int startingIndex;
    int length;
    Warrior *warriorList;
} WarriorJobInfo;

// Updates a group of warriors
void warriorJob(void *ptr) {
    WarriorJobInfo *info = ptr;
    for (int i = info->startingIndex; i < info->startingIndex + info->length; i++) {
        Warrior *warrior = &info->warriorList[i];
        warrior->position[0] += warrior->velocity[0];
        warrior->position[1] += warrior->velocity[1];
        if (warrior->position[0] < ROOM_BOUNDS.position[0] || warrior->position[0] > ROOM_BOUNDS.position[0] + ROOM_BOUNDS.size[0])
            warrior->velocity[0] *= -1;
        if (warrior->position[1] < ROOM_BOUNDS.position[1] || warrior->position[1] > ROOM_BOUNDS.position[1] + ROOM_BOUNDS.size[1])
            warrior->velocity[1] *= -1;
    }
}

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
    gWarriorList = oct_Malloc(gAllocator, sizeof(struct Warrior_t) * WARRIOR_COUNT);
    for (int i = 0; i < WARRIOR_COUNT; i++) {
        gWarriorList[i].id = 1000 + i;
        gWarriorList[i].position[0] = oct_Random(ROOM_BOUNDS.position[0], ROOM_BOUNDS.size[0]);
        gWarriorList[i].position[1] = oct_Random(ROOM_BOUNDS.position[1], ROOM_BOUNDS.size[1]);
        gWarriorList[i].velocity[0] = oct_Random(-3, 3);
        gWarriorList[i].velocity[1] = oct_Random(-3, 3);
        oct_InitSpriteInstance(&gWarriorList[i].sprite, gSprPaladinWalkRight, true);
        gWarriorList[i].sprite.frame = i;
    }

    return null;
}

// Called each logical frame, whatever you return is passed to either the next update or shutdown
void *update(void *ptr) {
    oct_DrawClear(&(Oct_Colour){0, 0.6, 1, 1});

    // Update warriors
    const int jobCount = 10;
    WarriorJobInfo jobInfo[10] = {0};
    for (int i = 0; i < jobCount; i++) {
        jobInfo[i].warriorList = gWarriorList;
        jobInfo[i].length = WARRIOR_COUNT / jobCount;
        jobInfo[i].startingIndex = (WARRIOR_COUNT / jobCount) * i;
        oct_QueueJob(warriorJob, &jobInfo[i]);
    }

    // Draw warriors
    oct_WaitJobs();
    for (int i = 0; i < WARRIOR_COUNT; i++) {
        oct_DrawSpriteInt(OCT_INTERPOLATE_ALL, gWarriorList[i].id, gSprPaladinWalkRight, &gWarriorList[i].sprite, gWarriorList[i].position);
    }

    oct_DrawText(
            gPixelFontAtlas,
            (Oct_Vec2){320, 250},
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
