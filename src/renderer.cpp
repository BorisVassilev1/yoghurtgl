#include <renderer.h>

#include <transformation.h>
#include <assert.h>
#include <ostream>
#include <string>
#include <texture.h>
#include <yoghurtgl.h>
#include <asset_manager.h>
#include <material.h>
#include <entities.h>

#include <imgui.h>

ygl::Light::Light(glm::mat4 transform, glm::vec3 color, float intensity, ygl::Light::Type type)
	: transform(transform), color(color), intensity(intensity), type(type) {}

ygl::Light::Light(ygl::Transformation transformation, glm::vec3 color, float intensity, ygl::Light::Type type)
	: transform(transformation.getWorldMatrix()), color(color), intensity(intensity), type(type) {}

std::ostream &ygl::operator<<(std::ostream &out, const Light &l) {
	return out << "transform: <matrix4x4>"
			   << " color: (" << l.color.x << " " << l.color.y << " " << l.color.z << ") intensity: " << l.intensity
			   << " type: " << l.type;
}

ygl::FrameBuffer::FrameBuffer(FrameBufferAttachable *buff1, GLenum attachment1, FrameBufferAttachable *buff2,
							  GLenum attachment2, const char *name) {
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);

	color = buff1;
	if (color != nullptr) { color->BindToFrameBuffer(*this, attachment1, 0, 0); }

	depth_stencil = buff2;
	if (depth_stencil != nullptr) { depth_stencil->BindToFrameBuffer(*this, attachment2, 0, 0); }

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		dbLog(LOG_WARNING, "Framebuffer is not complete upon creation!");
	}

	if (name) {
		GLsizei size = strlen(name);
		glObjectLabel(GL_FRAMEBUFFER, id, size, name);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ygl::FrameBuffer::~FrameBuffer() {
	glDeleteFramebuffers(1, &id);
	if (color != nullptr) delete color;
	if (depth_stencil != nullptr) delete depth_stencil;
}

void ygl::FrameBuffer::clear() const { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }

void ygl::FrameBuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, id); }

void ygl::FrameBuffer::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

ygl::Texture2d *ygl::FrameBuffer::getColor() { return (Texture2d *)color; }

ygl::Texture2d *ygl::FrameBuffer::getDepthStencil() { return (Texture2d *)depth_stencil; }

