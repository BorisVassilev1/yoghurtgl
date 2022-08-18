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

// class VTFShader : public Shader {
//    public:
// 	VTFShader(const char *vertex, const char *tess, const char *fragment) {
// 		createShader(GL_VERTEX_SHADER, 0);
// 		createShader(GL_TESS_EVALUATION_SHADER, 1);
// 		createShader(GL_FRAGMENT_SHADER, 2);

// 		finishProgramCreation();
// 	}
// };

int main(int argc, char *argv[]) {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	Window window = Window(800, 600, "Test Window", true);

	Mesh	 *cubeMesh		= makeCube();
	VFShader *shader		= new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	VFShader *terrainShader = new VFShader("./shaders/terrain.vs", "./shaders/simple.fs");
	Camera	  cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();
	scene.registerComponent<RendererComponent>();

	Texture2d *tex = new Texture2d("./res/images/heightmap.jpg");
	((ITexture *)tex)->bind();

	Renderer *renderer = scene.registerSystem<Renderer>();
	scene.setSystemSignature<Renderer, Transformation, RendererComponent>();
	renderer->setUseTexture(true);

	Mesh *terrainMesh = makePlane(glm::vec2(5, 5));

	Entity terrain = scene.createEntity();
	scene.addComponent<Transformation>(terrain, Transformation(glm::vec3(), glm::vec3(), glm::vec3(10)));
	RendererComponent terrainRenderer;
	terrainRenderer.materialIndex = renderer->addMaterial(Material(
		glm::vec3(1., 1., 1.), 0.02, glm::vec3(0), 1.0, glm::vec3(1.0), 0.0, glm::vec3(1.0), 0.0, 0.0, 0, 1.0));
	terrainRenderer.shaderIndex	  = renderer->addShader(terrainShader);
	terrainRenderer.meshIndex	  = renderer->addMesh(terrainMesh);
	scene.addComponent(terrain, terrainRenderer);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.),
							 0.7, Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.1, Light::Type::AMBIENT));

	renderer->loadData();

	glClearColor(0, 0, 0, 0);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update(window.deltaTime);
		cam.update();

		renderer->doWork();

		window.swapBuffers();
	}

	// clean up and exit
	window.~Window();
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}