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
#include <entities.h>
#include <bvh.h>

#include <iostream>
#include <random>
#include <time.h>

using namespace std;

struct alignas(16) Sphere {
	glm::vec3 position;
	float	  radius = 1;
	uint	  matIdx;

	Sphere() {}
	Sphere(const glm::vec3 &position, float radius, uint matIdx) : position(position), radius(radius), matIdx(matIdx) {}
};

struct alignas(16) Box {
	glm::vec3 min;
	uint	  matIdx;
	glm::vec3 max;

	Box() {}
	Box(const glm::vec3 &min, const glm::vec3 &max, uint matIdx) : min(min), max(max), matIdx(matIdx) {}

   private:
};

struct SpherePrimitive : Primitive {
	glm::vec3 position;
	float	  radius = 1;
	uint	  matIdx;

	SpherePrimitive() {}
	SpherePrimitive(const glm::vec3 &position, float radius, uint matIdx) : position(position), radius(radius), matIdx(matIdx) {
		box = BBox(position - radius, position + radius);
	}
	
	bool intersect(const Ray &ray, float tMin, float tMax, Intersection &intersection) { return false; }
	
	glm::vec3 getCenter() { return position; }
};

struct BoxPrimitive : Primitive {
	uint	  matIdx;

	BoxPrimitive() {}
	BoxPrimitive(const glm::vec3 &min, const glm::vec3 &max, uint matIdx) : matIdx(matIdx) {
		box = BBox(min, max);
	}
	
	bool intersect(const Ray &ray, float tMin, float tMax, Intersection &intersection) override { return false; }
   private:
};

ygl::Window *window;
ygl::Mouse	*mouse;

ygl::Texture2d	  *tex;
ygl::Scene		  *scene;
ygl::Renderer	  *renderer;
ygl::VFShader	  *shader;
ygl::VFShader	  *unlitShader;
uint			   unlitShaderIndex = -1;
ygl::Camera		  *camera;
ygl::FPController *controller;
ygl::Mesh		  *sphereMesh;
ygl::Mesh		  *bunnyMesh;
ygl::Mesh		  *cubeMesh;

ygl::Entity bunny;

ygl::ComputeShader	*pathTracer;
ygl::ComputeShader	*normalizer;
ygl::Texture2d		*renderTexture;
ygl::Texture2d		*rawTexture;
ygl::VFShader		*textureOnScreen;
ygl::Mesh			*screenQuad;
ygl::TextureCubemap *skybox;

Sphere *spheres;
int		sphereCount;
GLuint	spheresBuff;

Box	  *boxes;
int	   boxesCount;
GLuint boxesBuff;

bool pathTrace = false;
bool shade	   = true;
bool cullFace  = true;

int sampleCount = 0;
int maxSamples	= 100000;

BVHTree *bvh = new BVHTree();

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
	delete[] spheres;
	delete[] boxes;
	delete skybox;
	delete mouse;
	delete window;
}

void initScene() {
	bunnyMesh  = (ygl::Mesh *)ygl::getModel(ygl::loadScene("./res/models/bunny_uv/bunny_uv.obj"));
	sphereMesh = ygl::makeUnitSphere();
	cubeMesh   = ygl::makeBox(glm::vec3(1, 1, 1), glm::vec3(1, 1, 1));

	shader		= new ygl::VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	unlitShader = new ygl::VFShader("./shaders/unlit.vs", "./shaders/unlit.fs");
	camera		= new ygl::Camera(glm::radians(70.f), *window, 0.01, 1000);
	controller	= new ygl::FPController(window, mouse, camera->transform);

	scene = new ygl::Scene(window);
	scene->registerComponent<ygl::Transformation>();

	renderer = scene->registerSystem<ygl::Renderer>();

	tex = new ygl::Texture2d("./res/images/uv_checker.png");
	tex->bind(GL_TEXTURE1);		// albedo texture

	bunny = scene->createEntity();
	scene->addComponent<ygl::Transformation>(bunny,
											 ygl::Transformation(glm::vec3(-3, 0, 0), glm::vec3(0), glm::vec3(10)));
	ygl::RendererComponent bunnyRenderer;

	unsigned int shaderIndex = renderer->addShader(shader);
	renderer->setDefaultShader(shaderIndex);
	scene->addComponent<ygl::RendererComponent>(
		bunny,
		ygl::RendererComponent(-1, renderer->addMesh(bunnyMesh),
							   renderer->addMaterial(ygl::Material(glm::vec3(1., 1., 1.), .2, glm::vec3(0.), 1.0,
																   glm::vec3(0.0), 0.0, glm::vec3(1.), 0.0, 1.0, 0.))));

	unlitShaderIndex = renderer->addShader(unlitShader);

	addSkybox(*scene, "./res/images/skybox");

	renderer->addLight(ygl::Light(ygl::Transformation(glm::vec3(0), glm::vec3(-1, -2.9, 0), glm::vec3(1)),
								  glm::vec3(1., 1., 1.), 3, ygl::Light::Type::DIRECTIONAL));
	renderer->addLight(ygl::Light(ygl::Transformation(), glm::vec3(1., 1., 1.), 0.01, ygl::Light::Type::AMBIENT));

	renderer->loadData();
}

