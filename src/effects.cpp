#include <yoghurtgl.h>

#include <effects.h>
#include <renderer.h>
#include <stdexcept>
#include "asset_manager.h"

uint ygl::GrassSystem::GrassBladeMesh::count = 0;

ygl::GrassSystem::GrassBladeMesh::GrassBladeMesh(glm::ivec2 resolution, int LOD) {
	this->LOD = LOD;
	this->createVAO();
	glBindVertexArray(this->getVAO());

	this->verticesCount = 15;
	GLuint indices[]	= {2, 1, 0, 2, 3,  1, 4, 3,	 2,	 4, 5,	3,	6,	5,	4,	6,	7,	5,	8, 7,
						   6, 8, 9, 7, 10, 9, 8, 10, 11, 9, 12, 11, 10, 12, 13, 11, 14, 13, 12};
	this->createIBO(indices, 13 * 3);
	
	this->vCount1 = this->verticesCount;
	this->ibo1 = this->ibo;
	this->indicesCount1 = this->indicesCount;

	this->verticesCount = 7;
	GLuint indicesLow[] = {2, 1, 0, 2, 3, 1, 4, 3, 2, 4, 5, 3, 6, 5, 4};
	this->createIBO(indicesLow, 5 * 3);

	this->vCount2 = this->verticesCount;
	this->ibo2 = this->ibo;
	this->indicesCount2 = this->indicesCount;

	glGenBuffers(1, &grassData);
	setResolution(resolution);

	this->addVBO(0, 4, grassData, 1, sizeof(BladeData), 0);
	this->addVBO(1, 4, grassData, 1, sizeof(BladeData), (const void *)(4 * sizeof(float)));
	this->addVBO(2, 1, grassData, 1, sizeof(BladeData), (const void *)(8 * sizeof(float)));
	glBindVertexArray(0);

	cullFace = false;

	this->index = count++;
}

