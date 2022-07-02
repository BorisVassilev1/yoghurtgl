#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

namespace ygl {
class ITexture {

   public:
	ITexture(){};
	virtual void save(std::string fileName) = 0;

	virtual void bind(int textureUnit) = 0;
	void bind() { bind(GL_TEXTURE0); }

	virtual void unbind(int textureUnit) = 0;
	void unbind() { unbind(GL_TEXTURE0); }

	virtual void bindImage(int unit) = 0;
	virtual void unbindImage(int unit) = 0;

	virtual int getID() = 0;
	~ITexture() {};
};

class Texture2d : public ITexture {
	GLsizei width, height;
	int channelCount = 4; // TODO: add a sensible way to change this
	GLuint		 id;

   public:
	Texture2d(GLsizei width, GLsizei height);
	Texture2d(GLsizei width, GLsizei height, GLint internalFormat, GLenum format);
	Texture2d(std::string fileName);
	void save(std::string filename) override;
	void bind(int textureUnit) override;
	void unbind(int textureUnit) override;
	void bindImage(int unit) override;
	void unbindImage(int unit) override;
	int	 getID() override;
	~Texture2d();
};

}	  // namespace ygl