#pragma once

// COPY PASTA FROM https://github.com/BorisVassilev1/urban-spork
// with some edits, of course

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <timer.h>

#include <material.h>
#include <mesh.h>

/// Data for an intersection between a ray and scene primitive
struct Intersection {
	float		   t = -1.f;			   ///< Position of the intersection along the ray
	glm::vec3	   p;					   ///< The intersection point
	glm::vec3	   normal;				   ///< The normal at the intersection
	ygl::Material *material = nullptr;	   ///< Material of the intersected primitive
};

/// Ray represented by origin and direction
struct Ray {
	glm::vec3 origin;
	glm::vec3 dir;

	Ray() {}

	Ray(const glm::vec3 &origin, const glm::vec3 &dir) : origin(origin), dir(dir) {}

	glm::vec3 at(float t) const { return origin + dir * t; }
};

struct BBox {
	glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	BBox() = default;

	BBox(const glm::vec3 &min, const glm::vec3 &max);

	bool isEmpty() const;

	void add(const BBox &other);

	/// @brief Expand the box with a single point
	void add(const glm::vec3 &point);

	bool inside(const glm::vec3 &point) const;

	/// @brief Split the box in 8 equal parts, children are not sorted in any way
	/// @param parts [out] - where to write the children
	void octSplit(BBox parts[8]) const;

	/// @brief Compute the intersection with another box
	///	@return empty box if there is no intersection
	BBox boxIntersection(const BBox &other) const;

	/// @brief Check if a ray intersects the box
	bool testIntersect(const Ray &ray, float &t) const;

	bool testIntersect(const Ray &ray) const;

	// get the center of the box
	glm::vec3 center() const;

	float surfaceArea() const;
};

struct Intersectable {
	/// @brief Called after scene is fully created and before rendering starts
	///	       Used to build acceleration structures
	virtual void onBeforeRender() {}

	/// @brief Intersect a ray with the primitive allowing intersection in (tMin, tMax) along the ray
	/// @param ray - the ray
	/// @param tMin - near clip distance
	/// @param tMax - far clip distance
	/// @param intersection [out] - data for intersection if one is found
	/// @return true when intersection is found, false otherwise
	virtual bool intersect(const Ray &ray, float tMin, float tMax, Intersection &intersection) = 0;

	/// @brief Default implementation intersecting the bbox of the primitive, overriden if possible more efficiently
	virtual bool boxIntersect(const BBox &other) = 0;

	/// @brief Default implementation adding the whole bbox, overriden for special cases
	virtual void expandBox(BBox &other) = 0;

	/// @brief Get the center
	virtual glm::vec3 getCenter() = 0;

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

typedef std::unique_ptr<Primitive> PrimPtr;
typedef std::shared_ptr<Primitive> SharedPrimPtr;

struct Triangle : public Primitive {
	int indices[3];
	union {
		glm::vec3 positions[3];
		static struct {
			glm::vec3 A, B, C;
		};
	};

	Triangle(int v1, int v2, int v3, glm::vec3 A, glm::vec3 B, glm::vec3 C) : indices{v1, v2, v3}, A(A), B(B), C(C) {
		box.add(A);
		box.add(B);
		box.add(C);
	}

	bool	  intersect(const Ray &ray, float tMin, float tMax, Intersection &intersection) override;
	bool	  boxIntersect(const BBox &box) override;
	void	  expandBox(BBox &box) override;
	glm::vec3 getCenter() override;
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

	/// @brief Implement intersect from Intersectable but don't inherit the Interface
	virtual bool intersect(const Ray &ray, float tMin, float tMax, Intersection &intersection) = 0;

	virtual ~IntersectionAccelerator() = default;
};

typedef std::unique_ptr<IntersectionAccelerator> AcceleratorPtr;
AcceleratorPtr									 makeDefaultAccelerator();

struct BVHTree : IntersectionAccelerator {
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

	// the simplest possible allocator that could be
	template <class T>
	class StackAllocator {
		T				 *buff	  = nullptr;
		long unsigned int maxSize = 0;
		long unsigned	  size	  = 0;

	   public:
		StackAllocator(){};

		void init(size_t maxSize) {
			if (buff != nullptr) delete buff;

			this->maxSize = maxSize;
			size		  = 0;
			buff		  = new T[maxSize];
			memset(buff, 0, maxSize * sizeof(T));
		}

		T *alloc(size_t size) {
			assert(size + this->size <= maxSize);
			T *res = &(buff[this->size]);
			this->size += size;
			return res;
		}

		~StackAllocator() {
			if (buff != nullptr) {
				delete[] buff;
				buff = nullptr;
			}
		}

		long unsigned int getSize() { return size; }
	};

	// all primitives added
	std::vector<Intersectable *> allPrimitives;
	// root of the construction tree
	Node *root = nullptr;
	// nodes of the fast traversal tree
	std::vector<FastNode> fastNodes;
	// the primitives sorted for fast traversal
	StackAllocator<Intersectable *> fastTreePrimitives;
	bool							built = false;
	// cost for traversing a parent node. It is assumed that the intersection cost with a primitive is 1.0
	static constexpr float SAH_TRAVERSAL_COST = 0.125;
	// the number of splits SAH will try.
	static constexpr int SAH_TRY_COUNT		  = 5;
	static constexpr int MAX_DEPTH			  = 50;
	static constexpr int MIN_PRIMITIVES_COUNT = 4;
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
	static constexpr int PERFECT_SPLIT_THRESHOLD = 0;

	int		 depth			 = 0;
	int		 leafSize		 = 0;
	long int leavesCount	 = 0;
	long int nodeCount		 = 0;
	long int primitivesCount = 0;

	void addPrimitive(Intersectable *prim) override;

	void clear(Node *node);
	void clear() override;
	void clearConstructionTree();

	void build(Node *node, int depth);
	void build(Purpose purpose) override;
	bool isBuilt() const override { return built; }

	bool intersect(long unsigned int nodeIndex, const Ray &ray, float tMin, float &tMax, Intersection &intersection);
	bool intersect(const Ray &ray, float tMin, float tMax, Intersection &intersection) override;

	// computes the SAH cost for a split on a given axis.
	// ratio equals the size of the left child on the chosen axis over
	// the size of the parent node size on that axis.
	float costSAH(const Node *node, int axis, float ratio);

	// builds a tree for fast traversal
	void buildFastTree();

	void buildFastTree(Node *node, std::vector<FastNode> &allNodes);

	// copies the data from a Node to a FastNode when constructing the traversal tree
	FastNode makeFastLeaf(Node *node);
};