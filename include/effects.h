#include <yoghurtgl.h>

#include <glm/glm.hpp>
#include <mesh.h>
#include <shader.h>
#include <ecs.h>

namespace ygl {
class GrassSystem : public ygl::ISystem {
	class GrassBladeMesh : public MultiBufferMesh {
		struct BladeData {
			glm::vec3 position;
			float	  windStrength;
			glm::vec2 facing;
			glm::vec2 size;
			uint	  hash;

		   private:
			char padding[12];
		};

	   public:
		GLuint grassData  = -1;
		GLuint bladeCount = -1;

		GrassBladeMesh(GLuint bladeCount);
		void setBladeCount(GLuint bladeCount);
		~GrassBladeMesh();
	};

	struct GrassHolder {};

	glm::ivec2	 resolution = glm::ivec2(1, 1);
	unsigned int bladeCount = 1;

	GrassBladeMesh *bladeMesh	 = nullptr;
	ComputeShader	grassCompute = ComputeShader("./shaders/grass/grassCompute.comp");

	VFShader	 grassShader   = VFShader("./shaders/grass/grass.vs", "./shaders/grass/grass.fs");
	unsigned int materialIndex = -1;

   public:
	glm::vec2 size	  = glm::vec2(40, 40);
	float	  density = 3;

	using ISystem::ISystem;
	void init() override;
	~GrassSystem();
	void update(float time);
	void render(float time);
	void reload();
	void doWork() override;
};

}	  // namespace ygl