#define _USE_MATH_DEFINES

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
#include <effects.h>

#include <iostream>
#include <random>
#include <time.h>
#include <math.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

using namespace ygl;
using namespace std;

int main(int argc, char *argv[]) {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	Window window = Window(1200, 800, "Grass Test", true);

	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	Camera	  cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene(&window);
	scene.registerComponent<Transformation>();

	Texture2d uvChecker = Texture2d("./res/images/uv_checker.png");

	uvChecker.bind(GL_TEXTURE1);

	Renderer *renderer = scene.registerSystem<Renderer>();
	renderer->addShader(shader);
	renderer->setDefaultShader(0);

	GrassSystem *grassSystem = scene.registerSystem<GrassSystem>();

	Mesh *planeMesh = makePlane(glm::vec2(40, 40), glm::vec2(1, 1));

	Entity plane = scene.createEntity();
	scene.addComponent(plane, Transformation());
	scene.addComponent(plane, RendererComponent(-1, renderer->addMesh(planeMesh),
												renderer->addMaterial(Material(
													glm::vec3(0.1, 0.5, 0.1), .2, glm::vec3(0.), 0.99, glm::vec3(0.1),
													0.0, glm::vec3(1.), 0.0, 0.1, 0.0, false, 0., 0.0, 0.0))));

	// renderer->addLight(Light(Transformation(glm::vec3(0, 3, 0)), glm::vec3(0.2, 0.2, 1.0), 50, Light::Type::POINT));

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(0.5, -0.5, 0), glm::vec3(1)), glm::vec3(1., 1., 1.),
							 3, Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.01, Light::Type::AMBIENT));

	renderer->loadData();
	// scene is initialized

	glClearColor(0.07f, 0.13f, 0.17f, 1.0);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update(window.deltaTime);
		cam.update();

		renderer->doWork();

		grassSystem->doWork();

		ImGui::Begin("Grass controls");
		if (ImGui::SliderFloat("density", &(grassSystem->density), 1., 10.)) { grassSystem->reload(); }

		Transformation &groundTransform = scene.getComponent<Transformation>(plane);
		if (ImGui::SliderFloat3("ground rotation", (float *)&groundTransform.rotation, -M_PI, M_PI)) {
			groundTransform.updateWorldMatrix();
		}
		if (ImGui::SliderFloat3("ground position", (float *)&groundTransform.position, -20, 20)) {
			groundTransform.updateWorldMatrix();
		}
		ImGui::End();

		window.swapBuffers();
	}

	// clean up and exit
	window.~Window();
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}