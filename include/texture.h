#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <stb/stb_image.h>

#include <yoghurtgl.h>

namespace ygl {
class ITexture {
   public:
	ITexture(){};
	virtual void save(std::string fileName) = 0;

	virtual void bind(int textureUnit) = 0;
	virtual void bind() { bind(GL_TEXTURE0); }

	virtual void unbind(int textureUnit) = 0;
	virtual void unbind() { unbind(GL_TEXTURE0); }

	virtual void bindImage(int unit)   = 0;
	virtual void unbindImage(int unit) = 0;

	virtual int getID() = 0;
	virtual ~ITexture() {};
};

class Texture2d : public ITexture {
	GLsizei width = -1, height = -1;
	int		channelCount = 4;	  // TODO: add a sensible way to change this
	GLuint	id			 = -1;

   public:
	Texture2d(){};
	Texture2d(GLsizei width, GLsizei height);
	Texture2d(GLsizei width, GLsizei height, GLint internalFormat, GLenum format);
	Texture2d(std::string fileName);
	void save(std::string filename) override;
	void bind(int textureUnit) override;
	void bind() override;
	void unbind(int textureUnit) override;
	void unbind() override;
	void bindImage(int unit) override;
	void unbindImage(int unit) override;
	int	 getID() override;
	virtual ~Texture2d();
};

class TextureCubemap : public ITexture {
	static const constexpr char* faces[] = {"right", "left", "top", "bottom", "front", "back"};

	GLsizei width = -1, height = -1;
	GLuint	id = -1;
	int		channels = 4;

   public:
	TextureCubemap() {}
	TextureCubemap(std::string path, std::string format);

	void save(std::string fileName) override;
	void bind(int textureUnit) override;
	void bind() override;
	void unbind(int textureUnit) override;
	void unbind() override;
	void bindImage(int textureUnit) override;
	void unbindImage(int textureUnit) override;
	int getID() override;

	virtual ~TextureCubemap() {};
};


}	  // namespace ygl