#include <shader.h>

#include <assert.h>
#include <iterator>
#include <sstream>
#include <string>
#include <string.h>
#include <string_view>

ygl::Shader::~Shader() {
	if (shaders != nullptr) {
		deleteShaders();
	}
	glDeleteProgram(program);

	if (fileNames != nullptr) {
		delete fileNames;
		fileNames = nullptr;
	}
}

void ygl::Shader::deleteShaders() {
	for (int i = 0; i < shadersCount; ++i) {
		glDeleteShader(shaders[i]);
	}
	delete shaders;
	shaders = nullptr;
}

ygl::Shader::Shader(std::initializer_list<const char *> files) {
	shadersCount = files.size();
	shaders		 = new GLuint[shadersCount];
	fileNames	 = new const char *[shadersCount];

	auto it = files.begin();
	for (int i = 0; i < shadersCount; ++i && ++it) {
		fileNames[i] = *it;
	}

	program = glCreateProgram();
}

void ygl::Shader::createShader(GLenum type, GLuint target) {
	const char   *file = fileNames[target];
	std::ifstream in(file);
	if (in.fail()) {
		std::cerr << "Error: failed opening file: " << file << std::endl;
	}

	int	  sh = glCreateShader(type);
	char *source;
	int	  length;
	loadSource(file, source, length);

	glShaderSource(sh, 1, &source, &length);
	delete source;
	glCompileShader(sh);
	shaders[target] = sh;

	assert(checkCompileStatus(target));
}

void ygl::Shader::attachShaders() {
	for (int i = 0; i < shadersCount; ++i) {
		glAttachShader(program, shaders[i]);
	}
}

bool ygl::Shader::checkLinkStatus() {
	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint programLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &programLogLength);
		char *programLog = new char[programLogLength + 1];
		glGetProgramInfoLog(program, programLogLength, NULL, programLog);

		std::cerr << "Error: Program Linking: \n"
				  << programLog << std::endl;

		delete programLog;
		return false;
	}
	return true;
}

bool ygl::Shader::checkValidateStatus() {
	glValidateProgram(program);
	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint programLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &programLogLength);
		char *programLog = new char[programLogLength + 1];
		glGetProgramInfoLog(program, programLogLength, NULL, programLog);

		std::cerr << "Error: Program Validation: \n"
				  << programLog << std::endl;

		delete programLog;
		return false;
	}
	return true;
}

bool ygl::Shader::checkCompileStatus(int target) {
	GLuint shader = shaders[target];
	GLint  status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint shaderLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shaderLogLength);
		char *shaderLog = new char[shaderLogLength + 1];
		glGetShaderInfoLog(shader, shaderLogLength, NULL, shaderLog);

		std::cerr << "Error: Shader Compilation - \n"
				  << fileNames[target] << " - " << shaderLog << std::endl;

		delete shaderLog;
		return false;
	}
	return true;
}

void ygl::Shader::loadSource(const char *file, const char *includeDir, char *&source, int &length) {
	std::vector<std::string> lines;
	loadSourceRecursively(lines, file, includeDir, strlen(includeDir));

	length = 0;
	for (std::string &s : lines) {
		length += s.length() + 1;
	}

	source = new char[length + 1];

	unsigned int offset = 0;

	for (std::string &s : lines) {
		strcpy(source + offset, s.c_str());
		offset += s.length() + 1;
		source[offset - 1] = '\n';
	}
	source[offset] = 0;
}

void ygl::Shader::loadSource(const char *file, char *&source, int &length) {
	loadSource(file, ygl::Shader::DEFAULT_INCLUDE_DIRECTORY, source, length);
}

void ygl::Shader::loadSourceRecursively(std::vector<std::string> &lines, const char *file, const char *includeDir, int includeDirLength) {
	std::ifstream in(file);
	if (in.fail()) {
		std::cerr << "Error: cannot open file: \"" << file << "\"" << std::endl;
		return;
	}
	while (!in.eof()) {
		std::string line;
		std::getline(in, line);
		if (line.rfind("#include", 0) == 0) {
			size_t open	 = line.find('<', 0);
			size_t close = line.find('>', 0);
			assert(open != line.npos && close != line.npos && open < close && "invalid #include derivative");
			std::string file = line.substr(open + 1, close - open - 1);

			int	  fullPathLength = close - open - 1 + includeDirLength + 2;
			char *fullPath		 = new char[fullPathLength];
			snprintf(fullPath, fullPathLength, "%s/%s", includeDir, file.c_str());
			loadSourceRecursively(lines, fullPath, includeDir, includeDirLength);
			delete fullPath;
		} else {
			lines.push_back(line);
		}
	}
}

void ygl::Shader::finishProgramCreation() {
	attachShaders();
	assert(checkLinkStatus() && checkValidateStatus());
	deleteShaders();
	detectUniforms();
}

void ygl::Shader::bind() {
	glUseProgram(program);
}

void ygl::Shader::unbind() {
	glUseProgram(0);
}

