#include <serializable.h>
#include <istream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <yoghurtgl.h>
#include <asset_manager.h>
#include <mesh.h>
#include <shader.h>
#include <texture.h>
#include <ecs.h>

std::map<std::string, ygl::ResourceFactory::ResourceConstructor> ygl::ResourceFactory::fabricators =
	std::map<std::string, ygl::ResourceFactory::ResourceConstructor>();

ygl::ISerializable *ygl::ResourceFactory::fabricate(std::istream &in) {
	std::string name;
	std::getline(in, name, '\0');

	if (fabricators.find(name) == fabricators.end()) {
		THROW_RUNTIME_ERR("trying to read something that has not been registered: " + name);
	}
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
#ifndef YGL_NO_ASSIMP
	registerResource<ygl::MeshFromFile>();
#endif
	registerResource<ygl::PlaneMesh>();
	registerResource<ygl::QuadMesh>();
	registerResource<ygl::VFShader>();
#ifndef YGL_NO_COMPUTE_SHADERS
	registerResource<ygl::ComputeShader>();
#endif
};
