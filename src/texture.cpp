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
	glTexImage2D(GL_TEXTURE_2D, (GLuint)0, GL_RGBA32F, width, height, (GLint)0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);	

	stbi_image_free(data);
}

void ygl::Texture2d::save(std::string fileName) {
	float *buff = new float[width * height * channelCount];

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

void ygl::Texture2d::unbind(int textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ygl::Texture2d::bindImage(int unit) { glBindImageTexture(unit, id, 0, false, 0, GL_READ_WRITE, GL_RGBA32F); }
void ygl::Texture2d::unbindImage(int unit) { glBindImageTexture(unit, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F); }
int	 ygl::Texture2d::getID() { return id; }
ygl::Texture2d::~Texture2d() { glDeleteTextures(1, &id); }