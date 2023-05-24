#include <yoghurtgl.h>

#include <glm/glm.hpp>
#include <istream>
#include <ostream>
#include <mesh.h>
#include <shader.h>
#include <ecs.h>

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

	   public:
		GLuint grassData  = -1;
		GLuint bladeCount = -1;

		GrassBladeMesh(GLuint bladeCount);
		void setBladeCount(GLuint bladeCount);
		~GrassBladeMesh();
	};

	glm::ivec2	 resolution = glm::ivec2(1, 1);
	unsigned int bladeCount = 1;

	GrassBladeMesh *bladeMesh	 = nullptr;
	ComputeShader	grassCompute = ComputeShader("./shaders/grass/grassCompute.comp");

	VFShader	 grassShader   = VFShader("./shaders/grass/grass.vs", "./shaders/grass/grass.fs");
	unsigned int materialIndex = -1;

   public:
	static const char *name;
	struct GrassHolder : public ygl::Serializable {
		static const char *name;

		void serialize(std::ostream &out);
		void deserialize(std::istream &in);
	};

	glm::vec2 size	  = glm::vec2(40, 40);
	float	  density = 3;
	Window	 *window;

	using ISystem::ISystem;
	void init() override;
	~GrassSystem();
	void update(float time);
	void render(float time);
	void reload();
	void doWork() override;

	void write(std::ostream &out) override { static_cast<void>(out); }
	void read(std::istream &in) override { static_cast<void>(in); }
};

std::ostream &operator<<(std::ostream &out, const ygl::GrassSystem::GrassHolder &rhs);

}	  // namespace ygl
