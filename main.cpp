#include <yoghurtgl.h>
#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <transformation.h>
#include <camera.h>

#include <iostream>

int main(int argc, char *argv[]) {
	if (ygl::init()) {
		std::cerr << "ygl failed to init" << std::endl;
		exit(1);
	}

	ygl::Window window = ygl::Window(800, 600, "Test Window", true);
	ygl::initDebug();

	std::cout << glGetString(GL_VERSION) << std::endl;
	std::cout << glGetString(GL_VENDOR) << std::endl;
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	ygl::BasicMesh		cube = ygl::makeCube();
	ygl::VFShader		shader("./shaders/simple.vs", "./shaders/simple.fs");
	ygl::Transformation cubePos(glm::vec3(0, 0, -2), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1));
	ygl::Camera			cam(glm::radians(70.f), float(window.getWidth()) / window.getHeight(), 0.0001, 1000);
	ygl::Transformation camTransform(glm::vec3(0), glm::vec3(0), glm::vec3(1));
	cam.createMatricesUBO();
	cam.updateProjectionMatrix();
	cam.updateViewMatrix(camTransform);
	cam.updateMatricesUBO(camTransform.getWorldMatrix());

	shader.bind();
	cube.bind();

	while (!window.shouldClose()) {
		// process pending events
		glfwPollEvents();

		glClearColor(0, 0, 0, 0);	  // black
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cubePos.rotation.y += 0.01;
		cubePos.updateWorldMatrix();

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
