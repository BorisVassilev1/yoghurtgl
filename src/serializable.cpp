#include <serializable.h>
#include <istream>
#include <typeinfo>
#include "ecs.h"

auto ygl::SerializableFactory::fabricators = std::map<std::string, ygl::SerializableFactory::SerializableConstructor>();

void ygl::SerializableFactory::makeSerializable(const std::string &name, std::istream &in, Serializable *res) {
	return fabricators[name](in, res);
}

void ygl::SerializableFactory::registerSerializable(const std::string								 &name,
													ygl::SerializableFactory::SerializableConstructor f) {
	fabricators[name] = f;
}
