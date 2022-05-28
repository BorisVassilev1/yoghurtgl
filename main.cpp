#include <yoghurtgl.h>
#include <window.h>
#include <mesh.h>
#include <shader.h>

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

	GLfloat vertices[] = {
		-0.5, -0.5, -0.,
		0.5, -0.5, -0.,
		0.0, 0.5, -0.};
	GLfloat normals[] = {
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0};
	GLfloat colors[] = {
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0};
	GLuint indices[] = {
		0, 1, 2};

	ygl::BasicMesh cube((GLuint)9, vertices, normals, (GLfloat *)nullptr, colors, (GLuint)3, indices);

	ygl::VFShader shader("./shaders/simple.vs", "./shaders/simple.fs");
	if(shader.hasUniform("material_index")) {
		shader.bind();
		shader.setUniform("material_index", 1); // a uniform test
		shader.unbind();
	}

	while (!window.shouldClose()) {
		// process pending events
		glfwPollEvents();

		glClearColor(0, 0, 0, 1);	  // black
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.bind();
		cube.bind();
		glDrawElements(cube.getDrawMode(), cube.getIndicesCount(), GL_UNSIGNED_INT, 0);
		cube.unbind();
		shader.unbind();

		window.swapBuffers();
	}

	// clean up and exit
	window.~Window();
	glfwTerminate();
	std::cerr << std::endl;
	return 0;
}
