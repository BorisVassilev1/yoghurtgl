#pragma once

#include <yoghurtgl.h>
#include <chrono>
#include <vector>
#include <functional>

namespace ygl {
class Window {
	GLFWwindow									   *window = nullptr;
	int											   width = -1, height = -1;
	std::chrono::high_resolution_clock::time_point lastSwapTime	 = std::chrono::high_resolution_clock::now();
	double										   lastPrintTime = 0;
	void (*frameCallback)(long, long)							 = &defaultFrameCallback;

	inline static std::vector<std::function<void(GLFWwindow *, int, int)>> resizeCallbacks;

	Window(){};

   public:
	bool   shade	  = true;
	bool   cullFace	  = true;
	double deltaTime  = 0;
	double globalTime = 0;
	Window(int width, int height, const char *name, bool vsync, bool resizable, GLFWmonitor *monitor);
	Window(int width, int height, const char *name, bool vsync, bool resizable);
	Window(int width, int height, const char *name, bool vsync);
	Window(int width, int height, const char *name);

	~Window();

	int			getWidth();
	int			getHeight();
	GLFWwindow *getHandle();
	bool		shouldClose();
	void		swapBuffers();
	void		beginFrame();
	void		setFrameCallback(void (*callback)(long, long));
	void		destroy();
	bool		isFocused();
	static void handleResize(GLFWwindow *, int, int);
	void		addResizeCallback(const std::function<void(GLFWwindow *, int, int)> &);

	static void defaultFrameCallback(long, long);
};
}	  // namespace ygl