#include <serializable.h>
#include <istream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <texture.h>
#include <ecs.h>

auto ygl::SerializableFactory::fabricators = std::map<std::string, ygl::SerializableFactory::SerializableConstructor>();

ygl::ISerializable *ygl::SerializableFactory::makeSerializable(std::istream &in, const std::string &path) {
	std::string name;
	std::getline(in, name, '\0');

	if (fabricators.find(name) == fabricators.end())
		throw std::runtime_error("trying to read something that has not been registered: " + name);
	return fabricators[name](in, path);
}

void ygl::SerializableFactory::registerSerializable(const std::string								 &name,
													ygl::SerializableFactory::SerializableConstructor f) {
	fabricators[name] = f;
}

template <class T>
	requires ygl::IsISerializable<T>
void registerSerializable() {
	ygl::SerializableFactory::registerSerializable(
		T::name, [](std::istream &in, const std::string &path) { return new T(in, path); });
}
void ygl::SerializableFactory::init() {
	::registerSerializable<ygl::Texture2d>();
	::registerSerializable<ygl::TextureCubemap>();
};
