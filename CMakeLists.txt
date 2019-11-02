cmake_minimum_required(VERSION 2.8.3)
project(meme_visualizer)

add_definitions("-std=c++11 -O3 -Wno-write-strings") 

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include(FindPkgConfig)
pkg_search_module(SDL2 sdl2)

file(GLOB src 
    src/*.cpp
    src/color_utils.cpp
    src/pca.cpp
    src/filter_image.cpp
    src/connected_components.cpp
    src/regression.cpp
    src/svm.cpp
    src/utilities.cpp
)

include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} include)
add_executable(visualizer ${src})
target_link_libraries(visualizer ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
