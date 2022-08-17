#define _USE_MATH_DEFINES
#include <mesh.h>

#include <assert.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <yoghurtgl.h>
#include <math.h>


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

// note that 'data' is the vertex count, not the length or size of the VBO
void ygl::MultiBufferMesh::addVBO(GLuint attrLocation, GLuint coordSize, GLfloat *data, GLuint count) {
	assert(!(verticesCount && verticesCount != count));
	verticesCount = count;
	GLuint buff;
	glGenBuffers(1, &buff);
	glBindBuffer(GL_ARRAY_BUFFER, buff);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLfloat) * coordSize, data, GL_STATIC_DRAW);
	glVertexAttribPointer(attrLocation, coordSize, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vbos.push_back(VBO(attrLocation, buff, coordSize));
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

// constructs a simple mesh with vertex position (vec3), normal(vec3), texCoord(vec2) and color(vec4).
ygl::Mesh::Mesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
				GLuint indicesCount, GLuint *indices) {
	this->createVAO();
	glBindVertexArray(this->getVAO());
	this->createIBO(indices, indicesCount);
	this->addVBO(0, 3, vertices, vertexCount);
	this->addVBO(1, 3, normals, vertexCount);
	this->addVBO(2, 2, texCoords, vertexCount);
	this->addVBO(3, 4, colors, vertexCount);
	glBindVertexArray(0);
}

ygl::IMesh::VBO ygl::Mesh::getVertices() { return vbos[0]; }

ygl::IMesh::VBO ygl::Mesh::getNormals() { return vbos[1]; }

ygl::IMesh::VBO ygl::Mesh::getTexCoords() { return vbos[2]; }

ygl::IMesh::VBO ygl::Mesh::getColors() { return vbos[3]; }

ygl::Mesh *ygl::makeTriangle() {
	// clang-format off
	GLfloat vertices[] = {
		-0.5, -0.5, -0.,
		 0.5, -0.5, -0.,
		 0.0,  0.5, -0.};
	GLfloat normals[]  = {
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0};
	GLfloat colors[]   = {
		1.0, 0.0, 0.0,
		1.0, 0.0, 1.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 1.0};
	GLuint	indices[]  = {0, 1, 2};
	// clang-format on
	return new Mesh((GLuint)9, vertices, normals, (GLfloat *)nullptr, colors, (GLuint)3, indices);
}

// TODO: this must have better texture coords;
ygl::Mesh *ygl::makeBox(const glm::vec3 &size, const glm::vec3 &detail) {
	uint vertexCount =
		(detail.x + 1) * (detail.y + 1) + (detail.x + 1) * (detail.z + 1) + (detail.y + 1) * (detail.z + 1);
	vertexCount <<= 1;	   // 2 sides per axis

	GLfloat *vertices = new GLfloat[vertexCount * 3];
	GLfloat *normals  = new GLfloat[vertexCount * 3];
	GLfloat *colors	  = new GLfloat[vertexCount * 4];
	GLfloat *uvs	  = new GLfloat[vertexCount * 2];

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
					normals[k * 3 + axis2] = (u * 2 - 1);

					colors[k * 4]	  = i / detail[axis0];
					colors[k * 4 + 1] = j / detail[axis1];
					colors[k * 4 + 2] = 1;
					colors[k * 4 + 3] = 1;

					uvs[k * 2]	   = i / detail[axis0];
					uvs[k * 2 + 1] = j / detail[axis1];
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

	Mesh *mesh = new Mesh(vertexCount, vertices, normals, uvs, colors, faceCount * 6, indices);

	delete[] vertices;
	delete[] normals;
	delete[] colors;
	delete[] indices;
	delete[] uvs;

	return mesh;
}

