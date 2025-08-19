#include <yoghurtgl.h>
#include <cstdint>
#include <asset_manager.h>
#include <buffer.h>
#include <bvh.h>
#include <ecs.h>
#include <glm/ext/vector_int2.hpp>
#include <imgui.h>
#include <input.h>
#include <mesh.h>
#include <renderer.h>
#include <shader.h>
#include <texture.h>
#include <window.h>

using namespace ygl;

class FluidSimulation {
   public:
	bvh::BBox bounds	 = bvh::BBox(glm::vec3(-5), glm::vec3(5));
	Texture3d volume	 = Texture3d(glm::ivec3(bounds.size()), TextureType::RGBA32F, nullptr);
	float	  resolution = 1;

	struct alignas(16) ParticleData {
		glm::vec3 position = glm::vec3(0);
		float	  padding  = 0;
		glm::vec3 velocity = glm::vec3(0);
		float	  padding2 = 0;
	};

	uint	  particleMeshIndex	  = -1;
	uint	  materialIndex		  = -1;
	uint	  particleShaderIndex = -1;
	uint	  numParticles		  = 1000;
	Texture3d volumeData		  = Texture3d(glm::ivec3(bounds.size() / resolution), TextureType::RGBA32F, nullptr);
	using ParticleMesh			  = InstancedMesh<ParticleData>;
	SphereMesh particle			  = SphereMesh(0.1f, 5, 5);
	float	   particleSize		  = 1.0f;
	Scene	  &scene;

	uint		  updateParticlesShaderIndex;
	AssetManager *asman;
	Renderer	 *renderer;

	FluidSimulation(Scene &scene) : scene(scene) {
		InstancedMesh<ParticleData> *particleMesh = new InstancedMesh<ParticleData>();
		particleMesh->instanceData =
			MutableBuffer(GL_ARRAY_BUFFER, numParticles * sizeof(ParticleData), GL_DYNAMIC_DRAW);
		std::vector<ParticleData> particleData;
		for (int x = 0; x < 10; ++x) {
			for (int y = 0; y < 10; ++y) {
				for (int z = 0; z < 10; ++z) {
					ParticleData p;
					p.position = glm::mix(bounds.min, bounds.max, glm::vec3(x, y, z) / bounds.size());
					p.velocity = glm::vec3(rand() % 100 / 100.f - 0.5f, rand() % 100 / 100.f - 0.5f,
										   rand() % 100 / 100.f - 0.5f);
					particleData.push_back(p);
				}
			}
		}

		particleMesh->instanceData.set(particleData.data(), particleData.size() * sizeof(ParticleData));
		particleMesh->init(particle);

		renderer			= scene.getSystem<Renderer>();
		asman				= scene.getSystem<AssetManager>();
		particleMeshIndex	= asman->addMesh(particleMesh, "particleMesh", false);
		materialIndex		= renderer->addMaterial(Material(glm::vec4(.8)));
		particleShaderIndex = asman->addShader(new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/instancedSimple.vs",
																 YGL_RELATIVE_PATH "./shaders/simple.fs"),
											   "particleShader");

		updateParticlesShaderIndex = asman->addShader(
			new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/updateParticles.comp"), "updateParticles");
		ComputeShader *updateParticlesShader =
			(ComputeShader *)scene.getSystem<AssetManager>()->getShader(updateParticlesShaderIndex);
		updateParticlesShader->setSSBO(particleMesh->instanceData.getID(), 1);

		renderer->addDrawFunction([this]() {
			AssetManager  *asman  = this->scene.getSystem<AssetManager>();
			ygl::VFShader *shader = (ygl::VFShader *)asman->getShader(particleShaderIndex);
			shader->bind();

			ParticleMesh *mesh = (ParticleMesh *)asman->getMesh(particleMeshIndex);

			if (shader->hasUniform("worldMatrix")) shader->setUniform("worldMatrix", glm::mat4(1));
			if (shader->hasUniform("renderMode")) shader->setUniform("renderMode", 0u);
			if (shader->hasUniform("material_index")) shader->setUniform("material_index", materialIndex);
			if (shader->hasUniform("drawMode")) shader->setUniform("drawMode", mesh->getDrawMode());
			if (shader->hasUniform("numParticles")) shader->setUniform("numParticles", (int)numParticles);
			if (shader->hasUniform("particleSize")) shader->setUniform("particleSize", (int)particleSize);

			mesh->bind();

			mesh->draw();

			mesh->unbind();
			shader->unbind();
		});
	};

