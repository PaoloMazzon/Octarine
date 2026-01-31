# Basic project things
cmake_minimum_required(VERSION 3.20)
project(OctarineEngine)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# Add dependencies
find_package(Freetype REQUIRED)
add_subdirectory(Vulkan2D)
add_subdirectory(SDL_ttf)
add_subdirectory(physfs)

# Find relavant source files
file(GLOB OCT_C_FILES src/*.c)

# Include directories
include_directories(Vulkan2D/ physfs/src/ SDL_ttf/include Vulkan2D/VulkanMemoryAllocator/include include/ Vulkan2D/SDL/include ${Vulkan_INCLUDE_DIRS})

# Build dll for engine
add_library(${PROJECT_NAME} STATIC ${VMA_FILES} ${OCT_C_FILES} ${C_FILES} ${H_FILES} ${SDL_SOUND_C})

# Linking
target_link_libraries(${PROJECT_NAME} PRIVATE mimalloc-static Vulkan2D physfs SDL3_ttf::SDL3_ttf)