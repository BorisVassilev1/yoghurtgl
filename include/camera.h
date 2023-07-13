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
 * @beief A Camera
 */
class Camera {
   protected:
	struct matrices {
		glm::mat4x4 projectionMatrix;	  ///< any projection matrix
		glm::mat4x4 viewMatrix;			  ///< the view matrix, defined by the camera's transform
	} matrices;

	GLuint				uboMatrices;	 ///< a Uniform Bufffer Object that holds matrix data in VRAM

   public:
	ygl::Transformation transform;		 ///< transformation of the view point
	
	Camera(): transform(), uboMatrices(0) {}
	Camera(const Transformation &transform) : transform(transform) {}
	Camera(const Camera &other)			   = delete;
	Camera &operator=(const Camera &other) = delete;

	glm::mat4x4 getProjectionMatrix();
	glm::mat4x4 getViewMatrix();

	void createMatricesUBO();
	void enable();
	void disable();
	void updateMatricesUBO();
	void update();
	void updateViewMatrix();
	virtual void updateProjectionMatrix() = 0;
};

/**
 * @brief A perspective camera
 */
class PerspectiveCamera : public Camera {
	float fov;		  ///< Field Of View
	float aspect;	  ///< aspect ratio
	float zNear;	  ///< near clip plane depth
	float zFar;		  ///< far clip plane depth

   public:
	PerspectiveCamera(float fov, float aspect, float zNear, float zFar);
	PerspectiveCamera(float fov, float aspect, float zNear, float zFar, ygl::Transformation transform);
	PerspectiveCamera(float fov, ygl::Window &from_window, float zNear, float zFar);
	PerspectiveCamera(float fov, ygl::Window &from_window, float zNear, float zFar, ygl::Transformation transform);

	void updateProjectionMatrix() override;

	float getFov();
	void  setFov(float);
	float getAspect();
	void  setAspect(float);
	float getZNear();
	void  setZNear(float);
	float getZFar();
	void  setZFar(float);
};


/**
 * @brief A perspective camera
 */
class OrthographicCamera : public Camera {
	float width;
	float aspect;
	float zNear;	  ///< near clip plane depth
	float zFar;		  ///< far clip plane depth

   public:
	OrthographicCamera(float width, float aspect, float zNear, float zFar);
	OrthographicCamera(float width, float aspect, float zNear, float zFar, ygl::Transformation transform);
	OrthographicCamera(float width, ygl::Window &from_window, float zNear, float zFar);
	OrthographicCamera(float width, ygl::Window &from_window, float zNear, float zFar, ygl::Transformation transform);

	void updateProjectionMatrix() override;
};
}	  // namespace ygl