void initSpheres() {
	sphereCount = 4 + 7;
	spheres		= new Sphere[7 + 4];

	spheres[0] = Sphere(
		glm::vec3(0.2, 1.8, 0.2), 0.5,
		renderer->addMaterial(ygl::Material(glm::vec3(1.0, 1.0, 0.5), 1.0, glm::vec3(0.0, 0.0, 0.0), 1.0,
											glm::vec3(0.0, 0.0, 0.0), 0.0, glm::vec3(1.0, 1.0, 0.5), 0.00, 0.05, 0.)));
	spheres[1] =
		Sphere(glm::vec3(1.0, 0.7, 0.7), 0.7,
			   renderer->addMaterial(ygl::Material(glm::vec3(0.1, 1.0, 0.1), 0.2, glm::vec3(0.0, 0.0, 0.0), 1.0,
												   glm::vec3(0.0, 0.0, 0.0), 0.0, glm::vec3(1.), 0.00, 0.03, 0.)));
	spheres[2] =
		Sphere(glm::vec3(-0.7, 1.0, 0.2), 0.5,
			   renderer->addMaterial(ygl::Material(glm::vec3(0.0, 1.0, 1.0), 0.0, glm::vec3(0.0, 0.0, 0.0), 1.7,
												   glm::vec3(1.0, 0.0, 0.0), 1.0, glm::vec3(1.), 0.05, 0.05, 0.)));
	spheres[3] =
		Sphere(glm::vec3(-0.1, 1.8, 1.7), 0.4,
			   renderer->addMaterial(ygl::Material(glm::vec3(1.0, 1.0, 1.0), 0.0, glm::vec3(10.0, 10.0, 10.0), 1.0,
												   glm::vec3(0.0, 0.0, 0.0), 0.0, glm::vec3(1.), 0.00, 0.00, 0.)));

	renderer->addLight(ygl::Light(ygl::Transformation(spheres[3].position), glm::vec3(1.), 10, ygl::Light::POINT));

	ygl::Entity sphere;
	for (int i = 0; i < 7; ++i) {
		// glm::vec3 randomColor(rand() % 100 / 100., rand() % 100 / 100., rand() % 100 / 100.);
		glm::vec3 randomColor(1, 0.5, 0);

		spheres[i + 4] = Sphere(glm::vec3(i * 1.5 - 5, 0.5, 3), 0.5,
								renderer->addMaterial(ygl::Material(randomColor, 0.02, glm::vec3(0.0, 0.0, 0.0), 1.7,
																	glm::vec3(1.0) - randomColor, 1., glm::vec3(1.),
																	0.05 + i * 0.1, 0.05 + i * 0.1, 0.)));
	}

	uint meshIndex = renderer->addMesh(sphereMesh);
	for (int i = 0; i < sphereCount; ++i) {
		sphere = scene->createEntity();
		scene->addComponent<ygl::Transformation>(
			sphere, ygl::Transformation(spheres[i].position, glm::vec3(0), glm::vec3(spheres[i].radius)));
		scene->addComponent<ygl::RendererComponent>(sphere, ygl::RendererComponent(-1, meshIndex, spheres[i].matIdx));
	}

	renderer->loadData();
}

