cmake_minimum_required(VERSION 3.0.0)

project(YoghurtGL)


SET(OUT_FILE main)

file(GLOB_RECURSE SOURCES
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
	-L\"C:/msys64/mingw64/lib\" 
	-lglfw3 -lglew32 -lopengl32 -lassimp)
	set(INCLUDES
	C:/msys64/mingw64/include
	)
target_include_directories(${OUT_FILE} PRIVATE ${INCLUDES})
endif (WIN32)
	

if (UNIX)
	set(LIBS glfw GL X11 GLEW assimp)
endif (UNIX)

target_link_libraries(${OUT_FILE} ${LIBS})

set(CMAKE_CXX_FLAGS "-g -std=c++23 -Wall")

