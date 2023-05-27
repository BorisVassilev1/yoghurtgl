#pragma once

#include <cstring>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <bitset>
#include <queue>
#include <unordered_map>
#include <typeinfo>
#include <iostream>
#include <set>
#include "yoghurtgl.h"
#include <window.h>
#include <serializable.h>
#include <transformation.h>

/*
 * Idea for this implementation:
 * https://austinmorlan.com/posts/entity_component_system/
 *
 * it is not direct copy-paste
 */

/**
 * @file ecs.h
 * @brief This whole file is an implementation of the Entity-Component-System model.
 * A Scene holds information for a set of entities that all have components that are efficiently stored in typed arrays.
 * Systems have access to and manage entities that have some required set of components. This way all logic is written
 * in the systems and components are purely data holders.
 * This is an example of the so-called Data-Oriented-Design, where some principles that enforce simplicity in OOP code
 * are not used to achieve a better performing code through efficient use of the CPU cache and as little as possible
 * virtual function usage.
 */

namespace ygl {

/**
 * @brief The maximum number of components a Scene is able to hold. Increase this if it is needed.
 *
 * @return [TODO:description]
 */
const int MAX_COMPONENTS = 16;

/**
 * An Entity is an ID through which its components are accessed.
 */
typedef uint32_t Entity;

/**
 * A Signature is a bitmask where each bit corresponds to a ComponentType. The size of a Signature in bits is equal to
 * MAX_COMPONENTS.
 */
typedef std::bitset<MAX_COMPONENTS> Signature;

/**
 * A ComponentType is an ID that is unique for every component type. It is assigned to a component type when it is
 * registered in a Scene.
 */
typedef uint8_t ComponentType;

class EntityManager;

/**
 * @brief A concept for types that have a static member called 'name' of type 'const char*'.
 */
template <class T>
concept IsNamed = requires(T) {
					  { T::name } -> std::same_as<const char *&>;
				  };

/**
 * @brief A concept for types that are allowed to be components in the ECS structure.
 */
template <class T>
concept IsComponent = std::is_base_of<ygl::Serializable, T>::value && IsNamed<T>;

class ISystem;

/**
 * @brief A concept for types that are allowed to be Systems in the ECS structure.
 */
template <class T>
concept IsSystem = std::is_base_of<ygl::ISystem, T>::value && IsNamed<T>;

template <class T>
	requires IsComponent<T>
class ComponentArray;

class ComponentManager;
class Scene;

/**
 * @brief A class that manages Entity ID-s in a scene
 */
class EntityManager {
	std::vector<Signature> signatures;			///< signatures of all entities in the Scene
	std::queue<Entity>	   freePositions;		///< entity IDs that have been freed after the deletion of an Entity
	Entity				   entityCount = 0;		///< how many entities there are in the scene

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
	 * @param e - an Entity
	 */
	virtual void deleteEntity(Entity e) = 0;

	/**
	 * @brief Writes an entity's component to the stream provided
	 *
	 * @param e - Entity
	 * @param out - stream to werite to
	 */
	virtual void writeComponent(Entity e, std::ostream &out) = 0;

	/**
	 * @brief adds the array's component to the provided entity, constructing it from data in the stream.
	 *
	 * @param e - entity to assign the component to
	 * @param in - stream to read the data from
	 * @param scene - ALWAYS MUST BE THE SCENE THAT OWNS THIS IComponentArray
	 * @return Serializable& - a reference to the constructed component
	 */
	virtual Serializable &readComponent(Entity e, std::istream &in, Scene *scene) = 0;

	virtual ~IComponentArray() {}
};

/**
 * @brief An array of component data for entities
 *
 * @tparam T - Type of the component data stored
 */
template <class T>
	requires IsComponent<T>
class ComponentArray : public IComponentArray {
	std::vector<T> components;	   ///< the components of type T in the Scene
	const char	  *type_name;	   ///< ponts to T::name

	std::unordered_map<Entity, size_t> entityToIndexMap;	 ///< maps Entity to its component's index in the array
	std::unordered_map<size_t, Entity> indexToEntityMap;	 ///< maps a component index to the Entity it belongs to

   public:
	/**
	 * @brief Construct a new ComponentArray.
	 *
	 */
	ComponentArray() { type_name = T::name; }

	/**
	 * @brief How many entities is the ComponentArray storing data.
	 *
	 * @return size_t
	 */
	size_t count() { return components.size(); }

