#include <istream>
#include <assert.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

#include <yoghurtgl.h>
#include <glm/gtc/type_ptr.hpp>
#include <texture.h>
#include <mesh.h>
#include <asset_manager.h>

GLuint ygl::IMesh::createVAO() {
	glGenVertexArrays(1, &vao);
	return vao;
}

GLuint ygl::IMesh::createIBO(GLuint *data, int count) {
	indicesCount = count;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(GLuint), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return ibo;
}

void ygl::IMesh::bind() {
	if (cullFace) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glDepthFunc(depthfunc);
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	glBindVertexArray(vao);
	enableVBOs();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void ygl::IMesh::unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	disableVBOs();
	glBindVertexArray(0);
}

ygl::IMesh::~IMesh() {
	glDeleteVertexArrays(1, &vao);
	vao = -1;
	glDeleteBuffers(1, &ibo);
	ibo = -1;
}

GLenum ygl::IMesh::getDrawMode() { return drawMode; }

GLuint ygl::IMesh::getIndicesCount() { return indicesCount; }

GLuint ygl::IMesh::getVerticesCount() { return verticesCount; }

GLuint ygl::IMesh::getIBO() { return ibo; }

GLuint ygl::IMesh::getVAO() { return vao; }

void ygl::IMesh::setDrawMode(GLenum mode) { drawMode = mode; }

void ygl::IMesh::setCullFace(bool cullFace) { this->cullFace = cullFace; }

void ygl::IMesh::setDepthFunc(GLenum depthFunc) { this->depthfunc = depthFunc; }

void ygl::IMesh::setPolygonMode(GLenum polygonMode) { this->polygonMode = polygonMode; }

ygl::IMesh::IMesh(std::istream &in) {
	in.read((char *)&drawMode, sizeof(drawMode));
	in.read((char *)&depthfunc, sizeof(depthfunc));
	in.read((char *)&polygonMode, sizeof(polygonMode));
	in.read((char *)&cullFace, sizeof(cullFace));
}

void ygl::IMesh::serialize(std::ostream &out) {
	out.write((char *)&drawMode, sizeof(drawMode));
	out.write((char *)&depthfunc, sizeof(depthfunc));
	out.write((char *)&polygonMode, sizeof(polygonMode));
	out.write((char *)&cullFace, sizeof(cullFace));
}

void ygl::MultiBufferMesh::addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer, GLenum type,
								  GLuint indexDivisor, GLsizei stride, const void *pointer) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	if (type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_SHORT || type == GL_UNSIGNED_SHORT ||
		type == GL_INT || type == GL_UNSIGNED_INT)
		glVertexAttribIPointer(attrLocation, coordSize, type, stride, pointer);
	else glVertexAttribPointer(attrLocation, coordSize, type, GL_FALSE, stride, pointer);
	glVertexAttribDivisor(attrLocation, indexDivisor);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vbos.push_back(VBO(attrLocation, buffer, coordSize));
}

void ygl::MultiBufferMesh::addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer, GLenum type,
								  GLuint indexDivisor) {
	addVBO(attrLocation, coordSize, buffer, type, indexDivisor, 0, 0);
}

void ygl::MultiBufferMesh::addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer, GLenum type) {
	addVBO(attrLocation, coordSize, buffer, type, 0);
}

ygl::MultiBufferMesh::~MultiBufferMesh() {
	for (VBO &vbo : vbos) {
		glDeleteBuffers(1, &(vbo.bufferId));
		vbo.bufferId = -1;
	}
}
void ygl::MultiBufferMesh::enableVBOs() {
	for (VBO vbo : vbos) {
		glEnableVertexAttribArray(vbo.location);
	}
}
void ygl::MultiBufferMesh::disableVBOs() {
	for (VBO vbo : vbos) {
		glDisableVertexAttribArray(vbo.location);
	}
}

void ygl::Mesh::init(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
					 GLfloat *tangents, GLuint indicesCount, GLuint *indices) {
	this->createVAO();
	glBindVertexArray(this->getVAO());
	this->verticesCount = vertexCount;
	this->createIBO(indices, indicesCount);
	this->addVBO(0, 3, vertices, GL_FLOAT, vertexCount);
	this->addVBO(1, 3, normals, GL_FLOAT, vertexCount);
	this->addVBO(2, 2, texCoords, GL_FLOAT, vertexCount);
	this->addVBO(3, 4, colors, GL_FLOAT, vertexCount);
	this->addVBO(4, 3, tangents, GL_FLOAT, vertexCount);
	glBindVertexArray(0);
}