ygl::Mesh *ygl::makeBox(float x, float y, float z) {
	float sx = x / 2, sy = y / 2, sz = z / 2;
	// clang-format off
	GLfloat vertices[] = {
		sx, -sy, sz,
		-sx, -sy, sz,
		-sx, -sy, -sz,
		-sx, sy, -sz,
		-sx, sy, sz,
		sx, sy, sz,
		sx, sy, -sz,
		sx, sy, sz,
		sx, -sy, sz,
		sx, sy, sz,
		-sx, sy, sz,
		-sx, -sy, sz,
		-sx, -sy, sz,
		-sx, sy, sz,
		-sx, sy, -sz,
		sx, -sy, -sz,
		-sx, -sy, -sz,
		-sx, sy, -sz,
		sx, -sy, -sz,
		sx, sy, -sz,
		sx, -sy, -sz,
		sx, -sy, sz,
		-sx, -sy, -sz,
		sx, sy, -sz};
	GLfloat normals[] = {
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		-0.0f, 0.0f, 1.0f,
		-0.0f, 0.0f, 1.0f,
		-0.0f, 0.0f, 1.0f,
		-1.0f, -0.0f, -0.0f,
		-1.0f, -0.0f, -0.0f,
		-1.0f, -0.0f, -0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		-0.0f, 0.0f, 1.0f,
		-1.0f, -0.0f, -0.0f,
		0.0f, 0.0f, -1.0f};
	GLfloat texCoords[] = {
		1.0f     , 0.333333f,
		1.0f     , 0.666667f,
		0.666667f, 0.666667f,
		1.0f     , 0.333333f,
		0.666667f, 0.333333f,
		0.666667f, 0.0f     ,
		0.0f     , 0.333333f,
		0.0f     , 0.0f     ,
		0.333333f, 0.0f     ,
		0.333333f, 0.0f     ,
		0.666667f, 0.0f     ,
		0.666667f, 0.333333f,
		0.333333f, 1.0f     ,
		0.0f     , 1.0f     ,
		0.0f     , 0.666667f,
		0.333333f, 0.333333f,
		0.333333f, 0.666667f,
		0.0f     , 0.666667f,
		0.666667f, 0.333333f,
		1.0f     , 0.0f     ,
		0.333333f, 0.333333f,
		0.333333f, 0.333333f,
		0.333333f, 0.666667f,
		0.0f     , 0.333333f
	};
	GLfloat colors[] = {
		1, 0, 0, 1,
		0, 1, 0, 1,
		0, 0, 1, 1,
		1, 1, 0, 1,
		0, 1, 1, 1,
		1, 0, 1, 1,
		1, 1, 1, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,
		0, 1, 0, 1,
		0, 0, 1, 1,
		1, 1, 0, 1,
		0, 1, 1, 1,
		1, 0, 1, 1,
		1, 1, 1, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,
		0, 1, 0, 1,
		0, 0, 1, 1,
		1, 1, 0, 1,
		0, 1, 1, 1,
		1, 0, 1, 1,
		1, 1, 1, 1,
		1, 0, 0, 1,};
	GLuint indices[] = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11,
		12, 13, 14,
		15, 16, 17,
		18, 0, 2,
		19, 3, 5,
		20, 6, 8,
		21, 9, 11,
		22, 12, 14,
		23, 15, 17};
	// clang-format on
	return new Mesh((GLuint)24 * 3, vertices, normals, texCoords, colors, (GLuint)12 * 3, indices);
}

ygl::Mesh *ygl::makeBox(const glm::vec3 &dim) { return makeBox(dim.x, dim.y, dim.z); }

ygl::Mesh *ygl::makeCube(float size) { return makeBox(size, size, size); }

ygl::Mesh *ygl::makeCube() { return makeBox(1, 1, 1); }

ygl::Mesh *ygl::makeScreenQuad() {
	// clang-format off
	GLfloat vertices[] = {
		-1.0,  1.0, -0.,
		 1.0,  1.0, -0.,
		 1.0, -1.0, -0.,
		-1.0, -1.0, -0.};
	GLfloat texCoords[]  = {
		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0, 
		0.0, 1.0};
	GLfloat colors[]   = {
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 1.0, 1.0, 1.0
		};
	GLuint	indices[]  = {2, 1, 0, 3, 2, 0};
	// clang-format on
	return new Mesh((GLuint)12, vertices, (GLfloat *)nullptr, texCoords, colors, (GLuint)6, indices);
}

