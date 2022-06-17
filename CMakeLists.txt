cmake_minimum_required(VERSION 3.0.0)

project(YoghurtGL)


SET(OUT_FILE main)

file(GLOB_RECURSE SOURCES
	#./include/*
	./src/*.cpp
	./main.cpp
)

add_executable(${OUT_FILE} ${SOURCES})
target_include_directories(${OUT_FILE} PRIVATE include)

set_target_properties(${OUT_FILE} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ../
)


if (WIN32)
set(LIBS 
	-L\"D:/cpp_libraries/glfw-3.3.5-compiled/src\" 
	-L\"D:/cpp_libraries/glew-2.1.0-compiled/lib\" 
	-lglfw3 -lglew32 -lopengl32)
	set(INCLUDES
	D:/cpp_libraries/glfw-3.3.5-source/include
	D:/cpp_libraries/glew-2.1.0-source/include
	D:/cpp_libraries/glm)
target_include_directories(${OUT_FILE} PRIVATE ${INCLUDES})
endif (WIN32)
	

if (UNIX)
	set(LIBS glfw GL X11 GLEW assimp)
endif (UNIX)

target_link_libraries(${OUT_FILE} ${LIBS})

set(CMAKE_CXX_FLAGS "-O0 -std=c++17 -Wall")

