#include <serializable.h>
#include <istream>
#include <typeinfo>


auto ygl::SerializableFactory::fabricators = std::map<std::string, ygl::SerializableFactory::SerializableConstructor>();

ygl::Serializable *ygl::SerializableFactory::makeSerializable(const std::string &name, std::istream &in) {
	return fabricators[name](in);
}

void ygl::SerializableFactory::registerSerializable(const std::string								 &name,
													ygl::SerializableFactory::SerializableConstructor f) {
	fabricators[name] = f;
}
