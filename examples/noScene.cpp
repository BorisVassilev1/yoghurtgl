#include <yoghurtgl.h>
#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <input.h>
#include <renderer.h>
#include <texture.h>

#include <iostream>
#include "GLFW/glfw3.h"

using namespace ygl;

void run() {
	ygl::Window window = ygl::Window(1200, 800, "rayTracer", true);

	Mesh *bunnyMesh;
	try {
		bunnyMesh = new MeshFromFile("./res/models/bunny.obj");
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		ygl::terminate();
		exit(1);
	}
	Transformation bunnyTransform;

	AssetManager asman(nullptr);
	Keyboard::addKeyCallback([&asman](GLFWwindow *window, int key, int, int action, int mods) {
		if(key == GLFW_KEY_R && action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL) asman.reloadShaders();
	});

	VFShader		 *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	asman.addShader(shader, "shader");
	PerspectiveCamera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Material mat =
		Material(glm::vec3(1., 1., 0.), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.), 0.0, 0.1, 0.);
	GLuint matBuff = Renderer::loadMaterials(1, &mat);

	Light lights[2] = {Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 3,
							 Light::Type::DIRECTIONAL),
					   Light(Transformation(), glm::vec3(1., 1., 1.), 0.01, Light::Type::AMBIENT)};
	GLuint lightBuff = Renderer::loadLights(2, lights);

	Texture2d tex(1, 1);
	for (unsigned int i = 0; i < 11; ++i) {
		tex.bind(GL_TEXTURE0 + i);
	}

	while (!window.shouldClose()) {
		window.beginFrame();

		mouse.update();

		controller.update();
		cam.update();

		shader = (VFShader *)asman.getShader(0);
		Renderer::drawObject(bunnyTransform, shader, bunnyMesh, 0);

		window.swapBuffers();
	}

	glDeleteBuffers(1, &matBuff);
	glDeleteBuffers(1, &lightBuff);

	std::cerr << std::endl;
	delete bunnyMesh;
	delete shader;
}

int main() {
	if (ygl::init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	run();

	ygl::terminate();
	return 0;
}
