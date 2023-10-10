#include <texture.h>
#include <mesh.h>
#include <shader.h>
#include <istream>
#include <cstring>
#include <renderer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void ygl::getTypeParameters(TextureType type, GLint &internalFormat, GLenum &format, uint8_t &pixelSize,
							uint8_t &components, GLenum &_type) {
	switch (type) {
		case TextureType::RGBA32F: {
			internalFormat = GL_RGBA32F;
			format		   = GL_RGBA;
			pixelSize	   = 16;
			components	   = 4;
			_type		   = GL_FLOAT;
			break;
		}
		case TextureType::RGBA16F: {
			internalFormat = GL_RGBA16F;
			format		   = GL_RGBA;
			pixelSize	   = 16;
			components	   = 4;
			_type		   = GL_FLOAT;
			break;
		}
		case TextureType::RGB32F: {
			internalFormat = GL_RGB32F;
			format		   = GL_RGB;
			pixelSize	   = 12;
			components	   = 3;
			_type		   = GL_FLOAT;
			break;
		}
		case TextureType::RGB16F: {
			internalFormat = GL_RGB16F;
			format		   = GL_RGB;
			pixelSize	   = 12;
			components	   = 3;
			_type		   = GL_FLOAT;
			break;
		}
		case TextureType::SRGBA8: {
			internalFormat = GL_SRGB8_ALPHA8;
			format		   = GL_RGBA;
			pixelSize	   = 16;
			components	   = 4;
			_type		   = GL_UNSIGNED_BYTE;
			break;
		}
		case TextureType::SRGB8: {
			internalFormat = GL_SRGB8;
			format		   = GL_RGB;
			pixelSize	   = 12;
			components	   = 3;
			_type		   = GL_UNSIGNED_BYTE;
			break;
		}
		case TextureType::GREY32F: {
			internalFormat = GL_R32F;
			format		   = GL_RED;
			pixelSize	   = 4;
			components	   = 1;
			_type		   = GL_FLOAT;
			break;
		}
		case TextureType::GREY16F: {
			internalFormat = GL_R16F;
			format		   = GL_RED;
			pixelSize	   = 4;
			components	   = 1;
			_type		   = GL_FLOAT;
			break;
		}
		case TextureType::DEPTH_STENCIL_32F_8: {
			internalFormat = GL_DEPTH32F_STENCIL8;
			format		   = GL_DEPTH_STENCIL;
			pixelSize	   = 2;
			components	   = 1;
			_type		   = GL_UNSIGNED_INT_24_8;
			break;
		}
		case TextureType::DEPTH_24: {	   // TODO: this may be broken for texture2d
			internalFormat = GL_DEPTH_COMPONENT24;
			format		   = GL_DEPTH;
			pixelSize	   = 2;
			components	   = 1;
			_type		   = GL_UNSIGNED_INT_24_8;
			break;
		}
		default: {
			dbLog(ygl::LOG_WARNING, "unknown texture type ", type, ". default texture type will be RGBA32f");
			internalFormat = GL_RGBA32F;
			format		   = GL_RGBA;
			pixelSize	   = 16;
			components	   = 4;
			_type		   = GL_FLOAT;
			break;
		}
	}
}

void ygl::RenderBuffer::init(GLsizei width, GLsizei height, GLint internalFormat) {
	glGenRenderbuffers(1, &id);
	glBindRenderbuffer(GL_RENDERBUFFER, id);
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

ygl::RenderBuffer::RenderBuffer(GLsizei width, GLsizei height, TextureType type)
	: width(width), height(height), type(type) {
	GLint	internalFormat = 0;
	GLenum	format		   = 0;
	uint8_t pixelSize	   = 0;
	uint8_t components	   = 0;
	GLenum	_type		   = 0;

	getTypeParameters(type, internalFormat, format, pixelSize, components, _type);
	init(width, height, internalFormat);
}

ygl::RenderBuffer::~RenderBuffer() { glDeleteRenderbuffers(1, &id); }

void ygl::RenderBuffer::BindToFrameBuffer(const ygl::FrameBuffer &fb, GLenum attachment, uint image, uint level) {
	(void) image;
	(void)level;
	fb.bind();
	glBindRenderbuffer(GL_RENDERBUFFER, id);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, id);
}

void ygl::Texture2d::init(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize,
						  uint8_t components, GLenum type, void *data) {
	this->width		 = width;
	this->height	 = height;
	this->pixelSize	 = pixelSize;
	this->components = components;

	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexImage2D(GL_TEXTURE_2D, (GLuint)0, internalFormat, width, height, (GLint)0, format, type, data);
	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
	if (data != nullptr) { glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data); }

	if (format != GL_STENCIL_INDEX) glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ygl::Texture2d::init(GLsizei width, GLsizei height, TextureType type, void *data) {
	GLint	internalFormat = 0;
	GLenum	format		   = 0;
	uint8_t pixelSize	   = 0;
	uint8_t components	   = 0;
	GLenum	_type		   = 0;

	getTypeParameters(type, internalFormat, format, pixelSize, components, _type);

	init(width, height, internalFormat, format, pixelSize, components, _type, data);
}

