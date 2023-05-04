#include <ecs.h>

#include <assert.h>
#include <stdexcept>

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

/**
 * @brief Destroys an Entity that has been created with EntityManager::createEntity().
 * 
 * @param e The entity to be destroyed. 
 */
void ygl::EntityManager::destroyEntity(ygl::Entity e) {
	if(e >= entityCount) throw std::runtime_error("Entity out of range");
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
	if(e >= entityCount) throw std::runtime_error("Entity out of range");
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
	for(ygl::Entity e : this->entities) {
		std::cerr << e << " ";
	}
	std::cerr << std::endl;
}
