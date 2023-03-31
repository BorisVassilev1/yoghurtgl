#pragma once

#include <yoghurtgl.h>

#include <string>
#include <stb_image.h>

namespace ygl {
enum TexIndex {
	COLOR	  = GL_TEXTURE1,
	NORMAL	  = GL_TEXTURE2,
	HEIGHT	  = GL_TEXTURE3,
	ROUGHNESS = GL_TEXTURE4,
	AO		  = GL_TEXTURE5,
	EMISSION  = GL_TEXTURE6,
	METALLIC  = GL_TEXTURE10
};
class ITexture {
   public:
	enum Type {
		RGB,
		RGBA,
		SRGBA,
		SRGB,
		GREY,
		DEPTH_STENCIL,
		DIFFUSE	  = SRGB,
		NORMAL	  = RGB,
		ROUGHNESS = SRGB,
		METALLIC  = SRGB,
		AO		  = SRGB,
		EMISSIVE  = SRGB
	};

	ITexture(){};
	virtual void save(std::string fileName) = 0;

	virtual void bind(int textureUnit) = 0;
	virtual void bind() { bind(GL_TEXTURE0); }

	virtual void unbind(int textureUnit) = 0;
	virtual void unbind() { unbind(GL_TEXTURE0); }

	virtual void bindImage(int unit)   = 0;
	virtual void unbindImage(int unit) = 0;

	virtual int getID() = 0;
	virtual ~ITexture(){};

   protected:
	static void getTypeParameters(Type type, GLint &internalFormat, GLenum &format, uint8_t &pixelSize,
								  uint8_t &components, GLenum &_type);
};

class Texture2d : public ITexture {
	GLsizei width = -1, height = -1;
	uint8_t pixelSize  = 16;
	uint8_t components = 4;
	GLuint	id		   = -1;

	void init(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components,
			  GLenum type, stbi_uc *data);
	void init(GLsizei width, GLsizei height, Type type, stbi_uc *data);
	void init(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components);

   public:
	Texture2d(){};

	Texture2d(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components,
			  GLenum type, stbi_uc *data);
	Texture2d(GLsizei width, GLsizei height, Type type, stbi_uc *data);
	Texture2d(GLsizei width, GLsizei height);
	Texture2d(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components);
	Texture2d(std::string fileName, Type type);
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
	static const constexpr char *faces[] = {"right", "left", "top", "bottom", "front", "back"};

	GLsizei width = -1, height = -1;
	GLuint	id		 = -1;
	int		channels = 4;

   public:
	TextureCubemap() {}
	TextureCubemap(const std::string &path, const std::string &format);

	void save(std::string fileName) override;
	void bind(int textureUnit) override;
	void bind() override;
	void unbind(int textureUnit) override;
	void unbind() override;
	void bindImage(int textureUnit) override;
	void unbindImage(int textureUnit) override;
	int	 getID() override;

	virtual ~TextureCubemap(){};
};

}	  // namespace ygl
