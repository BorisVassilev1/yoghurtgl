#include <asset_manager.h>
#include <renderer.h>
#include <stdexcept>
#include <string>
#include <yoghurtgl.h>
#include <serializable.h>
#include <texture.h>

const char *ygl::AssetManager::name = "ygl::AssetManager";

uint ygl::AssetManager::addMesh(Mesh *mesh, const std::string &name, bool persist) {
	return meshes.add(mesh, name, persist);
}

uint ygl::AssetManager::addTexture(ITexture *tex, const std::string &name, bool persist) {
	return textures.add(tex, name, persist);
}

uint ygl::AssetManager::addShader(ygl::Shader *shader, const std::string &name, bool persist) {
	return shaders.add(shader, name, persist);
}

uint ygl::AssetManager::getMeshIndex(const std::string &name) { return meshes.getIndex(name); }

uint ygl::AssetManager::getTextureIndex(const std::string &name) { return textures.getIndex(name); }

uint ygl::AssetManager::getShaderIndex(const std::string &name) { return shaders.getIndex(name); }

ygl::Mesh *ygl::AssetManager::getMesh(uint i) { return meshes.get(i); }

ygl::ITexture *ygl::AssetManager::getTexture(uint i) { return textures.get(i); }

ygl::Shader *ygl::AssetManager::getShader(uint i) { return shaders.get(i); }

std::size_t ygl::AssetManager::getMeshCount() { return meshes.size(); }
std::size_t ygl::AssetManager::getTexturesCount() { return textures.size(); }
std::size_t ygl::AssetManager::getShadersCount() { return shaders.size(); }

void ygl::AssetManager::printTextures() { textures.print(); }

ygl::AssetManager::~AssetManager() {}

void ygl::AssetManager::write(std::ostream &out) {
	dbLog(ygl::LOG_INFO, "SAVING MESHES");
	meshes.write(out);
	dbLog(ygl::LOG_INFO, "SAVING TEXTURES");
	textures.write(out);
	dbLog(ygl::LOG_INFO, "SAVING SHADERS");
	shaders.write(out);
}

void ygl::AssetManager::read(std::istream &in) {
	dbLog(ygl::LOG_INFO, "READING MESHES");
	meshes.read(in);
	dbLog(ygl::LOG_INFO, "READING TEXTURES");
	textures.read(in);
	dbLog(ygl::LOG_INFO, "READING SHADERS");
	shaders.read(in);
}
