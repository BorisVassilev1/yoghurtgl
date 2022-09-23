#include <yoghurtgl.h>

#include <window.h>
#include <shader.h>
#include <input.h>
#include <camera.h>
#include <ecs.h>
#include <renderer.h>

using namespace ygl;

int main() {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	Window window = Window(800, 600, "Test Window", true);

	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");

	Camera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();
	scene.registerComponent<RendererComponent>();

	Texture2d *color	 = new Texture2d("./res/images/stones/albedo.png", ITexture::Type::SRGB);
	Texture2d *height	 = new Texture2d("./res/images/stones/height.png", ITexture::Type::GREY);
	Texture2d *normal	 = new Texture2d("./res/images/stones/normal.png", ITexture::Type::RGB);
	Texture2d *roughness = new Texture2d("./res/images/stones/roughness.png", ITexture::Type::GREY);
	Texture2d *ao		 = new Texture2d("./res/images/stones/ao.png", ITexture::Type::GREY);

	color->bind(GL_TEXTURE0);
	normal->bind(GL_TEXTURE1);
	height->bind(GL_TEXTURE2);
	roughness->bind(GL_TEXTURE3);
	ao->bind(GL_TEXTURE4);

	Renderer *renderer = scene.registerSystem<Renderer>();
	scene.setSystemSignature<Renderer, Transformation, RendererComponent>();

	Mesh *terrainMesh = makeBox(glm::vec3(1, 1, 1), glm::vec3(20, 20, 20));

	Entity terrain = scene.createEntity();
	scene.addComponent<Transformation>(terrain, Transformation(glm::vec3(), glm::vec3(0), glm::vec3(10)));
	RendererComponent terrainRenderer;
	terrainRenderer.materialIndex =
		renderer->addMaterial(Material(glm::vec3(1., 1., 1.), 0.02, glm::vec3(0), 1.0, glm::vec3(1.0), 0.0,
									   glm::vec3(1.0), 0.0, 0.0, 1.0, true, 0.0, 0.5, 1.0));
	terrainRenderer.shaderIndex = renderer->addShader(shader);
	terrainRenderer.meshIndex	= renderer->addMesh(terrainMesh);
	scene.addComponent(terrain, terrainRenderer);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 3,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.11, Light::Type::AMBIENT));

	renderer->loadData();

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update(window.deltaTime);
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