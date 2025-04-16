#include "Game.h"
#include <math.h>

Oct_Texture gTexMarble;
Oct_Allocator gAllocator;

// Called at the start of the game after engine initialization, whatever you return is passed to update
void *startup(Oct_Context ctx) {
    gAllocator = oct_CreateHeapAllocator();
    Oct_LoadCommand load = {
            .type = OCT_LOAD_COMMAND_TYPE_LOAD_TEXTURE,
            .Texture.filename = "data/marble.jpg"
    };
    gTexMarble = oct_Load(ctx, &load);
    return null;
}

// Called each logical frame, whatever you return is passed to either the next update or shutdown
void *update(Oct_Context ctx, void *ptr) {
    Oct_DrawCommand cmd = {
            .type = OCT_DRAW_COMMAND_TYPE_RECTANGLE,
            .interpolate = OCT_INTERPOLATE_ALL,
            .id = 1,
            .colour = {1, 1, 1, 1},
            .Rectangle = {
                    .rectangle = {
                            .position = {320 + (cosf(oct_Time(ctx)) * 200), 240 + (sinf(oct_Time(ctx)) * 200)},
                            .size = {40, 40},
                    },
                    .filled = true,
            }
    };

    Oct_DrawCommand textureCmd = {
            .type = OCT_DRAW_COMMAND_TYPE_TEXTURE,
            .interpolate = OCT_INTERPOLATE_ALL,
            .id = 2,
            .colour = {1, 1, 1, 1},
            .Texture = {
                    .texture = gTexMarble,
                    .position = {oct_MouseX(), oct_MouseY()},
                    .viewport = {
                            .position = {0, 0},
                            .size = {OCT_WHOLE_TEXTURE, OCT_WHOLE_TEXTURE}
                    },
                    .scale = {1, 1},
                    .origin = {OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE},
                    .rotation = oct_Time(ctx)
            }
    };

    oct_Draw(ctx, &cmd);
    oct_Draw(ctx, &textureCmd);

    // Check for errors
    if (oct_AssetLoadHasFailed()) {
        const char *s = oct_AssetErrorMessage(gAllocator);
        oct_Log("%s", s);
        oct_Free(gAllocator, (void*)s);
    }

    return null;
}

// Called once when the engine is about to be deinitialized
void shutdown(Oct_Context ctx, void *ptr) {
    oct_FreeAllocator(gAllocator);
}