void initBoxes() {
	boxesCount = 7;
	boxes	   = new Box[7];

	ygl::Entity box;
	for (int i = 0; i < 7; ++i) {
		glm::vec3 randomColor(rand() % 100 / 100., rand() % 100 / 100., rand() % 100 / 100.);

		glm::vec3 min = glm::vec3(i * 1.5 - 5, 0.5, 4.5);

		boxes[i] = Box(min, min + glm::vec3(1.0),
					   renderer->addMaterial(ygl::Material(randomColor, 0.01, glm::vec3(0.0, 0.0, 0.0), 1.45,
														   glm::vec3(1.0) - randomColor, 1.0, glm::vec3(1.),
														   0.05 + i * 0.1, 0.05 + i * 0.1, 0.)));
	}

	uint meshIndex = renderer->addMesh(cubeMesh);
	for (int i = 0; i < boxesCount; ++i) {
		box = scene->createEntity();
		scene->addComponent<ygl::Transformation>(box, ygl::Transformation(boxes[i].min + glm::vec3(0.5)));
		scene->addComponent<ygl::RendererComponent>(box, ygl::RendererComponent(-1, meshIndex, boxes[i].matIdx));
		bvh->addPrimitive(new BoxPrimitive(boxes[i].min, boxes[i].max, boxes[i].matIdx));
	}

	renderer->loadData();

	glGenBuffers(1, &boxesBuff);
	glBindBuffer(GL_UNIFORM_BUFFER, boxesBuff);
	glBufferData(GL_UNIFORM_BUFFER, boxesCount * sizeof(Box), boxes, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	pathTracer->setUBO(boxesBuff, 4);
}

void initPathTracer() {
	screenQuad = ygl::makeScreenQuad();

	pathTracer	  = new ygl::ComputeShader("./shaders/pathTracing/tracer.comp");
	normalizer	  = new ygl::ComputeShader("./shaders/pathTracing/normalizer.comp");
	renderTexture = new ygl::Texture2d(window->getWidth(), window->getHeight());
	renderTexture->bindImage(0);

	rawTexture = new ygl::Texture2d(window->getWidth(), window->getHeight());
	rawTexture->bindImage(1);

	skybox = new ygl::TextureCubemap("./res/images/skybox", ".jpg");
	skybox->bind(GL_TEXTURE6);

	initSpheres();
	initBoxes();
	bvh->build(BVHTree::Purpose::Generic);
	// TODO: write BVH to GPU memory

	pathTracer->bind();
	pathTracer->setUniform("resolution", glm::vec2(window->getWidth(), window->getHeight()));
	pathTracer->setUniform("img_output", 1);
	pathTracer->setUniform("fov", camera->getFov());
	pathTracer->setUniform("spheresCount", sphereCount);
	pathTracer->setUniform("boxesCount", boxesCount);
	pathTracer->setUniform("max_bounces", 20);
	pathTracer->setUniform("do_trace_spheres", true);
	pathTracer->setUniform("fov", glm::radians(70.f));
	pathTracer->unbind();

	renderTexture->bind(GL_TEXTURE10);
	rawTexture->bind(GL_TEXTURE11);

	textureOnScreen = new ygl::VFShader("./shaders/ui/textureOnScreen.vs", "./shaders/ui/textureOnScreen.fs");
	textureOnScreen->bind();
	textureOnScreen->setUniform("sampler_color", 10);
	textureOnScreen->setUniform("sampler_depth", 10);
	textureOnScreen->setUniform("sampler_stencil", 10);
	textureOnScreen->unbind();

	glGenBuffers(1, &spheresBuff);
	glBindBuffer(GL_UNIFORM_BUFFER, spheresBuff);
	glBufferData(GL_UNIFORM_BUFFER, sphereCount * sizeof(Sphere), spheres, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	pathTracer->setUBO(spheresBuff, 3);
}

int main() {
	if (ygl::init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	cout << endl << sizeof(Box) << endl << endl;

	window = new ygl::Window(1280, 1000, "Test Window", true, false);
	mouse  = new ygl::Mouse(*window);

	ygl::Keyboard::addKeyCallback([&](GLFWwindow *windowHandle, int key, int, int action, int) {
		if (windowHandle != window->getHandle()) return;
		if (key == GLFW_KEY_T && action == GLFW_RELEASE) {
			pathTrace = !pathTrace;
			glfwSwapInterval(!pathTrace);
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) { renderTexture->save("./res/images/result.png"); }
	});

	initScene();

	initPathTracer();

	tex->bind();

	renderer->setClearColor(glm::vec4(0, 0, 0, 1));
	while (!window->shouldClose()) {
		window->beginFrame();
		mouse->update();

		controller->update();
		camera->update();

		if (controller->hasChanged()) {
			float clearColor[]{0., 0., 0., 0.};
			glClearTexImage(rawTexture->getID(), 0, GL_RGBA, GL_FLOAT, &clearColor);
			sampleCount = 0;
		}

		if (!pathTrace) {
			renderer->doWork();
		} else {
			renderTexture->bind(GL_TEXTURE10);
			rawTexture->bind(GL_TEXTURE11);
			renderTexture->bindImage(0);
			rawTexture->bindImage(1);

			if (sampleCount < maxSamples) {
				pathTracer->bind();
				ygl::Transformation t = ygl::Transformation(camera->transform.position, -camera->transform.rotation,
															camera->transform.scale);
				pathTracer->setUniform("cameraMatrix", t.getWorldMatrix());
				pathTracer->setUniform("random_seed", (GLuint)rand());

				// pathTracer->unbind();

				ygl::Renderer::compute(pathTracer, window->getWidth(), window->getHeight(), 1);

				sampleCount++;
				normalizer->bind();
				normalizer->setUniform("samples", sampleCount);
				// normalizer->unbind();

				ygl::Renderer::compute(normalizer, window->getWidth(), window->getHeight(), 1);
			}

			ygl::Renderer::drawObject(textureOnScreen, screenQuad);
		}

		window->swapBuffers();
	}

	// clean up and exit
	cleanup();
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}
