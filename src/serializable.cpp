#include <serializable.h>
#include <istream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <asset_manager.h>
#include <mesh.h>
#include <shader.h>
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

void ygl::ResourceFactory::registerResource(const std::string &name, ygl::ResourceFactory::ResourceConstructor f) {
	fabricators[name] = f;
}

void ygl::ResourceFactory::init() {
	registerResource<ygl::Texture2d>();
	registerResource<ygl::TextureCubemap>();
	registerResource<ygl::BoxMesh>();
	registerResource<ygl::SphereMesh>();
	registerResource<ygl::MeshFromFile>();
	registerResource<ygl::PlaneMesh>();
	registerResource<ygl::QuadMesh>();
	registerResource<ygl::VFShader>();
	registerResource<ygl::ComputeShader>();
};
