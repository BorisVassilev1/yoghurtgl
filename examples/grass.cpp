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
#include <entities.h>
#include <importer.h>

#include <iostream>
#include <random>
#include <time.h>
#include <math.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <ImGuizmo.h>

using namespace ygl;
using namespace std;

void run() {
	Window window = Window(1200, 800, "Grass Test", true);

	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	Camera	  cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse			mouse(window);
	FPController	controller(&window, &mouse, cam.transform);
	TransformGuizmo guizmo(&window, &cam, (Transformation *)nullptr);

	Scene scene;
	scene.registerComponent<Transformation>();

	Renderer *renderer = scene.registerSystem<Renderer>(&window);
	renderer->addShader(shader);
	renderer->setDefaultShader(0);

	GrassSystem *grassSystem = scene.registerSystem<GrassSystem>();

	Mesh *planeMesh = makePlane(glm::vec2(40, 40), glm::vec2(1, 1));

	Entity plane = scene.createEntity();
	scene.addComponent(plane, Transformation());
	scene.addComponent(plane, RendererComponent(-1, renderer->addMesh(planeMesh),
												renderer->addMaterial(Material(glm::vec3(0.1, 0.5, 0.1), .2,
																			   glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0,
																			   glm::vec3(1.), 0.0, 0.3, 0.0, 0.0))));
	scene.addComponent(plane, GrassSystem::GrassHolder());

	Entity model =
		addModel(scene, (Mesh *)getModel(loadScene("./res/models/dragon.obj")), {0, 3, 0}, {10, 10, 10}, {1, 1, 1});
	Material &modelMat			= renderer->getMaterial(scene.getComponent<RendererComponent>(model));
	modelMat.specular_roughness = 0.7;

	Entity skybox = addSkybox(scene, "./res/images/skybox/");

	std::cerr << "renderer: ";
	renderer->printEntities();

	std::cerr << "grass: ";
	grassSystem->printEntities();

	// renderer->addLight(Light(Transformation(glm::vec3(0, 3, 0)), glm::vec3(0.2, 0.2, 1.0), 50, Light::Type::POINT));

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(0.5, -0.5, 0), glm::vec3(1)), glm::vec3(1., 1., 1.),
							 3, Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.01, Light::Type::AMBIENT));

	renderer->loadData();
	// scene is initialized

	glm::vec3 clearColor(0.07f, 0.13f, 0.17f);
	clearColor *= 1.5;
	clearColor = glm::pow(clearColor, glm::vec3(2.4));

	renderer->setClearColor(glm::vec4(clearColor, 1.));
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();
		controller.update();
		cam.update();

		grassSystem->doWork();
		renderer->doWork();

		Transformation &groundTransform = scene.getComponent<Transformation>(model);

		guizmo.update(&groundTransform);

		ImGui::Begin("Grass controls");
		if (ImGui::SliderFloat("density", &(grassSystem->density), 1., 10.)) { grassSystem->reload(); }

		if (ImGui::SliderFloat3("ground rotation", (float *)&groundTransform.rotation, -M_PI, M_PI)) {
			groundTransform.updateWorldMatrix();
		}
		if (ImGui::SliderFloat3("ground position", (float *)&groundTransform.position, -20, 20)) {
			groundTransform.updateWorldMatrix();
		}

		ImGui::Checkbox("Alpha Correction", &(renderer->effects[0]->enabled));
		ImGui::End();

		window.swapBuffers();
	}
}

int main(int argc, char *argv[]) {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	run();

	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}
