cmake_minimum_required(VERSION 3.2.0)

set(YGL_NO_ASSIMP OFF)
set(CMAKE_CXX_STANDARD 20)
set(YGL_STATIC OFF)
set(YGL_NDEBUG OFF)
project(YoghurtGL)


add_subdirectory(libs/)

# YoghurtGL compilation

set(COMPILE_ARGS
--pedantic-errors
# -fsanitize=address
)
set(LINK_ARGS
# -fsanitize=address
)

if (MSVC)
	set(COMPILE_ARGS ${COMPILE_ARGS} \Wall)
else ()
	set(COMPILE_ARGS ${COMPILE_ARGS} -Wall -Wextra -Wpedantic -g --pedantic-errors -O3)
endif ()

if (WIN32)
	set(LIBS glfw)
endif (WIN32)

if (UNIX)
	set(LIBS glfw GL X11)
endif (UNIX)

if (${YGL_NO_ASSIMP})
	set(COMPILE_ARGS ${COMPILE_ARGS} -DYGL_NO_ASSIMP)
	message(NO ASSIMP!!)
else (${YGL_NO_ASSIMP})
	set(LIBS ${LIBS} assimp::assimp)
endif (${YGL_NO_ASSIMP})



file(GLOB_RECURSE YGL_SOURCES
	./src/*.cpp
)
if (${YGL_STATIC})
	add_library(YoghurtGL STATIC ${YGL_SOURCES})
else (${YGL_STATIC})
	add_library(YoghurtGL SHARED ${YGL_SOURCES})
endif (${YGL_STATIC})

target_include_directories(YoghurtGL PUBLIC include)
target_include_directories(YoghurtGL PUBLIC libs/glfw/include)
target_include_directories(YoghurtGL PUBLIC libs/glm/)
target_include_directories(YoghurtGL PUBLIC libs/stb/)
target_include_directories(YoghurtGL PUBLIC libs/assimp/include)
target_link_directories(YoghurtGL PUBLIC libs/glfw/src)
target_link_libraries(YoghurtGL PUBLIC ImGui)
target_link_libraries(YoghurtGL PUBLIC ${LIBS})
if (${YGL_STATIC})
	target_link_libraries(YoghurtGL PUBLIC libglew_static)
else (${YGL_STATIC})
	target_link_libraries(YoghurtGL PUBLIC libglew_shared)
endif (${YGL_STATIC})
target_link_libraries(YoghurtGL PUBLIC ImGuizmo)

target_compile_options(YoghurtGL PRIVATE ${COMPILE_ARGS})
target_link_options(YoghurtGL PRIVATE ${LINK_ARGS})

set_target_properties(YoghurtGL PROPERTIES PUBLIC_HEADER include/)
set_target_properties(YoghurtGL PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
set_target_properties(YoghurtGL PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)

if (${YGL_NDEBUG})
	add_definitions(-DYGL_NDEBUG)
endif (${YGL_NDEBUG})


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

add_executable(testExec tests/test.cpp)
set_target_properties(testExec PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)
target_link_libraries(testExec PUBLIC YoghurtGL)
target_include_directories(testExec PUBLIC ${DOCTEST_INCLUDE_DIR})
target_link_options(testExec PRIVATE ${COMPILE_ARGS})
target_link_options(testExec PRIVATE ${LINK_ARGS})
add_dependencies(testExec YoghurtGL)

include(CTest)
enable_testing()
add_test(NAME allTests COMMAND "../testExec" --build-target=testExec)
