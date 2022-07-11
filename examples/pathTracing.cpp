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

#include <iostream>
#include <random>
#include <time.h>

using namespace ygl;
using namespace std;

struct Sphere {
	glm::vec3 position;
	float	  radius = 1;
	Material  mat;

	Sphere(){};
	Sphere(glm::vec3 position, float radius, Material mat) : position(position), radius(radius), mat(mat) {}
};

Window *window;
Mouse  *mouse;

Texture2d	 *tex;
Scene		 *scene;
Renderer	 *renderer;
VFShader	 *shader;
VFShader	 *unlitShader;
Camera		 *camera;
FPController *controller;
Mesh		 *sphereMesh;
Mesh		 *bunnyMesh;

Entity bunny;
Entity sphere;

ComputeShader  *pathTracer;
ComputeShader  *normalizer;
Texture2d	   *renderTexture;
Texture2d	   *rawTexture;
VFShader	   *textureOnScreen;
Mesh		   *screenQuad;
TextureCubemap *skybox;

Sphere *spheres;
int		sphereCount;
GLuint	spheresBuff;

bool pathTrace = false;

int sampleCount = 0;

void cleanup() {
	delete tex;
	delete scene;
	delete camera;
	delete controller;
	delete pathTracer;
	delete normalizer;
	delete renderTexture;
	delete rawTexture;
	delete textureOnScreen;
	delete screenQuad;
	delete spheres;
}

void initScene() {
	bunnyMesh = (Mesh *)getModel(loadScene("./res/models/bunny.obj"));
	// Mesh *bunnyMesh = makeScreenQuad();
	sphereMesh = makeSphere();

	shader		= new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	unlitShader = new VFShader("./shaders/unlit.vs", "./shaders/unlit.fs");
	camera		= new Camera(glm::radians(70.f), *window, 0.01, 1000);
	controller	= new FPController(window, mouse, camera->transform);

	tex = new Texture2d(1, 1);	   // just create some texture so that the shader does not complain
	tex->bind();

	scene = new Scene();
	scene->registerComponent<Transformation>();
	scene->registerComponent<RendererComponent>();

	renderer = scene->registerSystem<Renderer>();
	scene->setSystemSignature<Renderer, Transformation, RendererComponent>();

	bunny = scene->createEntity();
	scene->addComponent<Transformation>(bunny, Transformation());
	RendererComponent bunnyRenderer;

	unsigned int shaderIndex = renderer->addShader(shader);
	renderer->setDefaultShader(shaderIndex);
	scene->addComponent<RendererComponent>(
		bunny, RendererComponent(-1, renderer->addMesh(bunnyMesh),
								 renderer->addMaterial(Material(glm::vec3(1., 1., 0.), .2, glm::vec3(0.), 0.0,
																glm::vec3(0.1), 0.0, 0.0, 0.1, 0, 0.0))));

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(-1, -2.9, 0), glm::vec3(1)), glm::vec3(1., 1., 1.),
							 0.7, Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.1, Light::Type::AMBIENT));

	renderer->loadData();
}

void initSpheres() {
	sphereCount = 25 + 4;
	spheres		= new Sphere[sphereCount];

	Entity sphere;
	spheres[0] = Sphere(glm::vec3(0.2, 1.8, 0.2), 0.5,
						Material(glm::vec3(1.0, 1.0, 0.5), 1.0, glm::vec3(0.0, 0.0, 0.0), 0.0, glm::vec3(0.0, 0.0, 0.0),
								 0.0, 0.00, 0.05, 0, 0));
	spheres[1] = Sphere(glm::vec3(1.0, 0.7, 0.7), 0.7,
						Material(glm::vec3(0.1, 1.0, 0.1), 0.2, glm::vec3(0.0, 0.0, 0.0), 0.0, glm::vec3(0.0, 0.0, 0.0),
								 0.0, 0.00, 0.03, 0, 0));
	spheres[2] = Sphere(glm::vec3(-0.7, 1.0, 0.2), 0.5,
						Material(glm::vec3(0.9, 0.9, 1.0), 0.0, glm::vec3(0.0, 0.0, 0.0), 1.7,
								 glm::vec3(10.0, 0.0, 0.0), 1.0, 0.05, 0.05, 0, 0));
	spheres[3] = Sphere(glm::vec3(-0.1, 1.8, 1.7), 0.4,
						Material(glm::vec3(1.0, 1.0, 1.0), 0.0, glm::vec3(10.0, 10.0, 10.0), 0.0,
								 glm::vec3(0.0, 0.0, 0.0), 0.0, 0.00, 0.00, 0, 0));
	renderer->addLight(Light(Transformation(spheres[3].position), glm::vec3(1.), 1, Light::POINT));

	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			glm::vec3 randomColor(rand() % 100 / 100., rand() % 100 / 100., rand() % 100 / 100.);

			spheres[i * 5 + j + 4] = Sphere(glm::vec3(i * 1.5 - 5, 0., 3 + j * 1.5), 0.5,
											Material(randomColor, rand() % 10 / 10., glm::vec3(0.0, 0.0, 0.0), 1.7,
													 randomColor, 0.0, 0.00, 0.05, 0, 0));
		}
	}

	uint meshIndex = renderer->addMesh(sphereMesh);
	for (int i = 0; i < sphereCount; ++i) {
		sphere = scene->createEntity();
		scene->addComponent<Transformation>(
			sphere, Transformation(spheres[i].position, glm::vec3(0), glm::vec3(spheres[i].radius)));
		scene->addComponent<RendererComponent>(sphere,
											   RendererComponent(-1, meshIndex, renderer->addMaterial(spheres[i].mat)));
	}
	renderer->loadData();
}

