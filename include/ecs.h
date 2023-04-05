#pragma once

#include <vector>
#include <bitset>
#include <queue>
#include <unordered_map>
#include <typeinfo>
#include <assert.h>
#include <iostream>
#include <set>
#include <window.h>
#include <importer.h>

namespace ygl {

const int MAX_COMPONENTS = 16;

typedef uint32_t					Entity;
typedef std::bitset<MAX_COMPONENTS> Signature;
typedef uint8_t						ComponentType;

class EntityManager;
template <class T>
class ComponentArray;
class ComponentManager;
class ISystem;
class Scene;

/**
 * @brief A class that manages Entity ID-s in a scene
 */
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

/**
 * @brief Interface for ygl::ComponentArray
 *
 */
class IComponentArray {
   public:
	IComponentArray() {}
	/**
	 * @brief Deletes an entity's data from the array
	 *
	 * @param e
	 */
	virtual void deleteEntity(Entity e) = 0;
	virtual ~IComponentArray() {}
};

/**
 * @brief An array of component data for entities
 *
 * @tparam T Type of the component data stored
 */
template <class T>
class ComponentArray : public IComponentArray {
	std::vector<T> components;

	std::unordered_map<Entity, size_t> entityToIndexMap;
	std::unordered_map<size_t, Entity> indexToEntityMap;

   public:
	/**
	 * @brief Construct a new Component Array object.
	 *
	 */
	ComponentArray() {}

	/**
	 * @brief How many entities is the ComponentArray storing data.
	 *
	 * @return size_t
	 */
	size_t count() { return components.size(); }

	/**
	 * @brief Adds component to a given entity.
	 *
	 * @param e the entity to add a component to
	 * @param component the component
	 * @return T& a reference to the inserted data
	 */
	T &addComponent(Entity e, const T &component) {
		assert(entityToIndexMap.find(e) == entityToIndexMap.end() && "This entity already has that component");

		int index				= components.size();
		entityToIndexMap[e]		= index;
		indexToEntityMap[index] = e;
		components.push_back(component);
		return components.back();
	}

	/**
	 * @brief Removes a component from an entity.
	 *
	 * @param e The entity whose component is to be removed
	 */
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

	/**
	 * @brief Delete an entity's data from the array
	 *
	 * @param e an entity
	 */
	void deleteEntity(Entity e) override {
		if (entityToIndexMap.find(e) != entityToIndexMap.end()) { removeComponent(e); }
	}

