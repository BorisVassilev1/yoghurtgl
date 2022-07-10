#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ygl {
class Transformation {
	glm::mat4x4 worldMatrix;

   public:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	Transformation();
	Transformation(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale);

	glm::mat4 getWorldMatrix() const;
	void	  updateWorldMatrix();
};
}	  // namespace ygl
