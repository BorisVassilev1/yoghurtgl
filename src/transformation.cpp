#include <transformation.h>

ygl::Transformation::Transformation() : Transformation(glm::vec3(0), glm::vec3(0), glm::vec3(1)) {}

ygl::Transformation::Transformation(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale)
	: position(position), rotation(rotation), scale(scale) {
	updateWorldMatrix();
}

glm::mat4 ygl::Transformation::getWorldMatrix() const { return worldMatrix; }

void ygl::Transformation::updateWorldMatrix() {
	worldMatrix = glm::translate(glm::mat4(1.), position);
	worldMatrix = glm::rotate(this->worldMatrix, rotation.x, glm::vec3(1, 0, 0));
	worldMatrix = glm::rotate(this->worldMatrix, rotation.y, glm::vec3(0, 1, 0));
	worldMatrix = glm::rotate(this->worldMatrix, rotation.z, glm::vec3(0, 0, 1));
	worldMatrix = glm::scale(this->worldMatrix, scale);
}