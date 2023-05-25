#pragma once

#include <glm/vec2.hpp>
#include <window.h>
#include <transformation.h>
#include <vector>
#include <functional>
#include <camera.h>

#include <imgui.h>
#include <ImGuizmo.h>

/**
 * @file input.h
 * @brief Classes for user input
 */

namespace ygl {
// mouse will have different coordinates relative to different windows
// so it cannot be static.
class Mouse {
	glm::dvec2	 position, delta;
	ygl::Window &window;

	Mouse();

   public:
	bool visible						 = false;	  ///< is mouse visible
	bool lock							 = false;	  ///< is mouse locked
	bool disableMouseWhenLockedAndHidden = false;	  ///< it says it

	/**
	 * @brief Constructs a Mouse for \a window
	 *
	 * @param window - window to attach the mouse to
	 * @param lock - should mouse be locked in the center of the window
	 */
	Mouse(ygl::Window &window, bool lock);
	/**
	 * @brief Constructs a Mouse for \a window
	 *
	 * @param window - window to attach the mouse to
	 */
	Mouse(ygl::Window &window);
	/**
	 * @brief Updates the mouse. Should be called every frame!
	 */
	void update();
	/**
	 * @brief Hides the mouse
	 */
	void hide();
	/**
	 * @brief Shows the mouse
	 */
	void show();

	inline glm::dvec2 getPosition() { return position; }
	inline glm::dvec2 getDelta() { return delta; }
};

// keyboard will always have the same behaviour with all windows
// so it can be entirely static
/**
 * @brief static Keyboard class
 */
class Keyboard {
	inline static ygl::Window *window = nullptr;

	inline static std::vector<std::function<void(GLFWwindow *, int, int, int, int)>> callbacks;

	static void handleInput(GLFWwindow *, int, int, int, int);
	Keyboard();

   public:
	/**
	 * @brief initialize Keyboard for \a window. Can be called several times to capture input from multiple windows.
	 *
	 * @param window - a Window to attach to
	 */
	static void init(ygl::Window *window);
	/**
	 * @brief Get the state of a key
	 *
	 * @param window - a Window to check for keypress
	 * @param key - key to get the state of
	 * @return int - state of \a key
	 */
	static int getKey(ygl::Window &window, int key);
	/**
	 * @brief Get the state of a key. Same as getKey(ygl::Window &window, int key) , but for the last window that
	 * init(ygl::Window *window) has been called for.
	 *
	 * @param key - key to get the state of
	 * @return int - stateof \a key
	 */
	static int getKey(int key);
	/**
	 * @brief Adds a callback to be called on change of state of any key.
	 *
	 * @param callback - callback that will be called on every change of key state on every window.
	 */
	static void addKeyCallback(const std::function<void(GLFWwindow *, int, int, int, int)> &callback);
};

/**
 * @brief First Person Controller
 */
class FPController {
	ygl::Window			*window;
	ygl::Mouse			*mouse;
	ygl::Transformation &transform;

	bool changed = false;

	FPController();

   public:
	float speed	 = 4;		 ///< movement speed
	bool  active = true;	 ///< will the controller work
	/**
	 * @brief Constructs a First Person Controller
	 *
	 * @param window - window to take input from
	 * @param mouse - mouse to use for input
	 * @param transform - a Transformation to control
	 */
	FPController(ygl::Window *window, ygl::Mouse *mouse, ygl::Transformation &transform);

	/**
	 * @brief Updates the controller. Should be called every frame.
	 */
	void update();
	/**
	 * @brief Checks if the controller has moved in any way.
	 *
	 * @return true - if the controller has moved
	 * @return false - otherwise
	 */
	bool hasChanged() { return changed; }
};

/**
 * @brief A on-screen guizmo that allows movement of any Transform through mouse and keyboard
 */
class TransformGuizmo {
	ygl::Window			*window;
	ygl::Transformation *transform = nullptr;
	ygl::Camera			*camera;
	ImGuizmo::OPERATION	 operation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE		 mode	   = ImGuizmo::MODE::WORLD;

   public:
	/**
	 * @brief Constructs the guizmo for \a window that is looking through \a camera and controlls \a transformation.
	 *
	 * @param window - a Window
	 * @param camera - camera that the window is looking through
	 * @param transform - Transformation to move
	 */
	TransformGuizmo(ygl::Window *window, ygl::Camera *camera, ygl::Transformation *transform);
	/**
	 * @brief Updates the guizmo. If \a transform is not nullptr, then the guizmo will edit that transformation from now
	 * on.
	 *
	 * @param transform - transformation to move
	 */
	void update(ygl::Transformation *transform);
};
}	  // namespace ygl
