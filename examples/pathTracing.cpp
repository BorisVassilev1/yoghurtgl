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
#include <asset_manager.h>

#include <iostream>
#include <random>
#include <time.h>
#include <glm/gtc/random.hpp>
#include <material.h>
#include <timer.h>

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
	Box(const glm::vec3 &min, const glm::vec3 &max, uint matIdx) : min(min), matIdx(matIdx), max(max) {}

   private:
};

ygl::Window *window;
ygl::Mouse	*mouse;

ygl::Texture2d		   *tex;
ygl::Scene			   *scene;
ygl::Renderer		   *renderer;
ygl::VFShader		   *shader;
ygl::PerspectiveCamera *camera;
ygl::FPController	   *controller;
ygl::Mesh			   *sphereMesh;
ygl::Mesh			   *bunnyMesh;
ygl::Mesh			   *cubeMesh;

ygl::Entity bunny;

ygl::ComputeShader	*pathTracer;
ygl::ComputeShader	*normalizer;
ygl::Texture2d		*renderTexture;
ygl::Texture2d		*rawTexture;
ygl::VFShader		*textureOnScreen;
ygl::Mesh			*screenQuad;
ygl::TextureCubemap *skybox;

Sphere *spheres = nullptr;
int		sphereCount;

Box *boxes = nullptr;
int	 boxesCount;

bool pathTrace = false;
bool shade	   = true;
bool cullFace  = true;

int	  sampleCount = 0;
int	  maxSamples  = 100000;
float PI		  = glm::pi<float>();
Timer timer;

const GLuint RENDER_TEXTURE_BINDING = GL_TEXTURE15;
const GLuint RAW_TEXTURE_BINDING	= GL_TEXTURE16;

ygl::bvh::BVHTree *bvh = new ygl::bvh::BVHTree();

void cleanup() {
	if (tex != nullptr) delete tex;
	if (scene != nullptr) delete scene;
	if (camera != nullptr) delete camera;
	if (controller != nullptr) delete controller;
	if (pathTracer != nullptr) delete pathTracer;
	if (normalizer != nullptr) delete normalizer;
	if (renderTexture != nullptr) delete renderTexture;
	if (rawTexture != nullptr) delete rawTexture;
	if (textureOnScreen != nullptr) delete textureOnScreen;
	if (screenQuad != nullptr) delete screenQuad;
	if (bvh != nullptr) delete bvh;
	if (spheres != nullptr) delete[] spheres;
	if (spheres != nullptr) delete[] boxes;
	if (mouse != nullptr) delete mouse;
	if (window != nullptr) delete window;
}

