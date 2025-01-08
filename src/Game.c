#include "Game.h"

// Called at the start of the game after engine initialization, whatever you return is passed to update
void *startup(Oct_Context ctx) {
    return null;
}

// Called each logical frame, whatever you return is passed to either the next update or shutdown
void *update(Oct_Context ctx, void *ptr) {
    return null;
}

// Called once when the engine is about to be deinitialized
void shutdown(Oct_Context ctx, void *ptr) {

}
