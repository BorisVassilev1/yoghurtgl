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
	uint	  matIdx;

	Sphere() {}
	Sphere(const glm::vec3 &position, float radius, uint matIdx) : position(position), radius(radius), matIdx(matIdx) {}

   private:
	char padding[12];
};

struct Box {
	glm::vec3 min;
	char	  padding[4];
	glm::vec3 max;
	uint	  matIdx;

	Box() {}
	Box(const glm::vec3 &min, const glm::vec3 &max, uint matIdx) : min(min), max(max), matIdx(matIdx) {}

   private:
};

Window *window;
Mouse  *mouse;

Texture2d	  *tex;
Scene		  *scene;
Renderer	 *renderer;
VFShader	 *shader;
VFShader	 *unlitShader;
uint		  unlitShaderIndex = -1;
Camera	   *camera;
FPController *controller;
Mesh		 *sphereMesh;
Mesh		 *bunnyMesh;
Mesh		 *cubeMesh;

Entity bunny;

ComputeShader  *pathTracer;
ComputeShader  *normalizer;
Texture2d	  *renderTexture;
Texture2d	  *rawTexture;
VFShader		 *textureOnScreen;
Mesh			 *screenQuad;
TextureCubemap *skybox;

Sphere *spheres;
int		sphereCount;
GLuint	spheresBuff;

Box	*boxes;
int	   boxesCount;
GLuint boxesBuff;

bool pathTrace = false;
bool shade	   = true;
bool cullFace  = true;

int sampleCount = 0;
int maxSamples	= 10;

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
	bunnyMesh  = (Mesh *)getModel(loadScene("./res/models/bunny.obj"));
	sphereMesh = makeUnitSphere();
	cubeMesh   = makeBox(glm::vec3(1, 1, 1), glm::vec3(1, 1, 1));

	shader		= new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	unlitShader = new VFShader("./shaders/unlit.vs", "./shaders/unlit.fs");
	camera		= new Camera(glm::radians(70.f), *window, 0.01, 1000);
	controller	= new FPController(window, mouse, camera->transform);

	tex = new Texture2d("./res/images/uv_checker.png");
	tex->bind();

	scene = new Scene();
	scene->registerComponent<Transformation>();
	scene->registerComponent<RendererComponent>();

	renderer = scene->registerSystem<Renderer>();
	scene->setSystemSignature<Renderer, Transformation, RendererComponent>();
	renderer->setUseTexture(true);

	bunny = scene->createEntity();
	scene->addComponent<Transformation>(bunny, Transformation(glm::vec3(-3, 0, 0)));
	RendererComponent bunnyRenderer;

	unsigned int shaderIndex = renderer->addShader(shader);
	renderer->setDefaultShader(shaderIndex);
	scene->addComponent<RendererComponent>(
		bunny,
		RendererComponent(-1, renderer->addMesh(bunnyMesh),
						  renderer->addMaterial(Material(glm::vec3(1., 1., 1.), .2, glm::vec3(0.), 1.0, glm::vec3(0.0),
														 0.0, glm::vec3(1.), 0.0, 1.0, 0, 0.0))));

	unlitShaderIndex = renderer->addShader(unlitShader);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(-1, -2.9, 0), glm::vec3(1)), glm::vec3(1., 1., 1.),
							 0.7, Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.1, Light::Type::AMBIENT));

	renderer->loadData();
}

void initSpheres() {
	sphereCount = 4 + 7;
	spheres		= new Sphere[7 + 4];

	spheres[0] = Sphere(
		glm::vec3(0.2, 1.8, 0.2), 0.5,
		renderer->addMaterial(Material(glm::vec3(1.0, 1.0, 0.5), 1.0, glm::vec3(0.0, 0.0, 0.0), 1.0,
									   glm::vec3(0.0, 0.0, 0.0), 0.0, glm::vec3(1.0, 1.0, 0.5), 0.00, 0.05, 0, 0)));
	spheres[1] =
		Sphere(glm::vec3(1.0, 0.7, 0.7), 0.7,
			   renderer->addMaterial(Material(glm::vec3(0.1, 1.0, 0.1), 0.2, glm::vec3(0.0, 0.0, 0.0), 1.0,
											  glm::vec3(0.0, 0.0, 0.0), 0.0, glm::vec3(1.), 0.00, 0.03, 0, 0)));
	spheres[2] =
		Sphere(glm::vec3(-0.7, 1.0, 0.2), 0.5,
			   renderer->addMaterial(Material(glm::vec3(0.0, 1.0, 1.0), 0.0, glm::vec3(0.0, 0.0, 0.0), 1.7,
											  glm::vec3(1.0, 0.0, 0.0), 1.0, glm::vec3(1.), 0.05, 0.05, 0, 0)));
	spheres[3] =
		Sphere(glm::vec3(-0.1, 1.8, 1.7), 0.4,
			   renderer->addMaterial(Material(glm::vec3(1.0, 1.0, 1.0), 0.0, glm::vec3(10.0, 10.0, 10.0), 1.0,
											  glm::vec3(0.0, 0.0, 0.0), 0.0, glm::vec3(1.), 0.00, 0.00, 0, 0)));

	renderer->addLight(Light(Transformation(spheres[3].position), glm::vec3(1.), 1, Light::POINT));

	Entity sphere;
	for (int i = 0; i < 7; ++i) {
		// glm::vec3 randomColor(rand() % 100 / 100., rand() % 100 / 100., rand() % 100 / 100.);
		glm::vec3 randomColor(1, 0.5, 0);

		spheres[i + 4] = Sphere(glm::vec3(i * 1.5 - 5, 0.5, 3), 0.5,
								renderer->addMaterial(Material(randomColor, 0.02, glm::vec3(0.0, 0.0, 0.0), 1.7,
															   glm::vec3(1.0) - randomColor, 1., glm::vec3(1.),
															   0.05 + i * 0.1, 0.05 + i * 0.1, 0, 0.0)));
	}

	uint meshIndex = renderer->addMesh(sphereMesh);
	for (int i = 0; i < sphereCount; ++i) {
		sphere = scene->createEntity();
		scene->addComponent<Transformation>(
			sphere, Transformation(spheres[i].position, glm::vec3(0), glm::vec3(spheres[i].radius)));
		scene->addComponent<RendererComponent>(sphere, RendererComponent(-1, meshIndex, spheres[i].matIdx));
	}

	renderer->loadData();
}

