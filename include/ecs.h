#pragma once

#include <vector>
#include <bitset>
#include <queue>
#include <unordered_map>
#include <typeinfo>
#include <assert.h>
#include <iostream>

namespace ygl {

const int MAX_COMPONENTS = 8;

typedef uint32_t					Entity;
typedef std::bitset<MAX_COMPONENTS> Signature;
typedef uint8_t						ComponentType;

class EntityManager {
	std::vector<Signature> signatures;
	std::queue<Entity>	   freePositions;
	Entity				   entityCount = 0;

   public:
	EntityManager();

	Entity	  createEntity();
	void	  destroyEntity(Entity);
	Signature getSignature(Entity);
	void	  setSignature(Entity, Signature);
};

class IComponentArray {
   public:
	IComponentArray();
	virtual void deleteEntity(Entity e) = 0;
};

template <class T>
class ComponentArray : public IComponentArray {
	std::vector<T> components;

	std::unordered_map<Entity, size_t> entityToIndexMap;
	std::unordered_map<size_t, Entity> indexToEntityMap;

   public:
	ComponentArray() {}

	size_t count() { return components.size(); }

	void addComponent(Entity e, T component) {
		assert(entityToIndexMap.find(e) == entityToIndexMap.end() && "This entity already has that component");

		int index				= components.size();
		entityToIndexMap[e]		= index;
		indexToEntityMap[index] = e;
		components.push_back(component);
	}

	void removeComponent(Entity e) {
		assert(entityToIndexMap.find(e) != entityToIndexMap.end() && "cannot remove a non-existing component");
		size_t index	  = entityToIndexMap[e];
		components[index] = components.back();

		Entity last_entity			  = indexToEntityMap[count() - 1];
		entityToIndexMap[last_entity] = index;
		indexToEntityMap[index]		  = last_entity;

		entityToIndexMap.erase(e);
		indexToEntityMap.erase(count() - 1);

		components.pop_back();
	}

	void deleteEntity(Entity e) {
		if (entityToIndexMap.find(e) != entityToIndexMap.end()) { removeComponent(e); }
	}

	T &getComponent(ygl::Entity e) {
		assert(entityToIndexMap.find(e) != entityToIndexMap.end() && "component not found on that entity");
		return components[entityToIndexMap[e]];
	}
};

class ComponentManager {
	std::unordered_map<const char *, ComponentType>		componentTypes;
	std::unordered_map<const char *, IComponentArray *> componentArrays;
	ComponentType										componentTypeCounter = 0;

   public:
	ComponentManager() {}

	~ComponentManager() {
		for (auto const &pair : componentArrays) {
			delete pair.second;
		}
	}

	template <typename T>
	void registerComponent() {
		ComponentType type	   = componentTypeCounter++;
		const char   *typeName = typeid(T).name();
		assert(componentTypes.find(typeName) == componentTypes.end() && "component already registered");
		componentTypes[typeName]  = type;
		componentArrays[typeName] = new ygl::ComponentArray<T>();
	}

	template <typename T>
	ComponentType getComponentType() {
		const char *typeName = typeid(T).name();
		assert(componentTypes.find(typeName) != componentTypes.end() && "component has not been registered");
		return componentTypes[typeName];
	}

	template <typename T>
	ComponentArray<T> *getComponentArray() {
		const char *typeName = typeid(T).name();
		assert(componentArrays.find(typeName) != componentArrays.end() && "component has not been registered");
		return static_cast<ComponentArray<T> *>(componentArrays[typeName]);
	}

	template <typename T>
	T &getComponent(Entity e) {
		return getComponentArray<T>()->getComponent(e);
	}

	template <typename T>
	void addComponent(Entity e, T component) {
		getComponentArray<T>()->addComponent(e, component);
	}

	template <typename T>
	void removeComponent(Entity e) {
		getComponentArray<T>()->removeComponent(e);
	}

	void deleteEntity(Entity e) {
		for (auto const &pair : componentArrays) {
			pair.second->deleteEntity(e);
		}
	}
};

class Scene {
	ComponentManager componentManager;
	EntityManager	 entityManager;

   public:
	Scene() {}

	Entity createEntity() { return entityManager.createEntity(); }

	void destroyEntity(Entity e) {
		componentManager.deleteEntity(e);
		entityManager.destroyEntity(e);
	}

	template <typename T>
	void registerComponent() {
		componentManager.registerComponent<T>();
	}

	template <typename T>
	void addComponent(Entity e, T component) {
		componentManager.addComponent<T>(e, component);

		auto signature = entityManager.getSignature(e);
		signature.set(componentManager.getComponentType<T>(), true);
		entityManager.setSignature(e, signature);
	}

	template <typename T>
	void removeComponent(Entity e) {
		componentManager.removeComponent<T>(e);

		auto signature = entityManager.getSignature(e);
		signature.set(componentManager.getComponentType<T>(), false);
		entityManager.setSignature(e, signature);
	}

	template <typename T>
	T &getComponent(Entity e) {
		return componentManager.getComponent<T>(e);
	}

	template <typename T>
	ComponentType getComponentType() {
		return componentManager.getComponentType<T>();
	}
};

class MeshComoponent {

};

class ShaderComponent {

};

}	  // namespace ygl