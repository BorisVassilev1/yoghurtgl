#include <yoghurtgl.h>

#include <window.h>
#include <shader.h>
#include <input.h>
#include <ecs.h>
#include <renderer.h>
#include <asset_manager.h>
#include <entities.h>
#include <fstream>
#include "assimp/postprocess.h"
#include "mesh.h"

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
	renderer->setMainCamera(&cam);
	addEffects(renderer);
	MeshFromFile::import_flags |= aiProcess_PreTransformVertices;

	Entity	  sphere		  = addSphere(scene, glm::vec3(-3, 0, 0), glm::vec3(1));
	Material &sphere_mat	  = renderer->getMaterial(scene.getComponent<RendererComponent>(sphere).materialIndex);
	sphere_mat.albedo		  = glm::vec3(0);
	sphere_mat.albedo_map	  = asman->addTexture(new Texture2d("./res/images/2k_earth_daymap.jpg"), "earth");
	sphere_mat.use_albedo_map = 0.4;
	sphere_mat.normal_map = asman->addTexture(new Texture2d("./res/images/earth_normals_lowres.jpg"), "earth_normal");
	sphere_mat.use_normal_map = 1.0;

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
	// addSkybox(scene, "res/images/blue_photo_studio_4k", ".hdr");
	// addSkybox(scene, "res/images/royal_esplanade_4k", ".hdr");
	addSkybox(scene, "res/images/meadow_4k", ".hdr");

	renderer->loadData();

	int textureViewIndex = 6;

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update();
		cam.update();

		renderer->doWork();

		renderer->drawMaterialEditor();
		renderer->drawGUI();

		ImGui::Begin("Texture View");
		ImGui::InputInt("Material ID", &textureViewIndex);
		ImGui::Image(textureViewIndex, ImVec2(256, 256));
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