void initScene() {
	try {
		bunnyMesh = new ygl::MeshFromFile("./res/models/Knight.obj", 0);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		cleanup();
		exit(0);
	}
	sphereMesh = new ygl::SphereMesh();
	cubeMesh   = new ygl::BoxMesh(glm::vec3(1, 1, 1), glm::vec3(1, 1, 1));

	shader	   = new ygl::VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	camera	   = new ygl::PerspectiveCamera(glm::radians(70.f), *window, 0.01, 1000);
	controller = new ygl::FPController(window, mouse, camera->transform);

	scene = new ygl::Scene();
	scene->registerComponent<ygl::Transformation>();

	renderer = scene->registerSystem<ygl::Renderer>(window);

	renderer->addMaterial(
		ygl::Material(glm::vec3(1.f), 0.1, glm::vec3(0.f), 1.0, glm::vec3(0.0), 0.0, glm::vec3(1.0), 0.0, 0.0, 0.0));

	ygl::AssetManager *asman = scene->getSystem<ygl::AssetManager>();
	ygl::Material	   geometryMaterial = 
		//ygl::MeshFromFile::getMaterial(ygl::MeshFromFile::loadedScene, asman, "./res/models/bunny_uv/", 0);
		ygl::Material(glm::vec3(1.f), 0.1, glm::vec3(0.f), 1.0, glm::vec3(0.0), 0.0, glm::vec3(1.0), 0.0, 0.0, 0.0);

	bunny = scene->createEntity();
	scene->addComponent<ygl::Transformation>(
		bunny, ygl::Transformation(glm::vec3(-2, 1, 3), glm::vec3(0, -PI / 2, 0), glm::vec3(0.01f)));
	ygl::RendererComponent bunnyRenderer;

	unsigned int shaderIndex = asman->addShader(shader, "defaultShader");
	renderer->setDefaultShader(shaderIndex);
	scene->addComponent<ygl::RendererComponent>(
		bunny, ygl::RendererComponent(-1, asman->addMesh(bunnyMesh, "bunny"), renderer->addMaterial(geometryMaterial)));

	ygl::Shader::setSSBO(bunnyMesh->getVertices().bufferId, 2);
	ygl::Shader::setSSBO(bunnyMesh->getNormals().bufferId, 3);
	ygl::Shader::setSSBO(bunnyMesh->getTexCoords().bufferId, 8);
	ygl::Shader::setSSBO(bunnyMesh->getTangents().bufferId, 9);
	ygl::Shader::setSSBO(bunnyMesh->getIBO(), 4);

	ygl::addSkybox(*scene, "res/images/meadow_4k", ".hdr");

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
		glm::vec3 randomColor(1, 0.5, 0);

		spheres[i + 4] = Sphere(glm::vec3(i * 1.5 - 5, 0.5, 3), 0.5,
								renderer->addMaterial(ygl::Material(randomColor, 0.02, glm::vec3(0.0, 0.0, 0.0), 1.7,
																	glm::vec3(1.0) - randomColor, 1., glm::vec3(1.),
																	0.05 + i * 0.1, 0.05 + i * 0.1, 0.)));
	}

	uint meshIndex = scene->getSystem<ygl::AssetManager>()->addMesh(sphereMesh, "sphere");
	for (int i = 0; i < sphereCount; ++i) {
		sphere = scene->createEntity();
		scene->addComponent<ygl::Transformation>(
			sphere, ygl::Transformation(spheres[i].position, glm::vec3(0), glm::vec3(spheres[i].radius)));
		scene->addComponent<ygl::RendererComponent>(sphere, ygl::RendererComponent(-1, meshIndex, spheres[i].matIdx));
		bvh->addPrimitive(new ygl::bvh::SpherePrimitive(spheres[i].position, spheres[i].radius, spheres[i].matIdx));
	}

	renderer->loadData();
}

void initBoxes() {
	boxesCount = 10;
	boxes	   = new Box[10];

	ygl::Entity box;
	for (int i = 0; i < boxesCount; ++i) {
		glm::vec3 randomColor(rand() % 100 / 100., rand() % 100 / 100., rand() % 100 / 100.);

		glm::vec3 min = glm::vec3(int(i / 2) * 1.5 - 5, 0.5 + 1.5 * (i % 2), -1.5);

		float roughness = glm::linearRand(0.f, 0.5f);

		boxes[i] = Box(min, min + glm::vec3(1.0),
					   renderer->addMaterial(ygl::Material(randomColor, 0.01, glm::vec3(0.0, 0.0, 0.0), 1.45,
														   glm::vec3(1.0) - randomColor, 1.0, glm::vec3(1.),
														   0.05 + roughness, 0.05 + roughness, 0.)));
	}

	uint meshIndex = scene->getSystem<ygl::AssetManager>()->addMesh(cubeMesh, "cube");
	for (int i = 0; i < boxesCount; ++i) {
		box = scene->createEntity();
		scene->addComponent<ygl::Transformation>(box, ygl::Transformation(boxes[i].min + glm::vec3(0.5)));
		scene->addComponent<ygl::RendererComponent>(box, ygl::RendererComponent(-1, meshIndex, boxes[i].matIdx));
		bvh->addPrimitive(new ygl::bvh::BoxPrimitive(boxes[i].min, boxes[i].max, boxes[i].matIdx));
	}

	renderer->loadData();
}

