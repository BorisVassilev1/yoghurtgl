#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <shader.h>
#include <mesh.h>
#include <ecs.h>

namespace ygl {

struct Material {
	glm::vec3 albedo;
	float	  specular_chance;

	glm::vec3 emission;
	float	  ior;

	glm::vec3 transparency_color;
	float	  refraction_chance;

	float refraction_roughness;
	float specular_roughness;

	Material(glm::vec3, float, glm::vec3, float, glm::vec3, float, float, float);
};

struct ECRenderer {
	unsigned int shaderIndex;
	unsigned int meshIndex;
	unsigned int materialIndex;
	ECRenderer() : shaderIndex(-1), meshIndex(-1), materialIndex(-1) {}
};

class Renderer {
	std::vector<Shader *>	  shaders;
	std::vector<Material> materials;
	std::vector<Mesh *>	  meshes;

   public:
	Shader   *getShader(ECRenderer &);
	Material &getMaterial(ECRenderer &);
	Mesh	 *getMesh(ECRenderer &);

	unsigned int addShader(Shader *);
	unsigned int addMaterial(const Material &);
	unsigned int addMesh(Mesh *);

	void render(Scene &);
};

}	  // namespace ygl