	/**
	 * @brief Adds component to a given entity.
	 *
	 * @param e - the entity to add a component to
	 * @param component - the component
	 * @return T& - a reference to the inserted data
	 */
	T &addComponent(Entity e, const T &component) {
		if (entityToIndexMap.find(e) != entityToIndexMap.end()) {
			dbLog(ygl::LOG_ERROR, "This entity already has that component: ", T::name);
			return this->getComponent(e);
		}

		int index				= components.size();
		entityToIndexMap[e]		= index;
		indexToEntityMap[index] = e;
		components.push_back(component);
		return components.back();
	}

	/**
	 * @brief Checks if a given entity has this array's component assigned to it
	 *
	 * @param e - the entity to check for the component
	 * @return true - if the entity has a component of the array's type , false otherwise
	 */
	bool hasComponent(Entity e) { return entityToIndexMap.find(e) != entityToIndexMap.end(); }

	/**
	 * @brief Removes a component from an Entity.
	 *
	 * @param e - The entity whose component is to be removed
	 */
	void removeComponent(Entity e) {
		if (entityToIndexMap.find(e) == entityToIndexMap.end()) {
			dbLog(ygl::LOG_ERROR, "cannot remove a non-existing component: ", T::name);
			return;
		}

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
	 * @param e - an Entity
	 */
	void deleteEntity(Entity e) override {
		if (entityToIndexMap.find(e) == entityToIndexMap.end()) {
			dbLog(ygl::LOG_ERROR, "Cannot delete a non-existing entity: ", e);
			return;
		}
		removeComponent(e);
	}

	/**
	 * @brief Get the Component object for an entity.
	 * @throws std::runtime_error if \a e does not have that component
	 *
	 * @param e - The entity whose component is accessed
	 * @return T& - A reference to the component data
	 */
	T &getComponent(ygl::Entity e) {
		if (entityToIndexMap.find(e) == entityToIndexMap.end()) {
			THROW_RUNTIME_ERR("component " + std::string(T::name) +
							  " not found on that entity. Bad things will happen");
		}
		return components[entityToIndexMap[e]];
	}

	void		  writeComponent(Entity e, std::ostream &out) override { getComponent(e).serialize(out); }
	Serializable &readComponent(Entity e, std::istream &in, Scene *scene) override;
};

/**
 * @brief An object that manages the components in a Scene.
 */
class ComponentManager {
	std::unordered_map<const char *, ComponentType> componentTypes;		///< map from a type name to its ComponentType
	std::unordered_map<ComponentType, IComponentArray *>
				  componentArrays;	   ///< map from ComponentType to IComponentArray that holds components of that type
	ComponentType componentTypeCounter =
		0;	   ///< keeps track of component type count so IDs can be assigned correctly

   public:
	ComponentManager() {}

	~ComponentManager() {
		// deletes all component arrays
		for (auto const &pair : componentArrays) {
			delete pair.second;
		}
	}

	/**
	 * @brief Registers a component type so it can be assigned to entities.
	 *
	 * @tparam T - a component type
	 */
	template <typename T>
		requires IsComponent<T>
	void registerComponent() {
		const char *typeName = T::name;
		if (componentTypes.find(typeName) != componentTypes.end()) {
			dbLog(ygl::LOG_ERROR, "Component already registered: ", typeName);
			return;
		}
		ComponentType type		 = componentTypeCounter++;
		componentTypes[typeName] = type;
		componentArrays[type]	 = new ygl::ComponentArray<T>();
	}

	/**
	 * @brief Checks if a component has been registered.
	 *
	 * @tparam T - component type
	 * @return true - if the registerComponent<T>() has been called
	 * @return false - otherwise
	 */
	template <typename T>
	bool isComponentRegistered() {
		const char *typeName = T::name;
		bool		res		 = componentTypes.find(typeName) != componentTypes.end();
		return res;
	}

	/**
	 * @brief Get the ComponentType that has been assigned to a component type \a T when it was registered.
	 *
	 * @tparam T - component type
	 * @return ComponentType - the assigned ComponentType
	 * @return -1 - if T has not been registered
	 */
	template <typename T>
		requires IsComponent<T>
	ComponentType getComponentType() {
		const char *typeName = T::name;
		if (!isComponentRegistered<T>()) {
			dbLog(ygl::LOG_ERROR, "component has not been registered: ", typeName);
			return -1;
		}
		return componentTypes[typeName];
	}

