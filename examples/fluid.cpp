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
	bvh::BBox  bounds	  = bvh::BBox(glm::vec3(-10), glm::vec3(10));
	float	   cellSize	  = 0.5f;
	glm::ivec3 resolution = glm::ivec3(bounds.size() / cellSize);

	struct alignas(16) ParticleData {
		glm::vec3 position = glm::vec3(0);
		float	  padding  = 0;
		glm::vec3 velocity = glm::vec3(0);
		float	  padding2 = 0;
	};

	uint	  particleMeshIndex	  = -1;
	uint	  gridMeshIndex		  = -1;
	uint	  materialIndex		  = -1;
	uint	  particleShaderIndex = -1;
	uint	  gridShaderIndex	  = -1;
	uint	  numParticles		  = 16000;
	Texture3d volumeData0		  = Texture3d(glm::ivec3(bounds.size() / cellSize), TextureType::RGBA32F, nullptr);
	Texture3d volumeData1		  = Texture3d(glm::ivec3(bounds.size() / cellSize), TextureType::RGBA32F, nullptr);

	Texture3d copy = Texture3d(glm::ivec3(bounds.size() / cellSize), TextureType::RGBA32F, nullptr);

	Texture3d *volumeFront = &volumeData0;
	Texture3d *volumeBack  = &volumeData1;

	using ParticleMesh		= InstancedMesh<ParticleData>;
	SphereMesh particle		= SphereMesh(0.5f, 5, 5);
	BoxMesh	   grid			= BoxMesh(1.0f);
	float	   particleSize = cellSize;
	Scene	  &scene;

	uint updateParticlesShaderIndex;
	uint hashParticlesShaderIndex;
	uint genSpatialLookupShaderIndex;
	uint solveGridShaderIndex;
	uint GPTransferShaderIndex;
	uint PGTransferShaderIndex;
	uint repelParticlesShaderIndex;

	ComputeShader copyImage3dShader = ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/copyImage3D.comp");

	bool drawGrid = false;

	MutableBuffer spatialHashBuffer;
	MutableBuffer spatialLookupBuffer;

	AssetManager *asman;
	Renderer	 *renderer;

	void reset() {
		std::vector<ParticleData> particleData;
		for (int x = 0; x < 40; ++x) {
			for (int y = 0; y < 20; ++y) {
				for (int z = 0; z < 20; ++z) {
					ParticleData p;
					p.position = glm::mix(glm::vec3(-8), glm::vec3(4), glm::vec3(x, y, z) / glm::vec3(20));
					p.position.y += 2;
					particleData.push_back(p);
				}
			}
		}

		auto *particleMesh = asman->getMesh<InstancedMesh<ParticleData>>(particleMeshIndex);
		particleMesh->instanceData.set(particleData.data(), particleData.size() * sizeof(ParticleData));
	}

	FluidSimulation(Scene &scene)
		: scene(scene),
		  spatialHashBuffer(GL_SHADER_STORAGE_BUFFER, nextPowerOf2(numParticles) * 2 * sizeof(unsigned int),
							GL_DYNAMIC_DRAW),
		  spatialLookupBuffer(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(unsigned int), GL_DYNAMIC_DRAW) {
		InstancedMesh<ParticleData> *particleMesh = new InstancedMesh<ParticleData>();
		particleMesh->instanceData =
			MutableBuffer(GL_ARRAY_BUFFER, numParticles * sizeof(ParticleData), GL_DYNAMIC_DRAW);

		InstancedMesh<int> *im = new InstancedMesh<int>();
		im->instanceData	   = MutableBuffer(GL_ARRAY_BUFFER, 128, GL_DYNAMIC_DRAW);

		renderer			= scene.getSystem<Renderer>();
		asman				= scene.getSystem<AssetManager>();
		particleMeshIndex	= asman->addMesh(particleMesh, "particleMesh", false);
		materialIndex		= renderer->addMaterial(Material(glm::vec4(.8)));
		particleShaderIndex = asman->addShader(new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/fluid/particles.vs",
																 YGL_RELATIVE_PATH "./shaders/fluid/particles.fs"),
											   "particleShader");
		reset();
		particleMesh->init(particle);

		gridMeshIndex	= asman->addMesh(im, "particleMesh", false);
		gridShaderIndex = asman->addShader(
			new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/fluid/grid.vs", YGL_RELATIVE_PATH "./shaders/fluid/grid.fs"),
			"gridShader");
		grid.setCullFace(false);
		im->init(grid);
		im->setCullFace(false);

		ComputeShader *updateParticlesShader =
			new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/updateParticles.comp");
		updateParticlesShaderIndex = asman->addShader(updateParticlesShader, "updateParticles");

		ComputeShader *hashParticlesShader = new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/hashParticles.comp");
		hashParticlesShaderIndex		   = asman->addShader(hashParticlesShader, "hashParticles");

		ComputeShader *genSpatialLookupShader =
			new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/genSpatialLookup.comp");
		genSpatialLookupShaderIndex = asman->addShader(genSpatialLookupShader, "genSpatialLookup");

		ComputeShader *PGTransferShader = new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/PGTransfer.comp");
		PGTransferShaderIndex			= asman->addShader(PGTransferShader, "updateVelocity");

		ComputeShader *solveGridShader = new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/solveGrid.comp");
		solveGridShaderIndex		   = asman->addShader(solveGridShader, "solveGrid");

		ComputeShader *GPTransferShader = new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/GPTransfer.comp");
		GPTransferShaderIndex			= asman->addShader(GPTransferShader, "GPTransfer");

		ComputeShader *repelParticlesShader = new ComputeShader(YGL_RELATIVE_PATH "./shaders/fluid/repelNearest.comp");
		repelParticlesShaderIndex			= asman->addShader(repelParticlesShader, "repelParticles");

		Shader::setSSBO(particleMesh->instanceData.getID(), 1);
		Shader::setSSBO(spatialHashBuffer.getID(), 2);
		Shader::setSSBO(spatialLookupBuffer.getID(), 3);

		dbLog(ygl::LOG_INFO, "particle data id: ", particleMesh->instanceData.getID());
		dbLog(ygl::LOG_INFO, "spatial hash id: ", spatialHashBuffer.getID());
		dbLog(ygl::LOG_INFO, "spatial lookup id: ", spatialLookupBuffer.getID());

		generateSpatialHash();

		renderer->addDrawFunction([this]() {
			AssetManager *asman = this->scene.getSystem<AssetManager>();
			{
				ygl::VFShader *shader = (ygl::VFShader *)asman->getShader(particleShaderIndex);
				shader->bind();

				ParticleMesh *mesh = (ParticleMesh *)asman->getMesh(particleMeshIndex);

				renderer->bindTexturesForMaterial(materialIndex, shader);

				shader->setUniformCond("worldMatrix", glm::mat4(1));
				shader->setUniformCond("material_index", materialIndex);
				shader->setUniformCond("numParticles", (int)numParticles);
				shader->setUniformCond("particleSize", (float)particleSize);

				mesh->bind();

				if (renderer->renderMode == 6) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

				mesh->draw();

				mesh->unbind();
				shader->unbind();
			}
			if (drawGrid) {
				ygl::VFShader *shader = (ygl::VFShader *)asman->getShader(gridShaderIndex);
				shader->bind();
				InstancedMesh<int> *mesh = (InstancedMesh<int> *)asman->getMesh(gridMeshIndex);
				renderer->bindTexturesForMaterial(materialIndex, shader);
				shader->setUniformCond("worldMatrix", glm::mat4(1));
				shader->setUniformCond("material_index", materialIndex);
				shader->setUniformCond("numParticles", (int)numParticles);
				shader->setUniformCond("particleSize", (float)particleSize);
				shader->setUniformCond("cellSize", cellSize);
				shader->setUniformCond("gridResolution", resolution);
				copy.bindImage(17);
				mesh->setCullFace(false);

				glDepthMask(GL_FALSE);

				mesh->bind();
				if (renderer->renderMode == 6) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
				mesh->draw(resolution.x * resolution.y * resolution.z);
				mesh->unbind();
				shader->unbind();

				glDepthMask(GL_TRUE);
			}
		});
	};

	void generateSpatialHash() {
		{
			Bind b(spatialHashBuffer);
			uint elem = -1;
			glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &elem);
		}

		ComputeShader *hashParticlesShader = asman->getShader<ComputeShader>(hashParticlesShaderIndex);
		hashParticlesShader->bind();
		hashParticlesShader->setUniformCond("N", numParticles);
		hashParticlesShader->setUniformCond("cellSize", cellSize);
		hashParticlesShader->setUniformCond("resolution", resolution);

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

	void update() {
		InstancedMesh<ParticleData> *particleMesh = asman->getMesh<InstancedMesh<ParticleData>>(particleMeshIndex);
		Shader::setSSBO(particleMesh->instanceData.getID(), 1);
		Shader::setSSBO(spatialHashBuffer.getID(), 2);
		Shader::setSSBO(spatialLookupBuffer.getID(), 3);

		glClearTexImage(volumeFront->getID(), 0, GL_RGBA, GL_FLOAT, nullptr);
		glClearTexImage(volumeBack->getID(), 0, GL_RGBA, GL_FLOAT, nullptr);


		{
			ComputeShader *updateParticlesShader = asman->getShader<ComputeShader>(updateParticlesShaderIndex);
			updateParticlesShader->bind();
			glm::ivec2 _resolution = glm::ivec2((numParticles + 31) / 32, 32);
			updateParticlesShader->setUniform("deltaTime", 0.016f);
			updateParticlesShader->setUniform("_resolution", _resolution);
			updateParticlesShader->setUniformCond("b_min", bounds.min);
			updateParticlesShader->setUniformCond("b_max", bounds.max);
			updateParticlesShader->setUniformCond("time", float(glfwGetTime()));
			Renderer::compute(updateParticlesShader, _resolution.x, _resolution.y, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
		
		{
			ComputeShader *repelParticlesShader = asman->getShader<ComputeShader>(repelParticlesShaderIndex);
			repelParticlesShader->bind();
			repelParticlesShader->setUniformCond("N", numParticles);
			repelParticlesShader->setUniformCond("cellSize", cellSize);
			repelParticlesShader->setUniformCond("resolution", resolution);
			repelParticlesShader->setUniformCond("b_min", bounds.min);
			repelParticlesShader->setUniformCond("b_max", bounds.max);
			for (int i = 0; i < 5; ++i) {
				Renderer::compute(repelParticlesShader, numParticles, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}
		}

		generateSpatialHash();

		{
			ComputeShader *PGTransferShader = asman->getShader<ComputeShader>(PGTransferShaderIndex);
			PGTransferShader->bind();
			PGTransferShader->setUniformCond("N", numParticles);
			PGTransferShader->setUniformCond("cellSize", cellSize);
			PGTransferShader->setUniformCond("time", float(glfwGetTime()));
			PGTransferShader->setUniformCond("resolution", resolution);
			volumeBack->bindImage(0);

			Renderer::compute(PGTransferShader, resolution.x, resolution.y, resolution.z);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}


		{
			{
				copyImage3dShader.bind();
				copyImage3dShader.setUniformCond("resolution", resolution);
				volumeBack->bindImage(0);
				copy.bindImage(1);

				Renderer::compute(&copyImage3dShader, resolution.x, resolution.y, resolution.z);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
				copyImage3dShader.unbind();
			}

			ComputeShader *solveGridShader = asman->getShader<ComputeShader>(solveGridShaderIndex);
			solveGridShader->bind();
			solveGridShader->setUniformCond("resolution", resolution);

			for (int i = 0; i < 5; ++i) {
				volumeBack->bindImage(1);
				volumeFront->bindImage(0);

				Renderer::compute(solveGridShader, resolution.x, resolution.y, resolution.z);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

				std::swap(volumeFront, volumeBack);
			}
		}

		{
			ComputeShader *GPTransferShader = asman->getShader<ComputeShader>(GPTransferShaderIndex);
			GPTransferShader->bind();
			GPTransferShader->setUniformCond("N", numParticles);
			GPTransferShader->setUniformCond("cellSize", cellSize);
			GPTransferShader->setUniformCond("resolution", resolution);
			volumeBack->bindImage(0);
			copy.bindImage(1);

			Renderer::compute(GPTransferShader, numParticles, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		std::swap(volumeFront, volumeBack);
	}

	void drawGui() {
		ImGui::Begin("Fluid Simulation");
		ImGui::Text("Num particles: %d", numParticles);
		if (ImGui::SliderFloat("Particle Size", &particleSize, 0.01f, 2.0f)) {}
		if (ImGui::Checkbox("Draw Grid", &drawGrid)) {}
		if (ImGui::Button("Reset")) { reset(); }
		ImGui::End();
	}
};

void createBounds(ygl::Scene &scene, const glm::vec3 &box) {
	Entity		  bounds   = scene.createEntity();
	AssetManager *asman	   = scene.getSystem<AssetManager>();
	Renderer	 *renderer = scene.getSystem<Renderer>();

	uint unlitShaderIndex = asman->addShader(
		new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/unlit.vs", YGL_RELATIVE_PATH "./shaders/unlit.fs"),
		"boundsShader");

	scene.addComponent(bounds, Transformation());
	scene.addComponent(bounds, RendererComponent(unlitShaderIndex, asman->addMesh(new LineBoxMesh(box), "boundsMesh"),
												 renderer->addMaterial(Material(glm::vec3(1)))));

	Entity arrows	 = scene.createEntity();
	Mesh  *arrowMesh = new CoordArrowsMesh();
	scene.addComponent(arrows, Transformation(glm::vec3(-11.f), glm::vec3(0), glm::vec3(20)));
	scene.addComponent(arrows, RendererComponent(unlitShaderIndex, asman->addMesh(arrowMesh, "arrowsMesh"),
												 renderer->addMaterial(Material(glm::vec3(1)))));

	arrowMesh->setLineWidth(5);

	// addSphere(scene, glm::vec3(0, 0, 10), glm::vec3(1), glm::vec3(1, 0, 0));
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

	VFShader *shader	= new VFShader("./shaders/simple.vs", "./shaders/simple.fs");
	uint	  shaderInd = scene.getSystem<AssetManager>()->addShader(shader, "defaultShader");
	renderer->setDefaultShader(shaderInd);
	renderer->setMainCamera(&cam);
	addEffects(renderer);

	FluidSimulation sim(scene);
	createBounds(scene, sim.bounds.size());

	addSkybox(scene, YGL_RELATIVE_PATH "./res/images/kloppenheim_06_puresky_4k", ".hdr");

	renderer->loadData();

	bool running = true;

	Keyboard::addKeyCallback([&](GLFWwindow *, int key, int, int action, int mods) {
		if (key == GLFW_KEY_L && action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL) { sim.reset(); }
		if (key == GLFW_KEY_H && action == GLFW_RELEASE) { sim.update(); }
		if (key == GLFW_KEY_P && action == GLFW_RELEASE) { running = !running; }
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) { sim.drawGrid = !sim.drawGrid; }
		if (key == GLFW_KEY_R && action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL) {
			scene.getSystem<AssetManager>()->reloadShaders();
		}
	});

	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();
		controller.update();
		cam.update();

		renderer->drawGUI();

		if (running) sim.update();
		sim.drawGui();

		scene.doWork();

		window.swapBuffers();
	}
}
