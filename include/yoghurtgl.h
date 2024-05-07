#pragma once

/**
 * @file yoghurtgl.h
 * @brief Global defines and debug macros.
 */

#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>

#define GLM_FORCE_SWIZZLE

// Include the Emscripten library only if targetting WebAssembly
#ifdef __EMSCRIPTEN__
	#include <emscripten/emscripten.h>
	#define GLFW_INCLUDE_ES3
	#define GLFW_INCLUDE_GLEXT
using GLdouble = double;
using GLuint   = unsigned int;
	  #define YGL_NO_COMPUTE_SHADERS
	  #define GL_FILL					0
	  #define GL_LINE					1
	  #define glPolygonMode(a, b)		0
	  #define glTextureBarrier()		0
	  #define glObjectLabel(a, b, c, d) 0
	  #define glUniform1d				glUniform1f
	  #define GL_SRGB_ALPHA				GL_SRGB_ALPHA_EXT
	  #define GL_UNIFORM_BLOCK			GL_UNIFORM_BLOCK_EXT
#else
	#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>

#ifdef _WIN32
	#include <Windows.h>
#endif

/**
 * @brief the Yoghurtgl namespace
 */
namespace ygl {

extern bool gl_init;		   ///< is gl initialized
extern bool glfw_init;		   ///< is glfw initialized
extern bool gl_debug_init;	   ///< is gl debug context initialized

/**
 * @brief Initializes Yoghurtgl.
 *
 * @return 0 if it failed
 * @return 1 if it succeeded
 */
int init();
/**
 * @brief Initializes yoghurtgl debug context.
 *
 * @return 0 if it failed
 * @return 1 if it succeeded
 */
int initDebug();
/**
 * @brief OpenGL debug callback. Do not call this!
 */
void GLAPIENTRY yglDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
										const GLchar *message, const void *userParam);
/**
 * @brief GLFW error callback. Do not call this!
 */
void glfwErrorCallback(int code, const char *err_msg);
/**
 * @brief terminates Yoghurtgl
 *
 */
void terminate();

/**
 *
 * @brief Log levels
 */
enum {
	LOG_INFO	= (0),
	LOG_DEBUG	= (1),
	LOG_WARNING = (2),
	LOG_ERROR	= (3),
};

#define COLOR_RESET	 "\033[0m"
#define COLOR_RED	 "\x1B[0;91m"
#define COLOR_GREEN	 "\x1B[0;92m"
#define COLOR_YELLOW "\x1B[0;93m"

static const char *log_colors[]{COLOR_RESET, COLOR_GREEN, COLOR_YELLOW, COLOR_RED};

/**
 * @brief prints endline to std::cerr
 * @return 1
 */
bool inline f_dbLog(std::ostream &out) {
	out << std::endl;
	return 1;
}

/**
 * @brief prints to std::cerr
 *
 * @return 1
 */
template <class T, class... Types>
bool inline f_dbLog(std::ostream &out, T arg, Types... args) {
	out << arg;
	f_dbLog(out, args...);
	return 1;
}

/**
 * @def dbLog(severity, ...)
 * If severity is greater than the definition YGL_LOG_LEVEL, prints all arguments to std::cerr
 */

#ifndef YGL_NDEBUG
	#define YGL_DEBUG
	#define YGL_LOG_LEVEL -1
	#ifdef __linux__
		#define dbLog(severity, ...)                                                                      \
			{                                                                                             \
				if (severity == ygl::LOG_ERROR) {                                                         \
					std::stringstream s;                                                                  \
					ygl::f_dbLog(s, __VA_ARGS__);                                                         \
					ygl::f_dbLog(std::cerr, ygl::log_colors[severity], "[", #severity, "] ", s.str());    \
					std::string cmd = "LC_ALL=C xmessage -default ok \"" + s.str() + "\"";                \
					system(cmd.c_str());                                                                  \
				} else if (severity >= YGL_LOG_LEVEL)                                                     \
					ygl::f_dbLog(std::cerr, ygl::log_colors[severity], "[", #severity, "] ", __VA_ARGS__, \
								 COLOR_RESET);                                                            \
			};
	#elif defined(_WIN32)
		#define dbLog(severity, ...)                                                                      \
			{                                                                                             \
				if (severity == ygl::LOG_ERROR) {                                                         \
					std::stringstream s;                                                                  \
					ygl::f_dbLog(s, __VA_ARGS__);                                                         \
					ygl::f_dbLog(std::cerr, ygl::log_colors[severity], "[", #severity, "] ", s.str());    \
					MessageBox(NULL, s.str().c_str(), "Title!", MB_ICONERROR | MB_OK);                    \
				} else if (severity >= YGL_LOG_LEVEL)                                                     \
					ygl::f_dbLog(std::cerr, ygl::log_colors[severity], "[", #severity, "] ", __VA_ARGS__, \
								 COLOR_RESET);                                                            \
			};
	#elif defined(__EMSCRIPTEN__)
		#define dbLog(severity, ...)                                                                      \
			{                                                                                             \
				if (severity == ygl::LOG_ERROR) {                                                         \
					std::stringstream s;                                                                  \
					ygl::f_dbLog(s, __VA_ARGS__);                                                         \
					ygl::f_dbLog(std::cout, ygl::log_colors[severity], "[", #severity, "] ", s.str());    \
					EM_ASM(alert("[" + #severity + "]" + UTF8ToString($0)), s.str().c_str());             \
				} else if (severity >= YGL_LOG_LEVEL)                                                     \
					ygl::f_dbLog(std::cout, ygl::log_colors[severity], "[", #severity, "] ", __VA_ARGS__, \
								 COLOR_RESET);                                                            \
			}
	#else
		#define dbLog(severity, ...)                                                                                   \
			severity >= YGL_LOG_LEVEL                                                                                  \
				? (ygl::f_dbLog(std::cerr, ygl::log_colors[severity], "[", #severity, "] ", __VA_ARGS__, COLOR_RESET)) \
				: 0;
	#endif
#else
	#define YGL_LOG_LEVEL		 3
	#define dbLog(severity, ...) ((void)0)
#endif

#ifdef _MSC_VER
	#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define THROW_RUNTIME_ERR(message) \
	throw std::runtime_error("At " + std::string(__PRETTY_FUNCTION__) + ":\n\t" + message + COLOR_RESET);

#define DELETE_COPY_AND_ASSIGNMENT(TYPE)         \
	TYPE(const TYPE &other)			   = delete; \
	TYPE &operator=(const TYPE &other) = delete;

}	  // namespace ygl

#define STRINGIFY__(X) #X
#define STRINGIFY(X)   STRINGIFY__(X)

#ifdef __cplusplus
extern "C"
#endif
	const char *
	__asan_default_options();

using uint	= unsigned int;
using uchar = unsigned char;
