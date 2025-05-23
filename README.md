# Octarine

Octarine is a game engine written in C for C/C++ designed to be used on 2D games with fixed-timestep logic. The
renderer is able to automatically interpolate draws to render at unlocked framerates while the user need not concern
themselves with anything other than game logic.

*Warning*: This engine is in early development still, and while it is usable for some small games at this moment, it may
still receive breaking changes in the near future.

## Notable Features

 + Logic is done on its own thread, drawing must be done by queueing draws for the main thread
 + Fixed-timestep logic with interpolation makes physics incredibly simple without sacrificing visuals
 + Performant and safe(r) allocators are provided for the user
 + Fixed time-step with interpolation is the default and incredibly simple to use
 + Robust [asset bundle system](docs/AssetBundle.md) that works with directories or archives (Thanks PhysFS!)

## Example
This example program displays a rectangle going in circles, where the rectangle will be drawn at an uncapped, interpolated
framerate but the `update` function will only update 30 times a second. To see the difference, draw another
rectangle near the original one without interpolation and you will see the difference clear as day. 

```c
#include <oct/Octarine.h>
#include <math.h>

void *startup() {
    // ...
}

void *update(void *ptr) {
    oct_DrawRectangleInt(
        ctx, 
        OCT_INTERPOLATE_ALL, 1,
        &(Oct_Rectangle){
            .position = {320 + (cosf(oct_Time(ctx)) * 200), 240 + (sinf(oct_Time(ctx)) * 200)},
            .size = {40, 40}
        },
        true, 0
    );
}

void shutdown(void *ptr) {
    // ...
}

int main(int argc, const char **argv) {
    Oct_InitInfo initInfo = {
            .sType = OCT_STRUCTURE_TYPE_INIT_INFO,
            .startup = startup,
            .update = update,
            .shutdown = shutdown,
            .argc = argc,
            .argv = argv,
            .windowTitle = "Octarine",
            .windowWidth = 640,
            .windowHeight = 480,
            .debug = true,
    };

    oct_Init(&initInfo);
    return 0;
}
```

## Libraries it uses
 
| Library | Purpose | License |
|---------|---------|---------|
| [Vulkan2D](https://github.com/PaoloMazzon/Vulkan2D) | Rendering | zlib |
| [VulkanMemoryAllocator (VMA)](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) | Vulkan2D uses it | MIT |
| [tinyobjloader-c](https://github.com/syoyo/tinyobjloader-c) | Vulkan2D uses it | MIT |
| [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) | Vulkan2D uses it | MIT |
| [SDL3](https://www.libsdl.org/) | Windowing/input/threads/audio | zlib |
| [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf) | Font rendering | zlib |
| [FreeType](https://gitlab.freedesktop.org/freetype/freetype/-/blob/master/docs/FTL.TXT) | SDL3_ttf uses it | FTL |
| [HarfBuzz](https://github.com/harfbuzz/harfbuzz/blob/main/COPYING) | SDL3_ttf uses it | MIT |
| [PlutoSVG](https://github.com/sammycage/plutosvg/blob/master/LICENSE) | SDL3_ttf uses it | MIT |
| [PlutoVG](https://github.com/sammycage/plutovg/blob/master/LICENSE) | SDL3_ttf uses it | MIT |
| [mi-malloc](https://github.com/microsoft/mimalloc) | Memory management | MIT |
| [flecs](https://github.com/SanderMertens/flecs?tab=readme-ov-file) | ECS | MIT |
| [PhysicsFS](https://github.com/icculus/physfs) | Asset system | zlib |
| [minivorbis](https://github.com/edubart/minivorbis) | Parsing OGGs | BSD 3-Clause |
| [minimp3](https://github.com/lieff/minimp3) | Parsing MP3s | CC0 |
| [cJSON](https://github.com/DaveGamble/cJSON) | Parsing JSONs | MIT |