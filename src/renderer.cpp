#include <renderer.h>

#include <transformation.h>
#include <assert.h>

ygl::Material::Material()
	: Material(glm::vec3(1., 1., 1.), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.0), 0.0, 0.1, 0., false,
			   0.0, 0.0, 0.0) {}

ygl::Material::Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior,
						glm::vec3 transparency_color, float refraction_chance, glm::vec3 specular_color,
						float refraction_roughness, float specular_roughness, float texture_influence,
						bool use_normal_map, float metallic, float use_roughness_map, float use_ao_map)
	: albedo(albedo),
	  specular_chance(specular_chance),
	  emission(emission),
	  ior(ior),
	  transparency_color(transparency_color),
	  refraction_chance(refraction_chance),
	  specular_color(specular_color),
	  refraction_roughness(refraction_roughness),
	  specular_roughness(specular_roughness),
	  texture_influence(texture_influence),
	  use_normal_map(use_normal_map),
	  metallic(metallic),
	  use_roughness_map(use_roughness_map),
	  use_ao_map(use_ao_map) {}

ygl::Light::Light(glm::mat4 transform, glm::vec3 color, float intensity, ygl::Light::Type type)
	: transform(transform), color(color), intensity(intensity), type(type) {}

ygl::Light::Light(ygl::Transformation transformation, glm::vec3 color, float intensity, ygl::Light::Type type)
	: transform(transformation.getWorldMatrix()), color(color), intensity(intensity), type(type) {}

ygl::RendererComponent::RendererComponent(unsigned int shaderIndex, unsigned int meshIndex, unsigned int materialIndex)
	: shaderIndex(shaderIndex), meshIndex(meshIndex), materialIndex(materialIndex) {}

void ygl::Renderer::init() {
	defaultTexture.bind(GL_TEXTURE1); 
	defaultTexture.bind(GL_TEXTURE2); 
	defaultTexture.bind(GL_TEXTURE3); 
	defaultTexture.bind(GL_TEXTURE4); 
	defaultTexture.bind(GL_TEXTURE5);

	scene->registerComponent<RendererComponent>();
	scene->setSystemSignature<Renderer, Transformation, RendererComponent>();
}

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
	// send material and light data to the GPU through UBOs
	if (materialsBuffer == 0) { glGenBuffers(1, &materialsBuffer); }
	glBindBuffer(GL_UNIFORM_BUFFER, materialsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, materials.size() * sizeof(Material), materials.data(), GL_DYNAMIC_DRAW);

	Shader::setUBO(materialsBuffer, 1);

	if (lightsBuffer == 0) { glGenBuffers(1, &lightsBuffer); }
	uint lightsCount = lights.size();
	glBindBuffer(GL_UNIFORM_BUFFER, lightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, 100 * sizeof(Light) + sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, lights.size() * sizeof(Light), lights.data());
	glBufferSubData(GL_UNIFORM_BUFFER, 100 * sizeof(Light), sizeof(uint), &lightsCount);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Shader::setUBO(lightsBuffer, 2);
}

void ygl::Renderer::setDefaultShader(int defaultShader) { this->defaultShader = defaultShader; }

void ygl::Renderer::doWork() {
	// bind default shader
	if (defaultShader != -1) { shaders[defaultShader]->bind(); }
	int prevShaderIndex = defaultShader;

	for (Entity e : entities) {
		ygl::Transformation	&transform = scene->getComponent<Transformation>(e);
		ygl::RendererComponent &ecr		  = scene->getComponent<RendererComponent>(e);

		Shader *sh;
		// binds the object's own shader if present.
		// Oherwise checks if the default shader has been bound by the previous object
		// and binds it only if needed
		if (ecr.shaderIndex != -1) {
			sh				= shaders[ecr.shaderIndex];
			prevShaderIndex = ecr.shaderIndex;
			sh->bind();
		} else {
			assert(defaultShader != -1 && "cannot use default shader when it is not defined");
			sh = shaders[defaultShader];
			if (prevShaderIndex != -1) { sh->bind(); }
		}

		Mesh *mesh = meshes[ecr.meshIndex];

		// always bind the mesh.
		// TODO: instancing
		mesh->bind();

		sh->setUniform("worldMatrix", transform.getWorldMatrix());
		if (sh->hasUniform("material_index")) sh->setUniform("material_index", (GLuint)ecr.materialIndex);

		glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);

		// mesh->unbind();
	}
	Shader::unbind();
}

ygl::Renderer::~Renderer() {
	for (Shader *sh : shaders) {
		delete sh;
	}
	for (Mesh *mesh : meshes) {
		delete mesh;
	}
}

void ygl::Renderer::drawObject(Transformation &transform, Shader *sh, Mesh *mesh, GLuint materialIndex,
							   bool useTexture) {
	sh->bind();

	mesh->bind();
	sh->setUniform("worldMatrix", transform.getWorldMatrix());
	sh->setUniform("material_index", materialIndex);

	glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
	mesh->unbind();
}

void ygl::Renderer::drawObject(Shader *sh, Mesh *mesh) {
	sh->bind();

	mesh->bind();
	// sh->setUniform("worldMatrix", transform.getWorldMatrix());

	glDrawElements(mesh->getDrawMode(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
	mesh->unbind();
}

void ygl::Renderer::compute(ComputeShader *shader, int domainX, int domainY, int domainZ) {
	shader->bind();
	int groupsX = domainX / shader->groupSize.x + (domainX % shader->groupSize.x > 0);
	int groupsY = domainY / shader->groupSize.y + (domainY % shader->groupSize.y > 0);
	int groupsZ = domainZ / shader->groupSize.z + (domainZ % shader->groupSize.z > 0);

	glDispatchCompute(groupsX, groupsY, groupsZ);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

GLuint ygl::Renderer::loadMaterials(int count, Material *materials) {
	GLuint materialsBuffer;
	glGenBuffers(1, &materialsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, materialsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, count * sizeof(Material), materials, GL_DYNAMIC_DRAW);
	Shader::setUBO(materialsBuffer, 1);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return materialsBuffer;
}

GLuint ygl::Renderer::loadLights(int count, Light *lights) {
	GLuint lightsBuffer;
	glGenBuffers(1, &lightsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, 100 * sizeof(Light) + sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, count * sizeof(Light), lights);
	glBufferSubData(GL_UNIFORM_BUFFER, 100 * sizeof(Light), sizeof(uint), &count);
	Shader::setUBO(lightsBuffer, 2);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return lightsBuffer;
}