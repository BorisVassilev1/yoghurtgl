#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

namespace ygl {
class Mesh {
   protected:
	GLuint vao = -1;
	GLuint ibo = -1;

	GLuint indicesCount = 0;
	GLuint verticesCount = 0;

	int drawMode = GL_TRIANGLES;

	GLuint createVAO();
	GLuint createIBO(GLuint* data, int size);

	Mesh(){};	  // protected constructor so that noone can instantiate this

   public:
	virtual ~Mesh();

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

	class VBO {
	   public:
		const GLuint location;
		const GLuint bufferId;
		const GLuint coordSize;
		VBO(GLuint location, GLuint bufferId, int coordSize) : location(location), bufferId(bufferId), coordSize(coordSize) {}
	};
};

class MultiBufferMesh : public Mesh {
   protected:
	std::vector<VBO> vbos;
	void addVBO(GLuint location, GLuint coordSize, GLfloat* data, GLuint count);
	MultiBufferMesh() {}

   public:
	virtual ~MultiBufferMesh();
	void enableVBOs();
	void disableVBOs();
};

class BasicMesh : public MultiBufferMesh {
	public:
	BasicMesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors, GLuint indicesCount, GLuint *indices);
	ygl::Mesh::VBO getVertices();
	ygl::Mesh::VBO getNormals();
	ygl::Mesh::VBO getTexCoords();
	ygl::Mesh::VBO getColors();
};

}	  // namespace ygl