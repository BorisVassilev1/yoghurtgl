#pragma once

#include <yoghurtgl.h>

#include <istream>
#include <string>
#include <type_traits>
#include <serializable.h>
#include <stb_image.h>

/**
 * @file texture.h
 * @brief Wrappers for OpenGL textures.
 */

namespace ygl {
/**
 * @brief different types of textues must be bound to different targets. Enum makes them more readable
 */
enum TexIndex {
	COLOR	  = GL_TEXTURE1,
	NORMAL	  = GL_TEXTURE2,
	HEIGHT	  = GL_TEXTURE3,
	ROUGHNESS = GL_TEXTURE4,
	AO		  = GL_TEXTURE5,
	EMISSION  = GL_TEXTURE6,
	METALLIC  = GL_TEXTURE10
};

/**
 * @brief Texture interface.
 */
class ITexture : public ISerializable {
   public:
	/// human-readable texture types, alias for several separate properties
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
	ITexture(const ITexture &other)			   = delete;
	ITexture &operator=(const ITexture &other) = delete;
	/**
	 * @brief save texture to file
	 *
	 * @param fileName - file to write to
	 */
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

/**
 * @brief a Texture inherits ygl::ITexture
 *
 * @tparam T - type
 */
template <class T>
concept IsTexture = std::is_base_of<ygl::ITexture, T>::value;

/**
 * @brief 2D Texture.
 */
class Texture2d : public ITexture {
	GLsizei		width = -1, height = -1;
	uint8_t		pixelSize  = 16;
	uint8_t		components = 4;
	GLuint		id		   = -1;
	Type		type;
	std::string fileName = "";

	void init(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components,
			  GLenum type, stbi_uc *data);
	void init(GLsizei width, GLsizei height, Type type, stbi_uc *data);
	void init(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components);

   public:
	static const char *name;
	Texture2d(){};

	Texture2d(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components,
			  GLenum type, stbi_uc *data);
	Texture2d(GLsizei width, GLsizei height, Type type, stbi_uc *data);
	Texture2d(GLsizei width, GLsizei height);
	Texture2d(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components);
	Texture2d(std::string fileName, Type type);
	Texture2d(std::string fileName);
	Texture2d(std::istream &in);

	void save(std::string filename) override;
	void bind(int textureUnit) override;
	void bind() override;
	void unbind(int textureUnit) override;
	void unbind() override;
	void bindImage(int unit) override;
	void unbindImage(int unit) override;
	int	 getID() override;
	virtual ~Texture2d();

	void serialize(std::ostream &out) override;
};

/**
 * @brief CubeMap Texture - six textures that wrap arround a cube.
 */
class TextureCubemap : public ITexture {
	static const constexpr char *faces[] = {"right", "left", "top", "bottom", "front", "back"};

	GLsizei		width = -1, height = -1;
	GLuint		id		 = -1;
	int			channels = 4;
	std::string path;
	std::string format;

	void init();

   public:
	static const char *name;
	TextureCubemap() {}
	TextureCubemap(const std::string &path, const std::string &format);
	TextureCubemap(std::istream &in);

	void save(std::string fileName) override;
	void bind(int textureUnit) override;
	void bind() override;
	void unbind(int textureUnit) override;
	void unbind() override;
	void bindImage(int textureUnit) override;
	void unbindImage(int textureUnit) override;
	int	 getID() override;

	virtual ~TextureCubemap(){};

	void serialize(std::ostream &out) override;
};

}	  // namespace ygl
