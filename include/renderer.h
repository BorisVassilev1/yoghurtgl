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

	float specular_roughness;
	float texture_influence;
	float use_normal_map;
	float metallic;

	float use_roughness_map;
	float use_ao_map;

   private:
	char padding[8];

   public:
	Material();
	Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior, glm::vec3 transparency_color,
			 float refraction_chance, glm::vec3 specular_color, float refraction_roughness, float specular_roughness,
			 float texture_influence, bool use_normal_map, float metallic, float use_roughness_map, float use_ao_map);
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

class FrameBuffer {
	GLuint	   id;
	Texture2d *color;
	Texture2d *depth_stencil;

   public:
	FrameBuffer(uint16_t width, uint16_t height) {
		glGenFramebuffers(1, &id);
		glBindFramebuffer(GL_FRAMEBUFFER, id);

		color		  = new Texture2d(width, height, ITexture::Type::RGBA, nullptr);
		depth_stencil = new Texture2d(width, height, ITexture::Type::DEPTH_STENCIL, nullptr);

		std::cout << color->getID() << std::endl;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->getID(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_stencil->getID(), 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			assert(false);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void bind() { glBindFramebuffer(GL_FRAMEBUFFER, id); }

	void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	Texture2d *getColor() { return color; }
	Texture2d *getDepthStencil() { return depth_stencil; }
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

	int		  defaultShader	 = -1;
	Texture2d defaultTexture = Texture2d(1, 1, ITexture::Type::RGBA, nullptr);

   public:
	using ISystem::ISystem;
	void init() override;

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