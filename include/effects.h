#include <yoghurtgl.h>

#include <glm/glm.hpp>
#include <istream>
#include <ostream>
#include "buffer.h"
#include <mesh.h>
#include <shader.h>
#include <ecs.h>
#include <renderer.h>

/// @file effects.h
/// @brief Special effects and other specific graphics

namespace ygl {
class GrassSystem : public ygl::ISystem {
	class GrassBladeMesh : public MultiBufferMesh {
		struct alignas(16) BladeData {
			glm::vec3 position;
			float	  windStrength;
			glm::vec2 facing;
			glm::vec2 size;
			uint	  hash;
		};

		uint vCount1;
		uint vCount2;
		uint indicesCount1;
		uint indicesCount2;

		void calcBladeCount(glm::ivec2 resolution);

	   public:
		GLuint		  ibo1;
		GLuint		  ibo2;
		MutableBuffer grassData;
		GLuint		  bladeCount = -1;
		glm::ivec2	  resolution;
		int			  LOD;

		uint		index;
		static uint count;

		GrassBladeMesh(glm::ivec2 resolution, int LOD);
		void setResolution(glm::ivec2 resolution);
		~GrassBladeMesh();

		void bind();
	};

	uint grassComputeIndex = -1;
	uint grassShaderIndex  = -1;

	unsigned int materialIndex = -1;

   public:
	static const char *name;
	struct GrassHolder : public ygl::Serializable {
		static const char *name;

		glm::vec2 size;
		uint	  meshIndex = -1;
		float	  density;
		int		  LOD;

		GrassHolder() : size(0), density(0) {}
		GrassHolder(const glm::vec2 size, float density, int LOD = 0) : size(size), density(density), LOD(LOD) {}

		void serialize(std::ostream &out);
		void deserialize(std::istream &in);
	};

	glm::vec2	  size		 = glm::vec2(20, 20);
	float		  density	 = 1.5;
	int			  bladeCount = 0;
	Window		 *window;
	Renderer	 *renderer;
	AssetManager *assetManager;

	float curvature	   = 0.6;
	float facingOffset = 0.8;
	float height	   = 1.8;
	float width		   = 0.15;

	using ISystem::ISystem;
	void init() override;
	~GrassSystem();
	void update(float time);
	void render(float time);
	void reload();
	void doWork() override;

	void write(std::ostream &out) override { static_cast<void>(out); }
	void read(std::istream &in) override { static_cast<void>(in); }

	Material &getMaterial();
	uint	  getMaterialIndex();

   private:
	void reload(GrassHolder &holder);
};

std::ostream &operator<<(std::ostream &out, const ygl::GrassSystem::GrassHolder &rhs);

class FXAAEffect : public IScreenEffect {
	int fxaaShader;

   public:
	FXAAEffect(Renderer *renderer) : IScreenEffect() {
		setRenderer(renderer);
		Shader *sh = new VFShader(YGL_RELATIVE_PATH "./shaders/ui/textureOnScreen.vs",
								  YGL_RELATIVE_PATH "./shaders/postProcessing/fxaa.fs");
		fxaaShader = renderer->getAssetManager()->addShader(sh, "fxaaShader", false);
	}
	~FXAAEffect() {}
	void apply(FrameBuffer *front, FrameBuffer *back) override {
		front->getColor()->bind(GL_TEXTURE7);
		if (back) back->bind();
		else FrameBuffer::bindDefault();

		Shader *shader = renderer->getAssetManager()->getShader(fxaaShader);
		shader->bind();
		shader->setUniform("u_fxaaOn", enabled);
		shader->setUniform(
			"u_texelStep",
			glm::vec2(1.f / renderer->getWindow()->getWidth(), 1.f / renderer->getWindow()->getHeight()) * 1.f);
		Renderer::drawObject(shader, renderer->getScreenQuad());
		front->getColor()->unbind(GL_TEXTURE7);
	}
};

}	  // namespace ygl
