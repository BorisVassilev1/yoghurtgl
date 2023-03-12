#pragma once

#include <yoghurtgl.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <transformation.h>
#include <window.h>

namespace ygl {
/// @brief A perspective camera
class Camera {
	struct matrices {
		glm::mat4x4 projectionMatrix;
		glm::mat4x4 viewMatrix;
	} matrices;

	GLuint uboMatrices;
	float  fov;
	float  aspect;
	float  zNear;
	float  zFar;

   public:
	ygl::Transformation transform;

	Camera(float fov, float aspect, float zNear, float zFar);
	Camera(float fov, float aspect, float zNear, float zFar, ygl::Transformation transform);
	Camera(float fov, ygl::Window &from_window, float zNear, float zFar);
	Camera(float fov, ygl::Window &from_window, float zNear, float zFar, ygl::Transformation transform);

	void updateProjectionMatrix();
	void updateViewMatrix();
	void createMatricesUBO();
	void enable();
	void disable();
	void updateMatricesUBO();
	void update();

	glm::mat4x4 getProjectionMatrix();
	glm::mat4x4 getViewMatrix();

	float getFov();
	void  setFov(float);
	float getAspect();
	void  setAspect(float);
	float getZNear();
	void  setZNear(float);
	float getZFar();
	void  setZFar(float);
};
}	  // namespace ygl