ygl::Mesh *ygl::makeSphere(float radius, uint detailX, uint detailY) {
	uint vertexCount = (detailX * 2 + 1) * detailY;

	GLfloat *vertices = new GLfloat[vertexCount * 3];
	GLfloat *normals  = new GLfloat[vertexCount * 3];
	GLfloat *colors	  = new GLfloat[vertexCount * 4];
	GLfloat *uvs	  = new GLfloat[vertexCount * 2];
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

			colors[index * 4]	  = i / (float)detailX;
			colors[index * 4 + 1] = j / (float)detailY / 2.;
			colors[index * 4 + 2] = 1.0;
			colors[index * 4 + 3] = 1.0;

			uvs[index * 2]	   = i / (float)detailX;
			uvs[index * 2 + 1] = j / (float)(detailY * 2 + 1);
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

	Mesh *mesh = new Mesh(vertexCount, vertices, normals, uvs, colors, faceCount * 6, indices);

	delete[] vertices;
	delete[] normals;
	delete[] colors;
	delete[] indices;
	delete[] uvs;

	return mesh;
}

ygl::Mesh *ygl::makeSphere(float radius) { return makeSphere(radius, 20, 20); }

ygl::Mesh *ygl::makeUnitSphere() { return makeSphere(1.); }

ygl::Mesh *ygl::makePlane(const glm::vec2 &size, const glm::vec2 &detail) {
	uint vertexCount = (detail.x + 1) * (detail.y + 1);

	GLfloat *vertices = new GLfloat[vertexCount * 3];
	GLfloat *normals  = new GLfloat[vertexCount * 3];
	GLfloat *colors	  = new GLfloat[vertexCount * 4];
	GLfloat *uvs	  = new GLfloat[vertexCount * 2];

	for (int i = 0; i < detail.x + 1; ++i) {
		for (int j = 0; j < detail.y + 1; ++j) {
			uint index = i * (detail.y + 1) + j;

			vertices[index * 3 + 0] = (i / detail.x - 0.5)* size.x;
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

	Mesh *mesh = new Mesh(vertexCount, vertices, normals, uvs, colors, faceCount * 6, indices);

	delete[] vertices;
	delete[] normals;
	delete[] colors;
	delete[] indices;
	delete[] uvs;

	return mesh;
}

ygl::Mesh *ygl::makePlane(const glm::vec2 &detail) {
	return makePlane(glm::vec2(1), detail);
}

Assimp::Importer *ygl::importer = nullptr;

const aiScene *ygl::loadScene(const std::string &file) {
	// Assimp::Importer importer;
	if (ygl::importer == nullptr) { ygl::importer = new Assimp::Importer(); }

	const aiScene *scene = ygl::importer->ReadFile(file, aiProcess_CalcTangentSpace | aiProcess_Triangulate |
															 aiProcess_JoinIdenticalVertices | aiProcess_GenNormals |
															 aiProcess_PreTransformVertices);

	if (!scene) {
		dbLog(ygl::LOG_ERROR, "[Assimp]", ygl::importer->GetErrorString());
		return nullptr;
	}
	return scene;
}

const ygl::Mesh *ygl::getModel(const aiScene *scene) {
	using namespace std;
	assert(scene->HasMeshes() && "scene does not have any meshes");
	if (!scene->HasMeshes()) { return nullptr; }

	aiMesh	   **meshes	   = scene->mMeshes;
	unsigned int numMeshes = scene->mNumMeshes;

	assert(numMeshes >= 1 && "no meshes in the scene?");

	aiMesh		*mesh		   = meshes[0];
	unsigned int verticesCount = mesh->mNumVertices;
	unsigned int indicesCount  = mesh->mNumFaces * 3;
	GLuint		*indices	   = new GLuint[indicesCount * sizeof(GLuint)];

	unsigned int indexCounter = 0;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		indices[indexCounter++] = mesh->mFaces[i].mIndices[0];
		indices[indexCounter++] = mesh->mFaces[i].mIndices[1];
		indices[indexCounter++] = mesh->mFaces[i].mIndices[2];
	}

	assert(indexCounter == indicesCount && "something went very wrong");

	if (!(mesh->HasTextureCoords(0))) { dbLog(ygl::LOG_WARNING, "tex coords cannot be loaded for model!"); }
	if (!(mesh->HasVertexColors(0))) { dbLog(ygl::LOG_WARNING, "colors cannot be loaded for model!"); }

	ygl::Mesh *result =
		new ygl::Mesh(verticesCount, (GLfloat *)mesh->mVertices, (GLfloat *)mesh->mNormals,
					  (GLfloat *)mesh->mTextureCoords[0], (GLfloat *)mesh->mColors[0], indicesCount, indices);
	// result->setDrawMode(GL_POINTS);

	delete[] indices;

	delete ygl::importer;

	return result;
}