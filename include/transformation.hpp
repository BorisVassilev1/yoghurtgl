#pragma once

namespace ygl {
class Transformation {
	glm::mat4x4 worldMatrix;

   public:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	Transformation();
	Transformation(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &);

	glm::mat4x4 getWorldMatrix() const;
	void		updateWorldMatrix();
};
}	  // namespace ygl

ygl::Transformation::Transformation() : worldMatrix(1), scale(1) {}

ygl::Transformation::Transformation(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale) : position(position), rotation(rotation), scale(scale) {
	updateWorldMatrix();
}

glm::mat4x4 ygl::Transformation::getWorldMatrix() const { return worldMatrix; }

void ygl::Transformation::updateWorldMatrix() {
	worldMatrix = glm::translate(glm::mat4x4(1.), position);
	worldMatrix = glm::rotate(worldMatrix, rotation.x, glm::vec3(1, 0, 0));
	worldMatrix = glm::rotate(worldMatrix, rotation.y, glm::vec3(0, 1, 0));
	worldMatrix = glm::rotate(worldMatrix, rotation.z, glm::vec3(0, 0, 1));
	worldMatrix = glm::scale(worldMatrix, scale);
}
