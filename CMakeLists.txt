cmake_minimum_required(VERSION 3.0.0)

project(YoghurtGL)


file(GLOB_RECURSE SOURCES
	./src/*.cpp
)

if (WIN32)
	set(LIBS 
		-L\"C:/msys64/mingw64/lib\" 
		-lglfw3 -lglew32 -lopengl32 -lassimp)
	set(INCLUDES
		C:/msys64/mingw64/include)
endif (WIN32)
	
	
if (UNIX)
	set(LIBS glfw GL X11 GLEW assimp)
endif (UNIX)


file(GLOB_RECURSE TARGET_FILES examples/*.cpp)

foreach(CURR_FILE ${TARGET_FILES})
	get_filename_component(CURR_TARGET ${CURR_FILE} NAME_WE)

	add_executable(${CURR_TARGET} ${SOURCES} ${CURR_FILE})
	set_target_properties(${CURR_TARGET} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ../
	)
	target_include_directories(${CURR_TARGET} PRIVATE include)
	target_include_directories(${CURR_TARGET} PRIVATE ${INCLUDES})
	target_link_libraries(${CURR_TARGET} ${LIBS})
endforeach(CURR_FILE ${TARGET_FILES})

set(CMAKE_CXX_FLAGS "-g -std=c++23 -Wall")

