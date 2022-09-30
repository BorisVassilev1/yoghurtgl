cmake_minimum_required(VERSION 3.2.0)

project(YoghurtGL)


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


add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)

file(GLOB_RECURSE SOURCES
	./src/*.cpp
)

if (WIN32)
	set(LIBS -lglfw3 -lglew32 -lopengl32 -lassimp)
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
	target_include_directories(${CURR_TARGET} PUBLIC include)
	target_include_directories(${CURR_TARGET} PUBLIC ${INCLUDES})
	target_include_directories(${CURR_TARGET} PUBLIC libs/glfw/include)
	target_include_directories(${CURR_TARGET} PUBLIC libs/glm/)
	target_link_directories(${CURR_TARGET} PUBLIC libs/glfw/src)
	target_link_libraries(${CURR_TARGET} PUBLIC ImGui)
	target_link_libraries(${CURR_TARGET} PUBLIC ${LIBS})
endforeach(CURR_FILE ${TARGET_FILES})

set(CMAKE_CXX_FLAGS "-g -std=c++23 -Wall")

