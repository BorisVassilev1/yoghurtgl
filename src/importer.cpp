#include <importer.h>
#include <renderer.h>

#ifndef YGL_NO_ASSIMP
	#include <assimp/Importer.hpp>
	#include <assimp/scene.h>
	#include <assimp/postprocess.h>

Assimp::Importer *ygl::importer = nullptr;

const aiScene *ygl::loadScene(const std::string &file, unsigned int flags) {
	if (ygl::importer == nullptr) { ygl::importer = new Assimp::Importer(); }

	const aiScene *scene = ygl::importer->ReadFile(file, flags);

	if (!scene) {
		dbLog(ygl::LOG_ERROR, "[Assimp]", ygl::importer->GetErrorString());
		return nullptr;
	}
	return scene;
}

const aiScene *ygl::loadScene(const std::string &file) {
	return loadScene(file, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
							   aiProcess_GenNormals | aiProcess_PreTransformVertices);
}

const ygl::Mesh *ygl::getModel(const aiScene *scene, unsigned int meshIndex) {
	using namespace std;
	assert(scene->HasMeshes() && "scene does not have any meshes");
	if (!scene->HasMeshes()) { return nullptr; }

	aiMesh	   **meshes	   = scene->mMeshes;
	unsigned int numMeshes = scene->mNumMeshes;

	assert(numMeshes >= 1 && "no meshes in the scene?");
	// assert(numMeshes < meshIndex && "no mesh with that index");

	aiMesh		*mesh		   = meshes[meshIndex];
	unsigned int verticesCount = mesh->mNumVertices;
	unsigned int indicesCount  = mesh->mNumFaces * 3;
	GLuint		*indices	   = new GLuint[indicesCount * sizeof(GLuint)];

	unsigned int indexCounter = 0;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		indices[indexCounter++] = mesh->mFaces[i].mIndices[0];
		indices[indexCounter++] = mesh->mFaces[i].mIndices[1];
		indices[indexCounter++] = mesh->mFaces[i].mIndices[2];
	}

	GLfloat *texCoords = nullptr;
	if (mesh->HasTextureCoords(0)) {
		texCoords = new GLfloat[verticesCount * sizeof(float) * 2];
		for (unsigned int i = 0; i < verticesCount; ++i) {
			texCoords[i * 2]	 = mesh->mTextureCoords[0][i].x;
			texCoords[i * 2 + 1] = mesh->mTextureCoords[0][i].y;
		}
	}

	assert(indexCounter == indicesCount && "something went very wrong");

	if (!(mesh->HasTextureCoords(0))) { dbLog(ygl::LOG_WARNING, "tex coords cannot be loaded for model!"); }
	if (!(mesh->HasVertexColors(0))) { dbLog(ygl::LOG_WARNING, "colors cannot be loaded for model!"); }

	ygl::Mesh *result = new ygl::Mesh(verticesCount, (GLfloat *)mesh->mVertices, (GLfloat *)mesh->mNormals, texCoords,
									  (GLfloat *)mesh->mColors[0], (GLfloat *)mesh->mTangents, indicesCount, indices);

	delete[] indices;
	if (texCoords != nullptr) delete[] texCoords;

	return result;
}

bool getTexture(aiMaterial *mat, aiTextureType type, std::string &fileName) {
	uint	 numTextures = mat->GetTextureCount(aiTextureType(type));	  // Amount of diffuse textures
	aiString textureName;	  // Filename of the texture using the aiString assimp structure
	aiReturn ret;

	if (numTextures) {
		// Get the file name of the texture by passing the variable by reference again
		// Second param is 0, which is the first diffuse texture
		// There can be more diffuse textures but for now we are only interested in the first one
		ret = mat->Get(AI_MATKEY_TEXTURE(aiTextureType(type), 0), textureName);
		if (ret != AI_SUCCESS) dbLog(ygl::LOG_ERROR, "failed getting texture type: ", type);

		fileName = textureName.data;	 // The actual name of the texture file
		return true;
	}
	return false;
}

