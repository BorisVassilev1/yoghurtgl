#include <yoghurtgl.h>
#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <input.h>
#include <renderer.h>
#include <texture.h>

#include <iostream>

using namespace ygl;

Window *window;
Transformation bunnyTransform;
AssetManager asman(nullptr);
PerspectiveCamera *cam;
Mouse *mouse;
FPController *controller;
Mesh *bunnyMesh;

void frame() {
	window->beginFrame();

	mouse->update();

	controller->update();
	cam->update();

	ygl::Shader *shader = (VFShader *)asman.getShader(0);
	Renderer::drawObject(bunnyTransform, shader, bunnyMesh, 0);

	window->swapBuffers();
}


void run() {
	window = new ygl::Window(1200, 800, "rayTracer", true);

	try {
		//bunnyMesh = new MeshFromFile("./res/models/bunny.obj");
		bunnyMesh = new BoxMesh(2.);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		ygl::terminate();
		exit(1);
	}

	Keyboard::addKeyCallback([](GLFWwindow *window, int key, int, int action, int mods) {
		if(key == GLFW_KEY_R && action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL) asman.reloadShaders();
	});

	VFShader		 *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	asman.addShader(shader, "shader");
	cam = new PerspectiveCamera(glm::radians(70.f), *window, 0.01, 1000);

	mouse = new Mouse(*window);
	mouse->disableMouseWhenLockedAndHidden = true;
	controller = new FPController(window, mouse, cam->transform);

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

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(frame, 0, false);
#else
	while (!window->shouldClose()) {
		frame();
	}
	glDeleteBuffers(1, &matBuff);
	glDeleteBuffers(1, &lightBuff);

	std::cerr << std::endl;
	delete bunnyMesh;
	delete shader;
#endif
}

int main() {
	if (ygl::init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	run();

#ifndef __EMSCRIPTEN__
	ygl::terminate();
#endif
	return 0;
}
