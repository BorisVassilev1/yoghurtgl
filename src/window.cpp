#include <window.h>
#include <yoghurtgl.h>
#include <input.h>

#include <iostream>
#include <iomanip>
#include <assert.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <ImGuizmo.h>

#define GL_CONTEXT_VERSION_MAJOR 4
#define GL_CONTEXT_VERSION_MINOR 6

ygl::Window::Window(int width, int height, const char *name, bool vsync, bool resizable, GLFWmonitor *monitor)
	: width(width), height(height) {
	assert(glfwGetPrimaryMonitor() != NULL);
	const GLFWvidmode *mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
#ifndef YGL_NDEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_CONTEXT_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_CONTEXT_VERSION_MINOR);

	if (resizable) glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	else glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, name, monitor, NULL);
	if (!window) {
		std::cerr << "glfwCreateWindow failed." << std::endl;
		glfwTerminate();
		exit(1);
	}

	ygl::Keyboard::init(this);

	glfwSetWindowCloseCallback(window, [](GLFWwindow *) { std::cerr << "closing window!"; });
	Keyboard::addKeyCallback([&](GLFWwindow *window, int key, int, int action, int) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		#ifndef YGL_NDEBUG
		else if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
			shade = !shade;
			if (shade) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			} else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
		} else if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
			cullFace = !cullFace;
			if (cullFace) {
				glEnable(GL_CULL_FACE);
			} else {
				glDisable(GL_CULL_FACE);
			}
		}
		#endif
	});
	glfwSetWindowSizeCallback(window, handleResize);
	addResizeCallback([this](GLFWwindow *window, int width, int height) {
		if (window != getHandle()) return;
		this->width	 = width;
		this->height = height;
	});

	glfwMakeContextCurrent(window);
	ygl::gl_init = true;

	glfwSwapInterval(vsync);
	if (glewInit() != GLEW_OK) {
		std::cerr << "glewInit failed." << std::endl;
		this->~Window();
		THROW_RUNTIME_ERR("GLEW_INIT FAILED");
	}

#ifndef YGL_NDEBUG
	ygl::initDebug();
#endif

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_STENCIL_TEST);

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	glDepthFunc(GL_LEQUAL);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(getHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 430 core");
}

ygl::Window::Window(int width, int height, const char *name, bool vsync, bool resizable)
	: Window(width, height, name, vsync, resizable, NULL) {}

ygl::Window::Window(int width, int height, const char *name, bool vsync) : Window(width, height, name, vsync, true) {}

ygl::Window::Window(int width, int height, const char *name) : Window(width, height, name, true) {}

int ygl::Window::getWidth() { return width; }
int ygl::Window::getHeight() { return height; }

GLFWwindow *ygl::Window::getHandle() { return window; }

bool ygl::Window::shouldClose() { return glfwWindowShouldClose(window); }

void ygl::Window::swapBuffers() {
	// finish Dear ImGui frame
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glFinish();		// TODO: THIS IS VERY BAD!!

	auto	  timeNow = std::chrono::high_resolution_clock::now();
	long long delta	  = std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - lastSwapTime).count();

	glfwSwapBuffers(window);

	timeNow				= std::chrono::high_resolution_clock::now();
	long long fullDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - lastSwapTime).count();
	deltaTime			= fullDelta / 1e9;
	globalTime += deltaTime;

	double logDelta = globalTime - lastPrintTime;

	if (logDelta > 1.) {
		frameCallback(delta, fullDelta);
		lastPrintTime = globalTime;
	}

	lastSwapTime = timeNow;
}

void ygl::Window::beginFrame() {
	glfwPollEvents();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetRect(0, 0, getWidth(), getHeight());
}

ygl::Window::~Window() {
	if (!ygl::glfw_init) return;
	if (window != nullptr) {
		// clean up all ImGui
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		ygl::gl_init = false;
		window		 = nullptr;
	}
}

bool ygl::Window::isFocused() { return glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE; }

void ygl::Window::handleResize(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
	for (auto callback : resizeCallbacks) {
		callback(window, width, height);
	}
}

void ygl::Window::addResizeCallback(const std::function<void(GLFWwindow *, int, int)> &callback) {
	resizeCallbacks.push_back(callback);
}

void ygl::Window::defaultFrameCallback(long draw_time, long frame_time) {
	std::cout << std::fixed << std::setprecision(2) << "\rdraw time: " << (draw_time / 1e6)
			  << "ms, FPS: " << int(1e9 / frame_time) << "         " << std::flush;		//<< std::endl;
}

void ygl::Window::setFrameCallback(void (*callback)(long, long)) { frameCallback = callback; }
