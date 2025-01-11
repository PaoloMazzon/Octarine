#include "Game.h"
#include <math.h>

// Called at the start of the game after engine initialization, whatever you return is passed to update
void *startup(Oct_Context ctx) {
    return null;
}

// Called each logical frame, whatever you return is passed to either the next update or shutdown
void *update(Oct_Context ctx, void *ptr) {
    Oct_DrawCommand cmd = {
            .type = OCT_DRAW_COMMAND_TYPE_RECTANGLE,
            .interpolate = true,
            .id = 1,
            .colour = {1, 1, 1, 1},
            .DrawInfo.Rectangle = {
                    .rectangle = {
                            .size = {20, 20},
                            .position = {310 + (cosf(oct_Time(ctx)) * 200), 230 + (sinf(oct_Time(ctx)) * 200)}
                    },
                    .filled = true
            }
    };
    oct_Draw(ctx, &cmd);
    return null;
}

// Called once when the engine is about to be deinitialized
void shutdown(Oct_Context ctx, void *ptr) {

}
