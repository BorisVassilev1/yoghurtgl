#pragma once

#include "serializable.h"
#ifndef YGL_NO_ASSIMP
	#include <assimp/Importer.hpp>
	#include <assimp/scene.h>
#endif

#include <mesh.h>
#include <texture.h>
#include <vector>
#include <unordered_map>
#include <material.h>
#include <ecs.h>

namespace ygl {

class AssetManager;

#ifndef YGL_NO_ASSIMP
extern Assimp::Importer *importer;

const aiScene *loadScene(const std::string &file, unsigned int flags);
const aiScene *loadScene(const std::string &file);
Material	   getMaterial(const aiScene *, AssetManager &asman, std::string filePath, uint i);

void terminateLoader();
#endif

class AssetManager : public ygl::ISystem {
	std::unordered_map<std::string, uint> meshNames;
	std::vector<Mesh *>					  meshes;
	std::unordered_map<std::string, uint> textureNames;
	std::vector<ITexture *>				  textures;

   public:
	static const char *name;
	uint			   addMesh(Mesh *mesh, std::string name);
	uint			   addTexture(ITexture *tex, std::string name);

	uint getMeshIndex(std::string name);
	uint getTextureIndex(std::string name);

	Mesh	 *getMesh(uint i);
	ITexture *getTexture(uint i);

	template <typename T>
		requires IsTexture<T>
	T getTexture(uint i) {
		return (T)getTexture(i);
	}

	void printTextures();
	AssetManager(Scene *scene) : ISystem(scene){};
	~AssetManager();

	void init() override {}
	void doWork() override {}
	void serialize(std::ostream &out) override;
	void deserialize(std::istream &in) override;
};

class MeshFromFile : public Mesh {
	std::string path;
	uint index;
	void init(const std::string &path, uint index);
   public:
	static const aiScene *loadedScene;
	static std::string loadedFile;
	static void loadSceneIfNeeded(const std::string &path);
	static const char *name;
	MeshFromFile(const std::string &path, uint index = 0);
	MeshFromFile(std::istream &in, const std::string &path);
	
	void serialize(std::ostream &out) override;
	void deserialize(std::istream &in) override;
};
}	  // namespace ygl
