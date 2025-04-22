# Octarine

Octarine is a game engine written in C for C/C++. 

## Notable Features

 + Logic is done on its own thread, drawing must be done by queueing draws for the main thread
 + Performant and safe(r) allocators are provided for the user
 + Fixed time-step with interpolation is the default
 + Flecs built-in and optional
 + Powerful and versatile asset loading system

## Libraries it uses

 + [Vulkan2D](https://github.com/PaoloMazzon/Vulkan2D) - Rendering (zlib)
 + [VulkanMemoryAllocator (VMA)](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Vulkan2D uses under the hood (MIT)
 + [tinyobjloader-c](https://github.com/syoyo/tinyobjloader-c) - Vulkan2D uses under the hood (MIT)
 + [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) - Vulkan2D uses under the hood (MIT)
 + [SDL3](https://www.libsdl.org/) - Windowing/input/threads (zlib)
 + [mi-malloc](https://github.com/microsoft/mimalloc) - General allocations (MIT)
 + [flecs](https://github.com/SanderMertens/flecs?tab=readme-ov-file) - ECS (MIT)
 + [PhysicsFS](https://github.com/icculus/physfs) - Asset Packing (zlib)