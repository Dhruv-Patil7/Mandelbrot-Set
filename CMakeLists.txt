cmake_minimum_required(VERSION 3.20)

project(MandelbrotGPU)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(MandelbrotGPU
    src/main.cpp
    external/glad/src/glad.c
)

target_include_directories(MandelbrotGPU PRIVATE
    external/glad/include
)

target_link_libraries(MandelbrotGPU PRIVATE
    glfw
    OpenGL::GL
)