void initPathTracer() {
	screenQuad = makeScreenQuad();

	pathTracer	  = new ComputeShader("./shaders/pathTracing/tracer.comp");
	normalizer	  = new ComputeShader("./shaders/pathTracing/normalizer.comp");
	renderTexture = new Texture2d(window->getWidth(), window->getHeight());
	renderTexture->bindImage(0);

	rawTexture = new Texture2d(window->getWidth(), window->getHeight());
	rawTexture->bindImage(1);

	skybox = new TextureCubemap("./res/images/skybox", ".jpg");
	skybox->bind(GL_TEXTURE0);

	initSpheres();

	pathTracer->bind();
	pathTracer->setUniform("resolution", glm::vec2(window->getWidth(), window->getHeight()));
	pathTracer->setUniform("img_output", 1);
	if (pathTracer->hasUniform("skybox")) pathTracer->setUniform("skybox", 0);
	pathTracer->setUniform("fov", camera->getFov());
	pathTracer->setUniform("spheres_count", sphereCount);
	pathTracer->setUniform("max_bounces", 5);
	pathTracer->setUniform("do_trace_spheres", true);
	pathTracer->setUniform("fov", glm::radians(70.f));
	// pathTracer->unbind();

	renderTexture->bind(GL_TEXTURE0);
	rawTexture->bind(GL_TEXTURE1);

	textureOnScreen = new VFShader("./shaders/ui/textureOnScreen.vs", "./shaders/ui/textureOnScreen.fs");
	textureOnScreen->bind();
	textureOnScreen->setUniform("texture_sampler", 0);
	textureOnScreen->unbind();

	glGenBuffers(1, &spheresBuff);
	glBindBuffer(GL_UNIFORM_BUFFER, spheresBuff);
	glBufferData(GL_UNIFORM_BUFFER, sphereCount * sizeof(Sphere), spheres, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	pathTracer->setUBO(spheresBuff, 1);
}

int main(int argc, char *argv[]) {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	window = new Window(1000, 800, "Test Window", true, false);
	mouse  = new Mouse(*window);
	Keyboard::init(window);

	Keyboard::addKeyCallback([&](GLFWwindow *windowHandle, int key, int scancode, int action, int mods) {
		if (windowHandle != window->getHandle()) return;
		if (key == GLFW_KEY_T && action == GLFW_RELEASE) {
			pathTrace = !pathTrace;
			glfwSwapInterval(!pathTrace);
		}
	});

	initScene();

	initPathTracer();

	glClearColor(0, 0, 0, 1);
	while (!window->shouldClose()) {
		window->beginFrame();
		mouse->update();

		controller->update(window->deltaTime);
		camera->update();

		if (!pathTrace) {
			renderer->doWork();
		} else {
			pathTracer->bind();
			Transformation t =
				Transformation(camera->transform.position, -camera->transform.rotation, camera->transform.scale);
			pathTracer->setUniform("cameraMatrix", t.getWorldMatrix());
			pathTracer->setUniform("random_seed", (GLuint)rand());

			if (controller->hasChanged()) {
				glClearTexImage(rawTexture->getID(), 0, GL_RGBA, GL_FLOAT, new GLfloat[]{0., 0., 0., 1.});
				sampleCount = 0;
			}
			Renderer::compute(pathTracer, window->getWidth(), window->getHeight(), 1);

			sampleCount++;
			normalizer->bind();
			normalizer->setUniform("samples", sampleCount);

			Renderer::compute(normalizer, window->getWidth(), window->getHeight(), 1);

			Renderer::drawObject(textureOnScreen, screenQuad);
		}
		window->swapBuffers();
	}

	// clean up and exit
	cleanup();
	delete window;
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}
