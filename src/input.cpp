#define _USE_MATH_DEFINES

#include <input.h>
#include <yoghurtgl.h>
#include <iostream>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

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
		position.x = window.getWidth() / 2;
		position.y = window.getHeight() / 2;
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

void ygl::Keyboard::addKeyCallback(
	const std::function<void(GLFWwindow *window, int key, int scancode, int action, int mods)> &callback) {
	callbacks.push_back(callback);
}

ygl::FPController::FPController(ygl::Window *window, ygl::Mouse *mouse, ygl::Transformation &transform)
	: window(window), mouse(mouse), transform(transform) {
	mouse->lock = true;
	mouse->hide();
	Keyboard::addKeyCallback([this](GLFWwindow *window, int key, int, int action, int) {
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

void ygl::FPController::update() {
	if (!active) return;
	changed = false;

	if (mouse->getDelta().x != 0.0 || mouse->getDelta().y != 0.0) { changed = true; }

	int realSpeed = speed;
	if(Keyboard::getKey(GLFW_KEY_LEFT_CONTROL)) realSpeed *= 5;

	transform.rotation.x -= mouse->getDelta().y / 500.;
	transform.rotation.y -= mouse->getDelta().x / 500.;

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
	forward *= window->deltaTime * realSpeed;

	glm::vec3 sideways = glm::vec4(1., 0., 0., 0.0) * rotationMat;
	sideways.y		   = 0;
	sideways		   = glm::normalize(sideways);
	sideways *= window->deltaTime * realSpeed;
	

	if (Keyboard::getKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
		transform.position.y += realSpeed * window->deltaTime;
		changed = true;
	}
	if (Keyboard::getKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		transform.position.y -= realSpeed * window->deltaTime;
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

ygl::TransformGuizmo::TransformGuizmo(ygl::Window *window, ygl::Camera *camera, ygl::Transformation *transform)
	: window(window), transform(transform), camera(camera) {
	Keyboard::addKeyCallback([&, this](GLFWwindow *windowHandle, int key, int, int action, int) {
		if (windowHandle != this->window->getHandle()) return;
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) { this->operation = ImGuizmo::OPERATION::TRANSLATE; }
		if (key == GLFW_KEY_X && action == GLFW_RELEASE) { this->operation = ImGuizmo::OPERATION::ROTATE; }
		if (key == GLFW_KEY_C && action == GLFW_RELEASE) { this->operation = ImGuizmo::OPERATION::SCALE; }
	});
}

void ygl::TransformGuizmo::update(Transformation *transform) {
	bool  snap		= Keyboard::getKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
	float snapValue = 1.0f;
	if (operation == ImGuizmo::OPERATION::ROTATE) snapValue = 15.0f;
	float snapVector[3] = {snapValue, snapValue, snapValue};
	if (transform == nullptr) transform = this->transform;
	else this->transform = transform;
	ImGuizmo::Manipulate(glm::value_ptr(camera->getViewMatrix()), glm::value_ptr(camera->getProjectionMatrix()),
						 operation, mode, glm::value_ptr(transform->getWorldMatrix()), nullptr,
						 snap ? snapVector : nullptr);
	if (ImGuizmo::IsUsing()) transform->updateVectors();
}
