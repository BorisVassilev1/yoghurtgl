cmake_minimum_required(VERSION 3.0.0)

project(YoghurtGL)


SET(OUT_FILE main)

file(GLOB_RECURSE SOURCES
    ./include/*
    ./src/*.cpp
    ./main.cpp
)

add_executable(${OUT_FILE} ${SOURCES})
target_include_directories(${OUT_FILE} PRIVATE include)

set_target_properties(${OUT_FILE} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ../
)


set(LIBS glfw GL X11 GLEW)
target_link_libraries(${OUT_FILE} ${LIBS})

set(CMAKE_CXX_FLAGS "-O0 -std=c++17")