// constructs a simple mesh with vertex position (vec3), normal(vec3), texCoord(vec2) and color(vec4).
ygl::Mesh::Mesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
				GLfloat *tangents, GLuint indicesCount, GLuint *indices) {
	init(vertexCount, vertices, normals, texCoords, colors, tangents, indicesCount, indices);
}

ygl::IMesh::VBO ygl::Mesh::getVertices() { return vbos[0]; }

ygl::IMesh::VBO ygl::Mesh::getNormals() { return vbos[1]; }

ygl::IMesh::VBO ygl::Mesh::getTexCoords() { return vbos[2]; }

ygl::IMesh::VBO ygl::Mesh::getColors() { return vbos[3]; }

ygl::IMesh::VBO ygl::Mesh::getTangents() { return vbos[4]; }

void ygl::AnimatedMesh::init(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords,
							 GLfloat *colors, GLfloat *tangents, GLint *boneIDs, GLfloat *weights, GLuint indicesCount,
							 GLuint *indices) {
	Mesh::init(vertexCount, vertices, normals, texCoords, colors, tangents, indicesCount, indices);
	glBindVertexArray(this->getVAO());
	this->addVBO(5, MAX_BONE_INFLUENCE, boneIDs, GL_INT, vertexCount);
	this->addVBO(6, MAX_BONE_INFLUENCE, weights, GL_FLOAT, vertexCount);
	glBindVertexArray(0);
}

ygl::AnimatedMesh::AnimatedMesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords,
								GLfloat *colors, GLfloat *tangents, GLint *boneIDs, GLfloat *weights,
								GLuint indicesCount, GLuint *indices) {
	init(vertexCount, vertices, normals, texCoords, colors, tangents, boneIDs, weights, indicesCount, indices);
}

ygl::IMesh::VBO ygl::AnimatedMesh::getBoneIds() { return vbos[5]; }
ygl::IMesh::VBO ygl::AnimatedMesh::getWeights() { return vbos[6]; }

const char *ygl::BoxMesh::name = "ygl::BoxMesh";

// TODO: this must have better texture coords;
void ygl::BoxMesh::init(const glm::vec3 &size, const glm::vec3 &detail) {
	uint vertexCount =
		(detail.x + 1) * (detail.y + 1) + (detail.x + 1) * (detail.z + 1) + (detail.y + 1) * (detail.z + 1);
	vertexCount <<= 1;	   // 2 sides per axis

	GLfloat *vertices = new GLfloat[vertexCount * 3];
	GLfloat *normals  = new GLfloat[vertexCount * 3];
	GLfloat *colors	  = new GLfloat[vertexCount * 4];
	GLfloat *uvs	  = new GLfloat[vertexCount * 2];
	GLfloat *tangents = new GLfloat[vertexCount * 3];

	uint faceCount = detail.x * detail.y + detail.x * detail.z + detail.y * detail.z;
	faceCount <<= 1;	 // 2 faces per axis

	GLuint *indices = new GLuint[faceCount * 6];

	// count offset for writing
	uint vertexOffset = 0;
	uint indexOffset  = 0;
	for (uint axis0 = 0; axis0 < 3; ++axis0) {
		// calculate replacements for x,y,z
		uint axis1 = (axis0 + 1) % 3;
		uint axis2 = (axis0 + 2) % 3;

		// vertex count for current face
		uint faceVertices = (detail[axis0] + 1) * (detail[axis1] + 1);
		for (uint i = 0; i < detail[axis0] + 1; ++i) {
			for (uint j = 0; j < detail[axis1] + 1; ++j) {
				uint k = i * (detail[axis1] + 1) + j + vertexOffset;
				for (uint u = 0; u < 2; ++u, k += faceVertices) {
					vertices[k * 3 + axis0] = i * size[axis0] / detail[axis0] - size[axis0] / 2;
					vertices[k * 3 + axis1] = j * size[axis1] / detail[axis1] - size[axis1] / 2;
					vertices[k * 3 + axis2] = u * size[axis2] - size[axis2] / 2;

					normals[k * 3 + axis0] = 0;
					normals[k * 3 + axis1] = 0;
					normals[k * 3 + axis2] = u * 2. - 1.;

					colors[k * 4]	  = i / detail[axis0];
					colors[k * 4 + 1] = j / detail[axis1];
					colors[k * 4 + 2] = 1;
					colors[k * 4 + 3] = 1;

					uvs[k * 2]	   = (1 - u) + (u * 2. - 1.) * i / detail[axis0];
					uvs[k * 2 + 1] = j / detail[axis1];

					tangents[k * 3 + axis0] = 1 - u * 2.;
					tangents[k * 3 + axis1] = 0;
					tangents[k * 3 + axis2] = 0;
				}
			}
		}

		uint faceIndices = detail[axis0] * detail[axis1];
		for (uint i = 0; i < detail[axis0]; ++i) {
			for (uint j = 0; j < detail[axis1]; ++j) {
				uint ki = i * detail[axis1] + j + indexOffset;
				uint kv = i * (detail[axis1] + 1) + j + vertexOffset;

				for (uint u = 0; u < 2; ++u, ki += faceIndices, kv += faceVertices) {
					int norm						   = (-u * 2 + 1);
					indices[ki * 6 + 5 * u + 0 * norm] = kv;
					indices[ki * 6 + 5 * u + 1 * norm] = kv + 1;
					indices[ki * 6 + 5 * u + 2 * norm] = kv + detail[axis1] + 1;
					indices[ki * 6 + 5 * u + 3 * norm] = kv + detail[axis1] + 1;
					indices[ki * 6 + 5 * u + 4 * norm] = kv + 1;
					indices[ki * 6 + 5 * u + 5 * norm] = kv + detail[axis1] + 2;
				}
			}
		}

		// add to the offset
		vertexOffset += faceVertices * 2;
		indexOffset += faceIndices * 2;
	}

	Mesh::init(vertexCount, vertices, normals, uvs, colors, tangents, faceCount * 6, indices);

	delete[] vertices;
	delete[] normals;
	delete[] colors;
	delete[] indices;
	delete[] uvs;
	delete[] tangents;
}