void ygl::GrassSystem::GrassBladeMesh::setResolution(glm::ivec2 resolution) {
	this->resolution = resolution / (LOD + 1);
	this->bladeCount = this->resolution.x * this->resolution.y;
	glBindBuffer(GL_ARRAY_BUFFER, grassData);
	glBufferData(GL_ARRAY_BUFFER, bladeCount * sizeof(BladeData), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ygl::GrassSystem::GrassBladeMesh::~GrassBladeMesh() { glDeleteBuffers(1, &grassData); }

void ygl::GrassSystem::GrassBladeMesh::bind() {
	if(LOD == 0) {
		ibo = ibo1;
		verticesCount = vCount1;
		indicesCount = indicesCount1;
	} else {
		ibo = ibo2;
		verticesCount = vCount2;
		indicesCount = indicesCount2;
	}
	MultiBufferMesh::bind();
}

// TODO:
void ygl::GrassSystem::GrassHolder::serialize(std::ostream &out) { static_cast<void>(out); }

void ygl::GrassSystem::GrassHolder::deserialize(std::istream &in) { static_cast<void>(in); }

void ygl::GrassSystem::init() {
	if (!scene->hasSystem<Renderer>()) THROW_RUNTIME_ERR("Renderer system must be registered in the scene.");
	if (!scene->hasSystem<AssetManager>()) THROW_RUNTIME_ERR("Asset Manager must be registered in the scene.")

	assetManager = scene->getSystem<AssetManager>();

	grassCompute = new ComputeShader("./shaders/grass/grassCompute.comp");
	assetManager->addShader(grassCompute, "Grass Compute");
	grassShader = new VFShader("./shaders/grass/grass.vs", "./shaders/grass/grass.fs");
	assetManager->addShader(grassShader, "Grass Shader");

	materialIndex = scene->getSystem<Renderer>()->addMaterial(
		Material(glm::vec3(0.1, 0.7, 0.1), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.), 0.0, 0.8, 0.0));
	grassCompute->bind();
	grassCompute->setUniform("time", 0.f);

	scene->registerComponent<GrassHolder>();
	scene->setSystemSignature<GrassSystem, Transformation, GrassHolder>();
	renderer = scene->getSystem<Renderer>();
	this->window	   = renderer->getWindow();
	renderer->addDrawFunction([this]() -> void { render(this->renderer->getWindow()->globalTime); });
}

ygl::GrassSystem::~GrassSystem() { }

void ygl::GrassSystem::update(float time) {
	for(Entity e : entities) {
		auto worldMatrix = scene->getComponent<Transformation>(e).getWorldMatrix();
		GrassHolder &holder = scene->getComponent<GrassHolder>(e);
		reload(holder);
		GrassBladeMesh *mesh = (GrassBladeMesh*)scene->getSystem<AssetManager>()->getMesh(holder.meshIndex);

		Shader::setSSBO(mesh->grassData, 1);
		grassCompute->bind();
		grassCompute->setUniform("time", time);
		grassCompute->setUniform("resolution", mesh->resolution);
		grassCompute->setUniform("size", holder.size);
		grassCompute->setUniform("anchorWorldMatrix", worldMatrix);
		grassCompute->setUniform("LOD", (uint)holder.LOD);
		grassCompute->unbind();
		Renderer::compute(grassCompute, mesh->resolution.x, mesh->resolution.y, 1);
	}
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ygl::GrassSystem::render(float time) {
	grassShader->bind();
	if (grassShader->hasUniform("time")) grassShader->setUniform("time", time);
	Renderer *renderer = scene->getSystem<Renderer>();
	if (grassShader->hasUniform("use_skybox")) grassShader->setUniform("use_skybox", renderer->hasSkybox());

	for(Entity e : entities) {
		auto worldMatrix = scene->getComponent<Transformation>(e).getWorldMatrix();
		GrassHolder &holder = scene->getComponent<GrassHolder>(e);
		GrassBladeMesh *mesh = (GrassBladeMesh*)scene->getSystem<AssetManager>()->getMesh(holder.meshIndex);

		mesh->bind();
		{
			grassShader->setUniform("worldMatrix", worldMatrix);
			grassShader->setUniform("renderMode", renderer->renderMode);
			grassShader->setUniform("material_index", materialIndex);
			grassShader->setUniform("curvature", curvature);
			grassShader->setUniform("facingOffset", facingOffset);
			grassShader->setUniform("height", height);
			grassShader->setUniform("width", width);
			grassShader->setUniform("blade_triangles", (uint)mesh->getVerticesCount());

			glDrawElementsInstanced(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0, mesh->bladeCount);
		}

		mesh->unbind();
	}
	grassShader->unbind();
}

void ygl::GrassSystem::reload(ygl::GrassSystem::GrassHolder &holder) {
	glm::ivec2 resolution = glm::floor(holder.size * holder.density);
	if(holder.meshIndex == (uint)-1) {
		GrassBladeMesh *mesh = new GrassBladeMesh(resolution, holder.LOD);
		holder.meshIndex = assetManager->addMesh((Mesh *)mesh, "grass_mesh_" + std::to_string(mesh->index), false);
		return;
	}
	GrassBladeMesh *mesh = (GrassBladeMesh *)assetManager->getMesh(holder.meshIndex);
	if(mesh->resolution != resolution || mesh->LOD != holder.LOD) {
		mesh->LOD = holder.LOD;
		mesh->setResolution(resolution);
	}
}

void ygl::GrassSystem::reload() {
	
}

void ygl::GrassSystem::doWork() { this->update((float)window->globalTime); }

ygl::Material &ygl::GrassSystem::getMaterial() {
	return scene->getSystem<ygl::Renderer>()->getMaterial(materialIndex);
}

uint ygl::GrassSystem::getMaterialIndex() {
	return materialIndex;
}

std::ostream &ygl::operator<<(std::ostream &out, const ygl::GrassSystem::GrassHolder &rhs) {
	static_cast<void>(rhs);
	return out << "GrassHolder";
}

const char *ygl::GrassSystem::name				= "ygl::GrassSystem";
const char *ygl::GrassSystem::GrassHolder::name = "ygl::GrassSystem::GrassHolder";
