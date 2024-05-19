#pragma once

#include <yoghurtgl.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <serializable.h>

/**
 * @file shader.h
 * @brief Wrappers for OpenGL shaders.
 */

namespace ygl {
/**
 * @brief Base Shader class
 */
class Shader : public ISerializable {
	Shader() {}

   protected:
	GLuint		 program	  = 0;
	uint		 shadersCount = 0;
	GLuint		*shaders	  = nullptr;
	const char **fileNames	  = nullptr;
	bool		 bound		  = false;

	std::unordered_map<std::string, GLint> uniforms;
	std::unordered_map<std::string, GLint> SSBOs;
	std::unordered_map<std::string, GLint> UBOs;

	static constexpr const char *DEFAULT_INCLUDE_DIRECTORY = "./shaders/include/";

	void loadSourceRecursively(std::vector<std::string> &lines, const char *file, const char *includeDir,
							   int includeDirLength);

	void init(std::vector<std::string> files);
	Shader(std::initializer_list<std::string> files);
	Shader(std::istream &in);

	void createShader(GLenum type, GLuint target, const char *file = nullptr);
	void attachShaders();

	bool checkLinkStatus();
	bool checkValidateStatus();
	bool checkCompileStatus(int target);

	void loadSource(const char *file, GLenum type, const char *includeDir, char *&source, int &length);
	void loadSource(const char *file, GLenum type, char *&source, int &length);

	void finishProgramCreation();
	void deleteShaders();
	void detachShaders();

	Shader(const Shader &other)			   = delete;
	Shader &operator=(const Shader &other) = delete;

   public:
	~Shader();

	void bind();
	void unbind();

	void createUniform(const char *uniformName);
	void registerUniform(const std::string &name, int location);

	void detectUniforms();
	void detectBlockUniforms();
	void detectStorageBlocks();

	GLuint getUniformLocation(const std::string &uniformName);
	bool   hasUniform(const std::string &uniformName);

	template <class T>
	void setUniform(const std::string &uniformName, T value) {
		setUniform(getUniformLocation(uniformName), value);
	}

	void setUniform(GLuint location, GLboolean value);
	void setUniform(GLuint location, GLint value);
	void setUniform(GLuint location, GLuint value);
	void setUniform(GLuint location, glm::mat4x4 value);
	void setUniform(GLuint location, glm::vec4 value);
	void setUniform(GLuint location, glm::vec3 value);
	void setUniform(GLuint location, glm::vec2 value);
	void setUniform(GLuint location, glm::ivec2 value);
	void setUniform(GLuint location, GLfloat value);
	void setUniform(GLuint location, GLdouble value);

	void  createSSBO(std::string &name, GLuint binding);
	void  createUBO(std::string &name, GLuint binding);
	void  setSSBO(std::string &name, GLuint bufferId);
	void  setUBO(std::string &name, GLuint bufferId);
	GLint getSSBOBinding(std::string &name);
	GLint getUBOBinding(std::string &name);

	bool isBound();

	static void setSSBO(GLuint bufferId, GLuint binding);
	static void setUBO(GLuint bufferId, GLuint binding);

	void serialize(std::ostream &out) override;
};

/**
 * @brief Vertex-Fragment Shader. Links a Vertex Shader and a Fragment Shader into a program. See OpenGL wiki.
 */
class VFShader : public Shader {
   public:
	static const char *name;
	VFShader(const char *vertex, const char *fragment);
	VFShader(std::istream &in);

	void serialize(std::ostream &out) override;
};

/**
 * @brief Compute Shader. See OpenGl wiki.
 */
class ComputeShader : public Shader {
   public:
	static const char *name;
	glm::ivec3		   groupSize;
	ComputeShader(const char *source);
	ComputeShader(std::istream &in);

	void serialize(std::ostream &out) override;
};
}	  // namespace ygl
