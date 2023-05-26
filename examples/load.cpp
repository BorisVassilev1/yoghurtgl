#include <yoghurtgl.h>

#include <window.h>
#include <input.h>
#include <camera.h>
#include <ecs.h>
#include <renderer.h>
#include <transformation.h>
#include <fstream>

using namespace ygl;

void run() {
	Window window = Window(1280, 1000, "Test Window", true, false);

	Camera cam(glm::radians(70.f), window, 0.01, 1000);

	Mouse		 mouse(window);
	FPController controller(&window, &mouse, cam.transform);

	Scene scene;
	scene.registerComponentIfCan<ygl::Transformation>();
	Renderer *renderer = scene.registerSystem<Renderer>(&window);

	std::ifstream is = std::ifstream("pbrDragon.sc");
	try {
		scene.read(is);
	} catch (std::exception &e) { dbLog(ygl::LOG_ERROR, e.what()); }
	is.close();

	glClearColor(0, 0, 0, 1);
	while (!window.shouldClose()) {
		window.beginFrame();
		mouse.update();

		controller.update();
		cam.update();

		renderer->doWork();

		window.swapBuffers();
	}
}

int main() {
	if (init()) {
		dbLog(ygl::LOG_ERROR, "ygl failed to init");
		exit(1);
	}

	run();

	ygl::terminate();
	std::cerr << std::endl;
	return 0;
}
