#include <yoghurtgl.h>

#include <window.h>
#include <shader.h>
#include <input.h>
#include <camera.h>
#include <ecs.h>
#include <renderer.h>
#include <importer.h>
#include <transformation.h>
#include <entities.h>

using namespace ygl;

int main() {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	Window window = Window(1280, 1000, "Test Window", true, false);

	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");

	Camera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene(&window);
	scene.registerComponent<Transformation>();

	Renderer *renderer = scene.registerSystem<Renderer>();

	const aiScene *aiscene = loadScene("./res/models/dragon-gltf/scene.gltf");

	uint shaderInd = renderer->addShader(shader);
	renderer->setDefaultShader(shaderInd);

	addModels(scene, aiscene, "./res/models/dragon-gltf/", [&scene, &renderer](Entity model) {
		RendererComponent &rc									   = scene.getComponent<RendererComponent>(model);
		renderer->getMaterial(rc.materialIndex).specular_roughness = 2.;
	});

	const aiScene *helmetScene = loadScene("./res/models/helmet/DamagedHelmet.gltf");

	addModels(scene, helmetScene, "./res/models/helmet/", [&scene](Entity model) {
		Transformation &tr = scene.getComponent<Transformation>(model);
		tr.position.x += 2;
		tr.rotation.y += glm::pi<float>();
		tr.updateWorldMatrix();
	});

	Entity skybox = addSkybox(scene, "./res/images/skybox/");

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, .3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 5,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.11, Light::Type::AMBIENT));

	renderer->loadData();

	Light &directional = renderer->getLight(0);

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update();
		cam.update();

		renderer->doWork();

		window.swapBuffers();
	}

	// clean up and exit
	window.~Window();
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}