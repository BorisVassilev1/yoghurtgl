#include <shader.h>

#include <assert.h>
#include <iterator>
#include <sstream>
#include <string>
#include <string.h>
#include <string_view>
#include <yoghurtgl.h>

ygl::Shader::~Shader() {
	if (shaders != nullptr) { deleteShaders(); }
	if (program != 0) {
		glDeleteProgram(program);
		program = 0;
	}
	for (uint i = 0; i < shadersCount; ++i) {
		delete[] fileNames[i];
	}
	delete[] fileNames;
	fileNames = nullptr;
}

void ygl::Shader::deleteShaders() {
	for (uint i = 0; i < shadersCount; ++i) {
		glDetachShader(program, shaders[i]);
		glDeleteShader(shaders[i]);
		shaders[i] = 0;
	}
	delete[] shaders;
	shaders = nullptr;
}

void ygl::Shader::init(std::vector<std::string> files) {
	shadersCount = files.size();
	shaders		 = new GLuint[shadersCount];
	fileNames	 = new const char *[shadersCount];

	for (uint i = 0; i < shadersCount; ++i) {
		fileNames[i] = new char[files[i].size() + 1];
		std::strcpy((char *)fileNames[i], files[i].c_str());
	}

	program = glCreateProgram();
}

ygl::Shader::Shader(std::initializer_list<std::string> files) { init(files); }

ygl::Shader::Shader(std::istream &in) {
	in.read((char *)&shadersCount, sizeof(shadersCount));
	std::vector<std::string> files;
	for (std::size_t i = 0; i < shadersCount; ++i) {
		std::string fileName;
		std::getline(in, fileName, '\0');
		files.push_back(fileName);
	}
	init(files);
}

void ygl::Shader::serialize(std::ostream &out) {
	out.write((char *)&shadersCount, sizeof(shadersCount));
	for (std::size_t i = 0; i < shadersCount; ++i) {
		out.write(fileNames[i], std::strlen(fileNames[i]) + 1);
	}
}

void ygl::Shader::createShader(GLenum type, GLuint target) {
	const char	 *file = fileNames[target];
	std::ifstream in(file);
	if (in.fail()) { std::cerr << "Error: failed opening file: " << file << std::endl; }

	int	  sh = glCreateShader(type);
	char *source;
	int	  length;
	loadSource(file, source, length);

	glShaderSource(sh, 1, &source, &length);

	delete[] source;
	glCompileShader(sh);
	shaders[target] = sh;

	assert(checkCompileStatus(target));
}

void ygl::Shader::attachShaders() {
	for (uint i = 0; i < shadersCount; ++i) {
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

		std::cerr << "Error: Program Linking: \n" << programLog << std::endl;

		delete[] programLog;
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

		std::cerr << "Error: Program Validation: \n" << programLog << std::endl;

		delete[] programLog;
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

		std::cerr << "Error: Shader Compilation - " << fileNames[target] << " :\n " << shaderLog << std::endl;

		delete[] shaderLog;
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

void ygl::Shader::loadSourceRecursively(std::vector<std::string> &lines, const char *file, const char *includeDir,
										int includeDirLength) {
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
			delete[] fullPath;
		} else {
			lines.push_back(line);
		}
	}
}

void ygl::Shader::finishProgramCreation() {
	attachShaders();
	if (!checkLinkStatus() || !checkValidateStatus()) THROW_RUNTIME_ERR("SHADER FAILED TO LINK");
	deleteShaders();
	detectUniforms();
	detectBlockUniforms();
	detectStorageBlocks();
}

void ygl::Shader::bind() {
	bound = true;
	glUseProgram(program);
}

void ygl::Shader::unbind() {
	bound = false;
	glUseProgram(0);
}

void ygl::Shader::createUniform(const char *uniformName) {
	GLint uniformLocation = glGetUniformLocation(program, uniformName);
	if (uniformLocation < 0) { std::cerr << "Could not find uniform: " << uniformName; }
	registerUniform(uniformName, uniformLocation);
}

void ygl::Shader::registerUniform(const std::string &name, int location) {
	if (hasUniform(name)) { std::cerr << "That uniform already exists" << std::endl; }
	uniforms.insert(std::pair(name, location));
}

/**
 * @brief detects non-block uniforms in the shader and registers them
 */
void ygl::Shader::detectUniforms() {
	GLint numUniforms;
	// glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

	// dbLog(LOG_INFO, fileNames[0], " uniforms count: ", numUniforms);
	GLenum properties[] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};

	for (int unif = 0; unif < numUniforms; ++unif) {
		GLint values[4];
		glGetProgramResourceiv(program, GL_UNIFORM, unif, 4, properties, 4, nullptr, values);

		if (values[0] != -1) { continue; }

		char *name = new char[values[2]];
		glGetProgramResourceName(program, GL_UNIFORM, unif, (GLsizei)values[2], nullptr, name);

		// dbLog(LOG_INFO, fileNames[0], " ", name);
		createUniform(name);
		delete[] name;
	}
}

