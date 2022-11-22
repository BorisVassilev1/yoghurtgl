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

	Window window = Window(1000, 800, "Test Window", true);

	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");

	Camera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene(&window);
	scene.registerComponent<Transformation>();

	Renderer *renderer = scene.registerSystem<Renderer>();

	Texture2d *color	 = new Texture2d("./res/images/stones/albedo.png", ITexture::Type::SRGB);
	Texture2d *height	 = new Texture2d("./res/images/stones/height.png", ITexture::Type::GREY);
	Texture2d *normal	 = new Texture2d("./res/images/stones/normal.png", ITexture::Type::RGB);
	Texture2d *roughness = new Texture2d("./res/images/stones/roughness.png", ITexture::Type::GREY);
	Texture2d *ao		 = new Texture2d("./res/images/stones/ao.png", ITexture::Type::GREY);

	// Texture2d *color	 = new Texture2d("./res/horse_statue_01_4k/textures/horse_statue_01_diff_4k.png",
	// ITexture::Type::SRGB); Texture2d *height	 = new Texture2d(1,1, ITexture::Type::GREY, nullptr); Texture2d *normal
	// = new Texture2d("./res/horse_statue_01_4k/textures/horse_statue_01_nor_gl_4k.png", ITexture::Type::RGB);
	// Texture2d *roughness = new Texture2d("./res/horse_statue_01_4k/textures/horse_statue_01_rough_4k.png",
	// ITexture::Type::GREY); Texture2d *ao		 = new
	// Texture2d("./res/horse_statue_01_4k/textures/horse_statue_01_ao_4k.png", ITexture::Type::GREY);

	color->bind(GL_TEXTURE1);
	normal->bind(GL_TEXTURE2);
	height->bind(GL_TEXTURE3);
	roughness->bind(GL_TEXTURE4);
	ao->bind(GL_TEXTURE5);

	// Mesh *modelMesh = makeBox(glm::vec3(1, 1, 1), glm::vec3(20, 20, 20));
	Mesh *modelMesh = (Mesh *)getModel(loadScene("./res/models/bunny_uv/bunny_uv.obj"));
	// Mesh *modelMesh = (Mesh *)getModel(loadScene("./res/horse_statue_01_4k/horse_statue_01_4k.fbx"));

	Entity model = scene.createEntity();
	scene.addComponent<Transformation>(model, Transformation(glm::vec3(), glm::vec3(0), glm::vec3(1)));
	RendererComponent modelRenderer;
	modelRenderer.materialIndex =
		renderer->addMaterial(Material(glm::vec3(1., 1., 1.), 0.02, glm::vec3(0), 1.0, glm::vec3(1.0), 0.0,
									   glm::vec3(1.0), 0.0, 0.0, 1.0, true, 0.0, 0.5, 1.0));
	modelRenderer.shaderIndex = renderer->addShader(shader);
	modelRenderer.meshIndex	  = renderer->addMesh(modelMesh);
	scene.addComponent(model, modelRenderer);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 5,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.11, Light::Type::AMBIENT));

	renderer->loadData();

	controller.speed = 0.4;

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		Transformation &transform = scene.getComponent<Transformation>(model);
		// transform.rotation += glm::vec3(0.3 * window.deltaTime);
		transform.rotation.y += 0.3 * window.deltaTime;
		transform.updateWorldMatrix();

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