ygl::BoxMesh::BoxMesh(const glm::vec3 &size, const glm::vec3 &detail) : Mesh(), size(size), resolution(detail) {
	init(size, detail);
}

ygl::BoxMesh::BoxMesh(const glm::vec3 &dim) : BoxMesh(dim, glm::vec3(1.)) {}

ygl::BoxMesh::BoxMesh(float size) : BoxMesh(glm::vec3(size)) {}

ygl::BoxMesh::BoxMesh() : BoxMesh(1.f) {}

void ygl::BoxMesh::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	IMesh::serialize(out);
	out.write((char *)glm::value_ptr(size), sizeof(size));
	out.write((char *)glm::value_ptr(resolution), sizeof(resolution));
}

ygl::BoxMesh::BoxMesh(std::istream &in) : Mesh(in) {
	in.read((char *)glm::value_ptr(size), sizeof(size));
	in.read((char *)glm::value_ptr(resolution), sizeof(resolution));
	init(size, resolution);
}

const char *ygl::SphereMesh::name = "ygl::SphereMesh";

void ygl::SphereMesh::init(float radius, uint detailX, uint detailY) {
	uint vertexCount = (detailX * 2 + 1) * detailY;

	GLfloat *vertices = new GLfloat[vertexCount * 3];
	GLfloat *normals  = new GLfloat[vertexCount * 3];
	GLfloat *colors	  = new GLfloat[vertexCount * 4];
	GLfloat *uvs	  = new GLfloat[vertexCount * 2];
	GLfloat *tangents = new GLfloat[vertexCount * 3];

	for (uint i = 0; i < detailX; ++i) {
		float lon = i * M_PI / (detailX - 1);
		for (uint j = 0; j < detailY * 2 + 1; ++j) {
			float lat = j * M_PI / (detailY);

			int index = j + i * (detailY * 2 + 1);

			vertices[index * 3]		= radius * sin(lon) * cos(lat);
			vertices[index * 3 + 1] = radius * cos(lon);
			vertices[index * 3 + 2] = radius * sin(lon) * sin(lat);

			normals[index * 3]	   = sin(lon) * cos(lat);
			normals[index * 3 + 1] = cos(lon);
			normals[index * 3 + 2] = sin(lon) * sin(lat);

			tangents[index * 3]		= sin(lon) * cos(lat + glm::pi<float>() / 2);
			tangents[index * 3 + 1] = cos(lon);
			tangents[index * 3 + 2] = sin(lon) * sin(lat + glm::pi<float>() / 2);

			colors[index * 4]	  = i / (float)detailX;
			colors[index * 4 + 1] = j / (float)detailY / 2.;
			colors[index * 4 + 2] = 1.0;
			colors[index * 4 + 3] = 1.0;

			uvs[index * 2]	   = j / (float)(detailY * 2);
			uvs[index * 2 + 1] = 1. - (i / (float)detailX);
		}
	}

	uint faceCount = (detailX - 1) * (detailY * 2 + 1);

	GLuint *indices = new GLuint[faceCount * 2 * 3];

	for (uint x = 0; x < detailX - 1; ++x) {
		for (uint y = 0; y < detailY * 2; ++y) {
			uint i = x * (detailY * 2 + 1) + y;

			indices[i * 6]	   = i;
			indices[i * 6 + 1] = i + 1;
			indices[i * 6 + 2] = i + (detailY * 2 + 1);

			indices[i * 6 + 3] = i + 1;
			indices[i * 6 + 4] = i + (detailY * 2 + 1) + 1;
			indices[i * 6 + 5] = i + (detailY * 2 + 1);
		}
	}

	Mesh::init(vertexCount, vertices, normals, uvs, colors, tangents, faceCount * 6, indices);

	delete[] vertices;
	delete[] normals;
	delete[] colors;
	delete[] indices;
	delete[] uvs;
	delete[] tangents;
}

