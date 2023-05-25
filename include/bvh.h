#pragma once

/// @file bvh.h
/// @brief Primitive intersection acceleration
///
/// COPY PASTA FROM https://github.com/BorisVassilev1/urban-spork
/// with a lot of edits, of course

#include <cstring>
#include <glm/glm.hpp>
#include <ostream>
#include <vector>
#include <memory>
#include "transformation.h"
#include <timer.h>

#include <material.h>
#include <mesh.h>

namespace ygl {
namespace bvh {
///
/// Data for an intersection between a ray and scene primitive
struct Intersection {
	float		   t = -1.f;			   ///< Position of the intersection along the ray
	glm::vec3	   p;					   ///< The intersection point
	glm::vec3	   normal;				   ///< The normal at the intersection
	ygl::Material *material = nullptr;	   ///< Material of the intersected primitive
};

///
/// Ray represented by origin and direction
struct Ray {
	glm::vec3 origin;
	glm::vec3 dir;

	Ray() {}

	Ray(const glm::vec3 &origin, const glm::vec3 &dir) : origin(origin), dir(dir) {}

	glm::vec3 at(float t) const { return origin + dir * t; }
};

/**
 * struct BBox - Axis Aligned Bouding Box
 */
struct BBox {
	glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	BBox() = default;

	BBox(const glm::vec3 &min, const glm::vec3 &max);

	/// @brief Checks if the box is small enough to be ignored
	/// @return true if it is small enough, false otherwise
	bool isEmpty() const;

	/// @brief Extends this BBox to include both itself and \a other
	void add(const BBox &other);

	/// @brief Expand the box with a single point
	void add(const glm::vec3 &point);

	/// @brief Checks if a point is in this Bounding Box
	bool inside(const glm::vec3 &point) const;

	/// @brief Split the box in 8 equal parts, children are not sorted in any way
	/// @param parts [out] - where to write the children
	void octSplit(BBox parts[8]) const;

	/// @brief Compute the intersection with another box
	///	@return empty box if there is no intersection
	BBox boxIntersection(const BBox &other) const;

	/// @brief Check if a ray intersects the box and find the distance
	bool testIntersect(const Ray &ray, float &t) const;

	/// @brief Check if a ray intersects the box
	bool testIntersect(const Ray &ray) const;

	/// @brief get the center of the box
	glm::vec3 center() const;

	/// @brief calculate the surface area of the BBox. Used for SAH
	float surfaceArea() const;
};

struct Intersectable {
	/// @brief Called after scene is fully created and before rendering starts
	///	       Used to build acceleration structures
	virtual void onBeforeRender() {}

	/// @brief Default implementation intersecting the bbox of the primitive, overriden if possible more efficiently
	virtual bool boxIntersect(const BBox &other) = 0;

	/// @brief Default implementation adding the whole bbox, overriden for special cases
	virtual void expandBox(BBox &other) = 0;

	/// @brief Get the center
	virtual glm::vec3 getCenter() = 0;

	/**
	 * @brief Writes the Intersectable to \a buff at position \a offset
	 *
	 * @param buff - buffer to write to
	 * @param offset - offset from the beginning of \a buff to write at
	 */
	virtual void writeTo(char *buff, std::size_t offset) = 0;
	/**
	 * @brief Prints the Intersectable in human-readable text format to \a os
	 *
	 * @param os - stream to write to
	 */
	virtual void print(std::ostream &os) = 0;

	virtual ~Intersectable() = default;
};

/// Base class for scene object
struct Primitive : Intersectable {
	BBox box;

	/// @brief Default implementation intersecting the bbox of the primitive, overriden if possible more efficiently
	bool boxIntersect(const BBox &other) override { return !box.boxIntersection(other).isEmpty(); }

	/// @brief Default implementation adding the whole bbox, overriden for special cases
	void expandBox(BBox &other) override { other.add(box); }

	/// @brief Get the center
	glm::vec3 getCenter() override { return box.center(); }

