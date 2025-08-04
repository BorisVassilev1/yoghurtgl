#include <camera.h>
#include <shader.h>

/// @brief constructor.
/// @param fov Field of view
/// @param aspect Aspect ratio
/// @param zNear Near plane z
/// @param zFar Far plane z
/// @param transform Transformation for the camera
ygl::PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float zNear, float zFar,
										  ygl::Transformation transform)
	: Camera(transform), fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {
	updateProjectionMatrix();
	matrices.viewMatrix = glm::mat4x4(1.0);
	createMatricesUBO();
}

/// @brief Constructor for a camera at world position (0,0,0) looking towards negative Z.
/// @param fov field of view
/// @param aspect aspect ratio of the screen
/// @param zNear near plane z
/// @param zFar far plane z
ygl::PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float zNear, float zFar)
	: PerspectiveCamera(fov, aspect, zNear, zFar, ygl::Transformation()) {}

/// @brief Constructor for camera for a window. Adds a resize callback that updates the projection matrix
/// whenever the window is resized
/// @param fov Field of view
/// @param from_window Window to attach the camera to
/// @param zNear Near plane z
/// @param zFar Far plane z
/// @param transform Transformation for the camera
ygl::PerspectiveCamera::PerspectiveCamera(float fov, ygl::Window &from_window, float zNear, float zFar,
										  ygl::Transformation transform)
	: PerspectiveCamera(fov, (float)from_window.getWidth() / from_window.getHeight(), zNear, zFar, transform) {
	from_window.addResizeCallback([&, this](GLFWwindow *window, int width, int height) {
		if (window != from_window.getHandle()) return;
		this->aspect = (float)width / height;
		updateProjectionMatrix();
	});
}

/// @brief Constructor for camera for a window. Adds a resize callback that updatess the projection matrix
/// whenever the window is resized
/// @param fov Field of view
/// @param from_window Window to attach the camera to
/// @param zNear Near plane z
/// @param zFar Far plane z
ygl::PerspectiveCamera::PerspectiveCamera(float fov, ygl::Window &from_window, float zNear, float zFar)
	: PerspectiveCamera(fov, from_window, zNear, zFar, ygl::Transformation()) {}

/// @brief Updates the projection matrix using the current set values.
void ygl::PerspectiveCamera::updateProjectionMatrix() {
	matrices.projectionMatrix = glm::perspective(fov, aspect, zNear, zFar);
}

/// @brief Updates the view matrix using the current transformation.
void ygl::Camera::updateViewMatrix() {
	matrices.viewMatrix = glm::translate(glm::mat4x4(1), transform.position);
	matrices.viewMatrix = glm::rotate(matrices.viewMatrix, transform.rotation.z, glm::vec3(0, 0, 1));
	matrices.viewMatrix = glm::rotate(matrices.viewMatrix, transform.rotation.y, glm::vec3(0, 1, 0));
	matrices.viewMatrix = glm::rotate(matrices.viewMatrix, transform.rotation.x, glm::vec3(1, 0, 0));
	matrices.viewMatrix = glm::scale(matrices.viewMatrix, transform.scale);
	matrices.viewMatrix = glm::inverse(matrices.viewMatrix);
}

void ygl::Camera::setViewMatrix(glm::mat4 &view) { this->matrices.viewMatrix = view; }

/// @brief Creates a Uniform Buffer Object so that the matrices can be sent to the GPU
void ygl::Camera::createMatricesUBO() {
	glGenBuffers(1, &uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4x4), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	enable();
}

/// @brief Enables the camera to be used by shaders. (actually just binds the UBO to the shader binding point)
void ygl::Camera::enable(int binding) { ygl::Shader::setUBO(uboMatrices, binding); }

/// @brief Disables the camera. (breaks the UBO binding)
void ygl::Camera::disable() { ygl::Shader::setUBO(0, 0); }

/// @brief Sends the view and projection matrices to the GPU.
void ygl::Camera::updateMatricesUBO() {
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	glBufferSubData(GL_UNIFORM_BUFFER, 0 * sizeof(glm::mat4), 2 * sizeof(glm::mat4), &matrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(transform.getWorldMatrix()));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/// @brief Updates the camera. Should be called before drawing every frame that some of its properties have changed
void ygl::Camera::update() {
	updateViewMatrix();
	updateMatricesUBO();
}

glm::mat4x4 ygl::Camera::getProjectionMatrix() { return matrices.projectionMatrix; }
glm::mat4x4 ygl::Camera::getViewMatrix() { return matrices.viewMatrix; }
float		ygl::PerspectiveCamera::getFov() { return fov; }
void		ygl::PerspectiveCamera::setFov(float fov) { this->fov = fov; }
float		ygl::PerspectiveCamera::getAspect() { return aspect; }
void		ygl::PerspectiveCamera::setAspect(float aspect) { this->aspect = aspect; }
float		ygl::PerspectiveCamera::getZNear() { return zNear; }
void		ygl::PerspectiveCamera::setZNear(float zNear) { this->zNear = zNear; }
float		ygl::PerspectiveCamera::getZFar() { return zFar; }
void		ygl::PerspectiveCamera::setZFar(float zFar) { this->zFar = zFar; }

ygl::OrthographicCamera::OrthographicCamera(float width, float aspect, float zNear, float zFar,
											ygl::Transformation transform)
	: Camera(transform), width(width), aspect(aspect), zNear(zNear), zFar(zFar) {
	updateProjectionMatrix();
	matrices.viewMatrix = glm::mat4x4(1.0);
	createMatricesUBO();
}

ygl::OrthographicCamera::OrthographicCamera(float width, float aspect, float zNear, float zFar)
	: OrthographicCamera(width, aspect, zNear, zFar, ygl::Transformation()) {}

ygl::OrthographicCamera::OrthographicCamera(float width, ygl::Window &from_window, float zNear, float zFar,
											ygl::Transformation transform)
	: OrthographicCamera(width, (float)from_window.getWidth() / from_window.getHeight(), zNear, zFar, transform) {
	from_window.addResizeCallback([&, this](GLFWwindow *window, int width, int height) {
		if (window != from_window.getHandle()) return;
		this->aspect = (float)width / height;
		updateProjectionMatrix();
		updateMatricesUBO();
	});
}

ygl::OrthographicCamera::OrthographicCamera(float width, ygl::Window &from_window, float zNear, float zFar)
	: OrthographicCamera(width, from_window, zNear, zFar, ygl::Transformation()) {}

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

void ygl::OrthographicCamera::updateProjectionMatrix() {
	float height			  = width / this->aspect;
	matrices.projectionMatrix = glm::ortho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, zNear, zFar);
}
