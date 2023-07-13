#pragma once
#include <functional>

#include <yoghurtgl.h>
#include <ecs.h>
#include <glm/glm.hpp>
#include <renderer.h>
#include <mesh.h>
#include <assimp/scene.h>


/**
 * @file entities.h
 * @brief Utility functions to simplify work with entities
 */

namespace ygl {

using uint = uint32_t;

/**
 * @brief Remember the canonincalCube so that multiple calls to addBox don't generate many meshes.
 */
static uint canonicalCubeIndex = -1;

/**
 * @brief Adds a ygl::BoxMesh to a \a scene
 *
 * @param scene - a Scene
 * @param position - position of the box
 * @param scale - size of the box
 * @param color - color for the box's material
 * @return Entity - the generated Entity, added to \a scene
 */
Entity addBox(Scene &scene, glm::vec3 position = glm::vec3(), glm::vec3 scale = glm::vec3(1.),
			  glm::vec3 color = glm::vec3(1.));

/**
 * @brief Remember the canonical Sphere so that multiple calls of addSphere don't generate many meshes.
 */
static uint canonicalSphereIndex = -1;

/**
 * @brief Adds a ygl::SphereMesh to a \a scene.
 *
 * @param scene - a Scene
 * @param position - position of the sphere
 * @param scale - size of the sphere
 * @param color - color for the sphere's material
 * @return Entity - the generated Entity, added to \a scene
 */
Entity addSphere(Scene &scene, glm::vec3 position = glm::vec3(), glm::vec3 scale = glm::vec3(1.),
				 glm::vec3 color = glm::vec3(1.));

/**
 * @brief Adds any ygl::Mesh to a \a scene.
 *
 * @param scene - a Scene
 * @param mesh - mesh to be added
 * @param position - position of the mesh
 * @param scale - size of the mesh
 * @param color - color for the mesh's material
 * @return Entity - the generated Entity, added to \a scene
 */
Entity addModel(Scene &scene, Mesh *mesh, glm::vec3 position = glm::vec3(), glm::vec3 scale = glm::vec3(1.),
				glm::vec3 color = glm::vec3(1.));

/**
 * @brief Adds a skybox to a \a scene
 *
 * @param scene - a Scene
 * @param path - path to the skybox image files
 * @return Entity - the generated Entity, added to \a scene
 */
Entity addSkybox(Scene &scene, const std::string &path);

/**
 * @brief Adds a mesh, loaded from \a filePath, index \a i in the file to a \a scene.
 *
 * @param scene - a Scene
 * @param filePath - path to the file to read
 * @param i - index of the mesh in the specified file
 * @return Entity - the generated Entity, added to \a scene
 */
Entity addModel(ygl::Scene &scene, std::string filePath, uint i);

/**
 * @brief Adds all
 *
 * @param scene - a Scene
 * @param filePath - path to the file to be loaded
 * @param edit - a function that accepts a generated Entity that will get called for every mesh in the file
 */
void addModels(
	ygl::Scene &scene, std::string filePath,
	const std::function<void(Entity)> &edit = [](Entity _) { static_cast<void>(_); });
}	  // namespace ygl
