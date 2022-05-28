#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>

namespace ygl {
class Window {
	GLFWwindow									   *window = nullptr;
	int											   width = -1, height = -1;
	std::chrono::high_resolution_clock::time_point lastSwapTime = std::chrono::high_resolution_clock::now();
	double lastPrintTime = 0;
	void (*frameCallback)(long, long) = &defaultFrameCallback;

	Window(){};
   public:
	Window(int width, int height, const char *name, bool vsync, GLFWmonitor *monitor);
	Window(int width, int height, const char *name, bool vsync);
	Window(int width, int height, const char *name);

	int	 getWidth();
	int	 getHeight();
	bool shouldClose();
	void swapBuffers();
	void setFrameCallback(void (*callback)(long, long));
	void destroy();

	static void defaultFrameCallback(long draw_time, long frame_time);
};
}	  // namespace ygl