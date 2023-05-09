#include <renderer.h>

#include <transformation.h>
#include <assert.h>
#include <iterator>
#include <ostream>

ygl::Light::Light(glm::mat4 transform, glm::vec3 color, float intensity, ygl::Light::Type type)
	: transform(transform), color(color), intensity(intensity), type(type) {}

ygl::Light::Light(ygl::Transformation transformation, glm::vec3 color, float intensity, ygl::Light::Type type)
	: transform(transformation.getWorldMatrix()), color(color), intensity(intensity), type(type) {}

std::ostream &ygl::operator<<(std::ostream &out, const Light &l) {
	return out << "transform: <matrix4x4>"
			   << " color: (" << l.color.x << " " << l.color.y << " " << l.color.z << ") intensity: " << l.intensity
			   << " type: " << l.type;
}

ygl::FrameBuffer::FrameBuffer(uint16_t width, uint16_t height, const char *name) {
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);

	color		  = new Texture2d(width, height, ITexture::Type::RGBA, nullptr);
	depth_stencil = new Texture2d(width, height, ITexture::Type::DEPTH_STENCIL, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->getID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_stencil->getID(), 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		assert(false);
	}

	if (name) {
		GLsizei size = strlen(name);
		glObjectLabel(GL_FRAMEBUFFER, id, size, name);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ygl::FrameBuffer::~FrameBuffer() {
	glDeleteFramebuffers(1, &id);
	delete color;
	delete depth_stencil;
}

void ygl::FrameBuffer::clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }

void ygl::FrameBuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, id); }

void ygl::FrameBuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

ygl::Texture2d *ygl::FrameBuffer::getColor() { return color; }

ygl::Texture2d *ygl::FrameBuffer::getDepthStencil() { return depth_stencil; }

void ygl::FrameBuffer::bindDefault() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

ygl::ACESEffect::ACESEffect() {
	colorGrader = new VFShader("./shaders/ui/textureOnScreen.vs", "./shaders/postProcessing/acesFilm.fs");
}

ygl::ACESEffect::~ACESEffect() { delete colorGrader; }

void ygl::ACESEffect::apply(FrameBuffer *front, FrameBuffer *back) {
	front->getColor()->bind(GL_TEXTURE7);
	if (back) back->bind();
	else FrameBuffer::bindDefault();

	colorGrader->bind();
	colorGrader->setUniform("doColorGrading", enabled);
	colorGrader->setUniform("doGammaCorrection", enabled);
	Renderer::drawObject(colorGrader, renderer->getScreenQuad());
	front->getColor()->unbind(GL_TEXTURE7);
}

ygl::BloomEffect::BloomEffect(Renderer *renderer) {
	Window *window = renderer->scene->window;
	tex1		   = new Texture2d(window->getWidth(), window->getHeight());
	tex2		   = new Texture2d(window->getWidth(), window->getHeight());

	blurShader	 = new ComputeShader("./shaders/postProcessing/blur.comp");
	filterShader = new ComputeShader("./shaders/postProcessing/filter.comp");
	filterShader->bind();
	filterShader->setUniform("img_input", 1);
	filterShader->setUniform("img_output", 0);
	filterShader->unbind();

	onScreen = new VFShader("./shaders/ui/textureOnScreen.vs", "./shaders/ui/textureOnScreen.fs");
	onScreen->bind();
	onScreen->setUniform("sampler_color", 7);
	onScreen->setUniform("sampler_depth", 7);
	onScreen->setUniform("sampler_stencil", 7);
	onScreen->unbind();
}

ygl::BloomEffect::~BloomEffect() {
	delete tex1;
	delete tex2;
	delete blurShader;
	delete filterShader;
	delete onScreen;
}

