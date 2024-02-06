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

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <ImGuizmo.h>

using namespace ygl;
using namespace std;

void run() {
	Window window = Window(1200, 800, "Grass Test", true);

	VFShader		 *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	PerspectiveCamera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse			mouse(window);
	FPController	controller(&window, &mouse, cam.transform);
	TransformGuizmo guizmo(&window, &cam, (Transformation *)nullptr);

	Scene scene;
	scene.registerComponent<Transformation>();

	Renderer	 *renderer	  = scene.registerSystem<Renderer>(&window);
	GrassSystem	 *grassSystem = scene.registerSystem<GrassSystem>();
	AssetManager *asman		  = scene.getSystem<AssetManager>();

	uint shaderIndex = asman->addShader(shader, "defaultShader");
	renderer->setDefaultShader(shaderIndex);

	glm::vec2 size			 = glm::vec2(30);
	Mesh	 *planeMesh		 = new PlaneMesh(size, glm::vec2(1, 1));
	uint	  planeMeshIndex = asman->addMesh(planeMesh, "plane");
	uint	  groundMaterial = renderer->addMaterial(
		 Material(glm::vec3(0, 0.1, 0), 0.02, glm::vec3(), 1.4, glm::vec3(0), 0, glm::vec3(0), 0, 0.8, 0));

	for (int i = 0; i < (int)7; ++i) {
		for (int j = 0; j < (int)7; ++j) {
			Entity plane = scene.createEntity();
			scene.addComponent(plane, Transformation(glm::vec3((i - 3) * size.x, 0, (j - 3) * size.y)));
			scene.addComponent(plane, RendererComponent(-1, planeMeshIndex, groundMaterial));
			scene.addComponent(plane, GrassSystem::GrassHolder(size, 3., 1));
		}
	}

	Entity model = -1;
	try {
		model			  = ygl::addModel(scene, "./res/models/gnome/scene.gltf", 0);
		Transformation &t = scene.getComponent<Transformation>(model);
		t.scale *= 0.01;
		t.position.y = 2;
		t.updateWorldMatrix();
	} catch (std::exception &e) { std::cerr << e.what() << std::endl; }

	addSkybox(scene, "res/images/meadow_4k", ".hdr");
	// Entity skybox = addSkybox(scene, "./res/images/skybox/");
	
	renderer->loadData();
	// scene is initialized

	glm::vec3 clearColor(0.07f, 0.13f, 0.17f);
	clearColor *= 1.5;
	clearColor = glm::pow(clearColor, glm::vec3(2.4));

	renderer->setClearColor(glm::vec4(clearColor, 1.));
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();
		controller.update();
		cam.update();

		for(Entity e : grassSystem->entities) {
			Transformation &transform = scene.getComponent<Transformation>(e); 
			GrassSystem::GrassHolder &holder = scene.getComponent<GrassSystem::GrassHolder>(e);
			float distance = glm::length(transform.position - cam.transform.position);
			holder.LOD = distance / 60;
		}

		grassSystem->doWork();
		renderer->doWork();

		Transformation &transform = scene.getComponent<Transformation>(model == (uint)-1 ? 0 : model);

		guizmo.update(&transform);

		if (grassSystem->getMaterial().drawImGui()) renderer->loadData();

		ImGui::Begin("Grass controls");
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
		ImGui::End();

		window.swapBuffers();
	}

	asman->printTextures();
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
