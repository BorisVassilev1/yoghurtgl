#define YGL_WINDOW_ERROR 0
#include <yoghurtgl.h>
#include <window.h>
#include <imgui.h>

int main(int argc, char **argv) {

	assert(argc == 3);

	if(ygl::init()) {
		dbLog(ygl::LOG_ERROR, "cannot initialize ygl");
	}

	ygl::Window window = ygl::Window(200, 200, argv[1]);

	int result = 0;
	while (!window.shouldClose()) {
		window.beginFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(window.getWidth(), window.getHeight()));
		ImGui::Begin("asd", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
		ImGui::Text("%s", argv[2]);

		if(ImGui::Button("Ignore")) {result = 0; window.close();}
		ImGui::SameLine();
		if(ImGui::Button("Debug")) {result = 1;window.close();}
		ImGui::SameLine();
		if(ImGui::Button("Terminate")) {result = 2;window.close();}
		ImGui::End();
		window.swapBuffers();
	}

	ygl::terminate();
	exit(result);
}
