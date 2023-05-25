#pragma once

#include <yoghurtgl.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <transformation.h>
#include <window.h>

/**
 * @file camera.h
 * @brief A simple camera class
 */

namespace ygl {
/**
 * @brief A camera
 */
class Camera {
	struct matrices {
		glm::mat4x4 projectionMatrix;	  ///< any projection matrix
		glm::mat4x4 viewMatrix;			  ///< the view matrix, defined by the camera's transform
	} matrices;

	GLuint uboMatrices;		///< a Uniform Bufffer Object that holds matrix data in VRAM
	float  fov;				///< Field Of View
	float  aspect;			///< aspect ratio
	float  zNear;			///< near clip plane depth
	float  zFar;			///< far clip plane depth

   public:
	ygl::Transformation transform;	   ///< transformation of the view point

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
