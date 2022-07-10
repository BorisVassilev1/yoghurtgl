#pragma once

#include <glm/vec2.hpp>
#include <window.h>
#include <transformation.h>
#include <vector>
#include <functional>

namespace ygl {
// mouse will have different coordinates relative to different windows
// so it cannot be static.
class Mouse {
	glm::dvec2	 position, delta;
	ygl::Window &window;

	Mouse();

   public:
	bool visible = false;
	bool lock	 = false;
	bool disableMouseWhenLockedAndHidden = false;

	Mouse(ygl::Window &, bool);
	Mouse(ygl::Window &);
	void update();
	void hide();
	void show();

	inline glm::dvec2 getPosition() { return position; }
	inline glm::dvec2 getDelta() { return delta; }
};

// keyboard will always have the same behaviour with all windows
// so it can be entirely static
class Keyboard {
	inline static ygl::Window														  *window = nullptr;
	inline static std::vector<std::function<void(GLFWwindow *, int, int, int, int)>> callbacks;

	static void handleInput(GLFWwindow *, int, int, int, int);
	Keyboard();

   public:
	static void init(ygl::Window *);
	static int	getKey(ygl::Window &, int);
	static int	getKey(int);
	static void addKeyCallback(std::function<void(GLFWwindow *, int, int, int, int)>);
};

class FPController {
	ygl::Window			*window;
	ygl::Mouse		   *mouse;
	ygl::Transformation &transform;

	bool changed = false;

	FPController();

   public:
	float speed		= 4;
	bool  active	= true;
	FPController(ygl::Window *, ygl::Mouse *, ygl::Transformation &);

	void update(double);
	bool hasChanged() { return changed; }
};
}	  // namespace ygl