ygl::SphereMesh::SphereMesh(float radius, uint detailX, uint detailY)
	: radius(radius), detailX(detailX), detailY(detailY) {
	init(radius, detailX, detailY);
}

ygl::SphereMesh::SphereMesh(float radius) : SphereMesh(radius, 20, 20) {}

ygl::SphereMesh::SphereMesh() : SphereMesh(1.f) {}

void ygl::SphereMesh::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	IMesh::serialize(out);
	out.write((char *)&radius, sizeof(radius));
	out.write((char *)&detailX, sizeof(detailX));
	out.write((char *)&detailY, sizeof(detailY));
}

ygl::SphereMesh::SphereMesh(std::istream &in) : Mesh(in) {
	in.read((char *)&radius, sizeof(radius));
	in.read((char *)&detailX, sizeof(detailX));
	in.read((char *)&detailY, sizeof(detailY));
	init(radius, detailX, detailY);
}

const char *ygl::QuadMesh::name = "ygl::QuadMesh";

void ygl::QuadMesh::init(float size) {
	float s = size / 2.;
	// clang-format off
	GLfloat vertices[] = {
		-s,  s, 0.,
		 s,  s, 0.,
		 s, -s, 0.,
		-s, -s, 0.,
	};
	GLfloat texCoords[]  = {
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0, 
		0.0, 0.0
	};
	GLfloat colors[]   = {
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 1.0, 1.0, 1.0
	};
	GLuint	indices[]  = {2, 1, 0, 3, 2, 0};
	// clang-format on
	Mesh::init((GLuint)4, vertices, (GLfloat *)nullptr, texCoords, colors, (GLfloat *)nullptr, (GLuint)6, indices);
}

ygl::QuadMesh::QuadMesh(float size) : size(size) { init(size); }

ygl::QuadMesh::QuadMesh() : size(2) { init(size); }

ygl::QuadMesh::QuadMesh(std::istream &in) : Mesh(in) { in.read((char *)&size, sizeof(size)); }

void ygl::QuadMesh::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	IMesh::serialize(out);
	out.write((char *)&size, sizeof(size));
}

const char *ygl::PlaneMesh::name = "ygl::PlaneMesh";

