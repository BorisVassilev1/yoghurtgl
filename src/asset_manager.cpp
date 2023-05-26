#include <asset_manager.h>
#include <renderer.h>
#include <stdexcept>
#include "yoghurtgl.h"
#include <assimp/types.h>
#include <serializable.h>
#include <texture.h>

const char *ygl::AssetManager::name = "ygl::AssetManager";

uint ygl::AssetManager::addMesh(Mesh *mesh, const std::string &name) {
	uint res = meshes.size();
	meshes.push_back(mesh);
	meshNames.insert({name, res});
	return res;
}

uint ygl::AssetManager::addTexture(ITexture *tex, const std::string &name) {
	uint res = textures.size();
	textures.push_back(tex);
	textureNames.insert({name, res});
	return res;
}

uint ygl::AssetManager::addShader(ygl::Shader *shader, const std::string &name) {
	uint res = shaders.size();
	shaders.push_back(shader);
	shaderNames.insert({name, res});
	return res;
}

uint ygl::AssetManager::getMeshIndex(const std::string &name) {
	auto it = meshNames.find(name);
	if (it == meshNames.end()) return -1;
	else return (*it).second;
}

uint ygl::AssetManager::getTextureIndex(const std::string &name) {
	auto it = textureNames.find(name);
	if (it == textureNames.end()) return -1;
	else return (*it).second;
}

uint ygl::AssetManager::getShaderIndex(const std::string &name) {
	auto it = shaderNames.find(name);
	if (it == shaderNames.end()) return -1;
	else return (*it).second;
}

ygl::Mesh *ygl::AssetManager::getMesh(uint i) {
	assert(i < meshes.size());
	return meshes[i];
}

ygl::ITexture *ygl::AssetManager::getTexture(uint i) {
	if (i >= textures.size()) std::cerr << "\n" << i << " invalid index!!\n";
	assert(i < textures.size());
	return textures[i];
}

ygl::Shader *ygl::AssetManager::getShader(uint i) {
	if (i >= shaders.size()) std::cerr << "\n" << i << " invalid index!!\n";
	assert(i < shaders.size());
	return shaders[i];
}

std::size_t ygl::AssetManager::getMeshCount() { return meshes.size(); }
std::size_t ygl::AssetManager::getTexturesCount() { return textures.size(); }
std::size_t ygl::AssetManager::getShadersCount() { return shaders.size(); }

void ygl::AssetManager::printTextures() {
	for (auto &it : textureNames) {
		std::cerr << it.first << ' ' << it.second << '\n';
	}
}

ygl::AssetManager::~AssetManager() {
	for (auto tex : this->textures) {
		delete tex;
	}

	for (auto mesh : this->meshes) {
		delete mesh;
	}

	for (auto shader : this->shaders) {
		delete shader;
	}
}

void ygl::AssetManager::write(std::ostream &out) {
	auto meshCount	  = meshNames.size();
	auto textureCount = textureNames.size();
	auto shaderCount  = shaderNames.size();
	out.write((char *)&meshCount, sizeof(meshCount));
	out.write((char *)&textureCount, sizeof(textureCount));
	out.write((char *)&shaderCount, sizeof(shaderCount));

	for (auto &pair : textureNames) {
		out.write(pair.first.c_str(), pair.first.size() + 1);
		out.write((char *)&pair.second, sizeof(uint));
		textures[pair.second]->serialize(out);
	}

	for (auto &pair : meshNames) {
		out.write(pair.first.c_str(), pair.first.size() + 1);
		out.write((char *)&pair.second, sizeof(uint));
		meshes[pair.second]->serialize(out);
	}

	for (auto &pair : shaderNames) {
		out.write(pair.first.c_str(), pair.first.size() + 1);
		out.write((char *)&pair.second, sizeof(uint));
		shaders[pair.second]->serialize(out);
	}
}

void ygl::AssetManager::read(std::istream &in) {
	auto meshCount	  = meshNames.size();
	auto textureCount = textureNames.size();
	auto shaderCount  = shaderNames.size();
	in.read((char *)&meshCount, sizeof(meshCount));
	in.read((char *)&textureCount, sizeof(textureCount));
	in.read((char *)&shaderCount, sizeof(shaderCount));

	textures.resize(textureCount);
	for (std::size_t i = 0; i < textureCount; ++i) {
		std::string texName;
		uint		texIndex;
		std::getline(in, texName, '\0');
		in.read((char *)&texIndex, sizeof(uint));
		dbLog(ygl::LOG_INFO, "Loading texture: ", texName);

		ITexture *tex	   = dynamic_cast<ITexture *>(ResourceFactory::fabricate(in));
		textures[texIndex] = tex;
	}

	meshes.resize(meshCount);
	for (std::size_t i = 0; i < meshCount; ++i) {
		std::string meshName;
		uint		meshIndex;
		std::getline(in, meshName, '\0');
		in.read((char *)&meshIndex, sizeof(uint));
		dbLog(ygl::LOG_INFO, "Loading mesh: ", meshName);

		Mesh *mesh		  = dynamic_cast<Mesh *>(ResourceFactory::fabricate(in));
		meshes[meshIndex] = mesh;
	}

	shaders.resize(shaderCount);
	for (std::size_t i = 0; i < shaderCount; ++i) {
		std::string shaderName;
		uint		shaderIndex;
		std::getline(in, shaderName, '\0');
		in.read((char *)&shaderIndex, sizeof(uint));
		dbLog(ygl::LOG_INFO, "Loading shader: ", shaderName);

		Shader *shader		 = dynamic_cast<Shader *>(ResourceFactory::fabricate(in));
		shaders[shaderIndex] = shader;
	}
}
