#pragma once

#ifndef YGL_NO_ASSIMP
	#include <assimp/Importer.hpp>
	#include <assimp/scene.h>
#endif

#include <mesh.h>
#include <texture.h>
#include <vector>
#include <unordered_map>
#include <material.h>

namespace ygl {

class AssetManager;

#ifndef YGL_NO_ASSIMP
extern Assimp::Importer *importer;

const aiScene *loadScene(const std::string &file, unsigned int flags);
const aiScene *loadScene(const std::string &file);
const Mesh	  *getModel(const aiScene *, unsigned int meshIndex = 0);
Material	   getMaterial(const aiScene *, AssetManager &asman, std::string filePath, uint i);

void terminateLoader();
#endif

class AssetManager {
	std::unordered_map<std::string, uint> meshNames;
	std::vector<Mesh *>					  meshes;
	std::unordered_map<std::string, uint> textureNames;
	std::vector<ITexture *>				  textures;

   public:
	uint addMesh(Mesh *mesh, std::string name);
	uint addTexture(ITexture *tex, std::string name);

	uint getMeshIndex(std::string name);
	uint getTextureIndex(std::string name);

	Mesh	 *getMesh(uint i);
	ITexture *getTexture(uint i);

	template <typename T, typename = std::enable_if<std::is_base_of<ITexture, T>::value>>
	T getTexture(uint i) {
		return (T)getTexture(i);
	}

	void printTextures();
};
}	  // namespace ygl