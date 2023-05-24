#pragma once

#include <serializable.h>
#include <shader.h>
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

	std::unordered_map<std::string, uint> shaderNames;
	std::vector<Shader *>				  shaders;

   public:
	static const char *name;
	uint			   addMesh(Mesh *mesh, const std::string &name);
	uint			   addTexture(ITexture *tex, const std::string &name);
	uint			   addShader(Shader *shader, const std::string &name);

	uint getMeshIndex(const std::string &name);
	uint getTextureIndex(const std::string &name);
	uint getShaderIndex(const std::string &name);

	std::size_t getMeshCount();
	std::size_t getTexturesCount();
	std::size_t getShadersCount();

	Mesh	 *getMesh(uint i);
	ITexture *getTexture(uint i);
	Shader *getShader(uint i);
	
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
	void write(std::ostream &out) override;
	void read(std::istream &in) override;
};

class MeshFromFile : public Mesh {
	std::string path;
	uint		index;
	void		init(const std::string &path, uint index);

   public:
	static const aiScene *loadedScene;
	static std::string	  loadedFile;
	static void			  loadSceneIfNeeded(const std::string &path);
	static const char	 *name;
	MeshFromFile(const std::string &path, uint index = 0);
	MeshFromFile(std::istream &in);

	void serialize(std::ostream &out) override;
};
}	  // namespace ygl
