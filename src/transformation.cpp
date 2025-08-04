#include <yoghurtgl.h>
#include <transformation.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

ygl::Transformation::Transformation() : Transformation(glm::vec3(0), glm::vec3(0), glm::vec3(1)) {}

ygl::Transformation::Transformation(const glm::vec3 &position) : Transformation(position, glm::vec3(0), glm::vec3(1)) {}

ygl::Transformation::Transformation(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale)
	: position(position), rotation(rotation), scale(scale) {
	updateWorldMatrix();
}
ygl::Transformation::Transformation(glm::mat4 mat) : worldMatrix(mat) {
	updateVectors();
}

glm::mat4 &ygl::Transformation::getWorldMatrix() { return worldMatrix; }

void ygl::Transformation::updateWorldMatrix() {
	worldMatrix = glm::translate(glm::mat4(1.), position);
	worldMatrix = glm::rotate(this->worldMatrix, rotation.x, glm::vec3(1, 0, 0));
	worldMatrix = glm::rotate(this->worldMatrix, rotation.y, glm::vec3(0, 1, 0));
	worldMatrix = glm::rotate(this->worldMatrix, rotation.z, glm::vec3(0, 0, 1));
	worldMatrix = glm::scale(this->worldMatrix, scale);
}
namespace ygl {
std::ostream &operator<<(std::ostream &os, const glm::vec3 rhs) {
	os << "( " << rhs.r << ", " << rhs.g << ", " << rhs.b << " )";
	return os;
}

std::ostream &operator<<(std::ostream &os, const Transformation &rhs) {
	os << " position: " << rhs.position << " rotation: " << rhs.rotation << " scale: " << rhs.scale;
	return os;
}
}	  // namespace ygl

void ygl::Transformation::updateVectors() { decomposeTransform(worldMatrix, position, rotation, scale); }

// stolen from Hazel engine:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Math/Math.cpp
bool ygl::Transformation::decomposeTransform(const glm::mat4 &transform, glm::vec3 &translation, glm::vec3 &rotation,
											 glm::vec3 &scale) {
	// From glm::decompose in matrix_decompose.inl

	using namespace glm;
	using T = float;

	mat4 LocalMatrix(transform);

	// Normalize the matrix.
	if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>())) return false;

	// First, isolate perspective.  This is the messiest.
	if (epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
		epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
		epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>())) {
		// Clear the perspective partition
		LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
		LocalMatrix[3][3]										  = static_cast<T>(1);
	}

	// Next take care of translation (easy).
	translation	   = vec3(LocalMatrix[3]);
	LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

	vec3 Row[3];

	// Now get scale and shear.
	for (length_t i = 0; i < 3; ++i)
		for (length_t j = 0; j < 3; ++j)
			Row[i][j] = LocalMatrix[i][j];

	// Compute X scale factor and normalize first row.
	scale.x = length(Row[0]);
	Row[0]	= detail::scale(Row[0], static_cast<T>(1));
	scale.y = length(Row[1]);
	Row[1]	= detail::scale(Row[1], static_cast<T>(1));
	scale.z = length(Row[2]);
	Row[2]	= detail::scale(Row[2], static_cast<T>(1));

	// At this point, the matrix (in rows[]) is orthonormal.
	// Check for a coordinate system flip.  If the determinant
	// is -1, then negate the matrix and the scaling factors.
#if 0
		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

	rotation.y = asin(-Row[0][2]);
	if (cos(rotation.y) != 0) {
		rotation.x = atan2(Row[1][2], Row[2][2]);
		rotation.z = atan2(Row[0][1], Row[0][0]);
	} else {
		rotation.x = atan2(-Row[2][0], Row[1][1]);
		rotation.z = 0;
	}

	return true;
}

bool ygl::Transformation::operator==(const ygl::Transformation &other) {
	return this->worldMatrix == other.worldMatrix;
}

void ygl::Transformation::deserialize(std::istream &in) {
	in.read((char*)glm::value_ptr(this->worldMatrix), sizeof(glm::mat4x4));
	updateVectors();
}

void ygl::Transformation::serialize(std::ostream &out) {
	updateWorldMatrix();
	out.write((char*)glm::value_ptr(this->worldMatrix), sizeof(glm::mat4x4));
}

glm::vec3 ygl::Transformation::forward() {
	return (getWorldMatrix() * glm::vec4(0,0,1,0)).xyz();
}

const char *ygl::Transformation::name = "ygl::Transformation";
