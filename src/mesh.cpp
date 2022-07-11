#include <mesh.h>

#include <assert.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <yoghurtgl.h>

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

ygl::Mesh *ygl::makeSphere() {
	int	  detailX = 20;
	int	  detailY = 20;
	float radius  = 1;

	uint vertexCount = detailX * 2 * detailY;

	GLfloat *vertices = new GLfloat[vertexCount * 3];
	GLfloat *normals  = new GLfloat[vertexCount * 3];
	GLfloat *colors	  = new GLfloat[vertexCount * 4];
	for (int i = 0; i < detailX; ++i) {
		float lon = i * M_PI / (detailX -1);
		for (int j = 0; j < detailY * 2; ++j) {
			float lat = j * M_PI / (detailY);

			int index = j + i * detailY * 2;

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
		}
	}

	uint faceCount = (detailX - 1) * (detailY * 2);

	GLuint *indices = new GLuint[faceCount * 2 * 3];
	for (int i = 0; i < faceCount; ++i) {
		indices[i * 6]	   = i;
		indices[i * 6 + 1] = i + 1;
		indices[i * 6 + 2] = i + detailY * 2;

		indices[i * 6 + 3] = i + 1;
		indices[i * 6 + 4] = i + detailY * 2;
		indices[i * 6 + 5] = i + detailY * 2 + 1;
	}

	Mesh *mesh = new Mesh(vertexCount, vertices, normals, (GLfloat *)nullptr, colors, faceCount * 6, indices);
	
	return mesh;
}

Assimp::Importer *ygl::importer = nullptr;

const aiScene *ygl::loadScene(const std::string &file) {
	// Assimp::Importer importer;
	if (ygl::importer == nullptr) { ygl::importer = new Assimp::Importer(); }

	const aiScene *scene = ygl::importer->ReadFile(file, aiProcess_CalcTangentSpace | aiProcess_Triangulate |
															 aiProcess_JoinIdenticalVertices | aiProcess_GenNormals |
															 aiProcess_PreTransformVertices);

	if (!scene) {
		std::cerr << ygl::importer->GetErrorString() << std::endl;
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