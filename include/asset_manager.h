#pragma once

#include <serializable.h>
#include <shader.h>

#include <mesh.h>
#include <texture.h>
#include <vector>
#include <unordered_map>
#include <material.h>
#include <ecs.h>

/**
 * @file asset_manager.h
 * @brief a System that manages assets
 */

namespace ygl {

/**
 * @brief A System that manages assets: meshes, textures, shaders
 */
class AssetManager : public ygl::ISystem {
	std::unordered_map<std::string, uint> meshNames;	 ///< map name to index in @ref meshes array
	std::vector<Mesh *>					  meshes;		 ///< array of all meshes

	std::unordered_map<std::string, uint> textureNames;		///< map name to index in @ref textures array
	std::vector<ITexture *>				  textures;			///< array of all textures

	std::unordered_map<std::string, uint> shaderNames;	   ///< map name to index in @ref shaders array
	std::vector<Shader *>				  shaders;		   ///< array of all shaders

	AssetManager(const AssetManager &other)			   = delete;
	AssetManager &operator=(const AssetManager &other) = delete;

   public:
	static const char *name;
	/**
	 * @brief Adds a Mesh to the list of assets.
	 * @note Takes ownership of \a mesh
	 *
	 * @param mesh - mesh to be added
	 * @param name - resource name
	 * @return index in the asset array
	 */
	uint addMesh(Mesh *mesh, const std::string &name);
	/**
	 * @brief Adds a Texture to the list of assets.
	 * @note Takes ownership of \a tex
	 *
	 * @param tex - Texture to be added
	 * @param name - resource name
	 * @return index in the asset array
	 */
	uint addTexture(ITexture *tex, const std::string &name);
	/**
	 * @brief Adds a Shader to the list of assets.
	 * @note Takes ownership of \a shader
	 *
	 * @param shader - Shader to be added
	 * @param name - resource name
	 * @return index in the asset array
	 */
	uint addShader(Shader *shader, const std::string &name);

	uint getMeshIndex(const std::string &name);
	uint getTextureIndex(const std::string &name);
	uint getShaderIndex(const std::string &name);

	std::size_t getMeshCount();
	std::size_t getTexturesCount();
	std::size_t getShadersCount();

	Mesh	 *getMesh(uint i);
	ITexture *getTexture(uint i);
	Shader	 *getShader(uint i);

	template <typename T>
		requires IsTexture<T>
	T getTexture(uint i) {
		return dynamic_cast<T>(getTexture(i));
	}

	void printTextures();
	AssetManager(Scene *scene) : ISystem(scene){};
	~AssetManager();

	void init() override {}
	void doWork() override {}
	void write(std::ostream &out) override;
	void read(std::istream &in) override;
};
}	  // namespace ygl
