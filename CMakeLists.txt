cmake_minimum_required(VERSION 2.8.3)
project(meme_visualizer)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_definitions("-std=c++11 -O3 -Wno-write-strings") 

include(FindPkgConfig)
pkg_search_module(SDL2 sdl2)

file(GLOB src 
    src/*.cpp
)

include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} include)
add_executable(visualizer ${src})
target_link_libraries(visualizer ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