void ygl::FrameBuffer::resize(uint width, uint height) {
	color->resize(width, height);
	depth_stencil->resize(width, height);
	color->BindToFrameBuffer(*this, GL_COLOR_ATTACHMENT0, 0, 0);
	depth_stencil->BindToFrameBuffer(*this, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
}

void ygl::FrameBuffer::bindDefault() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

ygl::ACESEffect::ACESEffect(ygl::Renderer *renderer) {
	this->setRenderer(renderer);
	auto sh		= new VFShader("./shaders/ui/textureOnScreen.vs", "./shaders/postProcessing/acesFilm.fs");
	colorGrader = renderer->getAssetManager()->addShader(sh, "color grading shader");
}

ygl::ACESEffect::~ACESEffect() {}

void ygl::ACESEffect::apply(FrameBuffer *front, FrameBuffer *back) {
	front->getColor()->bind(GL_TEXTURE7);
	if (back) back->bind();
	else FrameBuffer::bindDefault();

	VFShader *sh = (VFShader *)renderer->getAssetManager()->getShader(colorGrader);
	sh->bind();
	sh->setUniform("doColorGrading", enabled);
	sh->setUniform("doGammaCorrection", enabled);
	Renderer::drawObject(sh, renderer->getScreenQuad());
	front->getColor()->unbind(GL_TEXTURE7);
}

ygl::BloomEffect::BloomEffect(Renderer *renderer) {
	Window *window = renderer->getWindow();
	tex1		   = new Texture2d(window->getWidth(), window->getHeight(), TextureType::RGBA16F, nullptr);
	tex2		   = new Texture2d(window->getWidth(), window->getHeight(), TextureType::RGBA16F, nullptr);

	window->addResizeCallback([this, window](GLFWwindow *, int, int) {
		delete tex1;
		delete tex2;
		tex1		   = new Texture2d(window->getWidth(), window->getHeight(), TextureType::RGBA16F, nullptr);
		tex2		   = new Texture2d(window->getWidth(), window->getHeight(), TextureType::RGBA16F, nullptr);
		dbLog(ygl::LOG_DEBUG, "resizing textures!!");
	});

	dbLog(ygl::LOG_DEBUG, "window size: ", window->getWidth(), " ", window->getHeight());

	blurShader	 = new ComputeShader("./shaders/postProcessing/blur.comp");
	filterShader = new ComputeShader("./shaders/postProcessing/filter.comp");
	filterShader->bind();
	//filterShader->setUniform("img_input", 1);
	//filterShader->setUniform("img_output", 0);
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
	FrameBuffer::bindDefault();
	front->getColor()->bindImage(1);
	front->getColor()->unbind(GL_TEXTURE7);
	tex1->bindImage(0);
	Window *window = renderer->getWindow();

	filterShader->bind();
	if(filterShader->hasUniform("img_input")) filterShader->setUniform("img_input", 1);
	filterShader->setUniform("img_output", 0);
	glTextureBarrier();
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

ygl::RendererComponent::RendererComponent(unsigned int shaderIndex, unsigned int meshIndex, unsigned int materialIndex,
										  unsigned int shadowShaderIndex)
	: shaderIndex(shaderIndex),
	  meshIndex(meshIndex),
	  materialIndex(materialIndex),
	  shadowShaderIndex(shadowShaderIndex) {}

void ygl::RendererComponent::serialize(std::ostream &out) {
	out.write((char *)(&this->shaderIndex), sizeof(uint));
	out.write((char *)(&this->materialIndex), sizeof(uint));
	out.write((char *)(&this->meshIndex), sizeof(uint));
	out.write((char *)(&this->shadowShaderIndex), sizeof(uint));
	out.write((char *)(&this->isAnimated), sizeof(bool));
}

void ygl::RendererComponent::deserialize(std::istream &in) {
	in.read((char *)(&this->shaderIndex), sizeof(uint));
	in.read((char *)(&this->materialIndex), sizeof(uint));
	in.read((char *)(&this->meshIndex), sizeof(uint));
	in.read((char *)(&this->shadowShaderIndex), sizeof(uint));
	in.read((char *)(&this->isAnimated), sizeof(bool));
}

bool ygl::RendererComponent::operator==(const RendererComponent &other) {
	return this->shaderIndex == other.shaderIndex && this->materialIndex == other.materialIndex &&
		   this->meshIndex == other.meshIndex && this->shadowShaderIndex == other.shadowShaderIndex &&
		   this->isAnimated == other.isAnimated;
}

void ygl::Renderer::init() {
	GLint texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	for (int i = 0; i < texture_units; ++i) {
		defaultTexture.bind(GL_TEXTURE0 + i);
	}

	uint16_t width = window->getWidth(), height = window->getHeight();
	frontFrameBuffer =
		new FrameBuffer(new Texture2d(width, height, TextureType::RGBA16F, nullptr), GL_COLOR_ATTACHMENT0,
						new RenderBuffer(width, height, TextureType::DEPTH_STENCIL_32F_8), GL_DEPTH_STENCIL_ATTACHMENT,
						"Front frameBuffer");
	backFrameBuffer = new FrameBuffer(new Texture2d(width, height, TextureType::RGBA16F, nullptr), GL_COLOR_ATTACHMENT0,
									  new RenderBuffer(width, height, TextureType::DEPTH_STENCIL_32F_8),
									  GL_DEPTH_STENCIL_ATTACHMENT, "Back frameBuffer");
	shadowFrameBuffer = new FrameBuffer(nullptr, GL_COLOR_ATTACHMENT0,
										new Texture2d(shadowMapSize, shadowMapSize, TextureType::DEPTH_24, nullptr),
										GL_DEPTH_ATTACHMENT, "Shadow Framebuffer");
	dbLog(ygl::LOG_DEBUG, "shadow texture: ", shadowFrameBuffer->getDepthStencil()->getID());

	window->addResizeCallback([this](GLFWwindow *window, int width, int height) {
		(void)window;
		this->frontFrameBuffer->resize(width, height);
		this->backFrameBuffer->resize(width, height);
	});

	scene->registerComponentIfCan<RendererComponent>();
	scene->registerComponentIfCan<Transformation>();
	scene->setSystemSignature<Renderer, Transformation, RendererComponent>();

	scene->registerSystemIfCan<ygl::AssetManager>();
	asman = scene->getSystem<AssetManager>();

	//addScreenEffect(new BloomEffect(this));
	addScreenEffect(new ACESEffect(this));
	brdfTexture = asman->addTexture(createBRDFTexture(), "brdf_Texture", false);
}

ygl::Shader *ygl::Renderer::getShader(RendererComponent &comp) { return getShader(comp.shaderIndex); }

ygl::Shader *ygl::Renderer::getShader(uint index) { return asman->getShader(index); }

ygl::Material &ygl::Renderer::getMaterial(RendererComponent &comp) { return getMaterial(comp.materialIndex); }

ygl::Material &ygl::Renderer::getMaterial(uint index) {
	assert(index < materials.size() && "invalid index");
	return materials[index];
}

ygl::Mesh *ygl::Renderer::getMesh(RendererComponent &comp) { return getMesh(comp.meshIndex); }

ygl::Mesh *ygl::Renderer::getMesh(uint index) { return asman->getMesh(index); }

ygl::Mesh *ygl::Renderer::getScreenQuad() { return screenQuad; }

unsigned int ygl::Renderer::addMaterial(const Material &mat) {
	materials.push_back(mat);
	return materials.size() - 1;
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

ygl::IScreenEffect *ygl::Renderer::getScreenEffect(uint index) {
	if (index >= effects.size()) {
		dbLog(ygl::LOG_WARNING, "Access out of bounds index: ", index);
		return nullptr;
	}
	return effects[index];
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

	if (lightsCount >= 1 && lights[0].type == Light::Type::DIRECTIONAL) {
		Transformation x(lights[0].transform);
		shadowCamera.transform = x;
		shadowCamera.update();
	}

	Shader::setUBO(lightsBuffer, 2);
}

void ygl::Renderer::setDefaultShader(int defaultShader) { this->defaultShader = defaultShader; }

uint ygl::Renderer::getDefaultShader() { return defaultShader; }

void ygl::Renderer::setDefaultShadowShader(int defaultShadowShader) { this->defaultShadowShader = defaultShadowShader; }

uint ygl::Renderer::getDefaultShadowShader() { return defaultShadowShader; }

void ygl::Renderer::setClearColor(glm::vec4 color) {
	this->clearColor = color;
	glClearColor(color.x, color.y, color.z, color.w);
}

void ygl::Renderer::setShadow(bool shadow) { this->shadow = shadow; }

void ygl::Renderer::swapFrameBuffers() { std::swap(frontFrameBuffer, backFrameBuffer); }

void ygl::Renderer::drawScene() {
	// bind default shader
	uint prevShaderIndex;
	if (defaultShader != (uint)-1) {
		asman->getShader(defaultShader)->bind();
		prevShaderIndex = defaultShader;
	} else {
		if (asman->getShadersCount()) asman->getShader(0)->bind();	   // there has to be at least one shader
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
				asman->getShader(prevShaderIndex)->unbind();
				sh				= asman->getShader(ecr.shaderIndex);
				prevShaderIndex = ecr.shaderIndex;	   // the next previous is the current
				sh->bind();
			} else {
				sh = asman->getShader(prevShaderIndex);		// set sh so that its not null
			}
		} else {
			assert(defaultShader != (uint)-1 && "cannot use default shader when it is not defined");
			if (prevShaderIndex != defaultShader) {		// if the previous shader was different
				asman->getShader(prevShaderIndex)->unbind();
				sh				= asman->getShader(defaultShader);
				prevShaderIndex = defaultShader;
				sh->bind();
			} else {
				sh = asman->getShader(defaultShader);	  // set sh so its not null
			}
		}
		// sh is never null and the current bound shader

		if (materials[ecr.materialIndex].use_albedo_map)
			asman->getTexture(materials[ecr.materialIndex].albedo_map)->bind(ygl::TexIndex::COLOR);
		if (materials[ecr.materialIndex].use_normal_map)
			asman->getTexture(materials[ecr.materialIndex].normal_map)->bind(ygl::TexIndex::NORMAL);
		if (materials[ecr.materialIndex].use_roughness_map)
			asman->getTexture(materials[ecr.materialIndex].roughness_map)->bind(ygl::TexIndex::ROUGHNESS);
		if (materials[ecr.materialIndex].use_ao_map)
			asman->getTexture(materials[ecr.materialIndex].ao_map)->bind(ygl::TexIndex::AO);
		if (materials[ecr.materialIndex].use_emission_map)
			asman->getTexture(materials[ecr.materialIndex].emission_map)->bind(ygl::TexIndex::EMISSION);
		if (materials[ecr.materialIndex].use_metallic_map)
			asman->getTexture(materials[ecr.materialIndex].metallic_map)->bind(ygl::TexIndex::METALLIC);
		if (materials[ecr.materialIndex].use_transparency_map)
			asman->getTexture(materials[ecr.materialIndex].transparency_map)->bind(ygl::TexIndex::OPACITY);
		if (skyboxTexture != 0) asman->getTexture(skyboxTexture)->bind(ygl::TexIndex::SKYBOX);

		if (irradianceTexture != 0) asman->getTexture(irradianceTexture)->bind(ygl::TexIndex::IRRADIANCE_MAP);
		else defaultCubemap.bind(ygl::TexIndex::IRRADIANCE_MAP);
		if (prefilterTexture != 0) asman->getTexture(prefilterTexture)->bind(ygl::TexIndex::PREFILTER_MAP);
		else defaultCubemap.bind(ygl::TexIndex::PREFILTER_MAP);

		if (sh->hasUniform("use_skybox")) { sh->setUniform("use_skybox", this->hasSkybox()); }
		if (sh->hasUniform("renderMode")) { sh->setUniform("renderMode", renderMode); }

		asman->getTexture(brdfTexture)->bind(ygl::TexIndex::BDRF_MAP);

		if (sh->hasUniform("use_shadow")) { sh->setUniform<GLboolean>("use_shadow", shadow); }
		if (shadow) shadowFrameBuffer->getDepthStencil()->bind(ygl::TexIndex::SHADOW_MAP);

		Mesh *mesh = getMesh(ecr.meshIndex);
		mesh->bind();
		// set uniforms
		if (sh->hasUniform("worldMatrix")) sh->setUniform("worldMatrix", transform.getWorldMatrix());
		if (sh->hasUniform("material_index")) sh->setUniform("material_index", (GLuint)ecr.materialIndex);
		if (sh->hasUniform("animate")) sh->setUniform("animate", ecr.isAnimated);

		if (renderMode == 6) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

		// draw
		glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
		// clean up
		mesh->unbind();
	}
	if (asman->getShadersCount() > prevShaderIndex)
		asman->getShader(prevShaderIndex)->unbind();	 // unbind the last used shader
}

void ygl::Renderer::shadowPass() {
	shadowCamera.enable();
	shadowFrameBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, shadowMapSize, shadowMapSize);
	// drawScene();

	// bind default shader
	uint prevShaderIndex;
	if (defaultShadowShader != (uint)-1) {
		asman->getShader(defaultShadowShader)->bind();
		prevShaderIndex = defaultShadowShader;
	} else {
		if (asman->getShadersCount()) asman->getShader(0)->bind();	   // there has to be at least one shader
		prevShaderIndex = 0;
	}

	for (Entity e : entities) {
		ygl::Transformation	   &transform = scene->getComponent<Transformation>(e);
		ygl::RendererComponent &ecr		  = scene->getComponent<RendererComponent>(e);

		Shader *sh;
		// binds the object's own shader if present.
		// Oherwise checks if the default shader has been bound by the previous object
		// and binds it only if needed
		if (ecr.shadowShaderIndex != (uint)-1) {				// if object has a shader
			if (ecr.shadowShaderIndex != prevShaderIndex) {		// if its different from the previous one
				asman->getShader(prevShaderIndex)->unbind();
				sh				= asman->getShader(ecr.shadowShaderIndex);
				prevShaderIndex = ecr.shadowShaderIndex;	 // the next previous is the current
				sh->bind();
			} else {
				sh = asman->getShader(prevShaderIndex);		// set sh so that its not null
			}
		} else {
			assert(defaultShadowShader != (uint)-1 && "cannot use default shader when it is not defined");
			if (prevShaderIndex != defaultShadowShader) {	  // if the previous shader was different
				asman->getShader(prevShaderIndex)->unbind();
				sh				= asman->getShader(defaultShadowShader);
				prevShaderIndex = defaultShadowShader;
				sh->bind();
			} else {
				sh = asman->getShader(defaultShadowShader);		// set sh so its not null
			}
		}
		// sh is never null and the current bound shader

		Mesh *mesh = getMesh(ecr.meshIndex);
		mesh->bind();
		// set uniforms
		if (sh->hasUniform("worldMatrix")) sh->setUniform("worldMatrix", transform.getWorldMatrix());
		if (sh->hasUniform("animate")) sh->setUniform("animate", ecr.isAnimated);

		// draw
		glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
		// clean up
		mesh->unbind();
	}
	if (asman->getShadersCount() > prevShaderIndex)
		asman->getShader(prevShaderIndex)->unbind();	 // unbind the last used shader

	for (auto f : drawFunctions) {
		f();
	}

	shadowFrameBuffer->unbind();
}

