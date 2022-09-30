cmake_minimum_required(VERSION 3.2.0)

project(YoghurtGL)

add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)
# add_subdirectory(libs/assimp)
add_subdirectory(libs/glew-cmake)


# Set where the ImGui files are stored
set(IMGUI_PATH  "./libs/imgui")    
# Compile as static library 
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp) 
add_library("ImGui" STATIC 
${IMGUI_SOURCES} 
${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
)
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH})
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH}/backends/)
target_include_directories("ImGui" PUBLIC libs/glfw/include)
target_link_directories("ImGui" PUBLIC libs/glfw/src)


file(GLOB_RECURSE SOURCES
	./src/*.cpp
)

if (WIN32)
	set(LIBS glfw assimp)
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
	target_include_directories(${CURR_TARGET} PRIVATE libs/glfw/include)
	target_include_directories(${CURR_TARGET} PRIVATE libs/glm/)
	# target_include_directories(${CURR_TARGET} PUBLIC build/libs/assimp/include)
	# target_include_directories(${CURR_TARGET} PUBLIC libs/assimp/include)

	target_link_directories(${CURR_TARGET} PRIVATE libs/glfw/src)
	# target_link_directories(${CURR_TARGET} PUBLIC libs/assimp/code)
	target_link_libraries(${CURR_TARGET} PRIVATE ImGui)
	target_link_libraries(${CURR_TARGET} PRIVATE ${LIBS})
	target_link_libraries(${CURR_TARGET} PRIVATE libglew_static)
endforeach(CURR_FILE ${TARGET_FILES})

set(CMAKE_CXX_FLAGS "-g -std=c++23 -Wall")

