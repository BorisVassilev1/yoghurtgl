#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <transformation.hpp>

namespace ygl {
class Camera {
	glm::mat4x4 projectionMatrix;
	glm::mat4x4 viewMatrix;

	GLuint uboMatrices;
	float  fov;
	float  aspect;
	float  zNear;
	float  zFar;

   public:
	Camera(float fov, float aspect, float zNear, float zFar);

	inline void updateProjectionMatrix();
	void updateViewMatrix(ygl::Transformation &transform);
	void createMatricesUBO();
	inline void enable();
	inline void disable();
	void updateMatricesUBO(glm::mat4 worldMatrix);

	glm::mat4x4 getProjectionMatrix();
	glm::mat4x4 getViewMatrix();

	inline float getFov();
	inline void	 setFov(float fOV);
	inline float getAspect();
	inline void	 setAspect(float aspect);
	inline float getZNear();
	inline void	 setZNear(float zNear);
	inline float getZFar();
	inline void	 setZFar(float zFar);
};
}	  // namespace ygl