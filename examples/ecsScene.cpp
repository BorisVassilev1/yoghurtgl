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

int main(int argc, char *argv[]) {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	Window window = Window(800, 600, "Test Window", true);

	Mesh *bunnyMesh = (Mesh *)getModel(loadScene("./res/models/bunny_uv/bunny_uv.obj"));

	Mesh	 *cubeMesh = makeCube();
	VFShader *shader   = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	Camera	  cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();
	scene.registerComponent<RendererComponent>();

	// Texture2d *tex = new Texture2d("./res/models/bunny_uv/bunny_uv.jpg");
	Texture2d *tex = new Texture2d("./res/images/uv_checker.png");
	tex->bind();
	tex->bind(GL_TEXTURE1); // something has to be bound, otherwise the shaders throw a warning
	tex->bind(GL_TEXTURE2);

	Renderer *renderer = scene.registerSystem<Renderer>();
	scene.setSystemSignature<Renderer, Transformation, RendererComponent>();

	Entity			bunny	  = scene.createEntity();
	Transformation &transform = scene.addComponent<Transformation>(bunny, Transformation(glm::vec3(), glm::vec3(), glm::vec3(10)));
	transform.position.y	  = 1;
	transform.updateWorldMatrix();
	RendererComponent bunnyRenderer;

	unsigned int shaderIndex = renderer->addShader(shader);

	bunnyRenderer.meshIndex = renderer->addMesh(bunnyMesh);

	bunnyRenderer.shaderIndex	= shaderIndex;
	bunnyRenderer.materialIndex = renderer->addMaterial(
		Material(glm::vec3(1., 1., 1.), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.), 0.0, 0.1, 0, 1.0, false));

	scene.addComponent<RendererComponent>(bunny, bunnyRenderer);

	unsigned int cubeMeshIndex = renderer->addMesh(cubeMesh);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.),
							 0.7, Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.1, Light::Type::AMBIENT));

	renderer->addLight(
		Light(Transformation(glm::vec3(15, 4, 15), glm::vec3(), glm::vec3(1)), glm::vec3(1.), 100, Light::Type::POINT));

	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 20; ++j) {
			Entity curr = scene.createEntity();

			scene.addComponent<Transformation>(curr,
											   Transformation(glm::vec3(i * 2, -1, j * 2), glm::vec3(), glm::vec3(1)));
			RendererComponent rc(shaderIndex, cubeMeshIndex,
								 renderer->addMaterial(Material(
									 glm::vec3(rand() % 100 / 100., rand() % 100 / 100., rand() % 100 / 100.), .2,
									 glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.), 0.0, 0.1, 0, 0.5, false)));

			scene.addComponent<RendererComponent>(curr, rc);
		}
	}

	renderer->loadData();

	glClearColor(0, 0, 0, 0);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		Transformation &transform = scene.getComponent<Transformation>(bunny);
		transform.rotation.y += 1 * window.deltaTime;
		transform.updateWorldMatrix();

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
