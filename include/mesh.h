#pragma once

#include <yoghurtgl.h>
#include <iostream>
#include <ostream>
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <material.h>
#include <serializable.h>

#ifndef YGL_NO_ASSIMP
	#include <assimp/Importer.hpp>
	#include <assimp/scene.h>
#endif

typedef unsigned int uint;

/**
 * @file mesh.h
 * @brief Mesh definitions
 */

namespace ygl {
/**
 * @brief Interface for Mesh classes
 */
class IMesh : public ISerializable {
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

	IMesh(const IMesh &other)			 = delete;
	IMesh &operator=(const IMesh &other) = delete;

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

	void serialize(std::ostream &out) override;

	/**
	 * @brief Vertex Buffer Object. see the OpenGl wiki
	 */
	class VBO {
	   public:
		GLuint location;
		GLuint bufferId;
		GLuint coordSize;
		VBO(GLuint location, GLuint bufferId, int coordSize)
			: location(location), bufferId(bufferId), coordSize(coordSize) {}
	};
};

/**
 * @brief A Mesh that allocates separate buffers for the different vertex atttributes like position, colors, normals,
 * etc. Each such buffer is a VBO that has to be added via addVBO. @see Mesh for possible implementation.
 */
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

/**
 * @brief Default Mesh that has Vertices, Normals, Texture Coordinates, Colors and Tangents for its vertices
 */
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

/**
 * @brief A simple Box Mesh.
 */
class BoxMesh : public Mesh {
	glm::vec3 size;
	glm::vec3 resolution;

   protected:
	void init(const glm::vec3 &size, const glm::vec3 &detail);

   public:
	BoxMesh(std::istream &in);
	BoxMesh(const glm::vec3 &size, const glm::vec3 &detail);
	BoxMesh(const glm::vec3 &dim);
	BoxMesh(float size);
	BoxMesh();

	static const char *name;
	void			   serialize(std::ostream &out) override;
};

/**
 * @brief A Simple Sphere Mesh
 */
class SphereMesh : public Mesh {
	float radius;
	uint  detailX, detailY;

   protected:
	void init(float radius, uint detailX, uint detailY);

   public:
	SphereMesh(float radius, uint detailX, uint detailY);
	SphereMesh(float radius);
	SphereMesh();
	SphereMesh(std::istream &in);

	static const char *name;
	void			   serialize(std::ostream &out) override;
};

/**
 * @brief The mesh of a Quad with 4 vertices
 */
class QuadMesh : public Mesh {
	float size;

   protected:
	void init(float size);

   public:
	QuadMesh(float size);
	QuadMesh();
	QuadMesh(std::istream &in);

	static const char *name;
	void			   serialize(std::ostream &out) override;
};

/**
 * @brief a Plane Mesh.
 */
class PlaneMesh : public Mesh {
	glm::vec2 size, detail;

   protected:
	void init(const glm::vec2 &size, const glm::vec2 &detail);

   public:
	PlaneMesh(const glm::vec2 &size, const glm::vec2 &detail);
	PlaneMesh(const glm::vec2 &size);
	PlaneMesh();
	PlaneMesh(std::istream &in);

	static const char *name;
	void			   serialize(std::ostream &out) override;
};

#ifndef YGL_NO_ASSIMP
class AssetManager;

class MeshFromFile : public Mesh {
	std::string path;
	uint		index;
	void		init(const std::string &path, uint index);

	static const aiScene	*loadScene(const std::string &file, unsigned int flags);
	static const aiScene	*loadScene(const std::string &file);
	static Assimp::Importer *importer;

   public:
	static void			  terminateLoader();
	static Material		  getMaterial(const aiScene *, AssetManager *asman, std::string filePath, uint i);
	static const aiScene *loadedScene;
	static std::string	  loadedFile;
	static void			  loadSceneIfNeeded(const std::string &path);
	static const char	 *name;
	MeshFromFile(const std::string &path, uint index = 0);
	MeshFromFile(std::istream &in);

	void serialize(std::ostream &out) override;
};
#endif
}	  // namespace ygl