void ygl::PlaneMesh::init(const glm::vec2 &size, const glm::vec2 &detail) {
	uint vertexCount = (detail.x + 1) * (detail.y + 1);

	GLfloat *vertices = new GLfloat[vertexCount * 3];
	GLfloat *normals  = new GLfloat[vertexCount * 3];
	GLfloat *colors	  = new GLfloat[vertexCount * 4];
	GLfloat *uvs	  = new GLfloat[vertexCount * 2];
	GLfloat *tangents = new GLfloat[vertexCount * 3];

	for (int i = 0; i < detail.x + 1; ++i) {
		for (int j = 0; j < detail.y + 1; ++j) {
			uint index = i * (detail.y + 1) + j;

			vertices[index * 3 + 0] = (i / detail.x - 0.5) * size.x;
			vertices[index * 3 + 1] = 0;
			vertices[index * 3 + 2] = (j / detail.y - 0.5) * size.y;

			normals[index * 3 + 0] = 0.;
			normals[index * 3 + 1] = 1.;
			normals[index * 3 + 2] = 0.;

			colors[index * 4 + 0] = i / detail.x;
			colors[index * 4 + 1] = 1.;
			colors[index * 4 + 2] = j / detail.y;
			colors[index * 4 + 3] = 1.;

			uvs[index * 2 + 0] = i / detail.x;
			uvs[index * 2 + 1] = j / detail.y;

			tangents[index * 3 + 0] = 0;
			tangents[index * 3 + 1] = 0;
			tangents[index * 3 + 2] = 1;
		}
	}

	uint faceCount = detail.x * detail.y;

	GLuint *indices = new GLuint[faceCount * 6];

	for (int i = 0; i < detail.x; ++i) {
		for (int j = 0; j < detail.y; ++j) {
			uint index = i * detail.y + j;

			indices[index * 6 + 0] = i * (detail.x + 1) + j;
			indices[index * 6 + 1] = i * (detail.x + 1) + j + 1;
			indices[index * 6 + 2] = (i + 1) * (detail.x + 1) + j;
			indices[index * 6 + 3] = i * (detail.x + 1) + j + 1;
			indices[index * 6 + 4] = (i + 1) * (detail.x + 1) + j + 1;
			indices[index * 6 + 5] = (i + 1) * (detail.x + 1) + j;
		}
	}

	Mesh::init(vertexCount, vertices, normals, uvs, colors, tangents, faceCount * 6, indices);

	delete[] vertices;
	delete[] normals;
	delete[] colors;
	delete[] indices;
	delete[] uvs;
	delete[] tangents;
}

ygl::PlaneMesh::PlaneMesh(const glm::vec2 &size, const glm::vec2 &detail) : size(size), detail(detail) {
	init(size, detail);
}

ygl::PlaneMesh::PlaneMesh(const glm::vec2 &size) : size(size), detail(1) { init(size, detail); }

ygl::PlaneMesh::PlaneMesh() : size(1), detail(1) { init(size, detail); }
void ygl::PlaneMesh::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	IMesh::serialize(out);
	out.write((char *)glm::value_ptr(size), sizeof(size));
	out.write((char *)glm::value_ptr(detail), sizeof(detail));
}

ygl::PlaneMesh::PlaneMesh(std::istream &in) : Mesh(in) {
	in.read((char *)glm::value_ptr(size), sizeof(size));
	in.read((char *)glm::value_ptr(detail), sizeof(detail));
	init(size, detail);
}

#ifndef YGL_NO_ASSIMP
	#include <assimp/Importer.hpp>
	#include <assimp/scene.h>
	#include <assimp/postprocess.h>
	#include <assimp/material.h>
	#include <assimp/version.h>
	#include <assimp_glm_helpers.h>

Assimp::Importer *ygl::MeshFromFile::importer = nullptr;

void ygl::MeshFromFile::terminateLoader() { delete ygl::MeshFromFile::importer; }

const aiScene *ygl::MeshFromFile::loadScene(const std::string &file, unsigned int flags) {
	if (importer == nullptr) {
		importer = new Assimp::Importer();
		importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		uint version_major	  = aiGetVersionMajor();
		uint version_minor	  = aiGetVersionMinor();
		uint version_patch	  = aiGetVersionPatch();
		uint version_revision = aiGetVersionRevision();
		dbLog(ygl::LOG_INFO, "Assimp version:", version_major, ".", version_minor, ".", version_patch, " @ ", std::hex,
			  version_revision);
	}

	const aiScene *scene = importer->ReadFile(file, flags);

	if (!scene) { 
		dbLog(ygl::LOG_ERROR, "[Assimp] ", importer->GetErrorString());
		THROW_RUNTIME_ERR("[Assimp]" + importer->GetErrorString());
	}
	return scene;
}

const aiScene *ygl::MeshFromFile::loadScene(const std::string &file) { return loadScene(file, import_flags); }

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

