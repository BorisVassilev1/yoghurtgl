#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
#include <imgui.h>
#include "yoghurtgl.h"

#include <serializable.h>
#include <shader.h>
#include <mesh.h>
#include <texture.h>
#include <material.h>
#include <ecs.h>

/**
 * @file asset_manager.h
 * @brief a System that manages assets
 */

namespace ygl {

template <class A>
class AssetArray : public AppendableSerializable {
	std::unordered_map<std::string, uint> names;
	std::vector<std::pair<A *, bool>>	  assets;
	std::queue<uint>					  holes;

   public:
	inline uint add(A *asset, const std::string &name, bool persist = true) {
		uint res;
		if (!holes.empty()) {
			res = holes.front();
			holes.pop();
			assets[res] = {asset, persist};
		} else {
			res = assets.size();
			assets.push_back({asset, persist});
		}
		names.insert({name, res});
		return res;
	}
	inline uint getIndex(const std::string &name) {
		auto res = names.find(name);
		if (res == names.end()) return -1;
		return res->second;
	}
	inline A *get(uint index) {
		if (index >= assets.size()) THROW_RUNTIME_ERR("Out of bounds access: " + std::to_string(index));
		if (assets[index].first == nullptr)
			THROW_RUNTIME_ERR("Trying to access a non-existent texture: " + std::to_string(index));
		return assets[index].first;
	}
	inline uint size() { return names.size(); }

	void print() {
		for (auto &it : names) {
			std::cerr << it.first << ' ' << it.second << '\n';
		}
	}

	~AssetArray() {
		for (auto [asset, _] : this->assets) {
			delete asset;
		}
	}

	void write(std::ostream &out) override {
		auto count = std::count_if(assets.begin(), assets.end(), [](auto &p) { return p.second; });
		auto size  = assets.size();
		out.write((char *)&count, sizeof(count));
		out.write((char *)&size, sizeof(size));
		for (auto &[name, index] : names) {
			if (assets[index].second) {
				out.write(name.c_str(), name.size() + 1);
				out.write((char *)&index, sizeof(uint));
				assets[index].first->serialize(out);
			}
		}
	}

	void read(std::istream &in) override {
		auto count = assets.size();
		auto size  = assets.size();
		in.read((char *)&count, sizeof(count));
		in.read((char *)&size, sizeof(size));
		std::cout << "count " << count << std::endl;
		std::cout << "size " << size << std::endl;
		assets.resize(size);
		std::vector<bool> check;
		check.resize(size, false);

		for (auto [name, index] : names) {
			check[index] = true;
		}

		for (std::size_t i = 0; i < count; ++i) {
			std::string name;
			uint		index;
			std::getline(in, name, '\0');
			in.read((char *)&index, sizeof(uint));
			dbLog(ygl::LOG_DEBUG, "loading: ", name, " ", index);
			A *asset = dynamic_cast<A *>(ResourceFactory::fabricate(in));
			if (check[index]) {
				if (names.find(name) != names.end()) {
					dbLog(ygl::LOG_DEBUG, names.find(name)->second);
				}	  // else dbLog(ygl::LOG_DEBUG, *assets[index].first);
				dbLog(ygl::LOG_ERROR, "asset index conflict.\nOld asset will be lost and will leak.");
			}
			assets[index] = std::make_pair(asset, true);
			names[name]	  = index;
			check[index]  = true;
		}

		holes = std::queue<uint>();
		for (std::size_t i = 0; i < size; ++i) {
			if (!check[i]) {
				holes.push(i);
				std::cout << "hole: " << holes.back() << std::endl;
			}
		}
	}

	void clear() {
		for (auto [asset, _] : this->assets) {
			delete asset;
		}
		names.clear();
		assets.clear();
		holes = std::queue<uint>();
	}

	auto begin() { return assets.begin(); }
	auto end() { return assets.end(); }
};

/**
 * @brief A System that manages assets: meshes, textures, shaders
 */
class AssetManager : public ygl::ISystem {
	AssetArray<Mesh>	 meshes;
	AssetArray<ITexture> textures;
	AssetArray<Shader>	 shaders;

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
	uint addMesh(Mesh *mesh, const std::string &name, bool persist = true);
	/**
	 * @brief Adds a Texture to the list of assets.
	 * @note Takes ownership of \a tex
	 *
	 * @param tex - Texture to be added
	 * @param name - resource name
	 * @return index in the asset array
	 */
	uint addTexture(ITexture *tex, const std::string &name, bool persist = true);
	/**
	 * @brief Adds a Shader to the list of assets.
	 * @note Takes ownership of \a shader
	 *
	 * @param shader - Shader to be added
	 * @param name - resource name
	 * @return index in the asset array
	 */
	uint addShader(Shader *shader, const std::string &name, bool persist = true);

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
	T *getTexture(uint i) {
		return dynamic_cast<T *>(getTexture(i));
	}

	void printTextures();
	AssetManager(Scene *scene) : ISystem(scene) {};
	~AssetManager();

	void init() override {}
	void doWork() override {}
	void write(std::ostream &out) override;
	void read(std::istream &in) override;

	void reloadShaders();
};

}	  // namespace ygl