	void update() {
		ComputeShader *updateParticlesShader =
			(ComputeShader *)scene.getSystem<AssetManager>()->getShader(updateParticlesShaderIndex);
		updateParticlesShader->bind();
		glm::ivec2 resolution = glm::ivec2((numParticles + 31) / 32, 32);
		updateParticlesShader->setUniform("deltaTime", 0.016f);
		updateParticlesShader->setUniform("resolution", resolution);
		if (updateParticlesShader->hasUniform("b_min")) updateParticlesShader->setUniform("b_min", bounds.min);
		if (updateParticlesShader->hasUniform("b_max")) updateParticlesShader->setUniform("b_max", bounds.max);
		Renderer::compute(updateParticlesShader, resolution.x, resolution.y, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
};

void createBounds(ygl::Scene &scene, const glm::vec3 &box) {
	Entity		  bounds   = scene.createEntity();
	AssetManager *asman	   = scene.getSystem<AssetManager>();
	Renderer	 *renderer = scene.getSystem<Renderer>();
	scene.addComponent(bounds, Transformation());
	scene.addComponent(bounds,
					   RendererComponent(asman->addShader(new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/unlit.vs",
																			YGL_RELATIVE_PATH "./shaders/unlit.fs"),
														  "boundsShader"),
										 asman->addMesh(new LineBoxMesh(box), "boundsMesh"),
										 renderer->addMaterial(Material(glm::vec3(1)))));
}

auto nextPowerOf2(uint32_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

void bitonicMergeSort(Buffer &vec) {
	static ComputeShader bitonicMergeShader(YGL_RELATIVE_PATH "./shaders/fluid/bitonicMerge.comp");

	bitonicMergeShader.bind();
	bitonicMergeShader.setSSBO(vec.getID(), 0);
	uint N = vec.getSize() / sizeof(float);

	std::vector<float> data(N);
	dbLog(ygl::LOG_DEBUG, "Bitonic merge sort N: ", N);
	if (bitonicMergeShader.hasUniform("N")) bitonicMergeShader.setUniform("N", N);

	uint_fast32_t numPairs = nextPowerOf2(N) / 2;
	dbLog(ygl::LOG_DEBUG, "Bitonic merge sort numPairs: ", numPairs);
	uint_fast32_t numStages = uint_fast32_t(log2(numPairs * 2));
	for (uint_fast32_t stageIndex = 0; stageIndex < numStages; ++stageIndex) {
		uint subGroupSize = 1 << (stageIndex);
		if (bitonicMergeShader.hasUniform("subGroupSize")) bitonicMergeShader.setUniform("subGroupSize", subGroupSize);

		for (uint_fast32_t stepIndex = 0; stepIndex < stageIndex + 1; ++stepIndex) {
			uint stepSize = 1 << (stageIndex - stepIndex);
			if (bitonicMergeShader.hasUniform("stepSize")) bitonicMergeShader.setUniform("stepSize", stepSize);

			//dbLog(ygl::LOG_DEBUG, "Sorting stage: ", stageIndex, " step: ", stepIndex, " subGroupSize: ", subGroupSize,
			//	  " stepSize: ", stepSize);

			Renderer::compute(&bitonicMergeShader, numPairs, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
	}

	bitonicMergeShader.unbind();
}

int main() {
	if (ygl::init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	Window window = Window(800, 600, "Fluid Simulation", true, true);

	// window.setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 0.0f));

	PerspectiveCamera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponent<Transformation>();

	Renderer *renderer = scene.registerSystem<Renderer>(&window);
	renderer->setMainCamera(&cam);
	addEffects(renderer);
	renderer->addLight(Light(Transformation(glm::vec3(), glm::vec3(1, -2.9, 0), glm::vec3(1)), glm::vec3(1.), .8,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.01, Light::Type::AMBIENT));

	FluidSimulation sim(scene);

	unsigned int N = 1024 * 1024 * 16;
	srand(time(0));
	MutableBuffer	   b(GL_SHADER_STORAGE_BUFFER, N * sizeof(float), GL_DYNAMIC_DRAW);
	std::vector<float> data;
	{
		for (unsigned int i = 0; i < N; ++i) {
			data.push_back(rand());
		}
		b.set(data.data(), data.size() * sizeof(float));
		dbLog(ygl::LOG_DEBUG, "Buffer size: %d", b.getSize());
	}

	//b.get(data.data(), data.size() * sizeof(float));
	//for (unsigned int i = 0; i < N; ++i) {
	//	std::cout << data[i] << " ";
	//}
	//std::cout << std::endl;
	dbLog(ygl::LOG_DEBUG, "Sorting...       -----------------");
	auto start = std::chrono::high_resolution_clock::now();
	bitonicMergeSort(b);
	//std::sort(data.begin(), data.end());
	glFinish();
	auto end = std::chrono::high_resolution_clock::now();
	dbLog(ygl::LOG_DEBUG, "Sorting took: ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(),
		   "ms");


	//b.get(data.data(), data.size() * sizeof(float));
	//for (unsigned int i = 0; i < N; ++i) {
	//	std::cout << data[i] << " ";
	//}
	std::cout << std::endl;

	createBounds(scene, sim.bounds.size());

	renderer->loadData();

	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();
		controller.update();
		cam.update();
		sim.update();

		scene.doWork();

		window.swapBuffers();
	}
}
