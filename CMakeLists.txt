cmake_minimum_required(VERSION 3.2.0)

set(GLFW_BUILD_DOCS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

project(YoghurtGL)

add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)
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

# ImGuizmo target
set(IMGUIZMO_PATH "./libs/ImGuizmo")
file(GLOB IMGUIZMO_SOURCES ${IMGUIZMO_PATH}/*.cpp)
add_library("ImGuizmo" STATIC ${IMGUIZMO_SOURCES})
target_include_directories("ImGuizmo" PUBLIC ${IMGUIZMO_PATH})
target_link_libraries("ImGuizmo" PUBLIC "ImGui")

# YoghurtGL compilation
if (WIN32)
	set(LIBS glfw assimp)
endif (WIN32)

if (UNIX)
	set(LIBS glfw GL X11 GLEW assimp)
endif (UNIX)


file(GLOB_RECURSE YGL_SOURCES
	./src/*.cpp
)
add_library("YoghurtGL" STATIC ${YGL_SOURCES})
target_include_directories("YoghurtGL" PUBLIC include)
target_include_directories("YoghurtGL" PUBLIC libs/glfw/include)
target_include_directories("YoghurtGL" PUBLIC libs/glm/)
target_link_directories("YoghurtGL" PUBLIC libs/glfw/src)
target_link_libraries("YoghurtGL" PUBLIC ImGui)
target_link_libraries("YoghurtGL" PUBLIC ${LIBS})
target_link_libraries("YoghurtGL" PUBLIC libglew_static)
target_link_libraries("YoghurtGL" PUBLIC ImGuizmo)

set(COMPILE_ARGS
-Wall -Wextra --pedantic-errors -g
# -DYGL_NO_ASSIMP
# -DNDEBUG
# -fsanitize=address
)
set(LINK_ARGS
# -fsanitize=address
)

target_compile_options("YoghurtGL" PRIVATE ${COMPILE_ARGS})
target_link_options("YoghurtGL" PRIVATE ${LINK_ARGS})

file(GLOB_RECURSE TARGET_FILES examples/*.cpp)

foreach(CURR_FILE ${TARGET_FILES})
	get_filename_component(CURR_TARGET ${CURR_FILE} NAME_WE)

	add_executable(${CURR_TARGET} ${CURR_FILE})
	set_target_properties(${CURR_TARGET} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ../
	)
	target_link_libraries(${CURR_TARGET} PRIVATE YoghurtGL)

	target_link_options(${CURR_TARGET} PRIVATE ${COMPILE_ARGS})
	target_link_options(${CURR_TARGET} PRIVATE ${LINK_ARGS})

endforeach(CURR_FILE ${TARGET_FILES})