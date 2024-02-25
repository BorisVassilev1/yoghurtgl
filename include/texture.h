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
class TexIndex {
   public:
	enum {
		COLOR		   = GL_TEXTURE1,
		NORMAL		   = GL_TEXTURE2,
		HEIGHT		   = GL_TEXTURE3,
		ROUGHNESS	   = GL_TEXTURE4,
		AO			   = GL_TEXTURE5,
		EMISSION	   = GL_TEXTURE6,
		METALLIC	   = GL_TEXTURE10,
		SKYBOX		   = GL_TEXTURE11,
		IRRADIANCE_MAP = GL_TEXTURE12,
		PREFILTER_MAP  = GL_TEXTURE13,
		BDRF_MAP	   = GL_TEXTURE14,
		SHADOW_MAP	   = GL_TEXTURE15
	};
};

class FrameBuffer;
class FrameBufferAttachable {
   public:
	virtual void BindToFrameBuffer(const FrameBuffer &fb, GLenum attachment, uint image, uint level) = 0;
	virtual ~FrameBufferAttachable(){};
	virtual void resize(uint width, uint height) = 0;
};

/// human-readable texture types, alias for several separate properties
enum TextureType {
	RGB32F,
	RGB16F,
	RGBA32F,
	RGBA16F,
	SRGBA8,
	SRGB8,
	GREY32F,
	GREY16F,
	DEPTH_STENCIL_32F_8,
	DEPTH_24,
	HDR_CUBEMAP,
	RG16F,
	DIFFUSE	  = SRGB8,
	NORMAL	  = RGB16F,
	ROUGHNESS = SRGB8,
	METALLIC  = SRGB8,
	AO		  = SRGB8,
	EMISSIVE  = SRGB8,
};

void getTypeParameters(TextureType type, GLint &internalFormat, GLenum &format, uint8_t &pixelSize, uint8_t &components,
					   GLenum &_type);

/**
 * @brief Texture interface.
 */
class ITexture : public ISerializable, public FrameBufferAttachable {
   public:
	ITexture(){};
	DELETE_COPY_AND_ASSIGNMENT(ITexture)
	/**
	 * @brief save texture to file
	 *
	 * @param fileName - file to write to
	 */
	virtual void save(std::string fileName) = 0;

	virtual void bind(int textureUnit) const = 0;
	virtual void bind() const { bind(GL_TEXTURE0); }

	virtual void unbind(int textureUnit) const = 0;
	virtual void unbind() const { unbind(GL_TEXTURE0); }

	virtual void bindImage(int unit)   = 0;
	virtual void unbindImage(int unit) = 0;

	virtual int getID() = 0;
	virtual ~ITexture(){};
};

class RenderBuffer : public FrameBufferAttachable {
	GLuint		id	  = -1;
	GLsizei		width = -1, height = -1;
	TextureType type;

	void init(GLsizei width, GLsizei height, GLint internalFormat);

   public:
	RenderBuffer(GLsizei width, GLsizei height, TextureType type);
	~RenderBuffer();

	int getID();

	void BindToFrameBuffer(const FrameBuffer &fb, GLenum attachment, uint image, uint level) override;

	void bind();
	void unbind();

	void resize(uint width, uint height) override;
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
	GLuint		id	  = -1;
	GLsizei		width = -1, height = -1;
	uint8_t		pixelSize  = 16;
	uint8_t		components = 4;
	TextureType type;
	std::string fileName = "";

	void init(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components,
			  GLenum type, void *data);
	void init(GLsizei width, GLsizei height, TextureType type, void *data);
	void init(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components,
			  GLenum type);

   public:
	static const char *name;
	Texture2d(){};

	Texture2d(GLsizei width, GLsizei height, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components,
			  GLenum type, void *data);
	Texture2d(GLsizei width, GLsizei height, TextureType type, void *data);
	Texture2d(GLsizei width, GLsizei height);
	Texture2d(std::string fileName, GLint internalFormat, GLenum format, uint8_t pixelSize, uint8_t components,
			  GLenum type);
	Texture2d(std::string fileName, TextureType type);
	Texture2d(std::string fileName);
	Texture2d(std::istream &in);

	void save(std::string filename) override;
	void bind(int textureUnit) const override;
	void bind() const override;
	void unbind(int textureUnit) const override;
	void unbind() const override;
	void bindImage(int unit) override;
	void unbindImage(int unit) override;
	int	 getID() override;
	virtual ~Texture2d();

	void serialize(std::ostream &out) override;
	void BindToFrameBuffer(const FrameBuffer &fb, GLenum attachment, uint image, uint level) override;

	void resize(uint width, uint height) override;
};

/**
 * @brief CubeMap Texture - six textures that wrap arround a cube.
 */
class TextureCubemap : public ITexture {
	static const constexpr char *faces[] = {"right", "left", "top", "bottom", "front", "back"};

	GLuint		id	  = -1;
	GLsizei		width = -1, height = -1;
	int			channels = 4;
	std::string path;
	std::string format;

	void loadHDRCubemap();
	void loadCubemap();
	void loadEmptyCubemap();
	void init();

   public:
	static const char *name;
	TextureCubemap() {}
	TextureCubemap(uint32_t width, uint32_t height);
	TextureCubemap(const std::string &path, const std::string &format);
	TextureCubemap(std::istream &in);

	void save(std::string fileName) override;
	void bind(int textureUnit) const override;
	void bind() const override;
	void unbind(int textureUnit) const override;
	void unbind() const override;
	void bindImage(int textureUnit) override;
	void unbindImage(int textureUnit) override;
	int	 getID() override;

	virtual ~TextureCubemap();

	void serialize(std::ostream &out) override;
	void BindToFrameBuffer(const FrameBuffer &fb, GLenum attachment, uint image, uint level) override;

	void resize(uint width, uint height) override;
};

ygl::TextureCubemap *createIrradianceCubemap(const TextureCubemap *hdrCubemap);

ygl::TextureCubemap *createPrefilterCubemap(const TextureCubemap *hdrCubemap);

ygl::Texture2d *createBRDFTexture();

}	  // namespace ygl
