#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/vec3.hpp>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace ygl {
class IMesh {
   protected:
	GLuint vao = -1;
	GLuint ibo = -1;

	GLuint indicesCount	 = 0;
	GLuint verticesCount = 0;

	int drawMode = GL_TRIANGLES;

	GLuint createVAO();
	GLuint createIBO(GLuint *data, int size);

	IMesh(){};	  // protected constructor so that noone can instantiate this

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
	void			 addVBO(GLuint location, GLuint coordSize, GLfloat *data, GLuint count);
	MultiBufferMesh() {}

   public:
	virtual ~MultiBufferMesh();
	void enableVBOs();
	void disableVBOs();
};

class Mesh : public MultiBufferMesh {
   public:
	Mesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
			  GLuint indicesCount, GLuint *indices);
	ygl::IMesh::VBO getVertices();
	ygl::IMesh::VBO getNormals();
	ygl::IMesh::VBO getTexCoords();
	ygl::IMesh::VBO getColors();
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

extern Assimp::Importer *importer;

const aiScene	  *loadScene(const std::string &);
const Mesh *getModel(const aiScene *);

void terminateLoader();

}	  // namespace ygl