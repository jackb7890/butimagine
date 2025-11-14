cmake_minimum_required(VERSION 3.30)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT_F}/scripts/buildsystems/vcpkg.cmake)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLOG")

project(butimagine)

find_package(sdl2 CONFIG REQUIRED)
find_package(sdl2_image CONFIG REQUIRED)
find_package(sdl2_mixer CONFIG REQUIRED)
find_package(sdl2_ttf CONFIG REQUIRED)
find_package(sdl2_net CONFIG REQUIRED)

find_path(SDL2_INCLUDE_DIR NAMES SDL.h PATH_SUFFIXES SDL2)

add_executable(client src/client.cpp src/sockets/networking.cpp src/World.cpp src/util.cpp)
target_include_directories(client PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(client PRIVATE SDL2_net::SDL2_net)
target_link_libraries(client PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(client PRIVATE SDL2_image::SDL2_image)
target_link_libraries(client PRIVATE SDL2_mixer::SDL2_mixer)
target_link_libraries(client PRIVATE SDL2_ttf::SDL2_ttf)

add_executable(server src/server.cpp src/sockets/networking.cpp src/World.cpp src/util.cpp)
target_include_directories(server PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(server PRIVATE SDL2_net::SDL2_net)
target_link_libraries(server PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(server PRIVATE SDL2_image::SDL2_image)
target_link_libraries(server PRIVATE SDL2_mixer::SDL2_mixer)
target_link_libraries(server PRIVATE SDL2_ttf::SDL2_ttf)