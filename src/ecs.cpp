#include <ecs.h>

#include <assert.h>

ygl::EntityManager::EntityManager() {}

ygl::Entity ygl::EntityManager::createEntity() {
	signatures.push_back(Signature());

	if (freePositions.size()) {
		Entity e = freePositions.front();
		freePositions.pop();
		return e;
	} else return entityCount++;
}

void ygl::EntityManager::destroyEntity(ygl::Entity e) {
	assert(e < entityCount && "entity out of range");
	signatures[e].reset();
	freePositions.push(e);
	--entityCount;
}

ygl::Signature ygl::EntityManager::getSignature(ygl::Entity e) {
	assert(e < entityCount && "entity out of range");
	return signatures[e];
}

void ygl::EntityManager::setSignature(ygl::Entity e, ygl::Signature s) { signatures[e] = s; }

ygl::IComponentArray::IComponentArray() {}
