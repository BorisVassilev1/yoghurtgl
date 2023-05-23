#pragma once

#include <yoghurtgl.h>
#include <iostream>
#include <ostream>
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <serializable.h>

typedef unsigned int uint;

namespace ygl {
class IMesh : public ISerializable{
   protected:
	GLuint vao = -1;
	GLuint ibo = -1;

	GLuint indicesCount	 = 0;
	GLuint verticesCount = 0;

	GLenum drawMode	   = GL_TRIANGLES;
	GLenum depthfunc   = GL_LESS;
	GLenum polygonMode = GL_FILL;
	bool   cullFace	   = true;

	GLuint createVAO();
	GLuint createIBO(GLuint *data, int size);

	IMesh(){};	   // protected constructor so that noone can instantiate this
	IMesh(std::istream &in);
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

	void			   serialize(std::ostream &out) override;
	void			   deserialize(std::istream &in) override {};

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
	MultiBufferMesh(std::istream &in) : IMesh(in) {}

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
protected:
	Mesh() {}
	Mesh(std::istream &in) : MultiBufferMesh(in) {}

	void init(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
		 GLfloat *tangents, GLuint indicesCount, GLuint *indices);
   public:
	Mesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
		 GLfloat *tangents, GLuint indicesCount, GLuint *indices);
	ygl::IMesh::VBO getVertices();
	ygl::IMesh::VBO getNormals();
	ygl::IMesh::VBO getTexCoords();
	ygl::IMesh::VBO getColors();
	ygl::IMesh::VBO getTangents();
};

class BoxMesh : public Mesh {
	glm::vec3 size;
	glm::vec3 resolution;
	protected:
	void init(const glm::vec3 &size, const glm::vec3 &detail);
   public:
	BoxMesh(std::istream &in, const std::string &path);
	BoxMesh(const glm::vec3 &size, const glm::vec3 &detail);
	BoxMesh(const glm::vec3 &dim);
	BoxMesh(float size);
	BoxMesh();
	
	static const char *name;
	void			   serialize(std::ostream &out) override;
	void			   deserialize(std::istream &in) override;
};

class SphereMesh : public Mesh {
	float radius;
	uint detailX, detailY;
	protected:

	void init(float radius, uint detailX, uint detailY);
	public:
	
	SphereMesh(float radius, uint detailX, uint detailY);
	SphereMesh(float radius);
	SphereMesh();
	SphereMesh(std::istream &in, const std::string&path);

	static const char *name;
	void			   serialize(std::ostream &out) override;
	void			   deserialize(std::istream &in) override;
};

Mesh *makeTriangle();

Mesh *makeScreenQuad();

Mesh *makePlane(const glm::vec2 &size, const glm::vec2 &detail);
Mesh *makePlane(const glm::vec2 &detail);
}	  // namespace ygl
