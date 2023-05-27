#include <yoghurtgl.h>

#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <input.h>
#include <ecs.h>
#include <renderer.h>
#include <fstream>

using namespace ygl;
using namespace std;

void run() {
	Window window = Window(800, 600, "Test Window", true);

	Mesh *bunnyMesh = new MeshFromFile("./res/models/bunny_uv/bunny_uv.obj");

	Mesh	 *cubeMesh = new BoxMesh();
	VFShader *shader   = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	Camera	  cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();

	Renderer *renderer = scene.registerSystem<Renderer>(&window);

	Texture2d *tex = new Texture2d("./res/models/bunny_uv/bunny_uv.jpg");

	Entity			bunny = scene.createEntity();
	Transformation &transform =
		scene.addComponent<Transformation>(bunny, Transformation(glm::vec3(), glm::vec3(), glm::vec3(10)));
	transform.position.y = 1;
	transform.updateWorldMatrix();
	RendererComponent bunnyRenderer;

	Material bunnyMat(glm::vec3(1., 1., 1.), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.), 0.0, 0.1,
					  0.5);
	AssetManager *asman = scene.getSystem<AssetManager>();
	bunnyMat.albedo_map		= asman->addTexture(tex, "./res/models/bunny_uv/bunny_uv.jpg");
	bunnyMat.use_albedo_map = 1.0;

	unsigned int shaderIndex = asman->addShader(shader, "defaultShader");

	bunnyRenderer.meshIndex = asman->addMesh(bunnyMesh, "bunny");

	bunnyRenderer.shaderIndex	= shaderIndex;
	bunnyRenderer.materialIndex = renderer->addMaterial(bunnyMat);
	scene.addComponent<RendererComponent>(bunny, bunnyRenderer);

	unsigned int cubeMeshIndex = asman->addMesh(cubeMesh, "cube");

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 0,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.01, Light::Type::AMBIENT));

	renderer->addLight(
		Light(Transformation(glm::vec3(5, 4, 5), glm::vec3(), glm::vec3(1)), glm::vec3(1.), 100, Light::Type::POINT));

	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 20; ++j) {
			Entity curr = scene.createEntity();

			scene.addComponent<Transformation>(
				curr, Transformation(glm::vec3(i * 2 - 20, -1, j * 2 - 20), glm::vec3(), glm::vec3(1)));
			RendererComponent rc(shaderIndex, cubeMeshIndex,
								 renderer->addMaterial(Material(
									 glm::vec3(rand() % 100 / 100., rand() % 100 / 100., rand() % 100 / 100.), .2,
									 glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.), 0.0, 0.1, 0.0)));

			scene.addComponent<RendererComponent>(curr, rc);
		}
	}

	renderer->loadData();

	glClearColor(0.07f, 0.13f, 0.17f, 1.0);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		Transformation &transform = scene.getComponent<Transformation>(bunny);
		transform.rotation.y += 1 * window.deltaTime;
		transform.updateWorldMatrix();

		controller.update();
		cam.update();

		renderer->doWork();

		window.swapBuffers();
	}

	ofstream out("scene.sc");
	scene.write(out);
	out.close();
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
