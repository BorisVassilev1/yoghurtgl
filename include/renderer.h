#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <shader.h>
#include <mesh.h>
#include <ecs.h>
#include <transformation.h>
#include <texture.h>
#include <material.h>

namespace ygl {

struct Material;
struct Light;
class FrameBuffer;
class IScreenEffect;
class ACESEffect;
struct RendererComponent;
class Renderer;

struct alignas(16) Light {
	enum Type { AMBIENT, DIRECTIONAL, POINT };

	glm::mat4 transform;
	glm::vec3 color;
	float	  intensity;
	Type	  type;

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
	~FrameBuffer();

	void clear();
	void bind();
	void unbind();

	Texture2d *getColor();
	Texture2d *getDepthStencil();

	static void bindDefault();
};

class IScreenEffect {
   protected:
	Renderer *renderer;
	IScreenEffect() {}

   public:
	bool		 enabled = true;
	void		 setRenderer(Renderer *renderer) { this->renderer = renderer; }
	virtual void apply(FrameBuffer *front, FrameBuffer *back) = 0;
	virtual ~IScreenEffect(){};
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
	VFShader	  *onScreen;

	Texture2d *tex1, *tex2;

   public:
	BloomEffect(Renderer *renderer);

	void apply(FrameBuffer *front, FrameBuffer *back);

	~BloomEffect();
};

struct RendererComponent {
	uint shaderIndex;
	uint meshIndex;
	uint materialIndex;
	RendererComponent() : shaderIndex(-1), meshIndex(-1), materialIndex(-1) {}
	RendererComponent(uint shaderIndex, uint meshIndex, uint materialIndex);
};

class Renderer : public ygl::ISystem {
	std::vector<Shader *>  shaders;
	std::vector<Material>  materials;
	std::vector<Mesh *>	   meshes;
	std::vector<Light>	   lights;

	GLuint materialsBuffer = 0;
	GLuint lightsBuffer	   = 0;

	uint	  defaultShader	 = -1;
	Texture2d defaultTexture = Texture2d(1, 1, ITexture::Type::RGBA, nullptr);

	FrameBuffer *frontFrameBuffer;
	FrameBuffer *backFrameBuffer;
	Mesh		*screenQuad = makeScreenQuad();

	glm::vec4 clearColor = glm::vec4(0, 0, 0, 1);

	std::vector<std::function<void()> > drawFunctions;
   public:
	std::vector<IScreenEffect *> effects;

   private:
	void drawScene();
	void colorPass();
	void effectsPass();

   public:
	using ISystem::ISystem;
	void init() override;

	Shader	 *getShader(RendererComponent &);
	Shader	 *getShader(uint index);
	Material &getMaterial(RendererComponent &);
	Material &getMaterial(uint index);
	Mesh	 *getMesh(RendererComponent &);
	Mesh	 *getMesh(uint index);
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

	static void drawObject(Transformation &transform, Shader *shader, Mesh *mesh, GLuint materialIndex);
	static void drawObject(Shader *sh, Mesh *mesh);

	static void compute(ComputeShader *shader, int numGroupsX, int numGroupsY, int numGroupsZ);

	static GLuint loadMaterials(int count, Material *materials);
	static GLuint loadLights(int count, Light *materials);
};

}	  // namespace ygl