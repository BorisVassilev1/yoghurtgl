#include <yoghurtgl.h>

#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <transformation.h>
#include <camera.h>
#include <input.h>
#include <ecs.h>

#include <iostream>
using namespace ygl;

int main(int argc, char *argv[]) {
	if (init()) {
		std::cerr << "ygl failed to init" << std::endl;
		exit(1);
	}

	Window window = Window(800, 600, "Test Window", true);
	initDebug();

	std::cout << glGetString(GL_VERSION) << std::endl;
	std::cout << glGetString(GL_VENDOR) << std::endl;
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	BasicMesh		cube = makeCube();
	VFShader		shader("./shaders/simple.vs", "./shaders/simple.fs");
	Transformation cubePos(glm::vec3(0, 0, -2), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1));
	Camera			cam(glm::radians(70.f), window, 0.0001, 1000);
	
	Mouse mouse(window);
	Keyboard::init(&window);
	FPController controller(window, mouse, cam.transform);

	Scene scene;
	Entity e = scene.createEntity();
	scene.registerComponent<Transformation>();
	scene.registerComponent<Mesh>();

	scene.addComponent<Transformation>(e, Transformation());
	scene.removeComponent<Transformation>(e);

	shader.bind();
	cube.bind();

	while (!window.shouldClose()) {
		// process pending events
		glfwPollEvents();

		glClearColor(0, 0, 0, 0);	  // black
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mouse.update();

		cubePos.rotation.y += 0.01;
		cubePos.updateWorldMatrix();

		controller.update(window.deltaTime);
		cam.update();

		shader.setUniform("worldMatrix", cubePos.getWorldMatrix());

		glDrawElements(cube.getDrawMode(), cube.getIndicesCount(), GL_UNSIGNED_INT, 0);

		window.swapBuffers();
	}

	shader.unbind();
	cube.unbind();

	// clean up and exit
	window.~Window();
	glfwTerminate();
	std::cerr << std::endl;
	return 0;
}
