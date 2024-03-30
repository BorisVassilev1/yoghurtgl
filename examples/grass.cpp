#include <yoghurtgl.h>

#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <input.h>
#include <ecs.h>
#include <renderer.h>
#include <effects.h>
#include <entities.h>

#include <iostream>
#include "GLFW/glfw3.h"
#include "animations.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <ImGuizmo.h>
#include <glm/gtx/string_cast.hpp>

using namespace ygl;
using namespace std;

void run() {
	Window window = Window(1200, 800, "Grass Test", true);

	VFShader		 *shader	   = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	VFShader		 *shadowShader = new VFShader("./shaders/simple.vs", "./shaders/simple_shadow.fs");
	PerspectiveCamera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse			mouse(window);
	FPController	controller(&window, &mouse, cam.transform);
	TransformGuizmo guizmo(&window, &cam, (Transformation *)nullptr);

	Scene scene;
	scene.registerComponent<Transformation>();

	Renderer	 *renderer	  = scene.registerSystem<Renderer>(&window);
	GrassSystem	 *grassSystem = scene.registerSystem<GrassSystem>();
	AssetManager *asman		  = scene.getSystem<AssetManager>();

	uint shaderIndex	   = asman->addShader(shader, "defaultShader");
	uint shadowShaderIndex = asman->addShader(shadowShader, "defaultShadowShader");
	renderer->setDefaultShader(shaderIndex);
	renderer->setDefaultShadowShader(shadowShaderIndex);
	renderer->setMainCamera(&cam);
	renderer->setShadow(true);

	glm::ivec2 size			  = glm::ivec2(30);
	Mesh	  *planeMesh	  = new PlaneMesh(size, glm::vec2(1, 1));
	uint	   planeMeshIndex = asman->addMesh(planeMesh, "plane");
	uint	   groundMaterial = renderer->addMaterial(
		  Material(glm::vec3(0, 0.1, 0), 0.02, glm::vec3(), 1.4, glm::vec3(0), 0, glm::vec3(0), 0, 0.8, 0));

	int chunks = 3;
	for (int i = 0; i < chunks; ++i) {
		for (int j = 0; j < chunks; ++j) {
			Entity plane = scene.createEntity();
			scene.addComponent(plane,
							   Transformation(glm::vec3((i - chunks / 2) * size.x, 0, (j - chunks / 2) * size.y)));
			scene.addComponent(plane, RendererComponent(-1, planeMeshIndex, groundMaterial));
			scene.addComponent(plane, GrassSystem::GrassHolder(size, 3., 1));
		}
	}

	Entity model = -1;
	try {
		ygl::addModels(scene, "./res/models/gnome/scene.gltf", [&](Entity e) {
			Transformation &t = scene.getComponent<Transformation>(e);
			t.scale *= 0.01;
			t.position.y = 4;
			t.position.x = 6;
			t.updateWorldMatrix();
		});
	} catch (std::exception &e) { std::cerr << e.what() << std::endl; }

	try {
		ygl::addModels(scene, "./res/models/medieval_knight_fixed/scene.gltf", [&](Entity e) {
			Transformation &t = scene.getComponent<Transformation>(e);
			// t.scale *= 0.05;
			t.scale *= 5;
			t.position.x = -9;
			t.updateWorldMatrix();
			scene.getComponent<RendererComponent>(e).isAnimated = true;
			model												= e;
		});
	} catch (std::exception &e) { std::cerr << e.what() << std::endl; }

	Animation idle(MeshFromFile::loadedScene, 0);

	MeshFromFile::loadSceneIfNeeded("./res/models/medieval_knight/Falling Back Death.dae");
	Animation die(MeshFromFile::loadedScene, 0, Stop);

	MeshFromFile::loadSceneIfNeeded("./res/models/medieval_knight/Sword And Shield Run.dae");
	Animation run(MeshFromFile::loadedScene, 0);

	MeshFromFile::loadSceneIfNeeded("./res/models/medieval_knight/Sword And Shield Slash.dae");
	Animation attack(MeshFromFile::loadedScene, 0, Stop);

	auto	*meshToAnimate = (AnimatedMesh *)asman->getMesh(scene.getComponent<RendererComponent>(model).meshIndex);
	Animator animator(meshToAnimate, &idle);
	
	AnimationFSM fsm(&animator, &idle);
	fsm.addAnimation(&die);
	fsm.addAnimation(&run);
	fsm.addAnimation(&attack);

	addSkybox(scene, "res/images/meadow_4k", ".hdr");
	// addSkybox(scene, "res/images/blue_photo_studio_4k", ".hdr");
	//  Entity skybox = addSkybox(scene, "./res/images/skybox/");

	renderer->addLight(Light(Transformation(glm::vec3(0, 20, 0), glm::vec3(-0.7, 0.8, 0), glm::vec3(1.)), glm::vec3(1.),
							 4, Light::DIRECTIONAL));

	renderer->loadData();
	// scene is initialized

	glm::vec3 clearColor(0.07f, 0.13f, 0.17f);
	clearColor *= 1.5;
	clearColor = glm::pow(clearColor, glm::vec3(2.4));

	Keyboard::addKeyCallback([&](GLFWwindow *, int key, int, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL) { asman->reloadShaders(); }
		if (key == GLFW_KEY_H && action == GLFW_RELEASE) { fsm.transition(0); }
		if (key == GLFW_KEY_J && action == GLFW_RELEASE) { fsm.transition(1); }
		if (key == GLFW_KEY_K && action == GLFW_RELEASE) { fsm.transition(2); }
		if (key == GLFW_KEY_L && action == GLFW_RELEASE) { fsm.transition(3); }
	});


	int				 textureViewIndex = 8;
	ImGuiWindowFlags window_flags	  = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;

	float animationBlendFactor = 0.0;

	renderer->setClearColor(glm::vec4(clearColor, 1.));
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();
		controller.update();
		cam.update();

		//animator.UpdateAnimationBlended(window.deltaTime, animationBlendFactor);
		fsm.update(window.deltaTime);

		for (Entity e : grassSystem->entities) {
			Transformation			 &transform = scene.getComponent<Transformation>(e);
			GrassSystem::GrassHolder &holder	= scene.getComponent<GrassSystem::GrassHolder>(e);
			float					  distance	= glm::length(transform.position - cam.transform.position);
			holder.LOD							= distance / 120;
		}

		grassSystem->doWork();
		renderer->doWork();

		// Transformation &transform = scene.getComponent<Transformation>(model == (uint)-1 ? 0 : model);
		Transformation transform = renderer->getLight(0).transform;
		guizmo.update(&transform);
		renderer->getLight(0).transform = transform.getWorldMatrix();
		renderer->loadData();

		if (grassSystem->getMaterial().drawImGui()) renderer->loadData();

		ImGui::Begin("Grass controls", nullptr, window_flags);
		if (ImGui::SliderFloat("density", &(grassSystem->density), 1., 10.)) { grassSystem->reload(); }
		if (ImGui::SliderFloat("curvature", &grassSystem->curvature, 0, 1)) {}
		if (ImGui::SliderFloat("facingOffset", &grassSystem->facingOffset, 0, 2)) {}
		if (ImGui::SliderFloat("height", &grassSystem->height, 0, 2)) {}
		if (ImGui::SliderFloat("width", &grassSystem->width, 0, 1)) {}

		if (ImGui::SliderFloat3("ground rotation", (float *)&transform.rotation, -M_PI, M_PI)) {
			transform.updateWorldMatrix();
		}
		if (ImGui::SliderFloat3("ground position", (float *)&transform.position, -20, 20)) {
			transform.updateWorldMatrix();
		}

		ImGui::InputInt("Render Mode", (int *)&renderer->renderMode);
		ImGui::Checkbox("Alpha Correction", &(renderer->getScreenEffect(0)->enabled));
		ImGui::InputInt("Selection", (int *)&model);

		ImGui::InputInt("Texture ID", &textureViewIndex);
		ImGui::Image((void *)(std::size_t)textureViewIndex, ImVec2(256, 256));

		ImGui::SliderFloat("animation blend", &animationBlendFactor, 0, 1);

		ImGui::End();

		ImGui::Begin("Profiler");
		float time = window.deltaTime;
		ImGui::PlotHistogram("frame time", &time, 1, 0, NULL, 0.0f, 0.1f, ImVec2(0, 80));
		ImGui::End();

		window.swapBuffers();
	}

	ofstream out("scene.sc");
	scene.write(out);
	out.close();
}

int main() {
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
