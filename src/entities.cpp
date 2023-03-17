#include <entities.h>

ygl::Entity ygl::addBox(Scene &scene, glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
	Entity	  e		   = scene.createEntity();
	Renderer *renderer = scene.getSystem<Renderer>();
	if (canonicalCubeIndex == (uint)-1) { canonicalCubeIndex = renderer->addMesh(makeCube(1.)); }
	uint	  materialIndex = renderer->addMaterial(Material());
	Material &mat			= renderer->getMaterial(materialIndex);
	mat.albedo				= color;
	scene.addComponent<Transformation>(e, Transformation(position, glm::vec3(), scale));
	scene.addComponent<RendererComponent>(e, RendererComponent(-1, canonicalCubeIndex, materialIndex));

	return e;
}

ygl::Entity ygl::addSphere(Scene &scene, glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
	Entity	  e		   = scene.createEntity();
	Renderer *renderer = scene.getSystem<Renderer>();
	if (canonicalSphereIndex == (uint)-1) { canonicalSphereIndex = renderer->addMesh(makeUnitSphere()); }
	uint	  materialIndex = renderer->addMaterial(Material());
	Material &mat			= renderer->getMaterial(materialIndex);
	mat.albedo				= color;
	scene.addComponent<Transformation>(e, Transformation(position, glm::vec3(), scale));
	scene.addComponent<RendererComponent>(e, RendererComponent(-1, canonicalSphereIndex, materialIndex));

	return e;
}

ygl::Entity ygl::addModel(Scene &scene, Mesh *mesh, glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
	Entity	  e				= scene.createEntity();
	Renderer *renderer		= scene.getSystem<Renderer>();
	uint	  meshIndex		= renderer->addMesh(mesh);
	uint	  materialIndex = renderer->addMaterial(Material());
	Material &mat			= renderer->getMaterial(materialIndex);
	mat.albedo				= color;
	scene.addComponent<Transformation>(e, Transformation(position, glm::vec3(), scale));
	scene.addComponent<RendererComponent>(e, RendererComponent(-1, meshIndex, materialIndex));

	return e;
}

ygl::Entity ygl::addSkybox(Scene &scene, const std::string &path) {
	Entity	  e		   = scene.createEntity();
	Renderer *renderer = scene.getSystem<Renderer>();

	Mesh* mesh = makeCube(1.);
	mesh->setCullFace(false);
	mesh->setDepthFunc(GL_LEQUAL);
	uint meshIndex = renderer->addMesh(mesh);
	ygl::Material mat;
	mat.albedo_map	   = scene.assetManager.addTexture(new TextureCubemap(path, ".jpg"), path);
	mat.use_albedo_map = 1.0;
	uint materialIndex = renderer->addMaterial(mat);
	uint shaderIndex   = renderer->addShader(new VFShader("./shaders/skybox.vs", "./shaders/skybox.fs"));

	scene.addComponent(e, Transformation());
	scene.addComponent(e, RendererComponent(shaderIndex, meshIndex, materialIndex));

	return e;
}