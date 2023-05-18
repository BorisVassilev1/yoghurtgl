#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <istream>
#include <ostream>
#include <serializable.h>

namespace ygl {
class Transformation : public ygl::Serializable {
	glm::mat4x4 worldMatrix;

   public:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	Transformation();
	Transformation(const glm::vec3 &position);
	Transformation(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale);

	glm::mat4 &getWorldMatrix();
	void	   updateWorldMatrix();
	void	   updateVectors();

	static bool decomposeTransform(const glm::mat4 &transform, glm::vec3 &translation, glm::vec3 &rotation,
								   glm::vec3 &scale);

	bool operator==(const Transformation &other);

	friend std::ostream &operator<<(std::ostream &os, const Transformation &rhs);

	void serialize(std::ostream &out);
	void deserialize(std::istream &in);
};
}	  // namespace ygl
