#pragma once

//#define GLFW_INCLUDE_GLU
#ifdef _WIN32
	#define GLFW_DLL	 // -> windows
#elif __linux__
//
#endif

//#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace ygl {

extern bool gl_init;
extern bool glfw_init;
extern bool gl_debug_init;

int				init();
int				initDebug();
void GLAPIENTRY yglDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
										const GLchar *message, const void *userParam);
void			glfwErrorCallback(int code, const char *err_msg);
void			terminate();

enum {
	LOG_ERROR = (3),
	LOG_WARNING = (2),
	LOG_DEBUG = (1),
	LOG_INFO = (0),
};

bool inline f_dbLog() {
	std::cout << std::endl;
	return 1;
}

template <class T, class... Types>
bool inline f_dbLog(T arg, Types... args) {
	std::cout << arg;
	f_dbLog(args...);
	return 1;
}

#ifndef NDEBUG
	#define YGL_LOG_LEVEL		 -1
	#define dbLog(severity, ...) severity >= YGL_LOG_LEVEL ? ygl::f_dbLog("[", #severity, "]", __VA_ARGS__) : 0;
#else
	#define YGL_LOG_LEVEL		 3
	#define dbLog(severity, ...) 0
#endif

}	  // namespace ygl
