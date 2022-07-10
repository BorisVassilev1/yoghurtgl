#include <window.h>
#include <yoghurtgl.h>

#include <iostream>
#include <iomanip>
#include <assert.h>

#define GL_CONTEXT_VERSION_MAJOR 4
#define GL_CONTEXT_VERSION_MINOR 6

ygl::Window::Window(int width, int height, const char *name, bool vsync, bool resizable,  GLFWmonitor *monitor)
	: width(width), height(height) {
	assert(glfwGetPrimaryMonitor() != NULL);
	const GLFWvidmode *mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_CONTEXT_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_CONTEXT_VERSION_MINOR);

	if (resizable) glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	else glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, name, monitor, NULL);
	if (!window) {
		std::cerr << "glfwCreateWindow failed." << std::endl;
		glfwTerminate();
		exit(1);
	}

	glfwSetWindowCloseCallback(window, [](GLFWwindow *window) { std::cerr << "closing window!"; });
	glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
	});
	glfwSetWindowSizeCallback(window, handleResize);
	addResizeCallback([this](GLFWwindow *window, int width, int height) {
		if (window != getHandle()) return;
		this->width	 = width;
		this->height = height;
	});

	glfwMakeContextCurrent(window);
	ygl::gl_init = true;

	glfwSwapInterval(vsync);

	if (glewInit() != GLEW_OK) {
		std::cerr << "glewInit failed." << std::endl;
		this->~Window();
	}

#ifndef NDEBUG
	ygl::initDebug();
#endif

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
}

ygl::Window::Window(int width, int height, const char *name, bool vsync, bool resizable) : Window(width, height, name, vsync, resizable, NULL) {}

ygl::Window::Window(int width, int height, const char *name, bool vsync) : Window(width, height, name, vsync, true) {}

ygl::Window::Window(int width, int height, const char *name) : Window(width, height, name, true) {}

int ygl::Window::getWidth() { return width; }
int ygl::Window::getHeight() { return height; }

GLFWwindow *ygl::Window::getHandle() { return window; }

bool ygl::Window::shouldClose() { return glfwWindowShouldClose(window); }

void ygl::Window::swapBuffers() {
	auto timeNow = std::chrono::high_resolution_clock::now();
	long delta	 = std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - lastSwapTime).count();

	glfwSwapBuffers(window);

	timeNow		   = std::chrono::high_resolution_clock::now();
	long fullDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - lastSwapTime).count();
	deltaTime	   = fullDelta / 1e9;

	double glfwNow	= glfwGetTime();
	double logDelta = glfwNow - lastPrintTime;

	if (logDelta > 1.) {
		frameCallback(delta, fullDelta);
		lastPrintTime = glfwNow;
	}

	lastSwapTime = timeNow;
}

void ygl::Window::beginFrame() {
	glfwPollEvents();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

ygl::Window::~Window() {
	if (!ygl::glfw_init) return;
	if (window != nullptr) {
		glfwDestroyWindow(window);
		ygl::gl_init = false;
		window		 = nullptr;
	}
}

bool ygl::Window::isFocused() { return glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE; }

void ygl::Window::handleResize(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
	for (auto callback : resizeCallbacks) {
		callback(window, width, height);
	}
}

void ygl::Window::addResizeCallback(std::function<void(GLFWwindow *, int, int)> callback) {
	resizeCallbacks.push_back(callback);
}

void ygl::Window::defaultFrameCallback(long draw_time, long frame_time) {
	std::cout << std::fixed << std::setprecision(2) << "\rdraw time: " << (draw_time / 1e6)
			  << "ms, FPS: " << int(1e9 / frame_time) << "         " << std::flush;	 //<< std::endl;
}

void ygl::Window::setFrameCallback(void (*callback)(long, long)) { frameCallback = callback; }