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

	srand(time(NULL));

	Window window = Window(800, 600, "Test Window", true);

	// Mesh	 *cubeMesh = makeCube();
	VFShader *shader   = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	VTFShader *terrainShader =
		new VTFShader("./shaders/terrain.vs", "./shaders/terrain.tesc", "./shaders/terrain.tese", "./shaders/terrain.fs");

	Camera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();
	scene.registerComponent<RendererComponent>();

	Texture2d *tex = new Texture2d("./res/images/heightmap.png");
	Texture2d *normal = new Texture2d("./res/images/NormalMap.png");

	((ITexture *)tex)->bind(GL_TEXTURE0);
	normal->bind(GL_TEXTURE1);

	Renderer *renderer = scene.registerSystem<Renderer>();
	scene.setSystemSignature<Renderer, Transformation, RendererComponent>();
	renderer->setUseTexture(true);

	// Mesh *terrainMesh = makePlane(glm::vec2(1, 1), glm::vec2(20, 20));
	Mesh *terrainMesh = makeBox(glm::vec3(1), glm::vec3(20));
	// Mesh *terrainMesh = makeSphere(1, 20, 20);
	// Mesh *terrainMesh = makeCube();
	terrainMesh->setDrawMode(GL_PATCHES);
	Entity terrain = scene.createEntity();
	scene.addComponent<Transformation>(terrain, Transformation(glm::vec3(), glm::vec3(0), glm::vec3(10)));
	RendererComponent terrainRenderer;
	terrainRenderer.materialIndex = renderer->addMaterial(Material(
		glm::vec3(1., 1., 1.), 0.02, glm::vec3(0), 1.0, glm::vec3(1.0), 0.0, glm::vec3(1.0), 0.0, 0.0, 0, 1.0));
	terrainRenderer.shaderIndex	  = renderer->addShader(terrainShader);
	terrainRenderer.meshIndex	  = renderer->addMesh(terrainMesh);
	scene.addComponent(terrain, terrainRenderer);

	glPatchParameteri(GL_PATCH_VERTICES, 3);


	Mesh *planeMesh = makePlane(glm::vec2(20, 20), glm::vec2(10, 10));
	Entity plane = scene.createEntity();
	scene.addComponent<Transformation>(plane, Transformation(glm::vec3(0, -7, 0)));
	RendererComponent planeRenderer;
	planeRenderer.shaderIndex = renderer->addShader(shader);
	planeRenderer.meshIndex = renderer->addMesh(planeMesh);
	planeRenderer.materialIndex = renderer->addMaterial(Material(
		glm::vec3(1., 1., 0.), 0.02, glm::vec3(0), 1.0, glm::vec3(1.0), 0.0, glm::vec3(1.0), 0.0, 0.0, 0, 0.0));
	scene.addComponent(plane, planeRenderer);


	renderer->setUseTexture(false);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.),
							 0.7, Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.1, Light::Type::AMBIENT));

	renderer->loadData();

	glClearColor(0, 0, 0, 0);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		Transformation &terrainT = scene.getComponent<Transformation>(terrain);
		terrainT.rotation.x += 0.002;
		terrainT.rotation.y += 0.002;
		terrainT.rotation.z += 0.002;
		terrainT.updateWorldMatrix();

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