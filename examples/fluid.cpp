#include <yoghurtgl.h>
#include <cstdint>
#include <asset_manager.h>
#include <buffer.h>
#include <bvh.h>
#include <ecs.h>
#include <glm/ext/vector_int2.hpp>
#include "GLFW/glfw3.h"
#include <entities.h>
#include <imgui.h>
#include <input.h>
#include <mesh.h>
#include <renderer.h>
#include <shader.h>
#include <texture.h>
#include <window.h>

using namespace ygl;

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

template <typename T>
void bitonicMergeSort(Buffer &vec, uint_fast8_t keyOffset = 0) {
	static ComputeShader bitonicMergeShader(YGL_RELATIVE_PATH "./shaders/fluid/bitonicMerge.comp");
	static MutableBuffer stepsData(GL_UNIFORM_BUFFER, 10, GL_DYNAMIC_DRAW);

	bitonicMergeShader.bind();
	bitonicMergeShader.setSSBO(vec.getID(), 0);
	uint N = vec.getSize() / sizeof(T);

	// dbLog(ygl::LOG_DEBUG, "Bitonic merge sort N: ", N);
	if (bitonicMergeShader.hasUniform("N")) bitonicMergeShader.setUniform("N", N);
	assert(sizeof(T) % sizeof(uint) == 0 && "Data type size must be a multiple of uint size for bitonic sort");
	if (bitonicMergeShader.hasUniform("elementSize"))
		bitonicMergeShader.setUniform("elementSize", uint(sizeof(T) / sizeof(uint)));
	if (bitonicMergeShader.hasUniform("keyOffset")) bitonicMergeShader.setUniform("keyOffset", uint(keyOffset));

	uint_fast32_t numPairs = nextPowerOf2(N) / 2;
	// dbLog(ygl::LOG_DEBUG, "Bitonic merge sort numPairs: ", numPairs);
	uint_fast32_t numStages = uint_fast32_t(log2(numPairs * 2));

	stepsData.resize(numStages * (numStages + 1) * 2 * sizeof(uint));

	{
		static std::vector<uint> steps;
		steps.clear();
		for (uint_fast32_t stageIndex = 0; stageIndex < numStages; ++stageIndex) {
			uint subGroupSize = 1 << (stageIndex);
			for (uint_fast32_t stepIndex = 0; stepIndex < stageIndex + 1; ++stepIndex) {
				uint stepSize = 1 << (stageIndex - stepIndex);
				steps.push_back(subGroupSize);
				steps.push_back(stepSize);
			}
		}
		stepsData.set(steps.data(), steps.size() * sizeof(uint));
	}

	bitonicMergeShader.setUBO(stepsData.getID(), bitonicMergeShader.getUBOBinding("StepData"));

	uint I = 0;
	for (uint_fast32_t stageIndex = 0; stageIndex < numStages; ++stageIndex) {
		for (uint_fast32_t stepIndex = 0; stepIndex < stageIndex + 1; ++stepIndex) {
			// if(bitonicMergeShader.hasUniform("I"))
			bitonicMergeShader.setUniform("I", I++);

			Renderer::compute(&bitonicMergeShader, numPairs, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
	}

	bitonicMergeShader.unbind();
}

class FluidSimulation {
   public:
	bvh::BBox bounds	 = bvh::BBox(glm::vec3(-10), glm::vec3(10));
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
	SphereMesh particle			  = SphereMesh(0.5f, 5, 5);
	float	   particleSize		  = 1.0f;
	Scene	  &scene;

	uint updateParticlesShaderIndex;
	uint hashParticlesShaderIndex;
	uint genSpatialLookupShaderIndex;

	uint		  updateVelocityShaderIndex;
	MutableBuffer spatialHashBuffer;
	MutableBuffer spatialLookupBuffer;

	AssetManager *asman;
	Renderer	 *renderer;

	void reset() {
		std::vector<ParticleData> particleData;
		for (int x = 0; x < 10; ++x) {
			for (int y = 0; y < 10; ++y) {
				for (int z = 0; z < 10; ++z) {
					ParticleData p;
					p.position = glm::mix(glm::vec3(-5), glm::vec3(5), glm::vec3(x, y, z) / glm::vec3(10)) +
								 rand() % 100 / 100000.f * glm::vec3(1, 1, 1);
					particleData.push_back(p);
				}
			}
		}
		auto *particleMesh = asman->getMesh<InstancedMesh<ParticleData>>(particleMeshIndex);
		particleMesh->instanceData.set(particleData.data(), particleData.size() * sizeof(ParticleData));
	}

	FluidSimulation(Scene &scene)
		: scene(scene),
		  spatialHashBuffer(GL_SHADER_STORAGE_BUFFER, 1000 * 2 * sizeof(unsigned int), GL_DYNAMIC_DRAW),
		  spatialLookupBuffer(GL_SHADER_STORAGE_BUFFER, 1000 * sizeof(unsigned int), GL_DYNAMIC_DRAW) {
		InstancedMesh<ParticleData> *particleMesh = new InstancedMesh<ParticleData>();
		particleMesh->instanceData =
			MutableBuffer(GL_ARRAY_BUFFER, numParticles * sizeof(ParticleData), GL_DYNAMIC_DRAW);

		renderer			= scene.getSystem<Renderer>();
		asman				= scene.getSystem<AssetManager>();
		particleMeshIndex	= asman->addMesh(particleMesh, "particleMesh", false);
		materialIndex		= renderer->addMaterial(Material(glm::vec4(.8)));
		particleShaderIndex = asman->addShader(new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/fluid/particles.vs",
																 YGL_RELATIVE_PATH "./shaders/fluid/particles.fs"),
											   "particleShader");
		reset();
		particleMesh->init(particle);

		ComputeShader *updateParticlesShader =
			new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/updateParticles.comp");
		updateParticlesShaderIndex = asman->addShader(updateParticlesShader, "updateParticles");

		ComputeShader *hashParticlesShader = new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/hashParticles.comp");
		hashParticlesShaderIndex		   = asman->addShader(hashParticlesShader, "hashParticles");

		ComputeShader *genSpatialLookupShader =
			new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/genSpatialLookup.comp");
		genSpatialLookupShaderIndex = asman->addShader(genSpatialLookupShader, "genSpatialLookup");

		ComputeShader *updateVelocityShader = new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/PGTransfer.comp");
		updateVelocityShaderIndex			= asman->addShader(updateVelocityShader, "updateVelocity");

		Shader::setSSBO(particleMesh->instanceData.getID(), 1);
		Shader::setSSBO(spatialHashBuffer.getID(), 2);
		Shader::setSSBO(spatialLookupBuffer.getID(), 3);

		renderer->addDrawFunction([this]() {
			AssetManager  *asman  = this->scene.getSystem<AssetManager>();
			ygl::VFShader *shader = (ygl::VFShader *)asman->getShader(particleShaderIndex);
			shader->bind();

			ParticleMesh *mesh = (ParticleMesh *)asman->getMesh(particleMeshIndex);

			renderer->bindTexturesForMaterial(materialIndex, shader);

			shader->setUniformCond("worldMatrix", glm::mat4(1));
			shader->setUniformCond("material_index", materialIndex);
			shader->setUniformCond("numParticles", (int)numParticles);
			shader->setUniformCond("particleSize", (int)particleSize);

			mesh->bind();

			if(renderer->renderMode == 6) {glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);}

			mesh->draw();

			mesh->unbind();
			shader->unbind();
		});
	};

	void update() {
		{
			ComputeShader *updateParticlesShader = asman->getShader<ComputeShader>(updateParticlesShaderIndex);
			updateParticlesShader->bind();
			glm::ivec2 resolution = glm::ivec2((numParticles + 31) / 32, 32);
			updateParticlesShader->setUniform("deltaTime", 0.016f);
			updateParticlesShader->setUniform("resolution", resolution);
			updateParticlesShader->setUniformCond("b_min", bounds.min);
			updateParticlesShader->setUniformCond("b_max", bounds.max);
			Renderer::compute(updateParticlesShader, resolution.x, resolution.y, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		{
			ComputeShader *hashParticlesShader = asman->getShader<ComputeShader>(hashParticlesShaderIndex);
			hashParticlesShader->bind();
			hashParticlesShader->setUniformCond("N", numParticles);
			hashParticlesShader->setUniformCond("cellSize", 1.f);

			Renderer::compute(hashParticlesShader, numParticles, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			bitonicMergeSort<std::pair<uint, uint>>(spatialHashBuffer, 0);

			{
				Bind b(spatialLookupBuffer);
				uint elem = -1;
				glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &elem);
			}

			ComputeShader *genSpatialLookupShader = asman->getShader<ComputeShader>(genSpatialLookupShaderIndex);
			genSpatialLookupShader->bind();
			genSpatialLookupShader->setUniformCond("N", numParticles);
			Renderer::compute(genSpatialLookupShader, (numParticles + 127) / 128, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		{
			ComputeShader *updateVelocityShader = asman->getShader<ComputeShader>(updateVelocityShaderIndex);
			updateVelocityShader->bind();
			updateVelocityShader->setUniformCond("N", numParticles);
			updateVelocityShader->setUniformCond("cellSize", 1.f);

			Renderer::compute(updateVelocityShader, numParticles, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
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

	//addSphere(scene, glm::vec3(0, 0, 10), glm::vec3(1), glm::vec3(1, 0, 0));
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
	renderer->addLight(Light(Transformation(glm::vec3(), glm::vec3(1, -2.9, 0), glm::vec3(1)), glm::vec3(1.), .8,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.01, Light::Type::AMBIENT));

	
	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	uint shaderInd = scene.getSystem<AssetManager>()->addShader(shader, "defaultShader");
	renderer->setDefaultShader(shaderInd);
	renderer->setMainCamera(&cam);
	addEffects(renderer);

	FluidSimulation sim(scene);
	createBounds(scene, sim.bounds.size());

	//addSkybox(scene, YGL_RELATIVE_PATH "./res/images/meadow_4k", ".hdr");

	renderer->loadData();

	Keyboard::addKeyCallback([&](GLFWwindow *, int key, int, int action, int mods) {
		if (key == GLFW_KEY_L && action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL) { sim.reset(); }
	});

	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();
		controller.update();
		cam.update();

		renderer->drawGUI();

		sim.update();

		scene.doWork();

		window.swapBuffers();
	}
}