ygl::Material ygl::MeshFromFile::getMaterial(const aiScene *scene, ygl::AssetManager *asman, std::string filePath,
											 uint m) {
	if (!scene->HasMaterials()) { return Material(); }
	aiMaterial *material = scene->mMaterials[m];	 // Get the current material
	aiString	materialName;						 // The name of the material found in mesh file
	aiReturn	ret;	 // Code which says whether loading something has been successful of not
	std::string dir = filePath.substr(0, filePath.rfind('/') + 1);

	ret = material->Get(AI_MATKEY_NAME, materialName);	   // Get the material name (pass by reference)
	if (ret != AI_SUCCESS) materialName = "";			   // Failed to find material name so makes var empty

	aiColor3D diff(0, 0, 0);
	ret = material->Get(AI_MATKEY_COLOR_DIFFUSE, diff);
	if (ret != AI_SUCCESS) diff = aiColor3D(1, 0, 1);
	aiColor3D emission(0, 0, 0);
	ret = material->Get(AI_MATKEY_COLOR_EMISSIVE, emission);
	if (ret != AI_SUCCESS) emission = aiColor3D(0, 0, 0);
	aiColor3D specular(0, 0, 0);
	ret = material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
	if (ret != AI_SUCCESS) specular = aiColor3D(1, 1, 1);
	aiColor3D transparent(0, 0, 0);
	ret = material->Get(AI_MATKEY_COLOR_TRANSPARENT, transparent);
	if (ret != AI_SUCCESS) transparent = aiColor3D(0, 0, 0);
	float roughness_factor;
	ret = material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness_factor);
	if (ret != AI_SUCCESS) roughness_factor = 1.0f;
	float ior = 1.;
	ret		  = material->Get(AI_MATKEY_REFRACTI, ior);
	if (ret != AI_SUCCESS) ior = 1.;

	glm::vec3 glmAlbedo(diff.r, diff.g, diff.b);
	glm::vec3 glmEmission(emission.r, emission.g, emission.b);
	glm::vec3 glmSpecular(specular.r, specular.g, specular.b);
	glm::vec3 glmTransparent(transparent.r, transparent.g, transparent.b);

	const uint	  TEX_COUNT = 7;
	float		  use_map[TEX_COUNT]{0};
	uint		  map[TEX_COUNT]{0};
	std::string	  map_file[TEX_COUNT];
	aiTextureType mapType[TEX_COUNT]{aiTextureType_NORMALS,	  aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_LIGHTMAP,
									 aiTextureType_METALNESS, aiTextureType_DIFFUSE,		   aiTextureType_EMISSIVE,
									 aiTextureType_OPACITY};
	TextureType	  texType[TEX_COUNT]{TextureType::NORMAL,	TextureType::ROUGHNESS, TextureType::AO,
									 TextureType::METALLIC, TextureType::DIFFUSE,	TextureType::EMISSIVE,
									 TextureType::OPACITY};

	for (uint i = 0; i < TEX_COUNT; ++i) {
		use_map[i]	= getTexture(material, mapType[i], map_file[i]);
		map_file[i] = dir + map_file[i];

		if (use_map[i]) {
			map[i] = asman->getTextureIndex(map_file[i]);
			if (map[i] == (uint)-1) map[i] = asman->addTexture(new Texture2d(map_file[i], texType[i]), map_file[i]);
		}
	}

	ygl::Material mat(glmAlbedo, 0.02, glmEmission, ior, glmTransparent, 0.0, glmSpecular, roughness_factor,
					  roughness_factor, 0., map[0], use_map[0], map[1], use_map[1], map[2], use_map[2], map[3],
					  use_map[3], map[4], use_map[4], map[5], use_map[5], map[6], use_map[6]);
	return mat;
}

const char	  *ygl::MeshFromFile::name		  = "ygl::MeshFromFile";
const aiScene *ygl::MeshFromFile::loadedScene = nullptr;
std::string	   ygl::MeshFromFile::loadedFile  = "";

void ygl::MeshFromFile::loadSceneIfNeeded(const std::string &path) {
	if (loadedScene == nullptr || loadedFile != path) {
		loadedScene = loadScene(path);
		loadedFile	= path;
	}
}

