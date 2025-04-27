# Basic project info
cmake_minimum_required(VERSION 3.17)
project(Octarine)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "-pedantic")

# Compile the engine to link later
add_subdirectory(OctarineEngine)

# Find relavant source files
file(GLOB C_FILES src/*.c)

# Include directories
include_directories(include/ OctarineEngine/include)

# Final executable
add_executable(${PROJECT_NAME} main.c ${C_FILES})
target_link_libraries(${PROJECT_NAME} OctarineEngine)