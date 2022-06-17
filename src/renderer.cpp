#include <renderer.h>

#include <transformation.h>

ygl::Material::Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior,
						glm::vec3 transparency_color, float refraction_chance, float refraction_roughness,
						float specular_roughness)
	: albedo(albedo),
	  specular_chance(specular_chance),
	  emission(emission),
	  ior(ior),
	  transparency_color(transparency_color),
	  refraction_chance(refraction_chance),
	  refraction_roughness(refraction_roughness),
	  specular_roughness(specular_roughness) {}

ygl::Shader *ygl::Renderer::getShader(ECRenderer &comp) {
	assert(comp.shaderIndex >= 0 && "invalid index");
	return shaders[comp.shaderIndex];
}

ygl::Material &ygl::Renderer::getMaterial(ECRenderer &comp) {
	assert(comp.materialIndex >= 0 && "invalid index");
	return materials[comp.materialIndex];
}

ygl::Mesh *ygl::Renderer::getMesh(ECRenderer &comp) {
	assert(comp.meshIndex >= 0 && "invalid index");
	return meshes[comp.meshIndex];
}

unsigned int ygl::Renderer::addShader(Shader *shader) {
	shaders.push_back(shader);
	return shaders.size() - 1;
}

unsigned int ygl::Renderer::addMaterial(const Material &mat) {
	materials.push_back(mat);
	return materials.size() - 1;
}

unsigned int ygl::Renderer::addMesh(Mesh *mesh) {
	meshes.push_back(mesh);
	return meshes.size() - 1;
}

void ygl::Renderer::render(Scene& scene) {
	using namespace std;
	for(Entity e : scene.entities) {
		ygl::Transformation transform = scene.getComponent<Transformation>(e);
		ygl::ECRenderer ecr = scene.getComponent<ECRenderer>(e);

		Shader *sh = shaders[ecr.shaderIndex];
		Mesh *mesh = meshes[ecr.meshIndex];
		sh->bind();

		mesh->bind();
		sh->setUniform("worldMatrix", transform.getWorldMatrix());
		glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
		
		mesh->unbind();

	}
	Shader::unbind();
}