void initPathTracer() {
	screenQuad = new ygl::QuadMesh();

	pathTracer	  = new ygl::ComputeShader("./shaders/pathTracing/tracer.comp");
	normalizer	  = new ygl::ComputeShader("./shaders/pathTracing/normalizer.comp");
	renderTexture = new ygl::Texture2d(window->getWidth(), window->getHeight(), ygl::TextureType::RGBA32F, nullptr);
	renderTexture->bindImage(0);

	rawTexture = new ygl::Texture2d(window->getWidth(), window->getHeight(), ygl::TextureType::RGBA32F, nullptr);
	rawTexture->bindImage(1);

	//skybox = ygl::loadHDRCubemap("res/images/blue_photo_studio_4k", ".hdr");
	//ygl::addSkybox(*scene, "res/images/blue_photo_studio_4k", ".hdr");
	skybox = (ygl::TextureCubemap*) scene->getSystem<ygl::AssetManager>()->getTexture(renderer->skyboxTexture);
	//skybox = new ygl::TextureCubemap("./res/images/skybox", ".jpg");
	skybox->bind(ygl::TexIndex::SKYBOX);

	// initSpheres();
	initBoxes();
	bvh->addPrimitive(bunnyMesh, scene->getComponent<ygl::Transformation>(bunny));

	bvh->build();

	pathTracer->bind();
	pathTracer->setUniform("resolution", glm::vec2(window->getWidth(), window->getHeight()));
	pathTracer->setUniform("img_output", 1);
	pathTracer->setUniform("fov", camera->getFov());
	pathTracer->setUniform("max_bounces", 5);
	pathTracer->setUniform("fov", glm::radians(70.f));
	pathTracer->setUniform("bvh_matrix", scene->getComponent<ygl::Transformation>(bunny).getWorldMatrix());
	pathTracer->unbind();

	renderTexture->bind(RENDER_TEXTURE_BINDING);
	rawTexture->bind(RAW_TEXTURE_BINDING);

	textureOnScreen = new ygl::VFShader("./shaders/ui/textureOnScreen.vs", "./shaders/ui/textureOnScreen.fs");
	textureOnScreen->bind();
	textureOnScreen->setUniform("sampler_color", 15);
	textureOnScreen->setUniform("sampler_depth", 15);
	textureOnScreen->setUniform("sampler_stencil", 15);
	textureOnScreen->unbind();
}

int main() {
	if (ygl::init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	window = new ygl::Window(1280, 1000, "Test Window", true, false);
	mouse  = new ygl::Mouse(*window);

	ygl::Keyboard::addKeyCallback([&](GLFWwindow *windowHandle, int key, int, int action, int) {
		if (windowHandle != window->getHandle()) return;
		if (key == GLFW_KEY_T && action == GLFW_RELEASE) {
			pathTrace = !pathTrace;
			glfwSwapInterval(!pathTrace);
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
			std::cout << "Saving image...\n samples: " << sampleCount << "\n time: " << timer.toMs(timer.elapsedNs())
					  << std::endl;
			renderTexture->save("./res/images/result.png");
		}
	});

	initScene();

	initPathTracer();

	// tex->bind();

	int textureViewIndex = 6;
	int editMaterialIndex = 0;

	dbLog(ygl::LOG_INFO, rawTexture->getID());
	dbLog(ygl::LOG_INFO, renderTexture->getID());

	bool shouldReload = false;

	renderer->setClearColor(glm::vec4(0, 0, 0, 1));
	while (!window->shouldClose()) {
		window->beginFrame();
		mouse->update();

		controller->update();
		camera->update();

		if (controller->hasChanged() || shouldReload) {
			float clearColor[]{0., 0., 0., 0.};
			glClearTexImage(rawTexture->getID(), 0, GL_RGBA, GL_FLOAT, &clearColor);
			sampleCount = 0;
			timer		= Timer();
			shouldReload = false;
		}

		if (!pathTrace) {
			renderer->doWork();
		} else {
			renderTexture->bind(RENDER_TEXTURE_BINDING);
			rawTexture->bind(RAW_TEXTURE_BINDING);
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

		ImGui::Begin("Texture View");
		ImGui::InputInt("Texture ID", &textureViewIndex);
		ImGui::Image((void*)textureViewIndex, ImVec2(256, 256));
		ImGui::End();

		ImGui::Begin("Material Properties");
		ImGui::InputInt("Material ID", &editMaterialIndex);
		ImGui::End();
		shouldReload = shouldReload || renderer->getMaterial(editMaterialIndex).drawImGui();
		renderer->loadData();

		window->swapBuffers();
	}

	// clean up and exit
	cleanup();
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}
