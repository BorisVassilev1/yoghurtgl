#include <yoghurtgl.h>

#include <effects.h>
#include <renderer.h>

ygl::GrassSystem::GrassBladeMesh::GrassBladeMesh(GLuint bladeCount) {
	this->bladeCount = bladeCount;
	this->createVAO();
	glBindVertexArray(this->getVAO());
	this->verticesCount = 15;
	GLuint indices[]	= {2, 1, 0, 2, 3,  1, 4, 3,	 2,	 4, 5,	3,	6,	5,	4,	6,	7,	5,	8, 7,
						   6, 8, 9, 7, 10, 9, 8, 10, 11, 9, 12, 11, 10, 12, 13, 11, 14, 13, 12};
	this->createIBO(indices, 13 * 3);
	glGenBuffers(1, &grassData);

	setBladeCount(bladeCount);

	this->addVBO(0, 4, grassData, 1, sizeof(BladeData), 0);
	this->addVBO(1, 4, grassData, 1, sizeof(BladeData), (const void *)(4 * sizeof(float)));
	this->addVBO(2, 1, grassData, 1, sizeof(BladeData), (const void *)(8 * sizeof(float)));
	glBindVertexArray(0);
}

void ygl::GrassSystem::GrassBladeMesh::setBladeCount(GLuint bladeCount) {
	glBindBuffer(GL_ARRAY_BUFFER, grassData);
	glBufferData(GL_ARRAY_BUFFER, bladeCount * sizeof(BladeData), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
};

ygl::GrassSystem::GrassBladeMesh::~GrassBladeMesh() { glDeleteBuffers(1, &grassData); }

void ygl::GrassSystem::init() {
	reload();
	bladeMesh = new GrassBladeMesh(bladeCount);

	materialIndex = scene->getSystem<Renderer>()->addMaterial(Material(glm::vec3(0., 1., 0.), .2, glm::vec3(0.), 0.99,
																	   glm::vec3(0.1), 0.0, glm::vec3(1.), 0.0, 0.3,
																	   0.0, false, 0., 0.0, 0.0));
	Shader::setSSBO(bladeMesh->grassData, 1);
	grassCompute.bind();
	grassCompute.setUniform("time", 0.f);
	grassCompute.setUniform("resolution", resolution);
	grassCompute.setUniform("size", size);

	scene->registerComponent<GrassHolder>();
	scene->setSystemSignature<GrassSystem, Transformation, GrassHolder>();
	scene->getSystem<Renderer>()->addDrawFunction([this]() -> void { render(this->scene->window->globalTime); });
}

ygl::GrassSystem::~GrassSystem() { delete bladeMesh; }

void ygl::GrassSystem::update(float time) {
	auto worldMatrix = scene->getComponent<Transformation>(*entities.begin()).getWorldMatrix();

	grassCompute.bind();
	grassCompute.setUniform("time", time);
	grassCompute.setUniform("resolution", resolution);
	grassCompute.setUniform("size", size);
	grassCompute.setUniform("anchorWorldMatrix", worldMatrix);
	grassCompute.unbind();
	Renderer::compute(&grassCompute, resolution.x, resolution.y, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ygl::GrassSystem::render(float time) {
	grassShader.bind();
	if (grassShader.hasUniform("time")) grassShader.setUniform("time", time);

	bladeMesh->bind();
	{
		auto worldMatrix = scene->getComponent<Transformation>(*entities.begin()).getWorldMatrix();
		grassShader.setUniform("worldMatrix", worldMatrix);
		if (grassShader.hasUniform("material_index")) grassShader.setUniform("material_index", materialIndex);

		glDrawElementsInstanced(bladeMesh->getDrawMode(), bladeMesh->getIndicesCount(), GL_UNSIGNED_INT, 0, bladeCount);
	}
	bladeMesh->unbind();
	grassShader.unbind();
}

void ygl::GrassSystem::reload() {
	resolution = glm::floor(size * density);
	bladeCount = resolution.x * resolution.y;
	if (bladeMesh != nullptr) { bladeMesh->setBladeCount(bladeCount); }
}

void ygl::GrassSystem::doWork() { this->update((float)scene->window->globalTime); }