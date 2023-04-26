#include <functional>

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

Entity addSkybox(Scene &scene, const std::string &path);

Entity addModel(ygl::Scene &scene, const aiScene *aiscene, std::string filePath, uint i);

void addModels(
	ygl::Scene &scene, const aiScene *aiscene, std::string filePath,
	const std::function<void(Entity)> &edit = [](Entity _) {});

void addScene(
	ygl::Scene &scene, const std::string &filename, const std::function<void(Entity)> &edit = [](Entity _) {});
}	  // namespace ygl
