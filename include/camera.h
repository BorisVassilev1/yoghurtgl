#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <transformation.h>

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

	void updateProjectionMatrix();
	void updateViewMatrix(ygl::Transformation &transform);
	void createMatricesUBO();
	void enable();
	void disable();
	void updateMatricesUBO(glm::mat4 worldMatrix);

	glm::mat4x4 getProjectionMatrix();
	glm::mat4x4 getViewMatrix();

	float getFov();
	void  setFov(float fOV);
	float getAspect();
	void  setAspect(float aspect);
	float getZNear();
	void  setZNear(float zNear);
	float getZFar();
	void  setZFar(float zFar);
};
}	  // namespace ygl