	/**
	 * @brief Get the ComponentArray that contains the components of type \a T.
	 *
	 * @tparam T - component type
	 * @return ComponentArray<T>* - a pointer to the ComponentArray of the required type.
	 * @return nullptr if T has not been registered
	 */
	template <typename T>
		requires IsComponent<T>
	ComponentArray<T> *getComponentArray() {
		const char	 *typeName = T::name;
		ComponentType type	   = componentTypes[typeName];
		if (componentArrays.find(type) == componentArrays.end()) {
			dbLog(ygl::LOG_ERROR, "component has not been registered: ", typeName);
			return nullptr;
		}
		return static_cast<ComponentArray<T> *>(componentArrays[type]);
	}

	/**
	 * @brief Get the ComponentArray that contains the components of type \a T that has a ComponentType \a t .
	 * Same as getComponentArray<T>() , but does not require the type to be known compile-time.
	 *
	 * @param t - ComponentType of the component type
	 * @return IComponentArray* - pointer to ComponentArray<T>
	 */
	IComponentArray *getComponentArray(ComponentType t) {
		if (componentArrays.find(t) == componentArrays.end()) {
			dbLog(ygl::LOG_ERROR, "no component has that id: ", t);
			return 0;
		}
		return componentArrays[t];
	}

	/**
	 * @brief Get the component of type \a T from the Entity \a e.
	 *
	 * @tparam T - component type
	 * @param e - an Entity in the Scene
	 * @return T& - a reference to the component
	 */
	template <typename T>
		requires IsComponent<T>
	T &getComponent(Entity e) {
		return this->getComponentArray<T>()->getComponent(e);
	}

	/**
	 * @brief Checks if an Entity \a e has a component of type \a T.
	 * @see ComponentArray<T>::hasComponent(e) .
	 *
	 * @tparam T - component type
	 * @param e - an Entity
	 * @return true - if \a e has a component of type \a T.
	 * @return false - otherwise
	 */
	template <typename T>
	bool hasComponent(Entity e) {
		return this->getComponentArray<T>()->hasComponent(e);
	}

	/**
	 * @brief Add a component of type \a T to an Entity \a e.
	 * @see getComponentArray<T>() and ComponentArray<T>::addComponent(Entity e, )
	 *
	 * @tparam T - component type
	 * @param e - an Entity
	 * @param component - a component of type \a T to be added.
	 * @return T& - a reference to the copied contents of \a component
	 */
	template <typename T>
	T &addComponent(Entity e, const T &component) {
		return getComponentArray<T>()->addComponent(e, component);
	}

	/**
	 * @brief Removes a component of type \a T from the Entity \a e.
	 *
	 * @tparam T
	 * @param e
	 */
	template <typename T>
	void removeComponent(Entity e) {
		getComponentArray<T>()->removeComponent(e);
	}

	/**
	 * @brief Deletes an entity's data from all Component Arrays
	 *
	 * @param e - an Entity
	 */
	void deleteEntity(Entity e) {
		for (auto const &pair : componentArrays) {
			pair.second->deleteEntity(e);
		}
	}

	/**
	 * @brief Writes all component types to \a out in binary format. Used for serialization of the Scene.
	 *
	 * @param out - stream to write to
	 */
	void writeComponentTypes(std::ostream &out) {
		auto size = this->componentTypes.size();
		out.write((char *)&size, sizeof(size));
		for (auto &t : this->componentTypes) {
			out.write(t.first, strlen(t.first) + 1);
			out.write((char *)&t.second, sizeof(ComponentType));
		}
	}

	/**
	 * @brief Get how many components have been registered in this manager.
	 *
	 * @return uint - the count
	 */
	uint getComponentsCount() { return componentTypeCounter; }

	/**
	 * @brief Get all component types and their names. DO NOT USE THIS IF YOU DO NOT KNOW WHAT YOU ARE DOING!
	 *
	 * @return const std::unordered_map<const char *, ComponentType>& - a const reference to the map from component name
	 * to ComponentType
	 */
	const auto &getComponentTypes() { return componentTypes; }
};

/**
 * @brief An Interface for a system in a scene
 *
 */
class ISystem : public AppendableSerializable {
   public:
	std::set<Entity> entities;			  ///< entities that the System has access to
	Scene			*scene = nullptr;	  ///< points to the Scene the System is assigned to

	/**
	 * @brief Constructs a System, assigned to \a scene.
	 *
	 * @param scene - the Scene that the System is in.
	 */
	ISystem(Scene *scene) : scene(scene) {}
	virtual ~ISystem(){};

	/**
	 * @brief Here the System does all its Entity manipulation
	 *
	 */
	virtual void doWork() = 0;
	/**
	 * @brief initializes the System after the scene has registered it and it can safely set its own signature, and
	 * access the correct entities.
	 */
	virtual void init() = 0;
	/**
	 * @brief prints all the entities that the system has access to.
	 */
	void printEntities();
};

/**
 * @brief An object that manages objects of type ISystem.
 *
 */
class SystemManager {
	std::unordered_map<const char *, ISystem *> systems;		///< map system type name to a system
	std::unordered_map<const char *, Signature> signatures;		///< map system type name to its Signature

