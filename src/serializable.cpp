#include <serializable.h>
#include <istream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include "importer.h"
#include "mesh.h"
#include "shader.h"
#include <texture.h>
#include <ecs.h>

auto ygl::ResourceFactory::fabricators = std::map<std::string, ygl::ResourceFactory::ResourceConstructor>();

ygl::ISerializable *ygl::ResourceFactory::fabricate(std::istream &in) {
	std::string name;
	std::getline(in, name, '\0');

	if (fabricators.find(name) == fabricators.end())
		throw std::runtime_error("trying to read something that has not been registered: " + name);
	return fabricators[name](in);
}

void ygl::ResourceFactory::registerResource(const std::string								 &name,
													ygl::ResourceFactory::ResourceConstructor f) {
	fabricators[name] = f;
}

template <class T>
	requires ygl::IsResource<T>
void registerSerializable() {
	ygl::ResourceFactory::registerResource(
		T::name, [](std::istream &in) { return new T(in); });
}
void ygl::ResourceFactory::init() {
	::registerSerializable<ygl::Texture2d>();
	::registerSerializable<ygl::TextureCubemap>();
	::registerSerializable<ygl::BoxMesh>();
	::registerSerializable<ygl::SphereMesh>();
	::registerSerializable<ygl::MeshFromFile>();
	::registerSerializable<ygl::VFShader>();
	::registerSerializable<ygl::ComputeShader>();
};
