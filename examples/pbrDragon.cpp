#include <yoghurtgl.h>

#include <window.h>
#include <shader.h>
#include <input.h>
#include <ecs.h>
#include <renderer.h>
#include <asset_manager.h>
#include <entities.h>
#include <fstream>

using namespace ygl;

void run() {
	Window window = Window(1280, 1000, "Test Window", true, true);

	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");

	PerspectiveCamera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponentIfCan<ygl::Transformation>();

	Renderer	 *renderer = scene.registerSystem<Renderer>(&window);
	AssetManager *asman	   = scene.getSystem<AssetManager>();

	uint shaderInd = asman->addShader(shader, "defaultShader");
	renderer->setDefaultShader(shaderInd);

	addModels(scene, "./res/models/dragon-gltf/scene.gltf", [&scene, &renderer](Entity model) {
		RendererComponent &rc									   = scene.getComponent<RendererComponent>(model);
		renderer->getMaterial(rc.materialIndex).specular_roughness = 2.;
		renderer->getMesh(rc)->setCullFace(false);
	});

	addModels(scene, "./res/models/helmet/DamagedHelmet.gltf", [&scene](Entity model) {
		Transformation &tr = scene.getComponent<Transformation>(model);
		tr.position.x += 2;
		tr.rotation.y += glm::pi<float>();
		tr.updateWorldMatrix();
	});

	// addSkybox(scene, "./res/images/skybox/");
	addSkybox(scene, "res/images/blue_photo_studio_4k", ".hdr");

	//renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, .3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 5,
	//						 Light::Type::DIRECTIONAL));
	//renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.11, Light::Type::AMBIENT));

	renderer->loadData();

	int editMaterialIndex = 0;
	int textureViewIndex = 1;

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update();
		cam.update();

		renderer->doWork();

		ImGui::Begin("Material Properties");
		ImGui::InputInt("Material ID", &editMaterialIndex);
		ImGui::End();
		renderer->getMaterial(editMaterialIndex).drawImGui();
		renderer->loadData();

		ImGui::Begin("Texture View");
		ImGui::InputInt("Material ID", &textureViewIndex);
		ImGui::Image((void*)textureViewIndex, ImVec2(512, 512));
		ImGui::End();

		window.swapBuffers();
	}

	std::ofstream of = std::ofstream("scene.sc");
	scene.write(of);
	of.close();
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