   public:
	/**
	 * @brief registers a System of type \a T. Constructs it with a Scene and the other given arguments.
	 *
	 * @tparam T - a System type
	 * @tparam Args
	 * @param scene - scene to give to T's constructor
	 * @param args - other arguments for T's constructor
	 * @return T* - a pointer to the System created
	 */
	template <class T, typename... Args>
		requires IsSystem<T>
	T *registerSystem(Scene *scene, Args &&...args) {
		const char *type = T::name;

		if (systems.find(type) != systems.end()) {
			dbLog(ygl::LOG_ERROR, "system already registered: ", type);
			return nullptr;
		}

		T *sys = new T(scene, args...);
		systems.insert({type, sys});
		return sys;
	}

	/**
	 * @brief Get the System object
	 *
	 * @tparam T - type of the System
	 * @return T* - a pointer to the System object
	 */
	template <class T>
		requires IsSystem<T>
	T *getSystem() {
		const char *type = T::name;

		if (systems.find(type) == systems.end()) {
			dbLog(ygl::LOG_ERROR, "system type has not been registered: ", type);
			return nullptr;
		}

		return (T *)(systems[type]);
	}

	/**
	 * @brief Get the System object. Same as getSystem<T>() but without type safety. IT IS SLOW!
	 *
	 * @param name - name of the System
	 * @return ISystem* - a pointer to the System
	 */
	ISystem *getSystem(const std::string &name) {
		for (const auto &pair : systems) {
			if (pair.first == name) return pair.second;
		}
		return nullptr;
	}

	/**
	 * @brief Checks if a System of type \a T has been registered.
	 *
	 * @tparam T - a System type
	 * @return true - if registerSystem<T>(...) has been called
	 * @return false - otherwise
	 */
	template <class T>
		requires IsSystem<T> bool
	hasSystem() {
		return systems.find(T::name) != systems.end();
	}

	/**
	 * @brief Set the Signature of the System of type \a T.
	 *
	 * @tparam T - a System type.
	 * @param signature - a Signature
	 */
	template <class T>
		requires IsSystem<T>
	void setSignature(Signature signature) {
		const char *type = T::name;
		if (systems.find(type) == systems.end()) {
			dbLog(ygl::LOG_ERROR, "System not registered: " + std::string(type));
			return;
		}
		signatures[type] = signature;
	}

	/**
	 * @brief Updates an Entity's Signature when it's components change
	 *
	 * @param e - an Entity
	 * @param signature - a Signature
	 */
	void updateEntitySignature(Entity e, Signature signature) {
		for (auto pair : systems) {
			if ((signature & signatures[pair.first]) == signatures[pair.first]) {
				pair.second->entities.insert(e);
			} else {
				pair.second->entities.erase(e);
			}
		}
	}

	/**
	 * @brief Destroys an Entity.
	 *
	 * @param e - an Entity
	 */
	void destroyEntity(Entity e) {
		for (auto pair : systems) {
			pair.second->entities.erase(e);
		}
	}

	/**
	 * @brief Destructor
	 *
	 */
	~SystemManager() {
		for (auto pair : systems) {
			delete pair.second;
		}
	}

	/**
	 * @brief Writes all Systems' binary data to \a out.
	 *
	 * @param out - stream to write to
	 */
	void writeSystems(std::ostream &out) {
		auto size = this->systems.size();
		out.write((char *)&size, sizeof(size));
		for (auto &t : this->systems) {
			out.write(t.first, strlen(t.first) + 1);
			t.second->write(out);
		}
	}


