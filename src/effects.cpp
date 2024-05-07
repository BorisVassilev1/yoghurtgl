#include <yoghurtgl.h>

#include <effects.h>
#include <renderer.h>
#include <stdexcept>
#include "asset_manager.h"

#if !defined(YGL_NO_COMPUTE_SHADERS)
uint ygl::GrassSystem::GrassBladeMesh::count = 0;

ygl::GrassSystem::GrassBladeMesh::GrassBladeMesh(glm::ivec2 resolution, int LOD) {
	this->LOD = LOD;
	this->createVAO();
	glBindVertexArray(this->getVAO());

	this->verticesCount = 15;
	GLuint indices[]	= {2, 1, 0, 2, 3,  1, 4, 3,	 2,	 4, 5,	3,	6,	5,	4,	6,	7,	5,	8, 7,
						   6, 8, 9, 7, 10, 9, 8, 10, 11, 9, 12, 11, 10, 12, 13, 11, 14, 13, 12};
	this->createIBO(indices, 13 * 3);

	this->vCount1		= this->verticesCount;
	this->ibo1			= this->ibo;
	this->indicesCount1 = this->indicesCount;

	this->verticesCount = 7;
	GLuint indicesLow[] = {2, 1, 0, 2, 3, 1, 4, 3, 2, 4, 5, 3, 6, 5, 4};
	this->createIBO(indicesLow, 5 * 3);

	this->vCount2		= this->verticesCount;
	this->ibo2			= this->ibo;
	this->indicesCount2 = this->indicesCount;

	glGenBuffers(1, &grassData);
	setResolution(resolution);

	this->addVBO(0, 4, grassData, GL_FLOAT, 1, sizeof(BladeData), 0);
	this->addVBO(1, 4, grassData, GL_FLOAT, 1, sizeof(BladeData), (const void *)(4 * sizeof(float)));
	this->addVBO(2, 1, grassData, GL_UNSIGNED_INT, 1, sizeof(BladeData), (const void *)(8 * sizeof(float)));
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
	if (LOD == 0) {
		ibo			  = ibo1;
		verticesCount = vCount1;
		indicesCount  = indicesCount1;
	} else {
		ibo			  = ibo2;
		verticesCount = vCount2;
		indicesCount  = indicesCount2;
	}
	MultiBufferMesh::bind();
}

// TODO:
void ygl::GrassSystem::GrassHolder::serialize(std::ostream &out) {
	out.write((char *)glm::value_ptr(size), sizeof(size));
	out.write((char *)&density, sizeof(density));
	out.write((char *)&LOD, sizeof(LOD));
}

void ygl::GrassSystem::GrassHolder::deserialize(std::istream &in) { 
	in.read((char *)glm::value_ptr(size), sizeof(size));
	in.read((char *)&density, sizeof(density));
	in.read((char *)&LOD, sizeof(LOD));
}

void ygl::GrassSystem::init() {
	if (!scene->hasSystem<Renderer>()) THROW_RUNTIME_ERR("Renderer system must be registered in the scene.");
	if (!scene->hasSystem<AssetManager>()) THROW_RUNTIME_ERR("Asset Manager must be registered in the scene.")

	assetManager = scene->getSystem<AssetManager>();

	auto grassCompute = new ComputeShader("./shaders/grass/grassCompute.comp");
	grassComputeIndex = assetManager->addShader(grassCompute, "Grass Compute", false);
	auto grassShader  = new VFShader("./shaders/grass/grass.vs", "./shaders/grass/grass.fs");
	grassShaderIndex  = assetManager->addShader(grassShader, "Grass Shader", false);

	materialIndex = scene->getSystem<Renderer>()->addMaterial(
		Material(glm::vec3(0.1, 0.7, 0.1), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.), 0.0, 0.8, 0.0));
	grassCompute->bind();
	grassCompute->setUniform("time", 0.f);

	scene->registerComponent<GrassHolder>();
	scene->setSystemSignature<GrassSystem, Transformation, GrassHolder>();
	renderer	 = scene->getSystem<Renderer>();
	this->window = renderer->getWindow();
	renderer->addDrawFunction([this]() -> void { render(this->renderer->getWindow()->globalTime); });
}

ygl::GrassSystem::~GrassSystem() {}

