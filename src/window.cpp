#include <window.h>

#include <iostream>
#include <iomanip>

#define GL_CONTEXT_VERSION_MAJOR 4
#define GL_CONTEXT_VERSION_MINOR 6

ygl::Window::Window(int width, int height, const char *name, bool vsync, GLFWmonitor *monitor) {
	const GLFWvidmode *mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_CONTEXT_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_CONTEXT_VERSION_MINOR);

	window = glfwCreateWindow(width, height, name, monitor, NULL);
	if (!window) {
		std::cerr << "glfwCreateWindow failed." << std::endl;
		glfwTerminate();
		exit(1);
	}

	glfwSetWindowCloseCallback(window, [](GLFWwindow *window) {
		std::cerr << "closing window!";
	});
	glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	});
	glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
		width  = width;
		height = height;
	});

	glfwMakeContextCurrent(window);

	glfwSwapInterval(vsync);

	if (glewInit() != GLEW_OK) {
		std::cerr << "glewInit failed." << std::endl;
		destroy();
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
}

ygl::Window::Window(int width, int height, const char *name, bool vsync) : Window(width, height, name, vsync, NULL) {}

ygl::Window::Window(int width, int height, const char *name) : Window(width, height, name, 0) {}

int ygl::Window::getWidth() { return width; }
int ygl::Window::getHeight() { return height; }

bool ygl::Window::shouldClose() { return glfwWindowShouldClose(window); }

void ygl::Window::swapBuffers() {
	auto timeNow = std::chrono::high_resolution_clock::now();
	long delta	 = std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - lastSwapTime).count();

	glfwSwapBuffers(window);

	timeNow		   = std::chrono::high_resolution_clock::now();
	long fullDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - lastSwapTime).count();

	double glfwNow	= glfwGetTime();
	double logDelta = glfwNow - lastPrintTime;

	if (logDelta > 1.) {
		frameCallback(delta, fullDelta);
		lastPrintTime = glfwNow;
	}

	lastSwapTime = timeNow;
}

void ygl::Window::destroy() { glfwDestroyWindow(window); }

void ygl::Window::defaultFrameCallback(long draw_time, long frame_time) {
	std::cout << std::fixed << std::setprecision(2) << "\rdraw time: " << (draw_time / 1e6) << "ms, FPS: " << int(1e9 / frame_time) << std::endl;
}

void ygl::Window::setFrameCallback(void (*callback)(long, long)) {
	frameCallback = callback;
}