void ygl::Renderer::colorPass() {
	assert(mainCamera && "must have a main camera");
	mainCamera->enable();
	if (shadow) shadowCamera.enable(4);
	backFrameBuffer->bind();
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	backFrameBuffer->clear();
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, window->getWidth(), window->getHeight());

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
	if (shadow) shadowPass();
	colorPass();
	effectsPass();
	defaultTexture.bind(GL_TEXTURE0);	  // some things break when nothing is bound to texture0
}

ygl::Renderer::~Renderer() {
	for (IScreenEffect *effect : effects) {
		delete effect;
	}
	delete frontFrameBuffer;
	delete backFrameBuffer;
	delete shadowFrameBuffer;
	delete screenQuad;
}

void ygl::Renderer::addDrawFunction(const std::function<void()> &func) { drawFunctions.push_back(func); }

void ygl::Renderer::drawObject(Transformation &transform, Shader *sh, Mesh *mesh, GLuint materialIndex) {
	sh->bind();

	mesh->bind();
	if (sh->hasUniform("worldMatrix")) sh->setUniform("worldMatrix", transform.getWorldMatrix());
	if (sh->hasUniform("material_index")) sh->setUniform("material_index", materialIndex);

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

bool ygl::Renderer::hasSkybox() { return skyboxTexture && irradianceTexture && prefilterTexture; }

bool ygl::Renderer::hasShadow() { return shadow; }

void ygl::Renderer::drawGUI() {
	ImGui::Begin("Renderer Settings");

	ImGui::InputInt("Render Mode", (int *)&renderMode);
	ImGui::SeparatorText("Screen Effects");
	for (uint i = 0; i < effects.size(); ++i) {
		ImGui::Checkbox("Effect", &(effects[i]->enabled));
	}

	ImGui::SeparatorText("Texture Debug View");
	static int textureViewIndex = 1;
	ImGui::InputInt("Texture ID", &textureViewIndex);
	ImGui::Image((void *)(std::size_t)textureViewIndex, ImVec2(256, 256));

	ImGui::End();
}

void ygl::Renderer::drawMaterialEditor() {
	static int materialIndex = 0;
	ImGui::Begin("Material Editor");
	ImGui::InputInt("material index", &materialIndex);

	if (materialIndex < materials.size() && materialIndex > 0) {
		getMaterial(materialIndex).drawImGui();
	} else {
		ImGui::Text("Invalid Material Index");
	}

	ImGui::End();
}

void ygl::Renderer::write(std::ostream &out) {
	out.write((char *)&defaultShader, sizeof(defaultShader));

	std::size_t materialsCount = materials.size();
	out.write((char *)&materialsCount, sizeof(materialsCount));
	for (std::size_t i = 0; i < materialsCount; ++i) {
		out.write((char *)&materials[i], sizeof(Material));
	}

	std::size_t lightsCount = lights.size();
	out.write((char *)&lightsCount, sizeof(lightsCount));
	for (std::size_t i = 0; i < lightsCount; ++i) {
		out.write((char *)&lights[i], sizeof(Light));
	}
}

void ygl::Renderer::read(std::istream &in) {
	in.read((char *)&defaultShader, sizeof(defaultShader));

	std::size_t materialsCount;
	in.read((char *)&materialsCount, sizeof(materialsCount));
	materials.resize(materialsCount);
	for (std::size_t i = 0; i < materialsCount; ++i) {
		in.read((char *)&materials[i], sizeof(Material));
	}

	std::size_t lightsCount;
	in.read((char *)&lightsCount, sizeof(lightsCount));
	lights.resize(lightsCount);
	for (std::size_t i = 0; i < lightsCount; ++i) {
		in.read((char *)&lights[i], sizeof(Light));
	}
	loadData();
}

const char *ygl::Renderer::name			 = "ygl::Renderer";
const char *ygl::RendererComponent::name = "ygl::RendererComponent";

std::ostream &ygl::operator<<(std::ostream &out, const ygl::RendererComponent &rhs) {
	return out << "shader: " << rhs.shaderIndex << " material: " << rhs.materialIndex << " mesh: " << rhs.meshIndex
			   << "shadowShader: " << rhs.shadowShaderIndex;
}
