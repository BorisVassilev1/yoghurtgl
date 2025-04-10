#pragma once

#include <yoghurtgl.h>

#include <iostream>
#include <ostream>
#include <vector>
#include <unordered_map>

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
	void addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer, GLenum type, GLuint indexDivisor, GLsizei stride,
				const void *pointer);
	void addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer, GLenum type, GLuint indexDivisor);
	void addVBO(GLuint attrLocation, GLuint coordSize, GLuint buffer, GLenum type);
	template <class T>
	void addVBO(GLuint location, GLuint coordSize, T *data, GLenum type, GLuint count, GLuint indexDivisor);
	template <class T>
	void addVBO(GLuint location, GLuint coordSize, T *data, GLenum type, GLuint count);

	virtual ~MultiBufferMesh();
	void enableVBOs();
	void disableVBOs();
};

// note that 'count' is the vertex count, not the length or size of the VBO
template <class T>
void MultiBufferMesh::addVBO(GLuint attrLocation, GLuint coordSize, T *data, GLenum type, GLuint count,
								  GLuint indexDivisor) {
	GLuint buff;
	glGenBuffers(1, &buff);
	glBindBuffer(GL_ARRAY_BUFFER, buff);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(T) * coordSize, data, GL_STATIC_DRAW);
	addVBO(attrLocation, coordSize, buff, type, indexDivisor);
}

template <class T>
void MultiBufferMesh::addVBO(GLuint attrLocation, GLuint coordSize, T *data, GLenum type, GLuint count) {
	addVBO(attrLocation, coordSize, data, type, count, 0);
}

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
	IMesh::VBO getVertices();
	IMesh::VBO getNormals();
	IMesh::VBO getTexCoords();
	IMesh::VBO getColors();
	IMesh::VBO getTangents();
};

const int MAX_BONE_INFLUENCE = 4;
struct BoneInfo {
	uint	  id;
	glm::mat4 offset;
};

class AnimatedMesh : public Mesh {
   protected:
	std::unordered_map<std::string, BoneInfo> boneInfoMap;
	uint									  bonesCount = 0;
	void init(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
			  GLfloat *tangents, GLint *boneIDs, GLfloat *weights, GLuint indicesCount, GLuint *indices);

   public:
	AnimatedMesh(){};
	AnimatedMesh(std::istream &in) : Mesh(in) {}
	AnimatedMesh(GLuint vertexCount, GLfloat *vertices, GLfloat *normals, GLfloat *texCoords, GLfloat *colors,
				 GLfloat *tangents, GLint *boneIDs, GLfloat *weights, GLuint indicesCount, GLuint *indices);
	IMesh::VBO							   getBoneIds();
	IMesh::VBO							   getWeights();
	std::unordered_map<std::string, BoneInfo> &getBoneInfoMap() { return boneInfoMap; }
	uint									  &getBoneCount() { return bonesCount; }
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

class MeshFromFile : public AnimatedMesh {
	std::string path;
	uint		index;
	void		init(const std::string &path, uint index);

	static const aiScene	*loadScene(const std::string &file, unsigned int flags);
	static const aiScene	*loadScene(const std::string &file);
	static Assimp::Importer *importer;

   public:
	static void			  terminateLoader();
	static Material		  getMaterial(const aiScene *, AssetManager *asman, std::string filePath, uint i);
	static void			  getAnimations(const aiScene *, AssetManager *asman, std::string filePath);
	static const aiScene *loadedScene;
	static std::string	  loadedFile;
	static void			  loadSceneIfNeeded(const std::string &path);
	static const char	 *name;
	MeshFromFile(const std::string &path, uint index = 0);
	MeshFromFile(std::istream &in);

	void serialize(std::ostream &out) override;

	static int import_flags;
};

void fixMixamoBoneName(std::string &name);

#endif
}	  // namespace ygl
