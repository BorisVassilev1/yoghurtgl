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
}	  // namespace ygl