void ygl::Texture2d::init(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize,
						  uint8_t components, GLenum type) {
	stbi_set_flip_vertically_on_load(true);
	GLsizei width, height, channelCount;
	void   *data;
	if (type == GL_UNSIGNED_BYTE) {
		data = stbi_load(fileName.c_str(), &width, &height, &channelCount, components);
	} else if (type == GL_FLOAT) {
		data = stbi_loadf(fileName.c_str(), &width, &height, &channelCount, components);
	} else {
		dbLog(ygl::LOG_ERROR, "Image file [" + fileName + "] failed to load: ");
		return;
	}
	stbi_set_flip_vertically_on_load(false);

	if (data != nullptr) {
		init(width, height, internalFormat, format, pixelSize, components, type, data);
		stbi_image_free(data);
	} else {
		dbLog(ygl::LOG_ERROR, "Image file [" + fileName + "] failed to load: " + stbi_failure_reason());
		width = height = 2;
		data		   = new stbi_uc[16]{0, 0, 0, 255, 255, 0, 255, 255, 255, 0, 255, 255, 0, 0, 0, 255};
		init(2, 2, TextureType::RGBA16F, data);
		delete[] (stbi_uc *)data;
	}
}

ygl::Texture2d::Texture2d(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize,
						  uint8_t components, GLenum type, void *data) {
	init(width, height, internalFormat, format, pixelSize, components, type, data);
}

ygl::Texture2d::Texture2d(GLsizei width, GLsizei height, TextureType type, void *data)
	: width(width), height(height), type(type) {
	init(width, height, type, data);
}

ygl::Texture2d::Texture2d(GLsizei width, GLsizei height) { init(width, height, TextureType::RGBA16F, nullptr); }

ygl::Texture2d::Texture2d(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize,
						  uint8_t components, GLenum type)
	: fileName(fileName) {
	init(fileName, internalFormat, format, pixelSize, components, type);
}

ygl::Texture2d::Texture2d(std::string fileName, TextureType type) : type(type), fileName(fileName) {
	GLint	internalFormat = 0;
	GLenum	format		   = 0;
	uint8_t pixelSize	   = 0;
	uint8_t components	   = 0;
	GLenum	_type		   = 0;

	getTypeParameters(type, internalFormat, format, pixelSize, components, _type);

	init(fileName, internalFormat, format, pixelSize, components, _type);
}

ygl::Texture2d::Texture2d(std::string fileName) : Texture2d(fileName, TextureType::RGBA16F) {}

ygl::Texture2d::Texture2d(std::istream &in) {
	std::getline(in, fileName, '\0');
	in.read((char *)&type, sizeof(TextureType));

	GLint	internalFormat = 0;
	GLenum	format		   = 0;
	uint8_t pixelSize	   = 0;
	uint8_t components	   = 0;
	GLenum	_type		   = 0;

	getTypeParameters(type, internalFormat, format, pixelSize, components, _type);

	init(fileName, internalFormat, format, pixelSize, components, _type);
}

void ygl::Texture2d::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	out.write(fileName.c_str(), fileName.size() + 1);
	out.write((char *)&type, sizeof(TextureType));
}

void ygl::Texture2d::BindToFrameBuffer(const FrameBuffer &fb, GLenum attachment, uint image, uint level) {
	(void)image;
	fb.bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, id, level);
}

void ygl::Texture2d::save(std::string fileName) {
	uint8_t *buff = new uint8_t[width * height * pixelSize];

	glBindTexture(GL_TEXTURE_2D, id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buff);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_flip_vertically_on_write(true);

	stbi_write_png(fileName.c_str(), width, height, 4, buff, 4 * width);
}

void ygl::Texture2d::bind(int textureUnit) const {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, id);
	glActiveTexture(GL_TEXTURE0);
}

void ygl::Texture2d::bind() const { bind(GL_TEXTURE0); }

void ygl::Texture2d::unbind(int textureUnit) const {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
}

void ygl::Texture2d::unbind() const { unbind(GL_TEXTURE0); }

void ygl::Texture2d::bindImage(int unit) { glBindImageTexture(unit, id, 0, false, 0, GL_READ_WRITE, GL_RGBA32F); }
void ygl::Texture2d::unbindImage(int unit) { glBindImageTexture(unit, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F); }
int	 ygl::Texture2d::getID() { return id; }
ygl::Texture2d::~Texture2d() { glDeleteTextures(1, &id); }

ygl::TextureCubemap::TextureCubemap(uint32_t width, uint32_t height) {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB16F, width, height);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void ygl::TextureCubemap::loadCubemap() {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for (int i = 0; i < 6; i++) {
		std::string wholePath = path + "/" + faces[i] + format;
		stbi_set_flip_vertically_on_load(false);
		auto buff = stbi_load(wholePath.c_str(), &width, &height, &channels, 4);
		if (buff == nullptr) {
			dbLog(ygl::LOG_ERROR, "Image file [" + wholePath + "] failed to load: " + stbi_failure_reason());

			width = height		= 2;
			unsigned char tex[] = {0, 0, 0, 255, 255, 0, 255, 255, 255, 0, 255, 255, 0, 0, 0, 255};

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB_ALPHA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
		} else {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA,
						 GL_UNSIGNED_BYTE, buff);
		}
		stbi_image_free(buff);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void ygl::TextureCubemap::init() {
	loadCubemap();
}

ygl::TextureCubemap::TextureCubemap(const std::string &path, const std::string &format) : path(path), format(format) {
	init();
}

ygl::TextureCubemap::TextureCubemap(std::istream &in) {
	std::getline(in, path, '\0');
	std::getline(in, format, '\0');
	init();
}

void ygl::TextureCubemap::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	out.write(path.c_str(), path.size() + 1);
	out.write(format.c_str(), format.size() + 1);
}

