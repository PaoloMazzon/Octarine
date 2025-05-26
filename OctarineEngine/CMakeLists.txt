# Basic project things
cmake_minimum_required(VERSION 3.17)
project(OctarineEngine)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# Packages required
find_package(Vulkan)
find_package(mimalloc 2.1 REQUIRED)
find_package(Freetype REQUIRED)

# Compile flecs
add_subdirectory(flecs)
add_subdirectory(Vulkan2D/SDL)
add_subdirectory(SDL_ttf)
add_subdirectory(physfs)

# Find relavant source files
file(GLOB OCT_C_FILES src/*.c)
file(GLOB C_FILES Vulkan2D/VK2D/*.c)
file(GLOB H_FILES Vulkan2D/VK2D/*.h)
file(GLOB SDL_SOUND_C SDL_sound/src/*.c)
set(VMA_FILES Vulkan2D/VulkanMemoryAllocator/src/VmaUsage.cpp)
if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" AND WIN32)
    set(MINGW_LIBRARIES mingw32 m)
endif()

# Include directories
include_directories(Vulkan2D/ SDL_sound/include physfs/src/ SDL_ttf/include Vulkan2D/VulkanMemoryAllocator/include include/ Vulkan2D/SDL/include ${Vulkan_INCLUDE_DIRS})

# Build dll for engine
add_library(${PROJECT_NAME} STATIC ${VMA_FILES} ${OCT_C_FILES} ${C_FILES} ${H_FILES})

# Linking
target_link_libraries(${PROJECT_NAME} PRIVATE physfs SDL3::SDL3 SDL3_ttf::SDL3_ttf ${MINGW_LIBRARIES} ${Vulkan_LIBRARIES} mimalloc-static flecs)