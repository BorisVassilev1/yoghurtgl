#include <entities.h>
#include <importer.h>
#include <string>

ygl::Entity ygl::addBox(Scene &scene, glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
	Entity		  e		   = scene.createEntity();
	Renderer	 *renderer = scene.getSystem<Renderer>();
	AssetManager *asman	   = scene.getSystem<AssetManager>();
	if (canonicalCubeIndex == (uint)-1) { canonicalCubeIndex = asman->addMesh(makeCube(1.), "canonincalCube"); }
	uint	  materialIndex = renderer->addMaterial(Material());
	Material &mat			= renderer->getMaterial(materialIndex);
	mat.albedo				= color;
	scene.addComponent<Transformation>(e, Transformation(position, glm::vec3(), scale));
	scene.addComponent<RendererComponent>(e, RendererComponent(-1, canonicalCubeIndex, materialIndex));

	return e;
}

ygl::Entity ygl::addSphere(Scene &scene, glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
	Entity		  e		   = scene.createEntity();
	Renderer	 *renderer = scene.getSystem<Renderer>();
	AssetManager *asman	   = scene.getSystem<AssetManager>();
	if (canonicalSphereIndex == (uint)-1) {
		canonicalSphereIndex = asman->addMesh(makeUnitSphere(), "canonicalSphere");
	}
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
	AssetManager *asman = scene.getSystem<AssetManager>();
	uint	  meshIndex		= asman->addMesh(mesh, "some mesh??"); // TODO: this is bad!
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

	Mesh *mesh = makeCube(1.);
	mesh->setCullFace(false);
	mesh->setDepthFunc(GL_LEQUAL);
	AssetManager *asman = scene.getSystem<AssetManager>();
	uint		  meshIndex = asman->addMesh(mesh, "skybox");
	ygl::Material mat;
	mat.albedo_map	   = scene.getSystem<AssetManager>()->addTexture(new TextureCubemap(path, ".jpg"), path);
	mat.use_albedo_map = 1.0;
	uint materialIndex = renderer->addMaterial(mat);
	uint shaderIndex   = renderer->addShader(new VFShader("./shaders/skybox.vs", "./shaders/skybox.fs"));

	scene.addComponent(e, Transformation());
	scene.addComponent(e, RendererComponent(shaderIndex, meshIndex, materialIndex));

	return e;
}

ygl::Entity ygl::addModel(ygl::Scene &scene, const aiScene *aiscene, std::string filePath, uint i) {
	ygl::AssetManager &asman = *scene.getSystem<AssetManager>();

	Mesh *modelMesh = (Mesh *)getModel(aiscene, i);

	Entity model = scene.createEntity();
	scene.addComponent<Transformation>(model, Transformation(glm::vec3(), glm::vec3(0), glm::vec3(1.)));

	Material mat = getMaterial(aiscene, asman, filePath, i);

	Renderer *renderer = scene.getSystem<Renderer>();

	RendererComponent modelRenderer;
	modelRenderer.materialIndex = renderer->addMaterial(mat);
	modelRenderer.shaderIndex	= renderer->getDefaultShader();
	modelRenderer.meshIndex		= asman.addMesh(modelMesh, filePath + std::to_string(i));
	scene.addComponent(model, modelRenderer);
	return model;
}

void ygl::addModels(ygl::Scene &scene, const aiScene *aiscene, std::string filePath,
					const std::function<void(Entity)> &edit) {
	for (uint i = 0; i < aiscene->mNumMeshes; ++i) {
		ygl::Entity model = addModel(scene, aiscene, filePath, i);
		edit(model);
	}
}

void ygl::addScene(ygl::Scene &scene, const std::string &filename, const std::function<void(Entity)> &edit) {
	const aiScene *aiscene = ygl::loadScene(filename);
	std::size_t	   cut	   = filename.rfind('/');
	std::string	   dir	   = filename.substr(0, cut + 1);
	std::cout << dir << std::endl;
	addModels(scene, aiscene, dir, edit);
}