	~Primitive() = default;
};

enum PrimitiveType : uint {
	TRIANGLE = 0,
	SPHERE	 = 1,
	BOX		 = 2,
};

/**
 * struct Triangle - a Triangle Primitive
 */
struct Triangle : public Primitive {
	uint indices[3];	 ///< indices in the mesh
	uint matIdx;		 ///< material index for the Triangle
	union {
		glm::vec3 positions[3];
		static struct {
			glm::vec3 A, B, C;	   ///< the three points defining the Triangle
		};
	};

	/**
	 * @brief Constructs a Triangle from all data needed
	 *
	 * @param v1 - first index
	 * @param v2 - second index
	 * @param v3 - third index
	 * @param A - first point
	 * @param B - second point
	 * @param C - third point
	 * @param matIdx - index of the material to be used
	 */
	Triangle(uint v1, uint v2, uint v3, glm::vec3 A, glm::vec3 B, glm::vec3 C, uint matIdx)
		: indices{v1, v2, v3}, matIdx(matIdx), A(A), B(B), C(C) {
		box.add(A);
		box.add(B);
		box.add(C);
	}

	bool	  boxIntersect(const BBox &box) override;
	void	  expandBox(BBox &box) override;
	glm::vec3 getCenter() override;
	void	  writeTo(char *buff, std::size_t offset) override;
	void	  print(std::ostream &os) override;
};

/**
 * struct SpherePrimitive - A Sphere Primitive
 */
struct SpherePrimitive : Primitive {
	glm::vec3 position;
	float	  radius = 1;
	uint	  matIdx;

	SpherePrimitive() {}
	SpherePrimitive(const glm::vec3 &position, float radius, uint matIdx)
		: position(position), radius(radius), matIdx(matIdx) {
		box = BBox(position - radius, position + radius);
	}

	glm::vec3 getCenter() override { return position; }
	void	  writeTo(char *buff, std::size_t offset) override;
	void	  print(std::ostream &os) override;
};

/**
 * struct BoxPrimitive - A Box Primitive - same as the abstract class Primitive, but not abstract
 */
struct BoxPrimitive : Primitive {
	uint matIdx;

	BoxPrimitive() {}
	BoxPrimitive(const glm::vec3 &min, const glm::vec3 &max, uint matIdx) : matIdx(matIdx) { box = BBox(min, max); }

	void writeTo(char *buff, std::size_t offset) override;
	void print(std::ostream &os) override;
};

/// Interface for an acceleration structure for any intersectable primitives
struct IntersectionAccelerator {
	enum class Purpose { Generic, Mesh, Instances };

	/// @brief Add the primitive to the accelerated list
	/// @param prim - non owning pointer
	virtual void addPrimitive(Intersectable *prim) = 0;

	/// @brief Clear all data allocated by the accelerator
	virtual void clear() = 0;

	/// @brief Build all the internal data for the accelerator
	///	@param purpose - the purpose of the tree, implementation can use it as hint for internal parameters
	virtual void build(Purpose purpose = Purpose::Generic) = 0;

	/// @brief Check if the accelerator is built
	virtual bool isBuilt() const = 0;

	virtual ~IntersectionAccelerator() = default;
};

typedef std::unique_ptr<IntersectionAccelerator> AcceleratorPtr;
AcceleratorPtr									 makeDefaultAccelerator();

/**
 * @brief A Binary Volume Hierarchy.
 * build() builds a tree that can then be optimised for GPU usage and sent to the VRAM with buildGPUTree()
 *
 * @note some comments may be misleading since now this structure is used to do stuff on the GPU, the CPU intersection
 * has been removed
 */
class BVHTree : public IntersectionAccelerator {
	// Node structure
	// does not need a parent pointer since the intersection implementation will be recursive
	// I can afford that since the code will run on CPU which will not suffer from it.
	// This will simplify the implementation a lot.
	struct Node {
		BBox box;
		union {
			Node *children[2];
			static struct {
				Node *left = nullptr, *right = nullptr;
			};
		};
		std::vector<Intersectable *> primitives;
		char						 splitAxis;
		Node() : left(nullptr), right(nullptr), splitAxis(-1) {}
		bool isLeaf() { return children[0] == nullptr; }
	};