void ygl::TextureCubemap::BindToFrameBuffer(const FrameBuffer &fb, GLenum attachment, uint image, uint level) {
	fb.bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + image, id, level);
}

void ygl::TextureCubemap::save(std::string) {
	// TODO: implement this
}

void ygl::TextureCubemap::bind(int textureUnit) const {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void ygl::TextureCubemap::bind() const { bind(GL_TEXTURE0); }

void ygl::TextureCubemap::unbind(int textureUnit) const {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void ygl::TextureCubemap::unbind() const { unbind(GL_TEXTURE0); }

void ygl::TextureCubemap::bindImage(int textureUnit) {
	glBindImageTexture(textureUnit, id, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
}

void ygl::TextureCubemap::unbindImage(int textureUnit) {
	glBindImageTexture(textureUnit, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
}

int ygl::TextureCubemap::getID() { return id; }
ygl::TextureCubemap::~TextureCubemap() { glDeleteTextures(1, &id); }

const char *ygl::Texture2d::name	  = "ygl::Texture2d";
const char *ygl::TextureCubemap::name = "ygl::TextureCubemap";

ygl::TextureCubemap *ygl::loadHDRCubemap(const std::string &path, const std::string &format) {
	uint width = 1024, height = 1024;

	ygl::Texture2d		*hdrTexture = new ygl::Texture2d(path + format, ygl::TextureType::HDR_CUBEMAP);
	ygl::TextureCubemap *cubemap	= new TextureCubemap(width, height);

	ygl::VFShader *parsingShader =
		new ygl::VFShader("./shaders/equirectangularToCubemap.vs", "./shaders/equirectangularToCubemap.fs");
	ygl::Mesh *cubeMesh = new ygl::BoxMesh();
	cubeMesh->setCullFace(false);

	ygl::RenderBuffer *depthBuffer = new ygl::RenderBuffer(width, height, TextureType::DEPTH_24);
	ygl::FrameBuffer  *fb =
		new FrameBuffer(nullptr, GL_COLOR_ATTACHMENT0, depthBuffer, GL_DEPTH_ATTACHMENT, "hdr_convert_fb");

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[]	= {
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

	hdrTexture->bind();
	fb->bind();

	parsingShader->bind();
	parsingShader->setUniform("projection", captureProjection);
	parsingShader->setUniform("equirectangularMap", 0);

	glViewport(0, 0, width, height);

	for (unsigned int i = 0; i < 6; ++i) {
		parsingShader->bind();
		parsingShader->setUniform("view", captureViews[i]);
		cubemap->BindToFrameBuffer(*fb, GL_COLOR_ATTACHMENT0, i, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Renderer::drawObject(parsingShader, cubeMesh);
	}

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	hdrTexture->unbind();
	cubemap->unbind();
	fb->unbind();

	delete hdrTexture;
	delete parsingShader;
	delete cubeMesh;

	return cubemap;
}

ygl::TextureCubemap *ygl::createIrradianceCubemap(const ygl::TextureCubemap *hdrCubemap) {
	uint width = 32, height = 32;

	ygl::TextureCubemap *cubemap	= new TextureCubemap(width, height);

	ygl::VFShader *parsingShader =
		new ygl::VFShader("./shaders/computeIrradiance.vs", "./shaders/computeIrradiance.fs");
	ygl::Mesh *cubeMesh = new ygl::BoxMesh();
	cubeMesh->setCullFace(false);

	ygl::RenderBuffer *depthBuffer = new ygl::RenderBuffer(width, height, TextureType::DEPTH_24);
	ygl::FrameBuffer  *fb =
		new FrameBuffer(nullptr, GL_COLOR_ATTACHMENT0, depthBuffer, GL_DEPTH_ATTACHMENT, "hdr_irradiance_fb");

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[]	= {
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

	fb->bind();
	hdrCubemap->bind();

	parsingShader->bind();
	parsingShader->setUniform("projection", captureProjection);
	parsingShader->setUniform("environmentMap", 0);

	glViewport(0, 0, width, height);

	for (unsigned int i = 0; i < 6; ++i) {
		parsingShader->bind();
		parsingShader->setUniform("view", captureViews[i]);
		cubemap->BindToFrameBuffer(*fb, GL_COLOR_ATTACHMENT0, i, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Renderer::drawObject(parsingShader, cubeMesh);
	}

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	cubemap->unbind();
	fb->unbind();

	delete parsingShader;
	delete cubeMesh;

	return cubemap;
}

