#include "oct/Octarine.h"
#include "Game.h"

int main() {
    Oct_InitInfo initInfo = {
            .sType = OCT_STRUCTURE_TYPE_INIT_INFO,
            .startup = startup,
            .update = update,
            .shutdown = shutdown,

            // Change these to what you want
            .windowTitle = "Octarine",
            .windowWidth = 640,
            .windowHeight = 480,
    };
    oct_Init(&initInfo);
    return 0;
}
