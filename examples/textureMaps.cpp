#include <yoghurtgl.h>

#include <window.h>
#include <shader.h>
#include <input.h>
#include <camera.h>
#include <ecs.h>
#include <renderer.h>

using namespace ygl;

int main() {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	Window window = Window(800, 600, "Test Window", true);

	VFShader *shader = new VFShader("./shaders/simple.vs", "./shaders/simple.fs");

	Camera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene(&window);
	scene.registerComponent<Transformation>();

	Renderer *renderer = scene.registerSystem<Renderer>();

	Texture2d *color	 = new Texture2d("./res/images/stones/albedo.png", ITexture::Type::SRGB);
	Texture2d *height	 = new Texture2d("./res/images/stones/height.png", ITexture::Type::GREY);
	Texture2d *normal	 = new Texture2d("./res/images/stones/normal.png", ITexture::Type::RGB);
	Texture2d *roughness = new Texture2d("./res/images/stones/roughness.png", ITexture::Type::GREY);
	Texture2d *ao		 = new Texture2d("./res/images/stones/ao.png", ITexture::Type::GREY);

	// Texture2d *color	 = new Texture2d("./res/horse_statue_01_4k/textures/horse_statue_01_diff_4k.png",
	// ITexture::Type::SRGB); Texture2d *height	 = new Texture2d(1,1, ITexture::Type::GREY, nullptr); Texture2d *normal
	// = new Texture2d("./res/horse_statue_01_4k/textures/horse_statue_01_nor_gl_4k.png", ITexture::Type::RGB);
	// Texture2d *roughness = new Texture2d("./res/horse_statue_01_4k/textures/horse_statue_01_rough_4k.png",
	// ITexture::Type::GREY); Texture2d *ao		 = new
	// Texture2d("./res/horse_statue_01_4k/textures/horse_statue_01_ao_4k.png", ITexture::Type::GREY);

	color->bind(GL_TEXTURE1);
	normal->bind(GL_TEXTURE2);
	height->bind(GL_TEXTURE3);
	roughness->bind(GL_TEXTURE4);
	ao->bind(GL_TEXTURE5);

	// Mesh *modelMesh = makeBox(glm::vec3(1, 1, 1), glm::vec3(20, 20, 20));
	Mesh *modelMesh = (Mesh *)getModel(loadScene("./res/models/bunny_uv/bunny_uv.obj"));
	// Mesh *modelMesh = (Mesh *)getModel(loadScene("./res/horse_statue_01_4k/horse_statue_01_4k.fbx"));

	Entity model = scene.createEntity();
	scene.addComponent<Transformation>(model, Transformation(glm::vec3(), glm::vec3(0), glm::vec3(1)));
	RendererComponent modelRenderer;
	modelRenderer.materialIndex =
		renderer->addMaterial(Material(glm::vec3(1., 1., 1.), 0.02, glm::vec3(0), 1.0, glm::vec3(1.0), 0.0,
									   glm::vec3(1.0), 0.0, 0.0, 1.0, true, 0.0, 0.5, 1.0));
	modelRenderer.shaderIndex = renderer->addShader(shader);
	modelRenderer.meshIndex	  = renderer->addMesh(modelMesh);
	scene.addComponent(model, modelRenderer);

	renderer->addLight(Light(Transformation(glm::vec3(0), glm::vec3(1, -.3, 0), glm::vec3(1)), glm::vec3(1., 1., 1.), 3,
							 Light::Type::DIRECTIONAL));
	renderer->addLight(Light(Transformation(), glm::vec3(1., 1., 1.), 0.11, Light::Type::AMBIENT));

	renderer->loadData();

	Mesh	 *screenQuad = makeScreenQuad();
	VFShader onScreen("./shaders/ui/textureOnScreen.vs", "./shaders/ui/textureOnScreen.fs");
	onScreen.bind();
	if (onScreen.hasUniform("texture_sampler")) onScreen.setUniform("texture_sampler", 7);
	if (onScreen.hasUniform("texture_sampler_depth")) onScreen.setUniform("texture_sampler_depth", 8);
	if (onScreen.hasUniform("texture_sampler_stencil")) onScreen.setUniform("texture_sampler_stencil", 9);
	onScreen.unbind();

	FrameBuffer frameBuffer(window.getWidth(), window.getHeight());

	frameBuffer.getColor()->bind(GL_TEXTURE7);
	frameBuffer.getDepthStencil()->bind(GL_TEXTURE8);
	frameBuffer.getDepthStencil()->bind(GL_TEXTURE9);

	glActiveTexture(GL_TEXTURE8);
	frameBuffer.getDepthStencil()->bind(GL_TEXTURE8);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

	GLuint stencil_view;
	glGenTextures(1, &stencil_view);
	glTextureView(stencil_view, GL_TEXTURE_2D, frameBuffer.getDepthStencil()->getID(), GL_DEPTH32F_STENCIL8, 0, 1, 0, 1);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, stencil_view);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);

	glActiveTexture(GL_TEXTURE0);

	Keyboard::addKeyCallback([&](GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_2) {
			onScreen.bind();
			onScreen.setUniform("viewMode", 0);
			onScreen.unbind();
		} else if (key == GLFW_KEY_3) {
			onScreen.bind();
			onScreen.setUniform("viewMode", 1);
			onScreen.unbind();
		} else if (key == GLFW_KEY_4) {
			onScreen.bind();
			onScreen.setUniform("viewMode", 2);
			onScreen.unbind();
		}
	});

	controller.speed = 0.4;

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		Transformation &transform = scene.getComponent<Transformation>(model);
		// transform.rotation += glm::vec3(0.3 * window.deltaTime);
		transform.rotation.y += 0.3 * window.deltaTime;
		transform.updateWorldMatrix();

		controller.update();
		cam.update();

		frameBuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		renderer->doWork();
		frameBuffer.unbind();

		Renderer::drawObject(&onScreen, screenQuad);

		window.swapBuffers();
	}

	// clean up and exit
	window.~Window();
	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}