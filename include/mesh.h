#pragma once

#include <yoghurtgl.h>
#include <vector>
#include <glm/glm.hpp>
#include <string>

typedef unsigned int uint;

namespace ygl {
class IMesh {
   protected:
	GLuint vao = -1;
	GLuint ibo = -1;

	GLuint indicesCount	 = 0;
	GLuint verticesCount = 0;

	GLenum drawMode = GL_TRIANGLES;
	GLenum depthfunc = GL_LESS;
	GLenum polygonMode = GL_FILL;
	bool cullFace = true;

	GLuint createVAO();
	GLuint createIBO(GLuint *data, int size);

	IMesh(){};	   // protected constructor so that noone can instantiate this

   public:
	virtual ~IMesh();

	void bind();
	void unbind();

	virtual void enableVBOs()  = 0;
	virtual void disableVBOs() = 0;

	GLuint getIndicesCount();
	GLuint getVerticesCount();
	GLenum getDrawMode();
	GLuint getVAO();
	GLuint getIBO();

	void setDrawMode(GLenum drawMode);
	void setCullFace(bool cullFace);
	void setDepthFunc(GLenum depthFunc);
	void setPolygonMode(GLenum polygonMode);

	class VBO {
	   public:
		GLuint location;
		GLuint bufferId;
		GLuint coordSize;
		VBO(GLuint location, GLuint bufferId, int coordSize)
			: location(location), bufferId(bufferId), coordSize(coordSize) {}
	};

};

class MultiBufferMesh : public IMesh {
   protected:
	std::vector<VBO> vbos;
	MultiBufferMesh() {}

   public:
	void addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer, GLuint indexDivisor, GLsizei stride,
				const void *pointer);
	void addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer, GLuint indexDivisor);
	void addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer);
	void addVBO(GLuint location, GLuint coordSize, GLfloat *data, GLuint count, GLuint indexDivisor);
	void addVBO(GLuint location, GLuint coordSize, GLfloat *data, GLuint count);

	virtual ~MultiBufferMesh();
	void enableVBOs();
	void disableVBOs();
};

class Mesh : public MultiBufferMesh {
   public:
	Mesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
		 GLfloat *tangents, GLuint indicesCount, GLuint *indices);
	ygl::IMesh::VBO getVertices();
	ygl::IMesh::VBO getNormals();
	ygl::IMesh::VBO getTexCoords();
	ygl::IMesh::VBO getColors();
	ygl::IMesh::VBO getTangents();
};

Mesh *makeTriangle();

Mesh *makeBox(const glm::vec3 &size, const glm::vec3 &detail);
Mesh *makeBox(float x, float y, float z);
Mesh *makeBox(const glm::vec3 &dim);
Mesh *makeCube(float size);
Mesh *makeCube();

Mesh *makeScreenQuad();

Mesh *makeSphere(float radius, uint detailX, uint detailY);
Mesh *makeSphere(float radius);
Mesh *makeUnitSphere();

Mesh *makePlane(const glm::vec2 &size, const glm::vec2 &detail);
Mesh *makePlane(const glm::vec2 &detail);
}	  // namespace ygl
