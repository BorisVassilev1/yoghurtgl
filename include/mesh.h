#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/vec3.hpp>

namespace ygl {
class Mesh {
   protected:
	GLuint vao = -1;
	GLuint ibo = -1;

	GLuint indicesCount	 = 0;
	GLuint verticesCount = 0;

	int drawMode = GL_TRIANGLES;

	GLuint createVAO();
	GLuint createIBO(GLuint *data, int size);

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
		VBO(GLuint location, GLuint bufferId, int coordSize)
			: location(location), bufferId(bufferId), coordSize(coordSize) {}
	};
};

class MultiBufferMesh : public Mesh {
   protected:
	std::vector<VBO> vbos;
	void			 addVBO(GLuint location, GLuint coordSize, GLfloat *data, GLuint count);
	MultiBufferMesh() {}

   public:
	virtual ~MultiBufferMesh();
	void enableVBOs();
	void disableVBOs();
};

class BasicMesh : public MultiBufferMesh {
   public:
	BasicMesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
			  GLuint indicesCount, GLuint *indices);
	ygl::Mesh::VBO getVertices();
	ygl::Mesh::VBO getNormals();
	ygl::Mesh::VBO getTexCoords();
	ygl::Mesh::VBO getColors();
};

inline BasicMesh makeTriangle() {
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
	return BasicMesh((GLuint)9, vertices, normals, (GLfloat *)nullptr, colors, (GLuint)3, indices);
}

inline BasicMesh makeBox(float x, float y, float z) {
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
	return BasicMesh((GLuint)24 * 3, vertices, normals, texCoords, colors, (GLuint)12 * 3, indices);
}

inline BasicMesh makeBox(glm::vec3 dim) { return makeBox(dim.x, dim.y, dim.z); }

inline BasicMesh makeCube(float size) { return makeBox(size, size, size); }

inline BasicMesh makeCube() { return makeBox(1, 1, 1); }

}	  // namespace ygl