	/**
	 * @brief Get the Component object for an entity.
	 *
	 * @param e The entity whose component is accessed
	 * @return A reference to the component data
	 */
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
		const char	 *typeName = typeid(T).name();
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
	T &addComponent(Entity e, T component) {
		return getComponentArray<T>()->addComponent(e, component);
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

class ISystem {
   public:
	std::set<Entity> entities;
	Scene			*scene = nullptr;

	ISystem(Scene *scene) : scene(scene) {}
	virtual ~ISystem(){};

	virtual void doWork() = 0;
	virtual void init()	  = 0;
};

class SystemManager {
	std::unordered_map<const char *, ISystem *> systems;
	std::unordered_map<const char *, Signature> signatures;

   public:
	template <class T> requires std::is_base_of<ISystem, T>::value
	T *registerSystem(Scene *scene) {
		const char *type = typeid(T).name();

		assert(systems.find(type) == systems.end() && "system type already registered");

		T *sys = new T(scene);
		systems.insert({type, sys});
		return sys;
	}

	template <class T>
	T *getSystem() {
		const char *type = typeid(T).name();

		assert(systems.find(type) != systems.end() && "system type has not been registered");

		return (T *)(systems[type]);
	}

	template <class T>
	void setSignature(Signature signature) {
		const char *type = typeid(T).name();
		assert(systems.find(type) != systems.end() && "System not registered");

		signatures[type] = signature;
	}

	void updateEntitySignature(Entity e, Signature signature) {
		for (auto pair : systems) {
			if (signature == signatures[pair.first]) {
				pair.second->entities.insert(e);
			} else {
				pair.second->entities.erase(e);
			}
		}
	}

	void destroyEntity(Entity e) {
		for (auto pair : systems) {
			pair.second->entities.erase(e);
		}
	}

	~SystemManager() {
		for (auto pair : systems) {
			delete pair.second;
		}
	}
};

/**
 * @brief A Scene object.
 * A scene contains a set of entities that all have components and Systems that can manage entities and their data.
 * 
 * Components can be user-defined. Systems must extend ygl::ISystem. 
 * This type of Scene is modeled after Unity3D's ECS model 
 * 
 */
class Scene {
	ComponentManager componentManager;
	EntityManager	 entityManager;
	SystemManager	 systemManager;

	Scene();

   public:
	ygl::Window		*window;
	std::set<Entity> entities;
	ygl::AssetManager	 assetManager;

	/**
	 * @brief Construct an empty Scene
	 *
	 * @param window a window for the scene to be attached to
	 */
	Scene(ygl::Window *window) : window(window) {}

	/**
	 * @brief Create an Entity.
	 *
	 * @return The created Entity.
	 */
	Entity createEntity() {
		Entity res = entityManager.createEntity();
		entities.insert(res);
		return res;
	}

	/**
	 * @brief Destroys an Entity and all its components.
	 *
	 * @param e an Entity to be destroyed
	 */
	void destroyEntity(Entity e) {
		componentManager.deleteEntity(e);
		entityManager.destroyEntity(e);
		systemManager.destroyEntity(e);
		entities.erase(e);
	}

	/**
	 * @brief Register a component type for the scene.
	 *
	 * @tparam T Component data type
	 */
	template <typename T>
	void registerComponent() {
		componentManager.registerComponent<T>();
	}

	/**
	 * @brief Adds a Component to an Entity. The component type must have been registered in the scene.
	 * One component cannot be assigned twise to the same Entity.
	 *
	 * @tparam T Component to be added.
	 * @param e an Entity
	 * @param component an instance of the component
	 * @return a reference to the component added
	 */
	template <typename T>
	T &addComponent(Entity e, const T &component) {
		T &res = componentManager.addComponent<T>(e, component);

		auto signature = entityManager.getSignature(e);
		signature.set(componentManager.getComponentType<T>(), true);
		entityManager.setSignature(e, signature);
		systemManager.updateEntitySignature(e, signature);

		return res;
	}

	/**
	 * @brief Removes a component from an Entity. 
	 * 
	 * @tparam T Component type to be removed
	 * @param e Entity
	 */
	template <typename T>
	void removeComponent(Entity e) {
		componentManager.removeComponent<T>(e);

		auto signature = entityManager.getSignature(e);
		signature.set(componentManager.getComponentType<T>(), false);
		entityManager.setSignature(e, signature);
		systemManager.updateEntitySignature(e, signature);
	}

	/**
	 * @brief Get the Signature of an Entity.
	 * 
	 * @param e Entity
	 * @return e's Signature 
	 */
	Signature getSignature(Entity e) { return entityManager.getSignature(e); }

	/**
	 * @brief Set a System's Signature. The system will have access to all objects that have the required components.
	 * 
	 * @tparam System a system to set signature
	 * @tparam ...T component types for the system to require
	 */
	template <class System, class... T>
	void setSystemSignature() {
		Signature s;
		(s.set(componentManager.getComponentType<T>()), ...);
		systemManager.setSignature<System>(s);
	}

	/**
	 * @brief Get a System object if a System of that type exists.
	 * 
	 * @tparam T 
	 * @return a pointer to the system if it exists. If not, nullptr
	 */
	template <class T>
	T *getSystem() {
		return systemManager.getSystem<T>();
	}

	/**
	 * @brief Get an Entity's Component by type.
	 * 
	 * @tparam T the wanted component's type
	 * @param e Entity
	 * @return a reference to the component
	 */
	template <typename T>
	T &getComponent(Entity e) {
		return componentManager.getComponent<T>(e);
	}

	/**
	 * @brief Get the ComponentType assigned to a Component Type, registered in the system.
	 * 
	 * @tparam T the Component type.
	 * @return ComponentType 
	 */
	template <typename T>
	ComponentType getComponentType() {
		return componentManager.getComponentType<T>();
	}

	/**
	 * @brief Registers a System in the Scene. Creates an instance of the System and returns a reference to it.
	 * 
	 * @tparam T System Type
	 * @return T* A reference to the created instance
	 */
	template <class T>
	T *registerSystem() {
		T *sys = systemManager.registerSystem<T>(this);
		sys->init();
		return sys;
	}
};
};	   // namespace ygl