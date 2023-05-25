#pragma once

#include <yoghurtgl.h>
#include <chrono>
#include <vector>
#include <functional>

/**
 * @file window.h
 * @brief A Window wrapper
 */

namespace ygl {
/**
 * @brief A Window Wrapper.
 */
class Window {
	GLFWwindow									  *window = nullptr;
	int											   width = -1, height = -1;
	std::chrono::high_resolution_clock::time_point lastSwapTime	 = std::chrono::high_resolution_clock::now();
	double										   lastPrintTime = 0;
	void (*frameCallback)(long, long)							 = &defaultFrameCallback;

	inline static std::vector<std::function<void(GLFWwindow *, int, int)>> resizeCallbacks;

	Window(){};
	static void handleResize(GLFWwindow *, int, int);
	static void defaultFrameCallback(long, long);

   public:
	// TODO: this cannot override mesh settings, but should
	bool   shade	  = true; ///< does not work
	bool   cullFace	  = true; ///< does not work
	double deltaTime  = 0; ///< length of last frame
	double globalTime = 0; ///< time passed from opening the window until the beginning of the current frame
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
	/**
	 * @brief Set a callback to be called every second with data about the draw time of the last frame.
	 *
	 * @param callback - function to be called every frame
	 */
	void		setFrameCallback(void (*callback)(long, long));
	/**
	 * @brief Destroys and permanently invalidates the window. The destructor calls this.
	 */
	void		destroy();
	/**
	 * @brief Checks if the window has focus from the user
	 *
	 * @return true - if it has focus
	 * @return false - otherwise
	 */
	bool		isFocused();
	/**
	 * @brief Adds a callback that will be called every time the window is resized.
	 *
	 * @param callback - the function to be called. 
	 */
	void		addResizeCallback(const std::function<void(GLFWwindow *window, int width, int height)> &callback);
};
}	  // namespace ygl
