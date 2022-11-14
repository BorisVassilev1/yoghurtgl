#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <shader.h>
#include <mesh.h>
#include <ecs.h>
#include <transformation.h>
#include <texture.h>

namespace ygl {

struct Material;
struct Light;
class FrameBuffer;
struct ScreenEffect;
struct IScreenEffect;
struct ACESEffect;
struct RendererComponent;
class Renderer;

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
	FrameBuffer(uint16_t width, uint16_t height, const char *name = nullptr);

	void clear();
	void bind();
	void unbind();

	Texture2d *getColor();
	Texture2d *getDepthStencil();

	static void bindDefault();
};

struct ScreenEffect {
	VFShader *shader;

	ScreenEffect() { shader = new VFShader("./shaders/ui/textureOnScreen.vs", "./shaders/postProcessing/acesFilm.fs"); }

	~ScreenEffect() { delete shader; }

	VFShader *getShader() { return shader; }
};

class IScreenEffect {
   protected:
	Renderer *renderer;
	IScreenEffect() {}

   public:
	bool enabled = true;
	void		 setRenderer(Renderer *renderer) { this->renderer = renderer; }
	virtual void apply(FrameBuffer *front, FrameBuffer *back) = 0;
};

// TODO: this must go to the effects header
class ACESEffect : public IScreenEffect {
	VFShader *colorGrader;

   public:
	ACESEffect();

	void apply(FrameBuffer *front, FrameBuffer *back);

	~ACESEffect();
};

class BloomEffect : public IScreenEffect {
	ComputeShader *blurShader, *filterShader;
	VFShader *onScreen;

	Texture2d *tex1, *tex2;

	public:
	BloomEffect(Renderer *renderer);

	void apply(FrameBuffer *front, FrameBuffer *back);

	~BloomEffect();
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

	FrameBuffer *frontFrameBuffer;
	FrameBuffer *backFrameBuffer;
	Mesh		*screenQuad = makeScreenQuad();

	glm::vec4 clearColor = glm::vec4(0, 0, 0, 1);

	std::vector<std::function<void()> > drawFunctions;
	public:
	std::vector<IScreenEffect *>		effects;
	private:

	void drawScene();
	void colorPass();
	void effectsPass();

   public:
	using ISystem::ISystem;
	void init() override;

	Shader	 *getShader(RendererComponent &);
	Material &getMaterial(RendererComponent &);
	Mesh	 *getMesh(RendererComponent &);
	Mesh	 *getScreenQuad();

	unsigned int addShader(Shader *);
	unsigned int addMaterial(const Material &);
	unsigned int addMesh(Mesh *);
	Light		&addLight(const Light &);
	void		 addScreenEffect(IScreenEffect *);

	void setDefaultShader(int defaultShader);
	void setClearColor(glm::vec4 color);

	void loadData();

	void doWork() override;

	~Renderer() override;

	void addDrawFunction(std::function<void()> func);
	void swapFrameBuffers();

	static void drawObject(Transformation &transform, Shader *shader, Mesh *mesh, GLuint materialIndex,
						   bool useTexture);
	static void drawObject(Shader *sh, Mesh *mesh);

	static void compute(ComputeShader *shader, int numGroupsX, int numGroupsY, int numGroupsZ);

	static GLuint loadMaterials(int count, Material *materials);
	static GLuint loadLights(int count, Light *materials);
};

}	  // namespace ygl