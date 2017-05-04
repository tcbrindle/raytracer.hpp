
cmake_minimum_required(VERSION 3.1)

project(raytracer C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wextra")

# For Clang, increase the max number of constexpr steps. A lot.
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-steps=2147483647")
endif()

add_executable(raytracer-ct compile_time.cpp stb_image_write.c)
target_compile_definitions(raytracer-ct PRIVATE "IMAGE_WIDTH=32;IMAGE_HEIGHT=32")

add_executable(raytracer-rt run_time.cpp stb_image_write.c)
