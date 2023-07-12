#include <yoghurtgl.h>

#include <window.h>
#include <shader.h>
#include <input.h>
#include <camera.h>
#include <ecs.h>
#include <renderer.h>
#include <asset_manager.h>
#include <mesh.h>

using namespace ygl;

void run() {
	Window window = Window(1000, 800, "Test Window", true);

	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");

	PerspectiveCamera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();

	Renderer *renderer = scene.registerSystem<Renderer>(&window);

	Texture2d *color	 = new Texture2d("./res/images/stones/albedo.png", ITexture::Type::SRGB);
	Texture2d *normal	 = new Texture2d("./res/images/stones/normal.png", ITexture::Type::RGB);
	Texture2d *roughness = new Texture2d("./res/images/stones/roughness.png", ITexture::Type::GREY);
	Texture2d *ao		 = new Texture2d("./res/images/stones/ao.png", ITexture::Type::GREY);

	Mesh *modelMesh = new BoxMesh(glm::vec3(1, 1, 1), glm::vec3(20, 20, 20));

	Entity model = scene.createEntity();
	scene.addComponent<Transformation>(model, Transformation(glm::vec3(), glm::vec3(0), glm::vec3(1)));

	Material modelMat(glm::vec3(1.0, 0.5, 0.0), 0.02, glm::vec3(0), 1.0, glm::vec3(1.0), 0.0, glm::vec3(1.0), 0.0, 0.2,
					  0.0, 0.);
	AssetManager *asman		   = scene.getSystem<AssetManager>();
	modelMat.albedo_map		   = asman->addTexture(color, "color");
	modelMat.use_albedo_map	   = 1.0;
	modelMat.normal_map		   = asman->addTexture(normal, "normal");
	modelMat.use_normal_map	   = 1.0;
	modelMat.roughness_map	   = asman->addTexture(roughness, "roughness");
	modelMat.use_roughness_map = 0.0;
	modelMat.ao_map			   = asman->addTexture(ao, "ao");
	modelMat.use_ao_map		   = 1.0;

	RendererComponent modelRenderer;
	modelRenderer.materialIndex = renderer->addMaterial(modelMat);
	modelRenderer.shaderIndex	= asman->addShader(shader, "defaultShader");
	modelRenderer.meshIndex		= asman->addMesh(modelMesh, "canonicalCube");
	scene.addComponent(model, modelRenderer);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 3,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.01, Light::Type::AMBIENT));

	renderer->loadData();

	controller.speed = 0.6;

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		Transformation &transform = scene.getComponent<Transformation>(model);
		// transform.rotation += glm::vec3(0.3 * window.deltaTime);
		// transform.rotation.y += 0.3 * window.deltaTime;
		transform.updateWorldMatrix();

		controller.update();
		cam.update();

		renderer->doWork();

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
