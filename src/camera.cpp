#include <camera.h>
#include <shader.h>

ygl::Camera::Camera(float fov, float aspect, float zNear, float zFar) : fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {
	updateProjectionMatrix();
	viewMatrix = glm::mat4x4(1.0);
}

void ygl::Camera::updateProjectionMatrix() {
	projectionMatrix = glm::perspective(fov, aspect, zNear, zFar);
}

void ygl::Camera::updateViewMatrix(ygl::Transformation &transform) {
	viewMatrix = glm::translate(glm::mat4x4(1), transform.position);
	viewMatrix = glm::rotate(viewMatrix, transform.rotation.z, glm::vec3(0, 0, 1));
	viewMatrix = glm::rotate(viewMatrix, transform.rotation.y, glm::vec3(0, 1, 0));
	viewMatrix = glm::rotate(viewMatrix, transform.rotation.x, glm::vec3(1, 0, 0));
	viewMatrix = glm::scale(viewMatrix, transform.scale);
	viewMatrix = glm::inverse(viewMatrix);
}

void ygl::Camera::createMatricesUBO() {
	glGenBuffers(1, &uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4x4), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	enable();
}

void ygl::Camera::enable() {
	ygl::Shader::setUBO(uboMatrices, 0);
}
void ygl::Camera::disable() {
	ygl::Shader::setUBO(0, 0);
}
void ygl::Camera::updateMatricesUBO(glm::mat4 worldMatrix) {
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	glBufferSubData(GL_UNIFORM_BUFFER, 0 * sizeof(glm::mat4), sizeof(glm::mat4), &(projectionMatrix[0]));
	glBufferSubData(GL_UNIFORM_BUFFER, 1 * sizeof(glm::mat4), sizeof(glm::mat4), &(viewMatrix[0]));
	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), &(worldMatrix[0]));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

glm::mat4x4 ygl::Camera::getProjectionMatrix() { return projectionMatrix; }
glm::mat4x4 ygl::Camera::getViewMatrix() { return viewMatrix; }

float ygl::Camera::getFov() { return fov; }
void  ygl::Camera::setFov(float fov) { this->fov = fov; }
float ygl::Camera::getAspect() { return aspect; }
void  ygl::Camera::setAspect(float aspect) { this->aspect = aspect; }
float ygl::Camera::getZNear() { return zNear; }
void  ygl::Camera::setZNear(float zNear) { this->zNear = zNear; }
float ygl::Camera::getZFar() { return zFar; }
void  ygl::Camera::setZFar(float zFar) { this->zFar = zFar; }