cmake_minimum_required(VERSION 3.25.0)

set(YGL_NO_ASSIMP OFF CACHE BOOL "Link Assimp as a dependency")
set(CMAKE_CXX_STANDARD 20)
set(YGL_STATIC ON CACHE BOOL "link as static library")
set(ASAN_DETECT_LEAKS ON CACHE BOOL "turn the leak sanitizer on or off")
project(YoghurtGL)

message("emscripten: " ${EMSCRIPTEN})
if (EMSCRIPTEN)
	message("using emscripten!!!!")
else ()
	message("no emscripten!!!!")
endif ()

if(WIN32)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-O3")
	set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "")
else(WIN32)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fPIC -stdlib=libc++ -O3")
	set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-fPIC")
endif (WIN32)


add_subdirectory(libs/)

# YoghurtGL compilation


if (MSVC)
	set(COMPILE_ARGS ${COMPILE_ARGS} /Wall)
else ()
	set(COMPILE_ARGS
		#-fsanitize=address
	)
	set(LINK_ARGS
		#-fsanitize=address
	)
	set(COMPILE_ARGS ${COMPILE_ARGS} -Wall -Wextra -Wpedantic --pedantic-errors -g -O3)
endif ()

if (WIN32)
	set(LIBS glfw)
endif (WIN32)

if (UNIX)
	set(LIBS glfw GL)
endif (UNIX)

if (${YGL_NO_ASSIMP})
	set(COMPILE_ARGS ${COMPILE_ARGS} -DYGL_NO_ASSIMP)
	message("NO ASSIMP!!")
else (${YGL_NO_ASSIMP})
	MESSAGE("ASSIMP LIBRARIES: " ${ASSIMP_LIBRARIES})
	set(LIBS ${LIBS} ${ASSIMP_LIBRARIES} assimp::assimp)
endif (${YGL_NO_ASSIMP})

if (${ASAN_DETECT_LEAKS})
	set(COMPILE_ARGS ${COMPILE_ARGS} -DASAN_DETECT_LEAKS=1)
else (${ASAN_DETECT_LEAKS})
	set(COMPILE_ARGS ${COMPILE_ARGS} -DASAN_DETECT_LEAKS=0)
endif (${ASAN_DETECT_LEAKS})

file(GLOB_RECURSE YGL_SOURCES
	./src/*.cpp
)
if (${YGL_STATIC})
	message("linking static")
	add_library(YoghurtGL STATIC ${YGL_SOURCES})
else (${YGL_STATIC})
	message("linking dynamic")
	add_library(YoghurtGL SHARED ${YGL_SOURCES})
endif (${YGL_STATIC})

target_include_directories(YoghurtGL PUBLIC include)
target_include_directories(YoghurtGL PUBLIC libs/glfw/include)
target_include_directories(YoghurtGL PUBLIC libs/glm/)
target_include_directories(YoghurtGL PUBLIC libs/stb/)
if (NOT ${YGL_NO_ASSIMP})
	target_include_directories(YoghurtGL PUBLIC libs/assimp/include)
endif (NOT ${YGL_NO_ASSIMP})
	target_link_directories(YoghurtGL PUBLIC libs/glfw/src)
target_link_libraries(YoghurtGL PUBLIC ImGui)
target_link_libraries(YoghurtGL PUBLIC ${LIBS})

if (NOT EMSCRIPTEN)
	if (${YGL_STATIC})
		target_link_libraries(YoghurtGL PUBLIC libglew_static)
	else (${YGL_STATIC})
		target_link_libraries(YoghurtGL PUBLIC libglew_shared)
	endif (${YGL_STATIC})
endif ()
target_link_libraries(YoghurtGL PUBLIC ImGuizmo)

target_compile_options(YoghurtGL PRIVATE ${COMPILE_ARGS})
target_link_options(YoghurtGL PRIVATE ${LINK_ARGS})
target_link_options(YoghurtGL PRIVATE ${COMPILE_ARGS})

set_target_properties(YoghurtGL PROPERTIES PUBLIC_HEADER include/)
set_target_properties(YoghurtGL PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
set_target_properties(YoghurtGL PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)


message("BUILD TYPE: " ${CMAKE_BUILD_TYPE})
if (${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-DYGL_NDEBUG)
endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

file(GLOB_RECURSE TARGET_FILES examples/*.cpp)

foreach(CURR_FILE ${TARGET_FILES})
	get_filename_component(CURR_TARGET ${CURR_FILE} NAME_WE)

	add_executable(${CURR_TARGET} ${CURR_FILE})
	set_target_properties(${CURR_TARGET} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ../
	)
	target_link_libraries(${CURR_TARGET} PRIVATE YoghurtGL)

	target_compile_options(${CURR_TARGET} PRIVATE ${COMPILE_ARGS})
	target_link_options(${CURR_TARGET} PRIVATE ${COMPILE_ARGS})
	target_link_options(${CURR_TARGET} PRIVATE ${LINK_ARGS})

endforeach(CURR_FILE ${TARGET_FILES})

add_executable(testExec tests/test.cpp)
set_target_properties(testExec PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)
target_link_libraries(testExec PUBLIC YoghurtGL)
target_include_directories(testExec PUBLIC ${DOCTEST_INCLUDE_DIR})
target_compile_options(testExec PRIVATE ${COMPILE_ARGS})
target_link_options(testExec PRIVATE ${COMPILE_ARGS})
target_link_options(testExec PRIVATE ${LINK_ARGS})
add_dependencies(testExec YoghurtGL)

include(CTest)
enable_testing()
add_test(NAME allTests COMMAND "../testExec" --build-target=testExec)

if (EMSCRIPTEN)
	add_executable(wasm examples/noScene.cpp)
	set_target_properties(wasm PROPERTIES
		SUFFIX ".html"
		LINK_FLAGS "-s USE_WEBGL2=1 -s FULL_ES3 -s USE_GLFW=3 -s WASM=1 -s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1 -Os --use-preload-plugins --preload-file ../shaders@shaders/"
		)
	target_link_libraries(wasm PRIVATE YoghurtGL)
endif ()
