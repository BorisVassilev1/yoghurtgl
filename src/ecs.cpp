#include <ecs.h>

#include <assert.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <cstddef>
#include "serializable.h"

ygl::EntityManager::EntityManager() {}

/**
 * @brief Generates an ygl::Entity.
 *
 * @return The generated ygl::Entity
 */
ygl::Entity ygl::EntityManager::createEntity() {
	signatures.push_back(Signature());

	if (freePositions.size()) {
		Entity e = freePositions.front();
		freePositions.pop();
		return e;
	} else return entityCount++;
}

void ygl::EntityManager::createEntity(Entity e) {
	if (entityCount > e) throw std::runtime_error("trying to create an already existing entity");
	if (entityCount < e) throw std::runtime_error("create the smaller indexes first");
	signatures.push_back(Signature());
}

/**
 * @brief Destroys an Entity that has been created with EntityManager::createEntity().
 *
 * @param e The entity to be destroyed.
 */
void ygl::EntityManager::destroyEntity(ygl::Entity e) {
	if (e >= entityCount) throw std::runtime_error("Entity out of range");
	signatures[e].reset();
	freePositions.push(e);
	--entityCount;
}

/**
 * @brief Get the ygl::Signature of an entity.
 *
 * @param e
 * @return e's signature
 */
ygl::Signature ygl::EntityManager::getSignature(ygl::Entity e) {
	if (e >= entityCount) throw std::runtime_error("Entity out of range");
	return signatures[e];
}

/**
 * @brief Set an ygl::Entity's signature.
 *
 * @param e Entity whose Signature to be set
 * @param s Signature to be set
 */
void ygl::EntityManager::setSignature(ygl::Entity e, ygl::Signature s) { signatures[e] = s; }

void ygl::ISystem::printEntities() {
	for (ygl::Entity e : this->entities) {
		std::cerr << e << " ";
	}
	std::cerr << std::endl;
}

void ygl::Scene::serialize(std::ostream &out) {
	// Components
	this->componentManager.writeComponentTypes(out);
	// Systems
	this->systemManager.writeSystems(out);
	// Entities
	auto entitiesCount = this->entities.size();
	out.write((char *)&entitiesCount, sizeof(entitiesCount));

	for (Entity e : this->entities) {
		out.write((char *)&e, sizeof(Entity));
	}

	for (Entity e : this->entities) {
		Signature s = this->getSignature(e);
		// entity id
		out.write((char *)&e, sizeof(Entity));
		std::uint16_t componentsCount = 0;

		for (uint i = 0; i < componentManager.getComponentsCount(); ++i) {
			if (s[i]) ++componentsCount;
		}
		// Components Count
		out.write((char *)&componentsCount, sizeof(std::uint16_t));

		for (uint i = 0; i < componentManager.getComponentsCount(); ++i) {
			if (!s[i]) continue;
			ComponentType	 type(i);
			IComponentArray *array = this->componentManager.getComponentArray(type);
			// component type
			out.write((char *)&type, sizeof(ComponentType));
			array->writeComponent(e, out);
		}
	}
	out << std::flush;
}

void ygl::Scene::deserialize(std::istream &in) {
	using size_type = std::unordered_map<const char *, ComponentType>::size_type;

	auto nameToType = std::unordered_map<std::string, ygl::ComponentType>();
	auto typeToName = std::unordered_map<ygl::ComponentType, std::string>();

	// Components
	size_type componentsCount;
	in.read((char *)&componentsCount, sizeof(size_type));
	for (std::size_t i = 0; i < componentsCount; ++i) {
		std::string	  name;
		ComponentType type;
		std::getline(in, name, '\0');
		in.read((char *)&type, sizeof(ComponentType));

		nameToType[name] = type;
		typeToName[type] = name;
	}

	std::map<ygl::ComponentType, ygl::ComponentType> newComponentTypes;

	{
		// find how the scene being loaded's components map to the existing ones
		const auto &componentTypes = componentManager.getComponentTypes();
		for (const auto &pair : componentTypes) {
			const char	 *type_name = pair.first;
			ComponentType newType	= pair.second;

			ComponentType type		= nameToType[type_name];
			newComponentTypes[type] = newType;
		}
	}

	// Systems
	size_type systemsCount;
	in.read((char *)&systemsCount, sizeof(size_type));
	for (std::size_t i = 0; i < systemsCount; ++i) {
		std::string name;
		std::getline(in, name, '\0');
		ISystem *system = getSystem(name);
		if (system == nullptr)
			throw std::runtime_error("trying to load a system that has not been registered: " + name);
		system->deserialize(in);
		std::cout << name << std::endl;
	}

	// Entities
	size_type entitiesCount;
	in.read((char *)&entitiesCount, sizeof(size_type));

	// we need to create all entities in the loaded scene;
	// they will have new ids
	std::map<ygl::Entity, ygl::Entity> newIds;
	for (std::size_t i = 0; i < entitiesCount; ++i) {
		Entity e;
		in.read((char *)&e, sizeof(Entity));

		Entity newId = createEntity();
		newIds[e]	 = newId;
	}

	for (std::size_t i = 0; i < entitiesCount; ++i) {
		Entity		  e;
		std::uint16_t componentsCount;
		in.read((char *)&e, sizeof(Entity));
		in.read((char *)&componentsCount, sizeof(std::uint16_t));

		Entity result = newIds[e];
		for (std::size_t j = 0; j < componentsCount; ++j) {
			ComponentType type;
			in.read((char *)&type, sizeof(ComponentType));

			auto typeSearch = newComponentTypes.find(type);
			if (typeSearch == newComponentTypes.end())
				throw std::runtime_error("component is not registered in the scene: " + typeToName[type]);

			ComponentType	 newType = typeSearch->second;
			IComponentArray *array	 = this->componentManager.getComponentArray(newType);
			array->readComponent(result, in);
		}
	}
}