void ygl::MeshFromFile::init(const std::string &path, uint index) {
	loadSceneIfNeeded(path);
	#define scene loadedScene

	if (!scene->HasMeshes()) {
		dbLog(ygl::LOG_ERROR, "Cannot load mesh from file with no meshes in it: ", path);
		return;
	}

	aiMesh	   **meshes	   = scene->mMeshes;
	unsigned int numMeshes = scene->mNumMeshes;

	assert(numMeshes >= 1 && "no meshes in the scene?");
	if (numMeshes <= index) {
		dbLog(ygl::LOG_ERROR, "Error loading mesh from file: ", path, " mesh index out of bounds: ", index);
		return;
	}

	aiMesh		*mesh		   = meshes[index];
	unsigned int verticesCount = mesh->mNumVertices;
	unsigned int indicesCount  = mesh->mNumFaces * 3;
	GLuint		*indices	   = new GLuint[indicesCount];

	unsigned int indexCounter = 0;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		assert(mesh->mFaces->mNumIndices == 3);
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
	if (!(mesh->HasVertexColors(0))) { /*dbLog(ygl::LOG_WARNING, "colors cannot be loaded for model!");*/
	}

	GLint *boneIDs = new GLint[MAX_BONE_INFLUENCE * verticesCount];
	for (uint i = 0; i < MAX_BONE_INFLUENCE * verticesCount; ++i)
		boneIDs[i] = -1;
	GLfloat *boneWeights = new GLfloat[MAX_BONE_INFLUENCE * verticesCount];

	if (mesh->HasBones()) {
		int numBones = mesh->mNumBones;
		for (int boneIndex = 0; boneIndex < numBones; ++boneIndex) {
			int			boneId = -1;
			std::string name   = mesh->mBones[boneIndex]->mName.C_Str();

			fixMixamoBoneName(name);

			if (boneInfoMap.find(name) == boneInfoMap.end()) {
				BoneInfo newBoneInfo;
				newBoneInfo.id	   = bonesCount;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				boneInfoMap[name]  = newBoneInfo;
				boneId			   = bonesCount;
				bonesCount++;
			} else {
				boneId = boneInfoMap[name].id;
			}

			assert(boneId != -1);
			auto weights	= mesh->mBones[boneIndex]->mWeights;
			int	 numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
				unsigned vertexId = weights[weightIndex].mVertexId;
				float	 weight	  = weights[weightIndex].mWeight;
				if (weight == 0) continue;
				assert(vertexId < verticesCount);
				int	 baseIndex = vertexId * MAX_BONE_INFLUENCE;
				bool success   = false;
				for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
					if (boneIDs[baseIndex + i] < 0) {
						boneWeights[baseIndex + i] = weight;
						boneIDs[baseIndex + i]	   = boneId;
						success					   = true;
						break;
					}
				}
				if (!success) {
					dbLog(ygl::LOG_WARNING,
						  "failed to add bone weight!\n\tcurrent weights: ", boneWeights[baseIndex + 0], " ",
						  boneWeights[baseIndex + 1], " ", boneWeights[baseIndex + 2], " ", boneWeights[baseIndex + 3]);
				}
			}
		}
	}

	AnimatedMesh::init(verticesCount, (GLfloat *)mesh->mVertices, (GLfloat *)mesh->mNormals, texCoords,
					   (GLfloat *)mesh->mColors[0], (GLfloat *)mesh->mTangents, boneIDs, boneWeights, indicesCount,
					   indices);

	delete[] indices;
	delete[] boneIDs;
	delete[] boneWeights;
	if (texCoords != nullptr) delete[] texCoords;

	#undef scene
}

ygl::MeshFromFile::MeshFromFile(const std::string &path, uint index) : path(path), index(index) { init(path, index); }

ygl::MeshFromFile::MeshFromFile(std::istream &in) : AnimatedMesh(in) {
	std::getline(in, this->path, '\0');
	in.read((char *)&index, sizeof(index));
	init(this->path, index);
}

void ygl::MeshFromFile::serialize(std::ostream &out) {
	out.write(name, std::strlen(name) + 1);
	IMesh::serialize(out);
	out.write(path.c_str(), path.size() + 1);
	out.write((char *)&index, sizeof(index));
}

int ygl::MeshFromFile::import_flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenNormals;

void ygl::fixMixamoBoneName(std::string &name) {
	if (!name.starts_with("mixamorig")) return;

	auto underscore_pos = name.rfind('_');
	if (underscore_pos != std::string::npos && std::isdigit(name[underscore_pos + 1])) {
		name = name.substr(0, underscore_pos);
	}
	std::replace(name.begin(), name.end(), ':', '_');
}
#endif
