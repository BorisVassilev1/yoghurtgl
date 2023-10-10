#include <entities.h>
#include <asset_manager.h>
#include <string>
#include "texture.h"
#include "yoghurtgl.h"
#include <mesh.h>
#include <transformation.h>

ygl::Entity ygl::addBox(Scene &scene, glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
	Entity		  e		   = scene.createEntity();
	Renderer	 *renderer = scene.getSystem<Renderer>();
	AssetManager *asman	   = scene.getSystem<AssetManager>();
	if (canonicalCubeIndex == (uint)-1) { canonicalCubeIndex = asman->addMesh(new BoxMesh(1.), "canonincalCube"); }
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
		canonicalSphereIndex = asman->addMesh(new SphereMesh(), "canonicalSphere");
	}
	uint	  materialIndex = renderer->addMaterial(Material());
	Material &mat			= renderer->getMaterial(materialIndex);
	mat.albedo				= color;
	scene.addComponent<Transformation>(e, Transformation(position, glm::vec3(), scale));
	scene.addComponent<RendererComponent>(e, RendererComponent(-1, canonicalSphereIndex, materialIndex));

	return e;
}

ygl::Entity ygl::addModel(Scene &scene, Mesh *mesh, glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
	Entity		  e				= scene.createEntity();
	Renderer	 *renderer		= scene.getSystem<Renderer>();
	AssetManager *asman			= scene.getSystem<AssetManager>();
	uint		  meshIndex		= asman->addMesh(mesh, "some mesh??");	   // TODO: this is bad!
	uint		  materialIndex = renderer->addMaterial(Material());
	Material	 &mat			= renderer->getMaterial(materialIndex);
	mat.albedo					= color;
	scene.addComponent<Transformation>(e, Transformation(position, glm::vec3(), scale));
	scene.addComponent<RendererComponent>(e, RendererComponent(-1, meshIndex, materialIndex));

	return e;
}

ygl::Entity ygl::addSkybox(Scene &scene, const std::string &path, const std::string& format) {
	Entity	  e		   = scene.createEntity();
	Renderer *renderer = scene.getSystem<Renderer>();

	Mesh *mesh = new BoxMesh(1.);
	mesh->setCullFace(false);
	mesh->setDepthFunc(GL_LEQUAL);
	AssetManager *asman		= scene.getSystem<AssetManager>();
	uint		  meshIndex = asman->addMesh(mesh, "skycube");
	ygl::Material mat;
	if(format == ".hdr") {
		mat.albedo_map	   = scene.getSystem<AssetManager>()->addTexture(createIrradianceCubemap(loadHDRCubemap(path, format)), path);
	} else {
		mat.albedo_map	   = scene.getSystem<AssetManager>()->addTexture(new TextureCubemap(path, format), path);
	}
	renderer->skyboxTexture = mat.albedo_map;
	mat.use_albedo_map = 1.0;
	uint materialIndex = renderer->addMaterial(mat);
	uint shaderIndex   = asman->addShader(new VFShader("./shaders/skybox.vs", "./shaders/skybox.fs"), "skyShader");

	scene.addComponent(e, Transformation());
	scene.addComponent(e, RendererComponent(shaderIndex, meshIndex, materialIndex));

	return e;
}

#ifndef YGL_NO_ASSIMP
ygl::Entity ygl::addModel(ygl::Scene &scene, std::string filePath, uint i) {
	ygl::AssetManager *asman = scene.getSystem<AssetManager>();

	Mesh *modelMesh;
	try {
		modelMesh = new MeshFromFile(filePath, i);
	} catch (std::exception &e) { THROW_RUNTIME_ERR("Failed loading MeshFromFile: " + filePath); }
	Entity model = scene.createEntity();
	scene.addComponent<Transformation>(model, Transformation(glm::vec3(), glm::vec3(0), glm::vec3(1.)));

	Material mat = ygl::MeshFromFile::getMaterial(MeshFromFile::loadedScene, asman, filePath, i);

	Renderer *renderer = scene.getSystem<Renderer>();

	RendererComponent modelRenderer;
	modelRenderer.materialIndex = renderer->addMaterial(mat);
	modelRenderer.shaderIndex	= renderer->getDefaultShader();
	modelRenderer.meshIndex		= asman->addMesh(modelMesh, filePath + std::to_string(i));
	scene.addComponent(model, modelRenderer);
	return model;
}

void ygl::addModels(ygl::Scene &scene, std::string filePath, const std::function<void(Entity)> &edit) {
	try {
		// if this does not fail, the calls to addModel will not throw
		MeshFromFile::loadSceneIfNeeded(filePath);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return;
	}
	for (uint i = 0; i < MeshFromFile::loadedScene->mNumMeshes; ++i) {
		ygl::Entity model = addModel(scene, filePath, i);
		edit(model);
	}
}
#endif
