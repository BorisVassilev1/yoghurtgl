#pragma once

#include <glm/glm.hpp>
#include <istream>
#include <vector>
#include <functional>
#include <shader.h>
#include <mesh.h>
#include <ecs.h>
#include <transformation.h>
#include <texture.h>
#include <material.h>
#include <ostream>
#include <camera.h>
#include <imgui.h>
#include <asset_manager.h>

/**
 * @file renderer.h
 * @brief All definitions that make the Renderer system work
 */

namespace ygl {

struct Material;
struct Light;
class FrameBuffer;
class IScreenEffect;
class ACESEffect;
class BloomEffect;
struct RendererComponent;
class Renderer;

struct alignas(16) Light {
	enum Type { AMBIENT, DIRECTIONAL, POINT };

	glm::mat4 transform;
	glm::vec3 color;
	float	  intensity;
	Type	  type;

   public:
	Light() : transform(), color(0.), intensity(0), type(Type::DIRECTIONAL) {}
	Light(glm::mat4 transform, glm::vec3 color, float intensity, Type type);
	Light(ygl::Transformation transformation, glm::vec3 color, float intensity, Type type);
};

std::ostream &operator<<(std::ostream &out, const Light &l);

class FrameBuffer {
	GLuint				   id;
	FrameBufferAttachable *color;
	FrameBufferAttachable *depth_stencil;

   public:
	FrameBuffer(FrameBufferAttachable *buff1, GLenum attachment1, FrameBufferAttachable *buff2, GLenum attachment2,
				const char *name = nullptr);
	~FrameBuffer();

	DELETE_COPY_AND_ASSIGNMENT(FrameBuffer)

	void clear() const;
	void bind() const;
	void unbind() const;

	Texture2d *getColor();
	Texture2d *getDepthStencil();

	int	 getID() { return id; }
	void resize(uint width, uint height);

	static void bindDefault();
};

class IScreenEffect {
   protected:
	Renderer *renderer;
	IScreenEffect() {}

   public:
	DELETE_COPY_AND_ASSIGNMENT(IScreenEffect)

	bool		 enabled = true;
	void		 setRenderer(Renderer *renderer) { this->renderer = renderer; }
	virtual void apply(FrameBuffer *front, FrameBuffer *back) = 0;
	virtual ~IScreenEffect(){};
};

// TODO: this must go to the effects header
class ACESEffect : public IScreenEffect {
	uint	  colorGrader;

   public:
	DELETE_COPY_AND_ASSIGNMENT(ACESEffect)

	ACESEffect(Renderer *renderer);
	~ACESEffect();
	void apply(FrameBuffer *front, FrameBuffer *back);
};

class BloomEffect : public IScreenEffect {
	ComputeShader *blurShader, *filterShader;
	VFShader	  *onScreen;

	Texture2d *tex1, *tex2;

   public:
	DELETE_COPY_AND_ASSIGNMENT(BloomEffect)

	BloomEffect(Renderer *renderer);
	~BloomEffect();
	void apply(FrameBuffer *front, FrameBuffer *back);
};

struct RendererComponent : ygl::Serializable {
	static const char *name;
	uint			   shaderIndex;
	uint			   meshIndex;
	uint			   materialIndex;
	uint			   shadowShaderIndex;
	bool			   isAnimated = false;
	RendererComponent() : shaderIndex(-1), meshIndex(-1), materialIndex(-1), shadowShaderIndex(-1) {}
	RendererComponent(uint shaderIndex, uint meshIndex, uint materialIndex, uint shadowShaderIndex = -1);
	void serialize(std::ostream &out);
	void deserialize(std::istream &in);
	bool operator==(const RendererComponent &other);
};

class Renderer : public ygl::ISystem {
	std::vector<Material> materials;
	std::vector<Light>	  lights;

	GLuint materialsBuffer = 0;
	GLuint lightsBuffer	   = 0;

	uint		   defaultShader	   = -1;
	uint		   defaultShadowShader = -1;
	Texture2d	   defaultTexture	   = Texture2d(1, 1, TextureType::RGBA16F, nullptr);
	TextureCubemap defaultCubemap	   = TextureCubemap(1, 1);

	FrameBuffer		  *shadowFrameBuffer;
	OrthographicCamera shadowCamera	 = OrthographicCamera(60, 1., 0.1, 100);
	uint			   shadowMapSize = 2048;
	bool			   shadow		 = false;

	Camera *mainCamera = nullptr;

	FrameBuffer *frontFrameBuffer;
	FrameBuffer *backFrameBuffer;
	Mesh		*screenQuad = new QuadMesh();
	glm::vec4	 clearColor = glm::vec4(0, 0, 0, 1);

	std::vector<std::function<void()> > drawFunctions;
	Window							   *window = nullptr;
	AssetManager					   *asman;

	void drawScene();
	void shadowPass();
	void colorPass();
	void effectsPass();

	std::vector<IScreenEffect *> effects;

   public:
	static const char *name;
	uint			   skyboxTexture	 = 0;
	uint			   irradianceTexture = 0;
	uint			   prefilterTexture	 = 0;
	uint			   brdfTexture		 = 0;
	uint			   renderMode		 = 0;

	DELETE_COPY_AND_ASSIGNMENT(Renderer)

	Renderer(Scene *scene, Window *window) : ISystem(scene), window(window) {}
	void init() override;

	Shader	 *getShader(RendererComponent &);
	Shader	 *getShader(uint index);
	Material &getMaterial(RendererComponent &);
	Material &getMaterial(uint index);
	Mesh	 *getMesh(RendererComponent &);
	Mesh	 *getMesh(uint index);
	Mesh	 *getScreenQuad();

	unsigned int   addMaterial(const Material &);
	Light		  &addLight(const Light &);
	Light		  &getLight(uint index);
	void		   addScreenEffect(IScreenEffect *);
	IScreenEffect *getScreenEffect(uint index);

	void setDefaultShader(int defaultShader);
	uint getDefaultShader();
	void setDefaultShadowShader(int defaultShader);
	uint getDefaultShadowShader();
	void setClearColor(glm::vec4 color);
	void setShadow(bool shadow);

	void setMainCamera(Camera *cam) { this->mainCamera = cam; }

	void loadData();

	void doWork() override;

	~Renderer() override;

	void addDrawFunction(const std::function<void()> &func);
	void swapFrameBuffers();

	static void drawObject(Transformation &transform, Shader *shader, Mesh *mesh, GLuint materialIndex);
	static void drawObject(Shader *sh, Mesh *mesh);

	static void compute(ComputeShader *shader, int numGroupsX, int numGroupsY, int numGroupsZ);

	static GLuint loadMaterials(int count, Material *materials);
	static GLuint loadLights(int count, Light *materials);

	Window		 *getWindow() { return window; }
	AssetManager *getAssetManager() { return asman; }
	bool		  hasSkybox();
	bool		  hasShadow();

	void drawGUI();
	void drawMaterialEditor();

	void write(std::ostream &out) override;
	void read(std::istream &in) override;

	friend class IScreenEffect;
};

std::ostream &operator<<(std::ostream &out, const ygl::RendererComponent &rhs);

}	  // namespace ygl
