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

const int grassCountX = 60;
const int grassCountY = 60;
const int grassCount  = grassCountX * grassCountY;

struct BladeData {
	glm::vec3 position;
	float	  windStrength;
	glm::vec2 facing;
	glm::vec2 size;
	uint	  hash;

   private:
	char padding[12];
};

int main(int argc, char *argv[]) {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	srand(time(NULL));

	Window window = Window(1200, 800, "Test Window", true);

	VFShader *shader	  = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	VFShader *grassShader = new VFShader("./shaders/grass/grass.vs", "./shaders/grass/grass.fs");
	Camera	  cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();
	scene.registerComponent<RendererComponent>();

	// Texture2d *tex = new Texture2d("./res/models/bunny_uv/bunny_uv.jpg");
	Texture2d tex		= Texture2d(1, 1);
	Texture2d uvChecker = Texture2d("./res/images/uv_checker.png");

	uvChecker.bind(GL_TEXTURE0);
	tex.bind(GL_TEXTURE1);	   // something has to be bound, otherwise the shaders throw a warning
	tex.bind(GL_TEXTURE2);
	tex.bind(GL_TEXTURE3);
	tex.bind(GL_TEXTURE4);

	Renderer *renderer = scene.registerSystem<Renderer>();
	scene.setSystemSignature<Renderer, Transformation, RendererComponent>();
	renderer->addShader(shader);
	renderer->setDefaultShader(0);

	Mesh *grassBlade = (Mesh *)getModel(loadScene("./res/models/grass_blade.obj"));
	Mesh *planeMesh	 = makePlane(glm::vec2(20, 20), glm::vec2(1, 1));

	Entity plane = scene.createEntity();
	scene.addComponent(plane, Transformation());
	scene.addComponent(plane, RendererComponent(-1, renderer->addMesh(planeMesh),
												renderer->addMaterial(Material(glm::vec3(0.1, 0.5, 0.1), .2, glm::vec3(0.),
																			   0.99, glm::vec3(0.1), 0.0, glm::vec3(1.),
																			   0.0, 0.1, 0.0, false, 0., 0.0, 0.0))));

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(0.5, -0.5, 0), glm::vec3(1)), glm::vec3(1., 1., 1.),
							 3, Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.1, Light::Type::AMBIENT));

	GLuint grassMatIndex =
		renderer->addMaterial(Material(glm::vec3(0., 1., 0.), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0,
									   glm::vec3(1.), 0.0, 0.1, 0.0, false, 0., 0.0, 0.0));
	renderer->loadData();
	// scene is initialized

	// compute shaders for blade data
	ComputeShader grassCompute("./shaders/grass/grassCompute.comp");

	GLuint grassData = -1;
	glGenBuffers(1, &grassData);
	glBindBuffer(GL_ARRAY_BUFFER, grassData);
	glBufferData(GL_ARRAY_BUFFER, grassCount * sizeof(BladeData), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	Shader::setSSBO(grassData, 1);

	grassCompute.bind();
	grassCompute.setUniform("resolution", glm::vec2(grassCountX, grassCountY));
	grassCompute.setUniform("size", glm::vec2(20, 20));
	Renderer::compute(&grassCompute, grassCountX, grassCountY, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindVertexArray(grassBlade->getVAO());
	grassBlade->addVBO(5, 4, grassData, grassCount, 1, sizeof(BladeData), 0);
	grassBlade->addVBO(6, 4, grassData, grassCount, 1, sizeof(BladeData), (const void *)(4 * sizeof(float)));
	grassBlade->addVBO(7, 1, grassData, grassCount, 1, sizeof(BladeData), (const void *)(8 * sizeof(float)));
	// glBindVertexArray(0);

	glClearColor(0.07f, 0.13f, 0.17f, 1.0);
	float time = 0;
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update(window.deltaTime);
		cam.update();

		renderer->doWork();

		grassShader->bind();
		time += window.deltaTime;
		if (grassShader->hasUniform("time")) grassShader->setUniform("time", time);
		// cout << time << endl;
		grassBlade->bind();
		{
			grassShader->setUniform("worldMatrix", glm::mat4(1.0));
			if (grassShader->hasUniform("material_index")) grassShader->setUniform("material_index", grassMatIndex);

			glDrawElementsInstanced(grassBlade->getDrawMode(), grassBlade->getIndicesCount(), GL_UNSIGNED_INT, 0,
									grassCount);
		}
		grassBlade->unbind();
		grassShader->unbind();

		window.swapBuffers();
	}

	// clean up and exit
	window.~Window();
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}