	// faster intersection tree node;
	// left child will always be next in the array, right child is a index in the nodes array.
	struct FastNode {
		BBox			  box;
		long unsigned int right;	 // 4e9 is a puny number of primitives => long, not int
		Intersectable	**primitives;
		char			  splitAxis;
		bool			  isLeaf() {
			 // 0th node will always be the root so no node points to it as its right child
			 return right == 0;
		}
	};

	struct alignas(16) GPUNode {
		glm::vec3 min;
		uint	  parent;
		glm::vec3 max;
		uint	  right;
		uint	  primOffset;
		uint	  primCount;
		bool	  isLeaf() { return right == 0; }
	};

	// all primitives added
	std::vector<Intersectable *> allPrimitives;
	// root of the construction tree
	Node *root = nullptr;
	// nodes of the fast traversal tree
	std::vector<GPUNode> gpuNodes;
	bool				 built = false;
	// cost for traversing a parent node. It is assumed that the intersection cost with a primitive is 1.0
	static constexpr float SAH_TRAVERSAL_COST = 0.125;
	// the number of splits SAH will try.
	static constexpr int SAH_TRY_COUNT		  = 5;
	static constexpr int MAX_DEPTH			  = 50;
	static constexpr int MIN_PRIMITIVES_COUNT = 6;
	// when a node has less than that number of primitives it will sort them and always split in the middle
	//
	// theoretically:
	// the higher this is, the better the tree and the slower its construction
	//
	// practically:
	// when setting this to something higher, the tree construction time changes by verry little
	// and the render time goes up. When this is set to 1e6 (basically always sort), makes
	// rendering about 30% slower than when it is 0 (never split perfectly). (on the instanced dragons scene)
	// Perfect splits clearly produce shallower trees, which should make rendering faster ... but it doesn't.
	//
	// I guess SAH is too good
	static constexpr int PERFECT_SPLIT_THRESHOLD = 20;

	int		 depth			 = 0;	  ///< depth of the tree
	int		 leafSize		 = 0;	  ///< size of the largest leaf
	long int leavesCount	 = 0;	  ///< hOw MaNy LeAvEs
	long int nodeCount		 = 0;	  ///< HoW mAnY nOdEs
	long int primitivesCount = 0;	  ///< how many primitives are in the structure

	void clear(Node *node);				   ///< clears the entire CPU tree starting from \a node
	void clearConstructionTree();		   ///< clears the entire CPU tree
	void build(Node *node, int depth);	   ///< builds the CPU tree

	bool isBuilt() const override { return built; }		///< checks if the tree is built

	/// @brief computes the SAH cost for a split on a given axis.
	/// ratio equals the size of the left child on the chosen axis over
	/// the size of the parent node size on that axis.
	float costSAH(const Node *node, int axis, float ratio);

	/**
	 * helper function for constructing the GPU tree.
	 */
	void buildGPUTree_h(BVHTree::Node *node, unsigned long int parent, std::vector<BVHTree::GPUNode> &gpuNodes,
						std::vector<Intersectable *> &primitives);

	void buildGPUTree();	 ///< builds a tree for fast traversal on the GPU

   public:
	void addPrimitive(Intersectable *prim) override;
	/**
	 * @brief Adds all triangles in the given \a mesh and translates them with \a transform.
	 *
	 * @param mesh - the mesh to be added to the BVH
	 * @param transform - a Transformation for the model
	 */
	void addPrimitive(ygl::Mesh *mesh, ygl::Transformation &transform);
	void clear() override;
	void build(Purpose purpose = Purpose::Generic) override;
	~BVHTree();
};
}	  // namespace bvh
}	  // namespace ygl
