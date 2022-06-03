#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <transformation.h>
#include <window.h>

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
	ygl::Transformation transform;
	
	Camera(float, float, float, float);
	Camera(float, float, float, float, ygl::Transformation);
	Camera(float, ygl::Window &, float, float);
	Camera(float, ygl::Window &, float, float, ygl::Transformation);

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
