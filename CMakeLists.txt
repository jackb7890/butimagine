cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT_F}/scripts/buildsystems/vcpkg.cmake)

project(butimagine)

find_package(sdl2 CONFIG REQUIRED)
find_package(sdl2_image CONFIG REQUIRED)
find_package(sdl2_mixer CONFIG REQUIRED)
find_package(sdl2_ttf CONFIG REQUIRED)
find_package(sdl2_net CONFIG REQUIRED)

add_executable(butimagine main.cpp World.cpp)

find_path(SDL2_INCLUDE_DIR NAMES SDL.h PATH_SUFFIXES SDL2)
target_include_directories(butimagine PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(butimagine PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(butimagine PRIVATE SDL2_image::SDL2_image)
target_link_libraries(butimagine PRIVATE SDL2_mixer::SDL2_mixer)
target_link_libraries(butimagine PRIVATE SDL2_ttf::SDL2_ttf)
target_link_libraries(butimagine PRIVATE SDL2_net::SDL2_net)
