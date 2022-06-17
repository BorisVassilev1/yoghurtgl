#include <yoghurtgl.h>

#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <transformation.h>
#include <camera.h>
#include <input.h>
#include <ecs.h>
#include <renderer.h>

#include <iostream>

using namespace ygl;
using namespace std;

int main(int argc, char *argv[]) {
	if (init()) {
		std::cerr << "ygl failed to init" << std::endl;
		exit(1);
	}

	Window window = Window(800, 600, "Test Window", true);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	const aiScene *sc		 = loadScene("./models/bunny.obj");
	Mesh			 *bunnyMesh = (Mesh *)getModel(sc);

	Mesh	 cubeMesh = makeCube();
	VFShader shader("./shaders/simple.vs", "./shaders/simple.fs");
	Camera	 cam(glm::radians(70.f), window, 0.0001, 1000);

	Mouse mouse(window);
	Keyboard::init(&window);
	FPController controller(window, mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();
	scene.registerComponent<ECRenderer>();

	Entity cube = scene.createEntity();
	scene.addComponent<Transformation>(cube, Transformation());
	ECRenderer cubeECRenderer;

	Entity bunny = scene.createEntity();
	scene.addComponent<Transformation>(bunny, Transformation());
	ECRenderer bunnyECRenderer;

	Renderer renderer;
	bunnyECRenderer.meshIndex = renderer.addMesh(bunnyMesh);
	cubeECRenderer.meshIndex  = renderer.addMesh(&cubeMesh);

	bunnyECRenderer.shaderIndex = cubeECRenderer.shaderIndex = renderer.addShader(&shader);
	bunnyECRenderer.materialIndex							 = cubeECRenderer.materialIndex =
		renderer.addMaterial(Material(glm::vec3(1.), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, 0.0, 0.1));
	
	scene.addComponent<ECRenderer>(cube, cubeECRenderer);
	scene.addComponent<ECRenderer>(bunny, bunnyECRenderer);

	glClearColor(0, 0, 0, 0);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		Transformation &transform = scene.getComponent<Transformation>(bunny);
		transform.rotation.y += 1 * window.deltaTime;
		transform.updateWorldMatrix();

		controller.update(window.deltaTime);
		cam.update();

		renderer.render(scene);

		window.swapBuffers();
	}

	// clean up and exit
	window.~Window();
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}