void initBoxes() {
	boxesCount = 7;
	boxes	   = new Box[7];

	Entity box;
	for (int i = 0; i < 7; ++i) {
		glm::vec3 randomColor(rand() % 100 / 100., rand() % 100 / 100., rand() % 100 / 100.);

		glm::vec3 min = glm::vec3(i * 1.5 - 5, 0.5, 4.5);

		boxes[i] =
			Box(min, min + glm::vec3(1.0),
				renderer->addMaterial(Material(randomColor, 0.01, glm::vec3(0.0, 0.0, 0.0), 1.7,
											   glm::vec3(1.0) - randomColor, 1.0, glm::vec3(1.), 0.00, 0.00, 0, 0.0)));
	}

	uint meshIndex = renderer->addMesh(cubeMesh);
	for (int i = 0; i < boxesCount; ++i) {
		box = scene->createEntity();
		scene->addComponent<Transformation>(box, Transformation(boxes[i].min + glm::vec3(0.5)));
		scene->addComponent<RendererComponent>(box, RendererComponent(-1, meshIndex, boxes[i].matIdx));
	}

	renderer->loadData();

	glGenBuffers(1, &boxesBuff);
	glBindBuffer(GL_UNIFORM_BUFFER, boxesBuff);
	glBufferData(GL_UNIFORM_BUFFER, boxesCount * sizeof(Box), boxes, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	pathTracer->setUBO(boxesBuff, 4);
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
	initBoxes();

	pathTracer->bind();
	pathTracer->setUniform("resolution", glm::vec2(window->getWidth(), window->getHeight()));
	pathTracer->setUniform("img_output", 1);
	if (pathTracer->hasUniform("skybox")) pathTracer->setUniform("skybox", 0);
	pathTracer->setUniform("fov", camera->getFov());
	pathTracer->setUniform("spheresCount", sphereCount);
	pathTracer->setUniform("boxesCount", boxesCount);
	pathTracer->setUniform("max_bounces", 10);
	pathTracer->setUniform("do_trace_spheres", true);
	pathTracer->setUniform("fov", glm::radians(70.f));
	// pathTracer->unbind();

	renderTexture->bind(GL_TEXTURE1);
	rawTexture->bind(GL_TEXTURE2);

	textureOnScreen = new VFShader("./shaders/ui/textureOnScreen.vs", "./shaders/ui/textureOnScreen.fs");
	textureOnScreen->bind();
	textureOnScreen->setUniform("texture_sampler", 1);
	textureOnScreen->unbind();

	glGenBuffers(1, &spheresBuff);
	glBindBuffer(GL_UNIFORM_BUFFER, spheresBuff);
	glBufferData(GL_UNIFORM_BUFFER, sphereCount * sizeof(Sphere), spheres, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	pathTracer->setUBO(spheresBuff, 3);
}

int main(int argc, char *argv[]) {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	window = new Window(1000, 800, "Test Window", true, false);
	mouse  = new Mouse(*window);

	Keyboard::addKeyCallback([&](GLFWwindow *windowHandle, int key, int scancode, int action, int mods) {
		if (windowHandle != window->getHandle()) return;
		if (key == GLFW_KEY_T && action == GLFW_RELEASE) {
			pathTrace = !pathTrace;
			// glfwSwapInterval(!pathTrace);
		}
	});

	initScene();

	initPathTracer();

	tex->bind();

	glClearColor(0, 0, 0, 0);
	while (!window->shouldClose()) {
		window->beginFrame();
		mouse->update();

		controller->update(window->deltaTime);
		camera->update();

		if (controller->hasChanged()) {
			glClearTexImage(rawTexture->getID(), 0, GL_RGBA, GL_FLOAT, new GLfloat[]{0., 0., 0., 0.});
			sampleCount = 0;
		}

		if (!pathTrace) {
			renderer->doWork();
		} else {
			if (sampleCount < maxSamples) {
				pathTracer->bind();
				Transformation t =
					Transformation(camera->transform.position, -camera->transform.rotation, camera->transform.scale);
				pathTracer->setUniform("cameraMatrix", t.getWorldMatrix());
				pathTracer->setUniform("random_seed", (GLuint)rand());

				Renderer::compute(pathTracer, window->getWidth(), window->getHeight(), 1);

				sampleCount++;
				normalizer->bind();
				normalizer->setUniform("samples", sampleCount);

				Renderer::compute(normalizer, window->getWidth(), window->getHeight(), 1);
			}
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
