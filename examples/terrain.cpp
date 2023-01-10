#include <yoghurtgl.h>

#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <transformation.h>
#include <camera.h>
#include <input.h>
#include <ecs.h>
#include <renderer.h>
#include <texture.h>

#include <iostream>
#include <random>
#include <time.h>

using namespace ygl;
using namespace std;

class VTFShader : public Shader {
   public:
	VTFShader(const char *vertex, const char *tessControl, const char *tessEval, const char *fragment)
		: Shader({vertex, tessControl, tessEval, fragment}) {
		createShader(GL_VERTEX_SHADER, 0);
		createShader(GL_TESS_CONTROL_SHADER, 1);
		createShader(GL_TESS_EVALUATION_SHADER, 2);
		createShader(GL_FRAGMENT_SHADER, 3);

		finishProgramCreation();
	}
};

int main(int argc, char *argv[]) {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	Window window = Window(800, 600, "Test Window", true);

	VTFShader *terrainShader = new VTFShader("./shaders/terrain.vs", "./shaders/terrain.tesc", "./shaders/terrain.tese",
											 "./shaders/terrain.fs");
	terrainShader->bind();
	terrainShader->setUniform("displacementPower", 0.06f);
	terrainShader->setUniform("displacementOffset", -0.4f);
	terrainShader->unbind();

	Camera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene(&window);
	scene.registerComponent<Transformation>();

	Renderer *renderer = scene.registerSystem<Renderer>();

	Texture2d *height = new Texture2d("./res/images/heightmap.png");
	Texture2d *normal = new Texture2d("./res/images/NormalMap.png");

	// Texture2d *height = new Texture2d("./res/images/bricks/displ.jpg");
	// Texture2d *normal = new Texture2d("./res/images/bricks/normal.jpg");
	Texture2d *color = new Texture2d("./res/images/bricks/albedo.jpg", ygl::ITexture::Type::SRGB);

	color->bind(GL_TEXTURE1);
	normal->bind(GL_TEXTURE2);
	height->bind(GL_TEXTURE3);
	height->bind(GL_TEXTURE4);

	Mesh *terrainMesh = makePlane(glm::vec2(1, 1), glm::vec2(20, 20));

	terrainMesh->setDrawMode(GL_PATCHES);
	Entity terrain = scene.createEntity();
	scene.addComponent<Transformation>(terrain, Transformation(glm::vec3(), glm::vec3(0, 0, 0), glm::vec3(10)));
	RendererComponent terrainRenderer;
	terrainRenderer.materialIndex =
		renderer->addMaterial(Material(glm::vec3(1., 1., 1.), 0.02, glm::vec3(0), 1.0, glm::vec3(1.0), 0.0,
									   glm::vec3(1.0), 0.0, 0.4, 1., true, 0, 0., 0.));
	terrainRenderer.shaderIndex = renderer->addShader(terrainShader);
	terrainRenderer.meshIndex	= renderer->addMesh(terrainMesh);
	scene.addComponent(terrain, terrainRenderer);

	glPatchParameteri(GL_PATCH_VERTICES, 3);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 3,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.01, Light::Type::AMBIENT));

	renderer->loadData();

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update();
		cam.update();

		renderer->doWork();

		window.swapBuffers();
	}

	// clean up and exit
	window.~Window();
	delete height;
	delete normal;
	delete color;
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}