void ygl::GrassSystem::update(float time) {
	auto grassCompute = (ComputeShader *)assetManager->getShader(grassComputeIndex);
	this->bladeCount  = 0;
	for (Entity e : entities) {
		auto		 worldMatrix = scene->getComponent<Transformation>(e).getWorldMatrix();
		GrassHolder &holder		 = scene->getComponent<GrassHolder>(e);
		if (holder.LOD > 1) continue;
		reload(holder);
		GrassBladeMesh *mesh = (GrassBladeMesh *)assetManager->getMesh(holder.meshIndex);
		this->bladeCount += mesh->bladeCount;

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
	auto grassShader = (VFShader *)assetManager->getShader(grassShaderIndex);
	grassShader->bind();
	if (grassShader->hasUniform("time")) grassShader->setUniform("time", time);
	Renderer *renderer = scene->getSystem<Renderer>();
	if (grassShader->hasUniform("use_skybox")) grassShader->setUniform("use_skybox", renderer->hasSkybox());
	if (grassShader->hasUniform("use_shadow")) grassShader->setUniform("use_shadow", renderer->hasShadow());

	for (Entity e : entities) {
		auto		 worldMatrix = scene->getComponent<Transformation>(e).getWorldMatrix();
		GrassHolder &holder		 = scene->getComponent<GrassHolder>(e);
		if (holder.LOD > 1) continue;
		GrassBladeMesh *mesh = (GrassBladeMesh *)scene->getSystem<AssetManager>()->getMesh(holder.meshIndex);

		mesh->bind();
		{
			if (grassShader->hasUniform("worldMatrix")) grassShader->setUniform("worldMatrix", worldMatrix);
			if (grassShader->hasUniform("renderMode")) grassShader->setUniform("renderMode", renderer->renderMode);
			if (grassShader->hasUniform("material_index")) grassShader->setUniform("material_index", materialIndex);
			if (grassShader->hasUniform("curvature")) grassShader->setUniform("curvature", curvature);
			if (grassShader->hasUniform("facingOffset")) grassShader->setUniform("facingOffset", facingOffset);
			if (grassShader->hasUniform("height")) grassShader->setUniform("height", height);
			if (grassShader->hasUniform("width")) grassShader->setUniform("width", width * (holder.LOD + 1));
			if (grassShader->hasUniform("blade_triangles"))
				grassShader->setUniform("blade_triangles", (uint)mesh->getVerticesCount());

			glDrawElementsInstanced(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0, mesh->bladeCount);
		}

		mesh->unbind();
	}
	grassShader->unbind();
}

void ygl::GrassSystem::reload(ygl::GrassSystem::GrassHolder &holder) {
	glm::ivec2 resolution = glm::floor(holder.size * holder.density);
	if (holder.meshIndex == (uint)-1) {
		GrassBladeMesh *mesh = new GrassBladeMesh(resolution, holder.LOD);
		holder.meshIndex	 = assetManager->addMesh((Mesh *)mesh, "grass_mesh_" + std::to_string(mesh->index), false);
		return;
	}
	GrassBladeMesh *mesh = (GrassBladeMesh *)assetManager->getMesh(holder.meshIndex);
	if (mesh->resolution != resolution || mesh->LOD != holder.LOD) {
		mesh->LOD = holder.LOD;
		mesh->setResolution(resolution);
	}
}

void ygl::GrassSystem::reload() {
	for (Entity e : entities) {
		GrassHolder &holder = scene->getComponent<GrassHolder>(e);
		holder.density		= this->density;
	}
}

void ygl::GrassSystem::doWork() { this->update((float)window->globalTime); }

ygl::Material &ygl::GrassSystem::getMaterial() { return scene->getSystem<ygl::Renderer>()->getMaterial(materialIndex); }

uint ygl::GrassSystem::getMaterialIndex() { return materialIndex; }

std::ostream &ygl::operator<<(std::ostream &out, const ygl::GrassSystem::GrassHolder &rhs) {
	static_cast<void>(rhs);
	return out << "size: (" << rhs.size.x << ", " << rhs.size.y << ") mesh: " << rhs.meshIndex
			   << " density: " << rhs.density << " LOD: " << rhs.LOD;
}

const char *ygl::GrassSystem::name				= "ygl::GrassSystem";
const char *ygl::GrassSystem::GrassHolder::name = "ygl::GrassSystem::GrassHolder";
#endif