void ygl::BloomEffect::apply(FrameBuffer *front, FrameBuffer *back) {
	if (!enabled) {
		if (back) {
			back->bind();
		} else FrameBuffer::bindDefault();

		front->getColor()->bind(GL_TEXTURE7);
		Renderer::drawObject(onScreen, renderer->getScreenQuad());

		return;
	}
	uint blurSize = 5;
	// first separate pixels that have to be blurred;
	front->getColor()->bindImage(1);
	tex1->bindImage(0);
	filterShader->bind();
	Window *window = renderer->scene->window;
	Renderer::compute(filterShader, window->getWidth(), window->getHeight(), 1);
	tex2->bindImage(1);

	for (uint i = 0; i < blurSize; ++i) {
		blurShader->bind();
		blurShader->setUniform("img_input", 0);
		blurShader->setUniform("img_output", 1);
		blurShader->setUniform("blurDirection", 0);
		Renderer::compute(blurShader, window->getWidth(), window->getHeight(), 1);

		blurShader->bind();
		blurShader->setUniform("img_input", 1);
		blurShader->setUniform("img_output", 0);
		blurShader->setUniform("blurDirection", 1);
		Renderer::compute(blurShader, window->getWidth(), window->getHeight(), 1);
	}

	if (back) {
		back->bind();
		glClearColor(0, 0, 0, 0);
		back->clear();
	} else FrameBuffer::bindDefault();

	glBlendFunc(GL_ONE, GL_ONE);

	front->getColor()->bind(GL_TEXTURE7);
	Renderer::drawObject(onScreen, renderer->getScreenQuad());
	tex1->bind(GL_TEXTURE7);

	Renderer::drawObject(onScreen, renderer->getScreenQuad());
	tex1->unbind(GL_TEXTURE7);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

ygl::RendererComponent::RendererComponent(unsigned int shaderIndex, unsigned int meshIndex, unsigned int materialIndex)
	: shaderIndex(shaderIndex), meshIndex(meshIndex), materialIndex(materialIndex) {}

void ygl::RendererComponent::serialize(std::ostream &out) {
	out.write((char *) (&this->shaderIndex), sizeof(uint));
	out.write((char *) (&this->materialIndex), sizeof(uint));
	out.write((char *) (&this->meshIndex), sizeof(uint));
}

void ygl::RendererComponent::deserialize(std::istream &in) {
	in.read((char *) (&this->shaderIndex), sizeof(uint));
	in.read((char *) (&this->materialIndex), sizeof(uint));
	in.read((char *) (&this->meshIndex), sizeof(uint));
} 

bool ygl::RendererComponent::operator==(const RendererComponent &other) {
	return this->shaderIndex == other.shaderIndex && this->materialIndex == other.materialIndex &&
		   this->meshIndex == other.meshIndex;
}

void ygl::Renderer::init() {
	GLint texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	for (int i = 0; i < texture_units; ++i) {
		defaultTexture.bind(GL_TEXTURE0 + i);
	}

	uint16_t width = scene->window->getWidth(), height = scene->window->getHeight();
	frontFrameBuffer = new FrameBuffer(width, height, "Front frameBuffer");
	backFrameBuffer	 = new FrameBuffer(width, height, "Back frameBuffer");

	// addScreenEffect(new BloomEffect(this));
	addScreenEffect(new ACESEffect());

	scene->registerComponent<RendererComponent>();
	scene->setSystemSignature<Renderer, Transformation, RendererComponent>();
}

ygl::Shader *ygl::Renderer::getShader(RendererComponent &comp) { return getShader(comp.shaderIndex); }

ygl::Shader *ygl::Renderer::getShader(uint index) {
	assert(index < shaders.size() && "invalid index");
	return shaders[index];
}

ygl::Material &ygl::Renderer::getMaterial(RendererComponent &comp) { return getMaterial(comp.materialIndex); }

ygl::Material &ygl::Renderer::getMaterial(uint index) {
	assert(index < materials.size() && "invalid index");
	return materials[index];
}

ygl::Mesh *ygl::Renderer::getMesh(RendererComponent &comp) { return getMesh(comp.meshIndex); }

ygl::Mesh *ygl::Renderer::getMesh(uint index) {
	assert(index < meshes.size() && "invalid index");
	return meshes[index];
}

ygl::Mesh *ygl::Renderer::getScreenQuad() { return screenQuad; }

unsigned int ygl::Renderer::addShader(Shader *shader) {
	shaders.push_back(shader);
	return shaders.size() - 1;
}

unsigned int ygl::Renderer::addMaterial(const Material &mat) {
	materials.push_back(mat);
	return materials.size() - 1;
}

unsigned int ygl::Renderer::addMesh(Mesh *mesh) {
	meshes.push_back(mesh);
	return meshes.size() - 1;
}

ygl::Light &ygl::Renderer::addLight(const Light &light) {
	lights.push_back(light);
	return lights.back();
}

ygl::Light &ygl::Renderer::getLight(uint index) { return lights[index]; }

void ygl::Renderer::addScreenEffect(IScreenEffect *effect) {
	effect->setRenderer(this);
	effects.push_back(effect);
}

void ygl::Renderer::loadData() {
	// send material and light data to the GPU through UBOs
	if (materialsBuffer == 0) { glGenBuffers(1, &materialsBuffer); }
	glBindBuffer(GL_UNIFORM_BUFFER, materialsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, materials.size() * sizeof(Material), materials.data(), GL_DYNAMIC_DRAW);

	Shader::setUBO(materialsBuffer, 1);

	if (lightsBuffer == 0) { glGenBuffers(1, &lightsBuffer); }
	uint lightsCount = lights.size();
	glBindBuffer(GL_UNIFORM_BUFFER, lightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, 100 * sizeof(Light) + sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, lights.size() * sizeof(Light), lights.data());
	glBufferSubData(GL_UNIFORM_BUFFER, 100 * sizeof(Light), sizeof(uint), &lightsCount);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Shader::setUBO(lightsBuffer, 2);
}

void ygl::Renderer::setDefaultShader(int defaultShader) { this->defaultShader = defaultShader; }

uint ygl::Renderer::getDefaultShader() { return defaultShader; }

void ygl::Renderer::setClearColor(glm::vec4 color) {
	this->clearColor = color;
	glClearColor(color.x, color.y, color.z, color.w);
}

void ygl::Renderer::swapFrameBuffers() { std::swap(frontFrameBuffer, backFrameBuffer); }

void ygl::Renderer::drawScene() {
	// bind default shader
	uint prevShaderIndex;
	if (defaultShader != (uint)-1) {
		shaders[defaultShader]->bind();
		prevShaderIndex = defaultShader;
	} else {
		if (shaders.size()) shaders[0]->bind();		// there has to be at least one shader
		prevShaderIndex = 0;
	}

	for (Entity e : entities) {
		ygl::Transformation	   &transform = scene->getComponent<Transformation>(e);
		ygl::RendererComponent &ecr		  = scene->getComponent<RendererComponent>(e);

		Shader *sh;
		// binds the object's own shader if present.
		// Oherwise checks if the default shader has been bound by the previous object
		// and binds it only if needed
		if (ecr.shaderIndex != (uint)-1) {				  // if object has a shader
			if (ecr.shaderIndex != prevShaderIndex) {	  // if its different from the previous one
				shaders[prevShaderIndex]->unbind();
				sh				= shaders[ecr.shaderIndex];
				prevShaderIndex = ecr.shaderIndex;	   // the next previous is the current
				sh->bind();
			} else {
				sh = shaders[prevShaderIndex];	   // set sh so that its not null
			}
		} else {
			assert(defaultShader != (uint)-1 && "cannot use default shader when it is not defined");
			if (prevShaderIndex != defaultShader) {		// if the previous shader was different
				shaders[prevShaderIndex]->unbind();
				sh				= shaders[defaultShader];
				prevShaderIndex = defaultShader;
				sh->bind();
			} else {
				sh = shaders[defaultShader];	 // set sh so its not null
			}
		}
		// sh is never null and the current bound shader

		if (materials[ecr.materialIndex].use_albedo_map)
			scene->assetManager.getTexture(materials[ecr.materialIndex].albedo_map)->bind(ygl::TexIndex::COLOR);
		if (materials[ecr.materialIndex].use_normal_map)
			scene->assetManager.getTexture(materials[ecr.materialIndex].normal_map)->bind(ygl::TexIndex::NORMAL);
		if (materials[ecr.materialIndex].use_roughness_map)
			scene->assetManager.getTexture(materials[ecr.materialIndex].roughness_map)->bind(ygl::TexIndex::ROUGHNESS);
		if (materials[ecr.materialIndex].use_ao_map)
			scene->assetManager.getTexture(materials[ecr.materialIndex].ao_map)->bind(ygl::TexIndex::AO);
		if (materials[ecr.materialIndex].use_emission_map)
			scene->assetManager.getTexture(materials[ecr.materialIndex].emission_map)->bind(ygl::TexIndex::EMISSION);
		if (materials[ecr.materialIndex].use_metallic_map)
			scene->assetManager.getTexture(materials[ecr.materialIndex].metallic_map)->bind(ygl::TexIndex::METALLIC);

		Mesh *mesh = meshes[ecr.meshIndex];
		mesh->bind();
		// set uniforms
		if (sh->hasUniform("worldMatrix")) sh->setUniform("worldMatrix", transform.getWorldMatrix());
		if (sh->hasUniform("material_index")) sh->setUniform("material_index", (GLuint)ecr.materialIndex);
		// draw
		glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
		// clean up
		mesh->unbind();
	}
	if (shaders.size() > prevShaderIndex) shaders[prevShaderIndex]->unbind();	  // unbind the last used shader
}

void ygl::Renderer::colorPass() {
	backFrameBuffer->bind();
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	backFrameBuffer->clear();
	if (scene->window->shade) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw all entities
	drawScene();

	// run draw calls from other systems
	for (auto f : drawFunctions) {
		f();
	}

	backFrameBuffer->unbind();
	swapFrameBuffers();
}

void ygl::Renderer::effectsPass() {
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int effectsCount = effects.size();
	assert(effectsCount != 0);
	for (int i = 0; i < effectsCount; ++i) {
		auto effect = effects[i];
		if (i == effectsCount - 1) {
			effect->apply(frontFrameBuffer, 0);
		} else {
			effect->apply(frontFrameBuffer, backFrameBuffer);
			swapFrameBuffers();
		}
	}
	FrameBuffer::bindDefault();
}

void ygl::Renderer::doWork() {
	colorPass();
	effectsPass();
	defaultTexture.bind(GL_TEXTURE0);	  // some things break when nothing is bound to texture0
}

ygl::Renderer::~Renderer() {
	for (Shader *sh : shaders) {
		delete sh;
	}
	for (Mesh *mesh : meshes) {
		delete mesh;
	}
	for (IScreenEffect *effect : effects) {
		delete effect;
	}
	delete frontFrameBuffer;
	delete backFrameBuffer;
	delete screenQuad;
}

void ygl::Renderer::addDrawFunction(const std::function<void()> &func) { drawFunctions.push_back(func); }

void ygl::Renderer::drawObject(Transformation &transform, Shader *sh, Mesh *mesh, GLuint materialIndex) {
	sh->bind();

	mesh->bind();
	sh->setUniform("worldMatrix", transform.getWorldMatrix());
	sh->setUniform("material_index", materialIndex);

	glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
	mesh->unbind();
	sh->unbind();
}

void ygl::Renderer::drawObject(Shader *sh, Mesh *mesh) {
	if (!sh->isBound()) sh->bind();
	mesh->bind();
	glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
	mesh->unbind();
	sh->unbind();
}

void ygl::Renderer::compute(ComputeShader *shader, int domainX, int domainY, int domainZ) {
	if (!shader->isBound()) shader->bind();
	int groupsX = domainX / shader->groupSize.x + (domainX % shader->groupSize.x > 0);
	int groupsY = domainY / shader->groupSize.y + (domainY % shader->groupSize.y > 0);
	int groupsZ = domainZ / shader->groupSize.z + (domainZ % shader->groupSize.z > 0);

	glDispatchCompute(groupsX, groupsY, groupsZ);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	shader->unbind();
}

GLuint ygl::Renderer::loadMaterials(int count, Material *materials) {
	GLuint materialsBuffer;
	glGenBuffers(1, &materialsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, materialsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, count * sizeof(Material), materials, GL_DYNAMIC_DRAW);
	Shader::setUBO(materialsBuffer, 1);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return materialsBuffer;
}

GLuint ygl::Renderer::loadLights(int count, Light *lights) {
	GLuint lightsBuffer;
	glGenBuffers(1, &lightsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, 100 * sizeof(Light) + sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, count * sizeof(Light), lights);
	glBufferSubData(GL_UNIFORM_BUFFER, 100 * sizeof(Light), sizeof(uint), &count);
	Shader::setUBO(lightsBuffer, 2);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return lightsBuffer;
}

std::ostream &ygl::operator<<(std::ostream &out, const ygl::RendererComponent &rhs) {
	return out << "shader: " << rhs.shaderIndex << " material: " << rhs.materialIndex << " mesh: " << rhs.meshIndex;
}
