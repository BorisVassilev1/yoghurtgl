#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <shader.h>
#include <mesh.h>
#include <ecs.h>
#include <transformation.h>
#include <texture.h>

namespace ygl {

struct Material {
	glm::vec3 albedo;
	float	  specular_chance;

	glm::vec3 emission;
	float	  ior;

	glm::vec3 transparency_color;
	float	  refraction_chance;

	glm::vec3 specular_color;
	float	  refraction_roughness;

	float		 specular_roughness;
	unsigned int texture_sampler;
	float		 texture_influence;
	float		 use_normal_map;

   private:
	//char padding[0];

   public:
	Material();
	Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior, glm::vec3 transparency_color,
			 float refraction_chance, glm::vec3 specular_color, float refraction_roughness, float specular_roughness,
			 unsigned int texture_sampler, float texture_influence, bool use_normal_map);
};

struct Light {
	enum Type { AMBIENT, DIRECTIONAL, POINT };

	glm::mat4 transform;
	glm::vec3 color;
	float	  intensity;
	Type	  type;

   private:
	char padding[12];

   public:
	Light(glm::mat4 transform, glm::vec3 color, float intensity, Type type);
	Light(ygl::Transformation transformation, glm::vec3 color, float intensity, Type type);
};

struct RendererComponent {
	int shaderIndex;
	int meshIndex;
	int materialIndex;
	RendererComponent() : shaderIndex(-1), meshIndex(-1), materialIndex(-1) {}
	RendererComponent(unsigned int shaderIndex, unsigned int meshIndex, unsigned int materialIndex);
};

class Renderer : public ygl::ISystem {
	std::vector<Shader *> shaders;
	std::vector<Material> materials;
	std::vector<Mesh *>	  meshes;
	std::vector<Light>	  lights;

	GLuint materialsBuffer = 0;
	GLuint lightsBuffer	   = 0;

	int	 defaultShader = -1;

   public:
	Shader   *getShader(RendererComponent &);
	Material &getMaterial(RendererComponent &);
	Mesh	 *getMesh(RendererComponent &);

	unsigned int addShader(Shader *);
	unsigned int addMaterial(const Material &);
	unsigned int addMesh(Mesh *);
	Light		  &addLight(const Light &);

	void setDefaultShader(int defaultShader);

	void loadData();

	void doWork() override;

	~Renderer() override;

	static void drawObject(Transformation &transform, Shader *shader, Mesh *mesh, GLuint materialIndex,
						   bool useTexture);
	static void drawObject(Shader *sh, Mesh *mesh);

	static void compute(ComputeShader *shader, int numGroupsX, int numGroupsY, int numGroupsZ);

	static GLuint loadMaterials(int count, Material *materials);
	static GLuint loadLights(int count, Light *materials);
};

}	  // namespace ygl