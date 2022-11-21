#include <yoghurtgl.h>
#include <ecs.h>
#include <glm/glm.hpp>
#include <renderer.h>
#include <mesh.h>

namespace ygl {

using uint = uint32_t;
static uint canonicalCubeIndex = -1;

Entity addBox(Scene &scene, glm::vec3 position = glm::vec3(), glm::vec3 scale = glm::vec3(1.)) {
    Entity e = scene.createEntity();
    Renderer *renderer = scene.getSystem<Renderer>();
    if(canonicalCubeIndex == (uint)-1) {
        canonicalCubeIndex = renderer->addMesh(makeCube(1.));
    }
    uint materialIndex = renderer->addMaterial(Material());
    scene.addComponent<Transformation>(e, Transformation(position, glm::vec3(), scale));
    scene.addComponent<RendererComponent>(e, RendererComponent(-1, canonicalCubeIndex, materialIndex));

    return e;
}

};