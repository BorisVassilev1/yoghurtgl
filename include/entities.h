#include <yoghurtgl.h>
#include <ecs.h>
#include <glm/glm.hpp>
#include <renderer.h>
#include <mesh.h>

namespace ygl {

using uint = uint32_t;

static uint canonicalCubeIndex = -1;

Entity addBox(Scene &scene, glm::vec3 position = glm::vec3(), glm::vec3 scale = glm::vec3(1.),
			  glm::vec3 color = glm::vec3(1.));

static uint canonicalSphereIndex = -1;

Entity addSphere(Scene &scene, glm::vec3 position = glm::vec3(), glm::vec3 scale = glm::vec3(1.),
				 glm::vec3 color = glm::vec3(1.));

Entity addModel(Scene &scene, Mesh *mesh, glm::vec3 position = glm::vec3(), glm::vec3 scale = glm::vec3(1.),
				glm::vec3 color = glm::vec3(1.));

};	   // namespace ygl