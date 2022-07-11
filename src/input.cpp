#define _USE_MATH_DEFINES

#include <input.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>

ygl::Mouse::Mouse(ygl::Window &window, bool lock) : delta(0), window(window), lock(lock) {
	position.x = window.getWidth() / 2.;
	position.y = window.getHeight() / 2.;
	glfwSetCursorPos(window.getHandle(), position.x, position.y);
	update();
}

ygl::Mouse::Mouse(ygl::Window &window) : Mouse(window, false) {}

void ygl::Mouse::update() {
	glm::dvec2 newPos;

	glfwGetCursorPos(window.getHandle(), &newPos.x, &newPos.y);

	delta = newPos - position;

	position = newPos;

	if (lock && !disableMouseWhenLockedAndHidden) {
		position.x = window.getWidth() / 2.;
		position.y = window.getHeight() / 2.;
		glfwSetCursorPos(window.getHandle(), position.x, position.y);
	}
}

void ygl::Mouse::hide() {
	if (lock && disableMouseWhenLockedAndHidden) {
		glfwSetInputMode(window.getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} else {
		glfwSetInputMode(window.getHandle(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	visible = false;
}

void ygl::Mouse::show() {
	glfwSetInputMode(window.getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	visible = true;
}

void ygl::Keyboard::init(ygl::Window *window) {
	ygl::Keyboard::window = window;
	glfwSetKeyCallback(window->getHandle(), handleInput);
}

void ygl::Keyboard::handleInput(GLFWwindow *window, int key, int scancode, int action, int mods) {
	for (auto fun : callbacks) {
		fun(window, key, scancode, action, mods);
	}
}

int ygl::Keyboard::getKey(ygl::Window &window, int key) { return glfwGetKey(window.getHandle(), key); }

int ygl::Keyboard::getKey(int key) { return glfwGetKey(window->getHandle(), key); }

void ygl::Keyboard::addKeyCallback(std::function<void(GLFWwindow *, int, int, int, int)> callback) {
	callbacks.push_back(callback);
}

ygl::FPController::FPController(ygl::Window *window, ygl::Mouse *mouse, ygl::Transformation &transform)
	: window(window), mouse(mouse), transform(transform) {
	mouse->lock = true;
	mouse->hide();
	Keyboard::addKeyCallback([this](GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (window != this->window->getHandle()) return;
		if (key == GLFW_KEY_1 && action == GLFW_RELEASE) {
			this->mouse->lock = !active;
			if (active) {
				this->mouse->show();
			} else {
				this->mouse->hide();
			}
			active = !active;
		}
	});
}

void ygl::FPController::update(double deltaTime) {
	if (!active) return;
	changed = false;

	if (mouse->getDelta().x != 0.0 || mouse->getDelta().y != 0.0) { changed = true; }

	transform.rotation.x -= mouse->getDelta().y / 500;
	transform.rotation.y -= mouse->getDelta().x / 500;

	if (transform.rotation.x > M_PI / 2) {
		transform.rotation.x = M_PI / 2;
	} else if (transform.rotation.x < -M_PI / 2) {
		transform.rotation.x = -M_PI / 2;
	}

	glm::mat4 rotationMat(1);
	rotationMat = glm::rotate(rotationMat, -transform.rotation.x, glm::vec3(1, 0, 0));
	rotationMat = glm::rotate(rotationMat, -transform.rotation.y, glm::vec3(0, 1, 0));
	rotationMat = glm::rotate(rotationMat, -transform.rotation.z, glm::vec3(0, 0, 1));

	glm::vec3 forward = glm::vec4(0., 0., -1., 0.0) * rotationMat;
	forward.y		  = 0;
	forward			  = glm::normalize(forward);
	forward *= deltaTime * speed;

	glm::vec3 sideways = glm::vec4(1., 0., 0., 0.0) * rotationMat;
	sideways.y		   = 0;
	sideways		   = glm::normalize(sideways);
	sideways *= deltaTime * speed;

	if (Keyboard::getKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
		transform.position.y += speed * deltaTime;
		changed = true;
	}
	if (Keyboard::getKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		transform.position.y -= speed * deltaTime;
		changed = true;
	}
	if (Keyboard::getKey(GLFW_KEY_A) == GLFW_PRESS) {
		transform.position -= sideways;
		changed = true;
	}
	if (Keyboard::getKey(GLFW_KEY_D) == GLFW_PRESS) {
		transform.position += sideways;
		changed = true;
	}
	if (Keyboard::getKey(GLFW_KEY_W) == GLFW_PRESS) {
		transform.position += forward;
		changed = true;
	}
	if (Keyboard::getKey(GLFW_KEY_S) == GLFW_PRESS) {
		transform.position -= forward;
		changed = true;
	}

	transform.updateWorldMatrix();
}