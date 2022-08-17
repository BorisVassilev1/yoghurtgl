#include <texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

ygl::Texture2d::Texture2d(GLsizei width, GLsizei height) : Texture2d(width, height, GL_RGBA32F, GL_RGBA) {}

ygl::Texture2d::Texture2d(GLsizei width, GLsizei height, GLint internalFormat, GLenum format)
	: width(width), height(height) {
	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, (GLuint)0, internalFormat, width, height, (GLint)0, format, GL_UNSIGNED_BYTE, nullptr);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

ygl::Texture2d::Texture2d(std::string fileName) {
	stbi_uc* data = stbi_load(fileName.c_str(), &width, &height, &channelCount, 4);

	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (data != nullptr) {
		glTexImage2D(GL_TEXTURE_2D, (GLuint)0, GL_RGBA32F, width, height, (GLint)0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	} else {
		dbLog(ygl::LOG_ERROR, "Image file [" + fileName + "] failed to load: " + stbi_failure_reason());

		unsigned char tex[] = {0, 0, 0, 255, 255, 0, 255, 255, 255, 0, 255, 255, 0, 0, 0, 255};

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	}

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
}

void ygl::Texture2d::save(std::string fileName) {
	float* buff = new float[width * height * channelCount];

	glBindTexture(GL_TEXTURE_2D, id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buff);
	glBindTexture(GL_TEXTURE_2D, 0);

	// TODO: test this
	stbi_write_png(fileName.c_str(), width, height, channelCount, buff, channelCount * sizeof(GLfloat) * width);
}

void ygl::Texture2d::bind(int textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void ygl::Texture2d::bind() { bind(GL_TEXTURE0); }

void ygl::Texture2d::unbind(int textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ygl::Texture2d::unbind() { unbind(GL_TEXTURE0); }

void ygl::Texture2d::bindImage(int unit) { glBindImageTexture(unit, id, 0, false, 0, GL_READ_WRITE, GL_RGBA32F); }
void ygl::Texture2d::unbindImage(int unit) { glBindImageTexture(unit, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F); }
int	 ygl::Texture2d::getID() { return id; }
ygl::Texture2d::~Texture2d() { glDeleteTextures(1, &id); }

// const constexpr char* ygl::TextureCubemap::faces[] = {"right", "left", "top", "bottom", "front", "back"};

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