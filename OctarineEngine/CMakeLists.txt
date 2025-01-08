# Basic project things
cmake_minimum_required(VERSION 3.17)
project(OctarineEngine C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-misleading-indentation")

# Packages required
find_package(Vulkan)
find_package(SDL2 REQUIRED)
find_package(mimalloc 2.1 REQUIRED)

# Find relavant source files
file(GLOB OCT_C_FILES src/*.c)
file(GLOB C_FILES Vulkan2D/VK2D/*.c)
file(GLOB H_FILES Vulkan2D/VK2D/*.h)
set(VMA_FILES Vulkan2D/VulkanMemoryAllocator/src/VmaUsage.cpp)
if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" AND WIN32)
    set(SDL2_LIBRARIES mingw32 SDL2main SDL2)
endif()

# Build VMA
add_subdirectory(Vulkan2D/VulkanMemoryAllocator)

# Include directories
include_directories(include/ Vulkan2D/ ${SDL2_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})

# Build dll for engine
add_library(${PROJECT_NAME} ${OCT_C_FILES} ${C_FILES} ${H_FILES})

# Linking
target_link_libraries(${PROJECT_NAME} PRIVATE m ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES} GPUOpen::VulkanMemoryAllocator mimalloc-static )