#include <mesh.h>

#include <assert.h>
#include <iostream>

GLuint ygl::Mesh::createVAO() {
	glGenVertexArrays(1, &vao);
	return vao;
}

GLuint ygl::Mesh::createIBO(GLuint *data, int count) {
	indicesCount = count;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(GLuint), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return ibo;
}

void ygl::Mesh::bind() {
	glBindVertexArray(vao);
	enableVBOs();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void ygl::Mesh::unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	disableVBOs();
	glBindVertexArray(0);
}

ygl::Mesh::~Mesh() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &ibo);
}

GLenum ygl::Mesh::getDrawMode() { return drawMode; }

GLuint ygl::Mesh::getIndicesCount() { return indicesCount; }

GLuint ygl::Mesh::getVerticesCount() { return verticesCount; }

GLuint ygl::Mesh::getIBO() { return ibo; }

GLuint ygl::Mesh::getVAO() { return vao; }

void ygl::Mesh::setDrawMode(GLenum mode) { drawMode = mode; }

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
ygl::BasicMesh::BasicMesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors, GLuint indicesCount, GLuint *indices) {
	this->createVAO();
	glBindVertexArray(this->getVAO());
	this->createIBO(indices, indicesCount);
	this->addVBO(0, 3, vertices, vertexCount);
	this->addVBO(1, 3, normals, vertexCount);
	this->addVBO(2, 2, texCoords, vertexCount);
	this->addVBO(3, 4, colors, vertexCount);
	glBindVertexArray(0);
}

ygl::Mesh::VBO ygl::BasicMesh::getVertices() { return vbos[0]; }

ygl::Mesh::VBO ygl::BasicMesh::getNormals() { return vbos[1]; }

ygl::Mesh::VBO ygl::BasicMesh::getTexCoords() { return vbos[2]; }

ygl::Mesh::VBO ygl::BasicMesh::getColors() { return vbos[3]; }