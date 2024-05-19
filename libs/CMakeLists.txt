set(GLFW_BUILD_DOCS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
set(ENABLE_BOOST_WORKAROUND ON)

set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_COLLADA_IMPORTER ON)
if (EMSCRIPTEN)
	set(ASSIMP_BUILD_ZLIB ON)
else ()
	set(ASSIMP_BUILD_ZLIB OFF)
endif ()

add_subdirectory(glm)

# glew gives errors when -O3
# set(buildType ${CMAKE_BUILD_TYPE})
# set(CMAKE_BUILD_TYPE Debug)
if (NOT EMSCRIPTEN)
	add_subdirectory(glew-cmake)
endif ()
# set(CMAKE_BUILD_TYPE ${buildType})

if (NOT ${YGL_STATIC})
	MESSAGE("SHARED LIBS BUILDING")
	set(BUILD_SHARED_LIBS ON)
else (NOT ${YGL_STATIC})
	MESSAGE("STATIC LIBS BUILDING")
	set(BUILD_SHARED_LIBS OFF)
endif (NOT ${YGL_STATIC})

if (NOT ${YGL_NO_ASSIMP})
	add_subdirectory(assimp/)
endif ()

if (NOT EMSCRIPTEN)
	add_subdirectory(glfw)
endif()

# Set where the ImGui files are stored
set(IMGUI_PATH  "./imgui")
# Compile as dynamic library
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)

if (${YGL_STATIC})
    add_library("ImGui" STATIC
    ${IMGUI_SOURCES}
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
    )
else (${YGL_STATIC})
    add_library("ImGui" SHARED
    ${IMGUI_SOURCES}
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
    )
endif (${YGL_STATIC})

target_include_directories("ImGui" PUBLIC ${IMGUI_PATH})
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH}/backends/)
target_include_directories("ImGui" PUBLIC glfw/include)
target_link_directories("ImGui" PUBLIC glfw/src)
target_link_libraries("ImGui" PUBLIC glfw)
set_target_properties("ImGui" PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../)


# ImGuizmo target
set(IMGUIZMO_PATH "./ImGuizmo")
file(GLOB IMGUIZMO_SOURCES ${IMGUIZMO_PATH}/*.cpp)

if (${YGL_STATIC})
    add_library("ImGuizmo" STATIC ${IMGUIZMO_SOURCES})
else (${YGL_STATIC})
    add_library("ImGuizmo" SHARED ${IMGUIZMO_SOURCES})
endif (${YGL_STATIC})

target_include_directories("ImGuizmo" PUBLIC ${IMGUIZMO_PATH})
target_link_libraries("ImGuizmo" PUBLIC "ImGui")
set_target_properties("ImGuizmo" PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../)

set(DOCTEST_INCLUDE_DIR ./libs/doctest/doctest/ CACHE INTERNAL "Path to include folder for doctest")

if (NOT ${YGL_NO_ASSIMP})
	set_target_properties(assimp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../../../)
endif (NOT ${YGL_NO_ASSIMP})

if (NOT ${EMSCRIPTEN})
	set_target_properties(glfw PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../../../)
	set_target_properties(libglew_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../../)
endif ()
