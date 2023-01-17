#include <yoghurtgl.h>
#include <iostream>
#include <assert.h>

bool ygl::gl_init		= false;
bool ygl::glfw_init		= false;
bool ygl::gl_debug_init = false;

int ygl::init() {
	if (ygl::glfw_init) return 0;
	ygl::glfw_init = true;

	glfwSetErrorCallback(ygl::glfwErrorCallback);

	if (!glfwInit()) {
		std::cerr << "glfwInit failed.";
		return 1;
	}

	return 0;
}

int ygl::initDebug() {
	assert(gl_init && "gl context has not been initialized");

	if (ygl::gl_debug_init) return 0;
	ygl::gl_debug_init = true;

	std::cout << "Init debug!" << std::endl;

	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(ygl::yglDebugMessageCallback, 0);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	} else {
		std::cerr << "GL debug context failed to initialize" << std::endl;
		return 1;
	}
	return 0;
}

void GLAPIENTRY ygl::yglDebugMessageCallback(GLenum source, GLenum type, GLuint, GLenum severity, GLsizei,
											 const GLchar *message, const void *) {
	std::cerr << "----------<GL DEBUG MESSAGE>----------\n\tSource: ";
	switch (source) {
		case GL_DEBUG_SOURCE_API: std::cerr << "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cerr << "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: std::cerr << "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION: std::cerr << "Application"; break;
		case GL_DEBUG_SOURCE_OTHER: std::cerr << "Other"; break;
	}
	std::cerr << "\n\tType: ";

	switch (type) {
		case GL_DEBUG_TYPE_ERROR: std::cerr << "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cerr << "Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY: std::cerr << "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: std::cerr << "Performance"; break;
		case GL_DEBUG_TYPE_MARKER: std::cerr << "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP: std::cerr << "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP: std::cerr << "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER: std::cerr << "Other"; break;
	}
	std::cerr << "\n\tSeverity: ";

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH: std::cerr << "high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: std::cerr << "medium"; break;
		case GL_DEBUG_SEVERITY_LOW: std::cerr << "low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "notification"; break;
	}
	std::cerr << "\n\tMessage: " << message << std::endl;
}

void ygl::glfwErrorCallback(int code, const char *err_msg) {
	std::cerr << "GLFW Error " << code << " : \n\t" << err_msg << std::endl;
}

void ygl::terminate() {
	glfwTerminate();
	ygl::gl_debug_init = false;
	ygl::gl_init	   = false;
	ygl::glfw_init	   = false;
}