void ygl::Shader::createUniform(const char *uniformName) {
	GLint uniformLocation = glGetUniformLocation(program, uniformName);
	if (uniformLocation < 0) {
		std::cerr << "Could not find uniform: " << uniformName;
	}
	registerUniform(uniformName, uniformLocation);
}

void ygl::Shader::registerUniform(const std::string &name, int location) {
	if (hasUniform(name)) {
		std::cerr << "That uniform already exists" << std::endl;
	}
	uniforms.insert(std::pair(name, location));
}

/**
 * @brief detects non-block uniforms in the shader and registers them
 */
void ygl::Shader::detectUniforms() {
	GLint numUniforms;
	glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
	GLenum properties[] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};

	for (int unif = 0; unif < numUniforms; ++unif) {
		GLint values[4];
		glGetProgramResourceiv(program, GL_UNIFORM, unif, 4, properties, 4, nullptr, values);

		if (values[0] != -1) {
			continue;
		}

		char *name = new char[values[2]];
		glGetProgramResourceName(program, GL_UNIFORM, unif, (GLsizei)values[2], nullptr, name);
		// std::cout << name << " " << values[1] << " "<< values[3] << std::endl;

		createUniform(name);
		delete name;
	}
}

int ygl::Shader::getUniformLocation(const std::string &uniformName) {
	if (!hasUniform(uniformName)) {
		std::cerr << "the uniform " << uniformName << " does not exist in this shader." << std::endl;
	}
	GLint location = (*(uniforms.find(uniformName))).second;
	return location;
}

bool ygl::Shader::hasUniform(const std::string &uniformName) {
	return uniforms.find(uniformName) != uniforms.end();
}

void ygl::Shader::setUniform(const std::string &uniformName, GLboolean value) {
	glUniform1i(getUniformLocation(uniformName), value ? 1 : 0);
}
void ygl::Shader::setUniform(const std::string &uniformName, GLint value) {
	glUniform1i(getUniformLocation(uniformName), value);
}
void ygl::Shader::setUniform(const std::string &uniformName, GLuint value) {
	glUniform1ui(getUniformLocation(uniformName), value);
}
void ygl::Shader::setUniform(const std::string &uniformName, glm::mat4x4 value) {
	glUniformMatrix4fv(getUniformLocation(uniformName), 1, GL_FALSE, glm::value_ptr(value));
}
void ygl::Shader::setUniform(const std::string &uniformName, glm::vec4 value) {
	glUniform4f(getUniformLocation(uniformName), value.x, value.y, value.z, value.w);
}
void ygl::Shader::setUniform(const std::string &uniformName, glm::vec3 value) {
	glUniform3f(getUniformLocation(uniformName), value.x, value.y, value.z);
}
void ygl::Shader::setUniform(const std::string &uniformName, glm::vec2 value) {
	glUniform2f(getUniformLocation(uniformName), value.x, value.y);
}
void ygl::Shader::setUniform(const std::string &uniformName, glm::ivec2 value) {
	glUniform2i(getUniformLocation(uniformName), value.x, value.y);
}
void ygl::Shader::setUniform(const std::string &uniformName, float value) {
	glUniform1f(getUniformLocation(uniformName), value);
}

void ygl::Shader::createSSBO(std::string &name, GLuint binding) {
	// https://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/
	SSBOs.insert(std::pair(name, binding));

	GLuint block_index = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name.c_str());

	// Sets the shader to look for the data in that buffer on the correct location.
	// Can be skipped if "layout (std430, binding=<something>)" is used
	glShaderStorageBlockBinding(program, block_index, binding);
}

void ygl::Shader::createUBO(std::string &name, GLuint binding) {
	UBOs.insert(std::pair(name, binding));

	GLuint block_index = glGetUniformBlockIndex(program, name.c_str());

	glUniformBlockBinding(program, block_index, binding);
}

void ygl::Shader::setSSBO(std::string &name, GLuint bufferId) {
	// https://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/
	GLuint binding_point_index = getSSBOBinding(name);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, bufferId);
}

void ygl::Shader::setUBO(std::string &name, GLuint bufferId) {
	GLuint binding_point_index = getUBOBinding(name);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, bufferId);
}

GLint ygl::Shader::getSSBOBinding(std::string &name) {
	auto res = SSBOs.find(name);
	if (res == SSBOs.end()) {
		std::cerr << "This SSBO does not exist or has not been created: " << name << std::endl;
		return -1;
	}
	return res->second;
}

GLint ygl::Shader::getUBOBinding(std::string &name) {
	auto res = UBOs.find(name);
	if (res == UBOs.end()) {
		std::cerr << "This UBO does not exist or has not been created: " << name << std::endl;
		return -1;
	}
	return res->second;
}

void ygl::Shader::setSSBO(GLuint bufferId, GLuint binding) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, bufferId);
}

void ygl::Shader::setUBO(GLuint bufferId, GLuint binding) {
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId);
}

ygl::VFShader::VFShader(const char *vertex, const char *fragment) : Shader({vertex, fragment}) {
	createShader(GL_VERTEX_SHADER, 0);
	createShader(GL_FRAGMENT_SHADER, 1);

	finishProgramCreation();
}
