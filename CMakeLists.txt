
cmake_minimum_required(VERSION 3.1)

project(raytracer C CXX)

add_executable(raytracer-ct compile_time.cpp stb_image_write.c)
target_compile_definitions(raytracer-ct PRIVATE "IMAGE_WIDTH=32;IMAGE_HEIGHT=32")
if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    target_compile_options(raytracer-ct PRIVATE -Wall -Wextra -fconstexpr-steps=2147483647)
elseif(${CMAKE_COMPILER_IS_GNUCXX})
    target_compile_options(raytracer-ct PRIVATE -Wall -Wextra)
endif()

add_executable(raytracer-rt run_time.cpp stb_image_write.c)

# Require C++17
set_target_properties(raytracer-ct raytracer-rt PROPERTIES
                      CXX_STANDARD 17 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)
