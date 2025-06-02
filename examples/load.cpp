#include <yoghurtgl.h>

#include <window.h>
#include <input.h>
#include <ecs.h>
#include <renderer.h>
#include <effects.h>
#include <fstream>
#include <thread>
#include "asset_manager.h"
#include "entities.h"
#include "transformation.h"

using namespace ygl;

void run() {
	Window window = Window(1280, 1000, "Test Window", true, false);

	PerspectiveCamera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponentIfCan<ygl::Transformation>();
	Renderer *renderer = scene.registerSystem<Renderer>(&window);
	scene.registerSystem<GrassSystem>();

	scene.getSystem<AssetManager>()->printTextures();

	renderer->setMainCamera(&cam);

	std::ifstream is = std::ifstream("scene.sc");
	try {
		scene.read(is);
	} catch (std::exception &e) { dbLog(ygl::LOG_ERROR, e.what()); }
	is.close();

	scene.getSystem<AssetManager>()->printTextures();

	std::cout << "ENTITY COUNT: " << scene.entitiesCount() << std::endl;

	for (Entity e : scene.entities) {
		std::cout << scene.getComponent<Transformation>(e) << std::endl;
		std::cout << scene.getComponent<RendererComponent>(e) << std::endl;
		if (scene.hasComponent<GrassSystem::GrassHolder>(e))
			std::cout << scene.getComponent<GrassSystem::GrassHolder>(e) << std::endl;
		std::cout << std::endl;
	}

	int textureViewIndex  = 6;

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update();
		cam.update();

		scene.doWork();
		renderer->drawMaterialEditor();

		ImGui::Begin("Texture View");
		ImGui::InputInt("Material ID", &textureViewIndex);
		ImGui::Image(textureViewIndex, ImVec2(256, 256));
		ImGui::End();

		window.swapBuffers();
	}
}

int main() {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	run();

	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}
