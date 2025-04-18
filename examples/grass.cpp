#include <yoghurtgl.h>

#include <iostream>

#include <window.h>
#include <mesh.h>
#include <shader.h>
#include <input.h>
#include <ecs.h>
#include <renderer.h>
#include <effects.h>
#include <entities.h>
#include <animations.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <ImGuizmo.h>
#include <glm/gtx/string_cast.hpp>
#include "GLFW/glfw3.h"

using namespace ygl;
using namespace std;

void run() {
	Window window = Window(1200, 800, "Grass Test", true);

	VFShader *shader = new VFShader(YGL_RELATIVE_PATH "./shaders/simple.vs", YGL_RELATIVE_PATH "./shaders/simple.fs");
	VFShader *shadowShader =
		new VFShader(YGL_RELATIVE_PATH "./shaders/simple.vs", YGL_RELATIVE_PATH "./shaders/simple_shadow.fs");
	PerspectiveCamera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse mouse(window);
	mouse.disableMouseWhenLockedAndHidden = true;
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
	addEffects(renderer);

	glm::ivec2 size			  = glm::ivec2(40);
	Mesh	  *planeMesh	  = new PlaneMesh(size, glm::vec2(1, 1));
	uint	   planeMeshIndex = asman->addMesh(planeMesh, "plane");
	uint	   groundMaterial = renderer->addMaterial(
		  Material(glm::vec3(0, 0.1, 0), 0.02, glm::vec3(), 1.4, glm::vec3(0), 0, glm::vec3(0), 0, 0.8, 0));

	int chunks = 10;
	for (int i = 0; i < chunks; ++i) {
		for (int j = 0; j < chunks; ++j) {
			Entity plane = scene.createEntity();
			scene.addComponent(plane,
							   Transformation(glm::vec3((i - chunks / 2) * size.x, 0, (j - chunks / 2) * size.y)));
			scene.addComponent(plane, RendererComponent(-1, planeMeshIndex, groundMaterial));
			scene.addComponent(plane, GrassSystem::GrassHolder(size, 3., 1));
		}
	}
	grassSystem->reload();

	Entity model = -1;
	try {
		ygl::addModels(scene, "./res/models/low_poly_fantasy_rune_stone/scene.gltf", [&](Entity e) {
			Transformation &t = scene.getComponent<Transformation>(e);
			t.scale *= 2;
			t.rotation.x += -glm::pi<float>() / 2.;
			t.position.y = 8;
			t.position.x = 6;
			t.updateWorldMatrix();
		});
	} catch (std::exception &e) { std::cerr << e.what() << std::endl; }

	Entity sword = -1;
	try {
		ygl::addModels(scene, "./res/models/elemental_cleaver_v2_gltf/scene.gltf", [&](Entity e) {
			Transformation &t = scene.getComponent<Transformation>(e);
			t.updateWorldMatrix();
			sword = e;
		});
	} catch (std::exception &e) { std::cerr << e.what() << std::endl; }

	float characterScale = 3;

	std::vector<Entity> character;
	Transformation		characterTransform;
	characterTransform.scale *= characterScale;
	characterTransform.position.x = -9;
	characterTransform.updateWorldMatrix();

	bool cameraToggle = false;

	try {
		ygl::addModels(scene, "./res/models/medieval_knight/scene.gltf", [&](Entity e) {
			Transformation &t = scene.getComponent<Transformation>(e);
			t				  = characterTransform;
			t.updateWorldMatrix();
			scene.getComponent<RendererComponent>(e).isAnimated = true;
			model												= e;
			character.push_back(e);
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
	// addSkybox(scene, "res/images/kloppenheim_06_puresky_4k", ".hdr");
	//  addSkybox(scene, "res/images/blue_photo_studio_4k", ".hdr");
	//  Entity skybox = addSkybox(scene, "./res/images/skybox/");

	Light &sun = renderer->addLight(Light(Transformation(glm::vec3(0, 30, 0), glm::vec3(-1.2, 0.8, 0), glm::vec3(1.)),
										  glm::vec3(1.), 4, Light::DIRECTIONAL));

	renderer->loadData();
	// scene is initialized

	glm::vec3 clearColor(0.07f, 0.13f, 0.17f);
	clearColor *= 1.5;
	clearColor = glm::pow(clearColor, glm::vec3(2.4));

	Keyboard::addKeyCallback([&](GLFWwindow *, int key, int, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL) { asman->reloadShaders(); }
		if (key == GLFW_KEY_UP && action == GLFW_PRESS) { fsm.transition(2); }
		if (key == GLFW_KEY_UP && action == GLFW_RELEASE) { fsm.transition(0); }
		if (cameraToggle) {
			if (key == GLFW_KEY_W && action == GLFW_PRESS) { fsm.transition(2); }
			if (key == GLFW_KEY_W && action == GLFW_RELEASE) { fsm.transition(0); }
		}

		if (key == GLFW_KEY_H && action == GLFW_RELEASE) { fsm.transition(0); }
		if (key == GLFW_KEY_J && action == GLFW_RELEASE) { fsm.transition(1); }
		if (key == GLFW_KEY_K && action == GLFW_RELEASE) { fsm.transition(2); }
		if (key == GLFW_KEY_L && action == GLFW_RELEASE) { fsm.transition(3); }

		if (key == GLFW_KEY_O && action == GLFW_RELEASE) { cameraToggle = !cameraToggle; }
	});

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;

	glm::vec3 camera_rotation = glm::vec3(0);
	float	  distance		  = 13.;

	Transformation	   handOffset(glm::vec3(-1.04, 1.70, 0.32), glm::vec3(1.22, -0.45, 0.22), glm::vec3(0.5));
	std::vector<float> history;

	renderer->setClearColor(glm::vec4(clearColor, 1.));
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();
		if (!cameraToggle) {
			controller.update();
		} else {
			camera_rotation.y -= mouse.getDelta().x * window.deltaTime * 0.3;
			camera_rotation.x -= mouse.getDelta().y * window.deltaTime * 0.3;

			glm::mat4 rotationMat  = glm::mat4(1.);
			rotationMat			   = glm::rotate(rotationMat, camera_rotation.z, glm::vec3(0, 0, 1));
			rotationMat			   = glm::rotate(rotationMat, camera_rotation.y, glm::vec3(0, 1, 0));
			rotationMat			   = glm::rotate(rotationMat, camera_rotation.x, glm::vec3(1, 0, 0));
			glm::vec3 forward	   = (rotationMat * glm::vec4(0, 0, 1, 0)).xyz();
			cam.transform.rotation = camera_rotation;
			cam.transform.position = characterTransform.position + forward * distance + glm::vec3(0, 7, 0);
			cam.transform.updateWorldMatrix();
		}
		cam.update();

		fsm.update(window.deltaTime);

		for (Entity e : grassSystem->entities) {
			Transformation			 &transform = scene.getComponent<Transformation>(e);
			GrassSystem::GrassHolder &holder	= scene.getComponent<GrassSystem::GrassHolder>(e);
			float					  distance	= glm::length(transform.position - characterTransform.position);
			holder.LOD							= distance / 80;
		}

		if (Keyboard::getKey(GLFW_KEY_UP) == GLFW_PRESS) {
			glm::vec4 forward = characterTransform.getWorldMatrix() * glm::vec4(0, 0, 1, 0);
			characterTransform.position +=
				forward.xyz() * (float)window.deltaTime * 2.f * characterScale * (1.f - fsm.getBlendFactor());
		}

		if (Keyboard::getKey(GLFW_KEY_LEFT) == GLFW_PRESS) { characterTransform.rotation.y += window.deltaTime * 5; }
		if (Keyboard::getKey(GLFW_KEY_RIGHT) == GLFW_PRESS) { characterTransform.rotation.y -= window.deltaTime * 5; }

		if (cameraToggle) {
			if (Keyboard::getKey(GLFW_KEY_W) == GLFW_PRESS) {
				glm::vec4 forward = characterTransform.getWorldMatrix() * glm::vec4(0, 0, 1, 0);
				characterTransform.position +=
					forward.xyz() * (float)window.deltaTime * 2.f * characterScale * (1.f - fsm.getBlendFactor());
			}
			if (Keyboard::getKey(GLFW_KEY_A) == GLFW_PRESS) { characterTransform.rotation.y += window.deltaTime * 5; }
			if (Keyboard::getKey(GLFW_KEY_D) == GLFW_PRESS) { characterTransform.rotation.y -= window.deltaTime * 5; }
		}

		if (Keyboard::getKey(GLFW_KEY_U) == GLFW_PRESS) { distance += window.deltaTime * 3; }
		if (Keyboard::getKey(GLFW_KEY_Y) == GLFW_PRESS) { distance -= window.deltaTime * 3; }

		characterTransform.updateWorldMatrix();

		glm::mat4 handPosition =
			animator.GetFinalBoneMatrices()[meshToAnimate->getBoneInfoMap().find("mixamorig_RightHand")->second.id];
		Transformation &swordTransform = scene.getComponent<Transformation>(sword);
		swordTransform.getWorldMatrix() =
			characterTransform.getWorldMatrix() * handPosition * handOffset.getWorldMatrix();

		for (Entity e : character) {
			Transformation &transform = scene.getComponent<Transformation>(e);
			transform				  = characterTransform;
			transform.updateWorldMatrix();
		}

		glm::vec3 sun_position = characterTransform.position + 50.f * (sun.transform * glm::vec4(0, 0, 1, 0)).xyz();
		sun.transform[3].x	   = sun_position.x;
		sun.transform[3].y	   = sun_position.y;
		sun.transform[3].z	   = sun_position.z;
		renderer->loadData();

		grassSystem->doWork();
		renderer->doWork();

		// Transformation &transform = scene.getComponent<Transformation>(model == (uint)-1 ? 0 : model);
		Transformation transform = renderer->getLight(0).transform;
		guizmo.update(&transform);
		renderer->getLight(0).transform = transform.getWorldMatrix();
		renderer->loadData();

		renderer->drawMaterialEditor();

		ImGui::Begin("Grass controls", nullptr, window_flags);
		if (ImGui::SliderFloat("density", &(grassSystem->density), 1., 10.)) { grassSystem->reload(); }
		if (ImGui::SliderFloat("curvature", &grassSystem->curvature, 0, 1)) {}
		if (ImGui::SliderFloat("facingOffset", &grassSystem->facingOffset, 0, 2)) {}
		if (ImGui::SliderFloat("height", &grassSystem->height, 0, 2)) {}
		if (ImGui::SliderFloat("width", &grassSystem->width, 0, 1)) {}
		ImGui::Text("Blade count: %d", grassSystem->bladeCount);

		if (ImGui::SliderFloat3("ground rotation", (float *)&transform.rotation, -M_PI, M_PI)) {
			transform.updateWorldMatrix();
		}
		if (ImGui::SliderFloat3("ground position", (float *)&transform.position, -20, 20)) {
			transform.updateWorldMatrix();
		}

		ImGui::InputInt("Selection", (int *)&model);

		ImGui::End();

		renderer->drawGUI();

		ImGui::Begin("Profiler", nullptr, window_flags);
		float time = window.deltaTime;
		history.push_back(time);
		if (history.size() > 150) history.erase(history.begin());
		ImGui::PlotHistogram("frame time", history.data(), history.size(), 0, NULL, FLT_MAX, FLT_MAX, ImVec2(0, 80));
		ImGui::Text("FPS: %ld", window.fps);

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
