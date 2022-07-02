#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ygl {
class Shader {
	GLuint		 program;
	int			 shadersCount;
	GLuint	   *shaders;
	const char **fileNames;

	std::unordered_map<std::string, GLint> uniforms;
	std::unordered_map<std::string, GLint> SSBOs;
	std::unordered_map<std::string, GLint> UBOs;

	static constexpr const char *DEFAULT_INCLUDE_DIRECTORY = "./shaders/include/";

	void loadSourceRecursively(std::vector<std::string> &lines, const char *file, const char *includeDir,
							   int includeDirLength);

   protected:
	Shader(std::initializer_list<const char *> files);

	void createShader(GLenum type, GLuint target);
	void attachShaders();

	bool checkLinkStatus();
	bool checkValidateStatus();
	bool checkCompileStatus(int target);

	void loadSource(const char *file, const char *includeDir, char *&source, int &length);
	void loadSource(const char *file, char *&source, int &length);

	void finishProgramCreation();
	void deleteShaders();

   public:
	~Shader();

	void		bind();
	static void unbind();

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

	void  createSSBO(std::string &name, GLuint binding);
	void  createUBO(std::string &name, GLuint binding);
	void  setSSBO(std::string &name, GLuint bufferId);
	void  setUBO(std::string &name, GLuint bufferId);
	GLint getSSBOBinding(std::string &name);
	GLint getUBOBinding(std::string &name);

	static void setSSBO(GLuint bufferId, GLuint binding);
	static void setUBO(GLuint bufferId, GLuint binding);
};

class VFShader : public Shader {
   public:
	VFShader(const char *vertex, const char *fragment);
};
}	  // namespace ygl