#include <camera.h>
#include <shader.h>

ygl::Camera::Camera(float fov, float aspect, float zNear, float zFar, ygl::Transformation transform)
	: fov(fov), aspect(aspect), zNear(zNear), zFar(zFar), transform(transform) {
	updateProjectionMatrix();
	matrices.viewMatrix = glm::mat4x4(1.0);
	createMatricesUBO();
}

ygl::Camera::Camera(float fov, float aspect, float zNear, float zFar)
	: Camera(fov, aspect, zNear, zFar, ygl::Transformation()) {}

ygl::Camera::Camera(float fov, ygl::Window &from_window, float zNear, float zFar, ygl::Transformation transform)
	: Camera(fov, (float)from_window.getWidth() / from_window.getHeight(), zNear, zFar, transform) {
	from_window.addResizeCallback([&, this](GLFWwindow *window, int width, int height) {
		if (window != from_window.getHandle()) return;
		this->aspect = (float)width / height;
		updateProjectionMatrix();
	});
}

ygl::Camera::Camera(float fov, ygl::Window &from_window, float zNear, float zFar)
	: Camera(fov, from_window, zNear, zFar, ygl::Transformation()) {}

void ygl::Camera::updateProjectionMatrix() { matrices.projectionMatrix = glm::perspective(fov, aspect, zNear, zFar); }

void ygl::Camera::updateViewMatrix() {
	matrices.viewMatrix = glm::translate(glm::mat4x4(1), transform.position);
	matrices.viewMatrix = glm::rotate(matrices.viewMatrix, transform.rotation.z, glm::vec3(0, 0, 1));
	matrices.viewMatrix = glm::rotate(matrices.viewMatrix, transform.rotation.y, glm::vec3(0, 1, 0));
	matrices.viewMatrix = glm::rotate(matrices.viewMatrix, transform.rotation.x, glm::vec3(1, 0, 0));
	matrices.viewMatrix = glm::scale(matrices.viewMatrix, transform.scale);
	matrices.viewMatrix = glm::inverse(matrices.viewMatrix);
}

void ygl::Camera::createMatricesUBO() {
	glGenBuffers(1, &uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4x4), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	enable();
}

void ygl::Camera::enable() { ygl::Shader::setUBO(uboMatrices, 0); }
void ygl::Camera::disable() { ygl::Shader::setUBO(0, 0); }
void ygl::Camera::updateMatricesUBO() {
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	glBufferSubData(GL_UNIFORM_BUFFER, 0 * sizeof(glm::mat4), 2 * sizeof(glm::mat4), &matrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), &(transform.getWorldMatrix()[0]));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ygl::Camera::update() {
	updateViewMatrix();
	updateMatricesUBO();
}

glm::mat4x4 ygl::Camera::getProjectionMatrix() { return matrices.projectionMatrix; }
glm::mat4x4 ygl::Camera::getViewMatrix() { return matrices.viewMatrix; }
float		ygl::Camera::getFov() { return fov; }
void		ygl::Camera::setFov(float fov) { this->fov = fov; }
float		ygl::Camera::getAspect() { return aspect; }
void		ygl::Camera::setAspect(float aspect) { this->aspect = aspect; }
float		ygl::Camera::getZNear() { return zNear; }
void		ygl::Camera::setZNear(float zNear) { this->zNear = zNear; }
float		ygl::Camera::getZFar() { return zFar; }
void		ygl::Camera::setZFar(float zFar) { this->zFar = zFar; }