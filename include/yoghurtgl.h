#pragma once

/**
 * @file yoghurtgl.h
 * @brief Global defines and debug macros.
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>


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
	LOG_ERROR	= (3),
	LOG_WARNING = (2),
	LOG_DEBUG	= (1),
	LOG_INFO	= (0),
};

#define COLOR_RESET	 "\033[0m"
#define COLOR_RED	 "\x1B[0;91m"
#define COLOR_YELLOW "\x1B[0;93m"

static const char *log_colors[]{
	COLOR_RESET,
	COLOR_RESET,
	COLOR_YELLOW,
	COLOR_RED
};

/**
 * @brief prints endline to std::cerr
 * @return 1
 */
bool inline f_dbLog() {
	std::cerr << std::endl;
	return 1;
}

/**
 * @brief prints to std::cerr
 *
 * @return 1
 */
template <class T, class... Types>
bool inline f_dbLog(T arg, Types... args) {
	std::cerr << arg;
	f_dbLog(args...);
	return 1;
}

/**
 * @def dbLog(severity, ...)
 * If severity is greater than the definition YGL_LOG_LEVEL, prints all arguments to std::cerr
 */

#ifndef YGL_NDEBUG
	#define YGL_DEBUG
	#define YGL_LOG_LEVEL -1
	#define dbLog(severity, ...)                                                                                    \
		severity >= YGL_LOG_LEVEL ? ygl::f_dbLog(ygl::log_colors[severity], "[", #severity, "] ", \
												 __VA_ARGS__, COLOR_RESET)                                          \
								  : 0;
#else
	#define YGL_LOG_LEVEL		 3
	#define dbLog(severity, ...) ((void)0)
#endif


#define THROW_RUNTIME_ERR(message) \
	throw std::runtime_error("At " + std::string(__PRETTY_FUNCTION__) + ":\n\t" + message + COLOR_RESET);

}	  // namespace ygl

using uint	= unsigned int;
using uchar = unsigned char;

