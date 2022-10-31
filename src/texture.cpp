#include <texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

void ygl::ITexture::getTypeParameters(Type type, GLint &internalFormat, GLenum &format, uint8_t &pixelSize,
									  uint8_t &components, GLenum &_type) {
	switch (type) {
		case Type::RGBA: {
			internalFormat = GL_RGBA32F;
			format		   = GL_RGBA;
			pixelSize	   = 16;
			components	   = 4;
			_type		   = GL_UNSIGNED_BYTE;
			break;
		}
		case Type::RGB: {
			internalFormat = GL_RGB32F;
			format		   = GL_RGB;
			pixelSize	   = 12;
			components	   = 3;
			_type		   = GL_UNSIGNED_BYTE;
			break;
		}
		case Type::SRGBA: {
			internalFormat = GL_SRGB8_ALPHA8;
			format		   = GL_RGBA;
			pixelSize	   = 16;
			components	   = 4;
			_type		   = GL_UNSIGNED_BYTE;
			break;
		}
		case Type::SRGB: {
			internalFormat = GL_SRGB8;
			format		   = GL_RGB;
			pixelSize	   = 12;
			components	   = 3;
			_type		   = GL_UNSIGNED_BYTE;
			break;
		}
		case Type::GREY: {
			internalFormat = GL_R32F;
			format		   = GL_RED;
			pixelSize	   = 4;
			components	   = 1;
			_type		   = GL_UNSIGNED_BYTE;
			break;
		}
		case Type::DEPTH_STENCIL: {
			internalFormat = GL_DEPTH32F_STENCIL8;
			format		   = GL_DEPTH_STENCIL;
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
			_type		   = GL_UNSIGNED_BYTE;
			break;
		}
	}
}

void ygl::Texture2d::init(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize,
						  uint8_t components, GLenum type, stbi_uc *data) {
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
	if(data != nullptr) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
	}

	if (format != GL_STENCIL_INDEX) glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ygl::Texture2d::init(GLsizei width, GLsizei height, Type type, stbi_uc *data) {
	GLint	internalFormat = 0;
	GLenum	format		   = 0;
	uint8_t pixelSize	   = 0;
	uint8_t components	   = 0;
	GLenum	_type		   = 0;

	ITexture::getTypeParameters(type, internalFormat, format, pixelSize, components, _type);

	init(width, height, internalFormat, format, pixelSize, components, _type, data);
}

void ygl::Texture2d::init(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize,
						  uint8_t components) {
	stbi_set_flip_vertically_on_load(true);
	GLsizei	 width, height, channelCount;
	stbi_uc *data = stbi_load(fileName.c_str(), &width, &height, &channelCount, components);
	stbi_set_flip_vertically_on_load(false);

	if (data != nullptr) {
		init(width, height, internalFormat, format, pixelSize, components, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	} else {
		dbLog(ygl::LOG_ERROR, "Image file [" + fileName + "] failed to load: " + stbi_failure_reason());
		data = new stbi_uc[]{0, 0, 0, 255, 255, 0, 255, 255, 255, 0, 255, 255, 0, 0, 0, 255};
		init(width, height, Type::RGBA, data);
		delete[] data;
	}
}

ygl::Texture2d::Texture2d(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize,
						  uint8_t components, GLenum type, stbi_uc *data) {
	init(width, height, internalFormat, format, pixelSize, components, type, data);
}

ygl::Texture2d::Texture2d(GLsizei width, GLsizei height, Type type, stbi_uc *data) : width(width), height(height) {
	init(width, height, type, data);
}

ygl::Texture2d::Texture2d(GLsizei width, GLsizei height) { init(width, height, Type::RGBA, nullptr); }

ygl::Texture2d::Texture2d(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize,
						  uint8_t components) {
	init(fileName, internalFormat, format, pixelSize, components);
}

ygl::Texture2d::Texture2d(std::string fileName, Type type) {
	GLint	internalFormat = 0;
	GLenum	format		   = 0;
	uint8_t pixelSize	   = 0;
	uint8_t components	   = 0;
	GLenum	_type		   = 0;

	ITexture::getTypeParameters(type, internalFormat, format, pixelSize, components, _type);

	init(fileName, internalFormat, format, pixelSize, components);
}

ygl::Texture2d::Texture2d(std::string fileName) : Texture2d(fileName, Type::RGBA) {}

void ygl::Texture2d::save(std::string fileName) {
	uint8_t *buff = new uint8_t[width * height * pixelSize];

	glBindTexture(GL_TEXTURE_2D, id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buff);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_flip_vertically_on_write(true);

	stbi_write_png(fileName.c_str(), width, height, 4, buff, 4 * width);
}

void ygl::Texture2d::bind(int textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, id);
	glActiveTexture(GL_TEXTURE0);
}

void ygl::Texture2d::bind() { bind(GL_TEXTURE0); }

void ygl::Texture2d::unbind(int textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
}

void ygl::Texture2d::unbind() { unbind(GL_TEXTURE0); }

void ygl::Texture2d::bindImage(int unit) { glBindImageTexture(unit, id, 0, false, 0, GL_READ_WRITE, GL_RGBA32F); }
void ygl::Texture2d::unbindImage(int unit) { glBindImageTexture(unit, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F); }
int	 ygl::Texture2d::getID() { return id; }
ygl::Texture2d::~Texture2d() { glDeleteTextures(1, &id); }

ygl::TextureCubemap::TextureCubemap(std::string path, std::string format) {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for (int i = 0; i < 6; i++) {
		std::string wholePath = path + "/" + faces[i] + format;
		stbi_set_flip_vertically_on_load(false);
		auto buff = stbi_load(wholePath.c_str(), &width, &height, &channels, 4);
		if (buff == nullptr) {
			dbLog(ygl::LOG_ERROR, "Image file [" + wholePath + "] failed to load: " + stbi_failure_reason());

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

void ygl::TextureCubemap::save(std::string fileName) {
	// TODO: implement this
}

void ygl::TextureCubemap::bind(int textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void ygl::TextureCubemap::bind() { bind(GL_TEXTURE0); }

void ygl::TextureCubemap::unbind(int textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void ygl::TextureCubemap::unbind() { unbind(GL_TEXTURE0); }

void ygl::TextureCubemap::bindImage(int textureUnit) {
	glBindImageTexture(textureUnit, id, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
}

void ygl::TextureCubemap::unbindImage(int textureUnit) {
	glBindImageTexture(textureUnit, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
}

int ygl::TextureCubemap::getID() { return id; }

// ygl::TextureCubemap::~TextureCubemap() { glDeleteTextures(1, &id); }