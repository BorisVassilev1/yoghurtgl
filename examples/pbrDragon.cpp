#include <yoghurtgl.h>

#include <window.h>
#include <shader.h>
#include <input.h>
#include <camera.h>
#include <ecs.h>
#include <renderer.h>
#include <importer.h>
#include <transformation.h>

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

	for (int i = 0; i < 4; ++i) {
		Mesh *modelMesh = (Mesh *)getModel(aiscene, i);

		Entity model = scene.createEntity();
		scene.addComponent<Transformation>(model, Transformation(glm::vec3(), glm::vec3(0), glm::vec3(1.)));

		Material mat = getMaterial(aiscene, scene.assetManager, "./res/models/dragon-gltf/", i);
		mat.specular_roughness *= 5;

		RendererComponent modelRenderer;
		modelRenderer.materialIndex = renderer->addMaterial(mat);
		modelRenderer.shaderIndex	= shaderInd;
		modelRenderer.meshIndex		= renderer->addMesh(modelMesh);
		scene.addComponent(model, modelRenderer);
	}

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, .3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 5,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.11, Light::Type::AMBIENT));

	renderer->loadData();

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