ygl::Material ygl::getMaterial(const aiScene *scene, ygl::AssetManager &asman, std::string filePath, uint m) {
	if (!scene->HasMaterials()) { return Material(); }
	aiMaterial *material = scene->mMaterials[m];	 // Get the current material
	aiString	materialName;						 // The name of the material found in mesh file
	aiReturn	ret;	 // Code which says whether loading something has been successful of not

	ret = material->Get(AI_MATKEY_NAME, materialName);	   // Get the material name (pass by reference)
	if (ret != AI_SUCCESS) materialName = "";			   // Failed to find material name so makes var empty

	// std::cerr << "Name : " << materialName.data << std::endl;

	aiColor3D diff(0, 0, 0);
	ret = material->Get(AI_MATKEY_COLOR_DIFFUSE, diff);
	aiColor3D emission(0, 0, 0);
	ret = material->Get(AI_MATKEY_COLOR_EMISSIVE, emission);
	aiColor3D specular(0, 0, 0);
	ret = material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
	aiColor3D transparent(0, 0, 0);
	ret = material->Get(AI_MATKEY_COLOR_TRANSPARENT, transparent);
	float roughness_factor;
	ret		  = material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness_factor);
	float ior = 1.;
	ret		  = material->Get(AI_MATKEY_REFRACTI, ior);

	glm::vec3 glmAlbedo(diff.r, diff.g, diff.b);
	glm::vec3 glmEmission(emission.r, emission.g, emission.b);
	glm::vec3 glmSpecular(specular.r, specular.g, specular.b);
	glm::vec3 glmTransparent(transparent.r, transparent.g, transparent.b);

	float		   use_map[6]{0};
	uint		   map[6]{0};
	std::string	   map_file[6];
	aiTextureType  mapType[6]{aiTextureType_NORMALS,   aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_LIGHTMAP,
							  aiTextureType_METALNESS, aiTextureType_DIFFUSE,			aiTextureType_EMISSIVE};
	ITexture::Type texType[6]{ITexture::Type::NORMAL,	ITexture::Type::ROUGHNESS, ITexture::Type::AO,
							  ITexture::Type::METALLIC, ITexture::Type::DIFFUSE,   ITexture::Type::EMISSIVE};

	for (int i = 0; i < 6; ++i) {
		use_map[i]	= getTexture(material, mapType[i], map_file[i]);
		map_file[i] = filePath + map_file[i];

		if (use_map[i]) {
			map[i] = asman.getTextureIndex(map_file[i]);
			if (map[i] == (uint)-1) map[i] = asman.addTexture(new Texture2d(map_file[i], texType[i]), map_file[i]);
		}
	}

	ygl::Material mat(glmAlbedo, 0.02, glmEmission, ior, glmTransparent, 0.0, glmSpecular, roughness_factor,
					  roughness_factor, 0., map[0], use_map[0], map[1], use_map[1], map[2], use_map[2], map[3],
					  use_map[3], map[4], use_map[4], map[5], use_map[5]);
	return mat;
}

#endif

uint ygl::AssetManager::addMesh(Mesh *mesh, std::string name) {
	uint res = meshes.size();
	meshes.push_back(mesh);
	meshNames.insert({name, res});
	return res;
}

uint ygl::AssetManager::addTexture(ITexture *tex, std::string name) {
	uint res = textures.size();
	textures.push_back(tex);
	textureNames.insert({name, res});
	return res;
}

uint ygl::AssetManager::getMeshIndex(std::string name) {
	auto it = meshNames.find(name);
	if (it == meshNames.end()) return -1;
	else return (*it).second;
}

uint ygl::AssetManager::getTextureIndex(std::string name) {
	auto it = textureNames.find(name);
	if (it == textureNames.end()) return -1;
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

void ygl::AssetManager::printTextures() {
	for (auto &it : textureNames) {
		std::cerr << it.first << ' ' << it.second << '\n';
	}
}