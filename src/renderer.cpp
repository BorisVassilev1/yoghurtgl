#include <renderer.h>

#include <transformation.h>
#include <assert.h>

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

ygl::Light::Light(glm::mat4 transform, glm::vec3 color, float intensity, ygl::Light::Type type)
	: transform(transform), color(color), intensity(intensity), type(type) {}

ygl::Light::Light(ygl::Transformation transformation, glm::vec3 color, float intensity, ygl::Light::Type type)
	: transform(transformation.getWorldMatrix()), color(color), intensity(intensity), type(type) {}

ygl::RendererComponent::RendererComponent(unsigned int shaderIndex, unsigned int meshIndex, unsigned int materialIndex)
	: shaderIndex(shaderIndex), meshIndex(meshIndex), materialIndex(materialIndex) {}

ygl::Shader *ygl::Renderer::getShader(RendererComponent &comp) {
	assert(comp.shaderIndex >= 0 && "invalid index");
	return shaders[comp.shaderIndex];
}

ygl::Material &ygl::Renderer::getMaterial(RendererComponent &comp) {
	assert(comp.materialIndex >= 0 && "invalid index");
	return materials[comp.materialIndex];
}

ygl::Mesh *ygl::Renderer::getMesh(RendererComponent &comp) {
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

ygl::Light &ygl::Renderer::addLight(const Light &light) {
	lights.push_back(light);
	return lights.back();
}

void ygl::Renderer::loadData() {
	if (materialsBuffer == 0) { glGenBuffers(1, &materialsBuffer); }
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, materials.size() * sizeof(Material), materials.data(), GL_DYNAMIC_DRAW);

	Shader::setSSBO(materialsBuffer, 0);

	if (lightsBuffer == 0) { glGenBuffers(1, &lightsBuffer); }
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(Light), lights.data(), GL_DYNAMIC_DRAW);

	Shader::setSSBO(lightsBuffer, 1);
}

void ygl::Renderer::doWork() {
	using namespace std;

	for (Entity e : scene->entities) {
		ygl::Transformation	   transform = scene->getComponent<Transformation>(e);
		ygl::RendererComponent ecr		 = scene->getComponent<RendererComponent>(e);

		Shader *sh	 = shaders[ecr.shaderIndex];
		Mesh	 *mesh = meshes[ecr.meshIndex];
		sh->bind();

		mesh->bind();
		assert(sh->hasUniform("worldMatrix"));
		sh->setUniform("worldMatrix", transform.getWorldMatrix());
		assert(sh->hasUniform("material_index"));
		sh->setUniform("material_index", ecr.materialIndex);

		glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
		mesh->unbind();
	}
	Shader::unbind();
}