void ygl::Shader::detectBlockUniforms() {
	GLint numBlocks;
	// glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);

	// dbLog(LOG_INFO, fileNames[0], " Block Uniforms Count: ", numBlocks);
	GLenum blockProperties[] = {GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NAME_LENGTH};

	for (int blockIx = 0; blockIx < numBlocks; ++blockIx) {
		GLint result[3];
		glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, blockIx, 3, blockProperties, 3, nullptr, result);

		GLchar *blockName = new GLchar[result[2]];
		glGetProgramResourceName(program, GL_UNIFORM_BLOCK, blockIx, result[2], nullptr, blockName);
		std::string blockNameString(blockName);

		// dbLog(LOG_INFO, fileNames[0], " ", blockNameString);

		createUBO(blockNameString, result[1]);
		delete[] blockName;
	}
}

void ygl::Shader::detectStorageBlocks() {
	GLint numSSBOs;
	glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numSSBOs);

	GLenum blockProperties[] = {GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_NAME_LENGTH};

	for (int block = 0; block < numSSBOs; block++) {
		int blockInfo[3];
		glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, block, 3, blockProperties, 3, nullptr, blockInfo);

		GLchar *blockName = new GLchar[blockInfo[2]];
		glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, block, blockInfo[2], nullptr, blockName);
		std::string blockNameString(blockName);

		createSSBO(blockNameString, blockInfo[1]);
		delete[] blockName;
	}
}

GLuint ygl::Shader::getUniformLocation(const std::string &uniformName) {
	if (!hasUniform(uniformName)) {
		dbLog(ygl::LOG_ERROR, "Shader does not have uniform: ", uniformName);
		THROW_RUNTIME_ERR("Shader does not have uniform: " + uniformName);
	}

	GLint location = (*(uniforms.find(uniformName))).second;
	return location;
}

bool ygl::Shader::hasUniform(const std::string &uniformName) { return uniforms.find(uniformName) != uniforms.end(); }

void ygl::Shader::setUniform(GLuint location, GLboolean value) { glUniform1i(location, value ? 1 : 0); }
void ygl::Shader::setUniform(GLuint location, GLint value) { glUniform1i(location, value); }
void ygl::Shader::setUniform(GLuint location, GLuint value) { glUniform1ui(location, value); }
void ygl::Shader::setUniform(GLuint location, glm::mat4x4 value) {
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
void ygl::Shader::setUniform(GLuint location, glm::vec4 value) {
	glUniform4f(location, value.x, value.y, value.z, value.w);
}
void ygl::Shader::setUniform(GLuint location, glm::vec3 value) { glUniform3f(location, value.x, value.y, value.z); }
void ygl::Shader::setUniform(GLuint location, glm::vec2 value) { glUniform2f(location, value.x, value.y); }
void ygl::Shader::setUniform(GLuint location, glm::ivec2 value) { glUniform2i(location, value.x, value.y); }
void ygl::Shader::setUniform(GLuint location, GLfloat value) { glUniform1f(location, value); }

void ygl::Shader::setUniform(GLuint location, GLdouble value) { glUniform1d(location, value); }

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

bool ygl::Shader::isBound() { return bound; }

void ygl::Shader::setSSBO(GLuint bufferId, GLuint binding) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, bufferId);
}

void ygl::Shader::setUBO(GLuint bufferId, GLuint binding) { glBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId); }

const char *ygl::VFShader::name = "ygl::VFShader";

ygl::VFShader::VFShader(const char *vertex, const char *fragment) : Shader({vertex, fragment}) {
	createShader(GL_VERTEX_SHADER, 0);
	createShader(GL_FRAGMENT_SHADER, 1);

	finishProgramCreation();
}

ygl::VFShader::VFShader(std::istream &in) : Shader(in) {
	createShader(GL_VERTEX_SHADER, 0);
	createShader(GL_FRAGMENT_SHADER, 1);

	finishProgramCreation();
}

void ygl::VFShader::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	Shader::serialize(out);
}

const char *ygl::ComputeShader::name = "ygl::ComputeShader";

ygl::ComputeShader::ComputeShader(const char *source) : Shader({source}) {
	createShader(GL_COMPUTE_SHADER, 0);
	finishProgramCreation();

	glGetProgramiv(program, GL_COMPUTE_WORK_GROUP_SIZE, glm::value_ptr(groupSize));
}

ygl::ComputeShader::ComputeShader(std::istream &in) : Shader(in) {
	createShader(GL_COMPUTE_SHADER, 0);
	finishProgramCreation();

	glGetProgramiv(program, GL_COMPUTE_WORK_GROUP_SIZE, glm::value_ptr(groupSize));
}
void ygl::ComputeShader::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	Shader::serialize(out);
}