	/**
	 * @brief Makes all systems do their work
	 */
	void doWork() {
		auto size = this->systems.size();
		for (auto &t : this->systems) {
			t.second->doWork();
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
class Scene : public ygl::AppendableSerializable {
	ComponentManager componentManager;
	EntityManager	 entityManager;
	SystemManager	 systemManager;

   public:
	std::set<Entity>   entities;
	static const char *name;

	/**
	 * @brief Construct an empty Scene
	 *
	 */
	Scene() {}

	/**
	 * @brief Create an Entity.
	 *
	 * @return Entity - The created Entity.
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
	 * @see ComponentManager::registerComponent<T>() .
	 *
	 * @tparam T - Component data type
	 */
	template <typename T>
		requires IsComponent<T>
	void registerComponent() {
		componentManager.registerComponent<T>();
	}

	/**
	 * @brief Registers a component type only if it has not been registered before.
	 * @see isComponentRegistered<T>(), registerComponent<T>() .
	 *
	 * @tparam T - a Component type
	 */
	template <typename T>
	void registerComponentIfCan() {
		if (!isComponentRegistered<T>()) registerComponent<T>();
	}

	/**
	 * @brief Checks if the given component is registered for the scene.
	 * @see ComponentManager::isComponentRegistered<T>() .
	 *
	 * @tparam T - component data type
	 */
	template <typename T>
	bool isComponentRegistered() {
		return componentManager.isComponentRegistered<T>();
	}

	/**
	 * @brief Adds a Component to an Entity. The component type must have been registered in the scene.
	 * One component cannot be assigned twice to the same Entity.
	 *
	 * @tparam T - Component to be added.
	 * @param e - an Entity
	 * @param component - an instance of the component
	 * @return T& - a reference to the component added
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
	 * @brief Checks of an Entity has a component of type \a T.
	 * @see ComponentManager::hasComponent<T>(Entity e)
	 *
	 * @tparam T - a component type
	 * @param e - an Entity
	 * @return true - if the component has been added to \e
	 * @return false - otherwise
	 */
	template <typename T>
	bool hasComponent(Entity e) {
		return componentManager.hasComponent<T>(e);
	}

	/**
	 * @brief Removes a component from an Entity.
	 *
	 * @tparam T - Component type to be removed
	 * @param e - Entity
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
	 * @param e - Entity
	 * @return Signature - e's Signature
	 */
	Signature getSignature(Entity e) { return entityManager.getSignature(e); }

	/**
	 * @brief Set a System's Signature. The system will have access to all objects that have the required
	 * components.
	 *
	 * @tparam System - a system to set signature
	 * @tparam ...T - component types for the system to require
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
	 * @tparam T - a System type
	 * @return T* - a pointer to the system if it exists. If not, nullptr
	 */
	template <class T>
	T *getSystem() {
		return systemManager.getSystem<T>();
	}

	/**
	 * @brief Get a System object if a System of that type exists.
	 *
	 * @param name - name of the system type
	 * @return ISystem* - the system or nullptr if it os not found
	 */
	ISystem *getSystem(const std::string &name) { return systemManager.getSystem(name); }

	template <class T>
		requires IsSystem<T> bool
	hasSystem() {
		return systemManager.hasSystem<T>();
	}

	/**
	 * @brief Get an Entity's Component by type.
	 *
	 * @tparam T - the wanted component's type
	 * @param e - Entity
	 * @return a - reference to the component
	 */
	template <typename T>
		requires IsComponent<T>
	T &getComponent(Entity e) {
		return componentManager.getComponent<T>(e);
	}

	/**
	 * @brief Get the ComponentType assigned to a Component Type, registered in the system.
	 *
	 * @tparam T - the Component type.
	 * @return ComponentType - T's ComponentType
	 */
	template <typename T>
	ComponentType getComponentType() {
		return componentManager.getComponentType<T>();
	}

	/**
	 * @brief Registers a System in the Scene. Creates an instance of the System and returns a reference to it.
	 *
	 * @tparam T - System Type
	 * @return T* - A reference to the created instance
	 */
	template <class T, class... Args>
	T *registerSystem(Args &&...args) {
		T *sys = systemManager.registerSystem<T>(this, args...);
		sys->init();
		return sys;
	}

	/**
	 * @brief Registers a System in the Scene if it has not been registered. Creates an instance of the System and
	 * returns a reference to it.
	 *
	 * @tparam T - a System type
	 * @tparam Args
	 * @param args - args for T's construction
	 * @return T* - a pointer to the created instance of T
	 */
	template <class T, class... Args>
	T *registerSystemIfCan(Args &&...args) {
		if (!systemManager.hasSystem<T>()) return registerSystem<T>(args...);
		return nullptr;
	}

	void write(std::ostream &out) override;
	void read(std::istream &in) noexcept(false) override;

	/**
	 * @brief Makes all Systems do their work
	 */
	void doWork();

	/**
	 * @brief How many entities does the Scene have
	 *
	 * @return std::size_t
	 */
	std::size_t entitiesCount() { return entities.size(); }
};

// this definition is outside the class because it uses a method from Scene
template <class T>
	requires IsComponent<T>
Serializable &ygl::ComponentArray<T>::readComponent(Entity e, std::istream &in, Scene *scene) {
	Serializable &res = scene->addComponent(e, T());
	((T *)&res)->deserialize(in);
	return res;
}
}	  // namespace ygl
