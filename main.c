#include "oct/Octarine.h"
#include "Game.h"

int main(int argc, const char **argv) {
    // Information Octarine needs to start the game
    Oct_InitInfo initInfo = {
            .sType = OCT_STRUCTURE_TYPE_INIT_INFO,
            .startup = startup,
            .update = update,
            .shutdown = shutdown,
            .argc = argc,
            .argv = argv,

            // Change these to what you want
            .windowTitle = "Octarine",
            .windowWidth = 640,
            .windowHeight = 480,
            .debug = true,
    };

    // Launches the game with the above parameters
    oct_Init(&initInfo);
    return 0;
}
