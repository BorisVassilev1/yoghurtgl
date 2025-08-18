#include <msgBox.h>

#define YGL_WINDOW_ERROR 0
#include <yoghurtgl.h>
#include <window.h>
#include <imgui.h>
#include <cxxabi.h>
#include <unistd.h>

#ifdef __linux__
#include <execinfo.h>
#include <sys/wait.h>

void ygl::createMessageBox(const std::string &msg, const std::string &desc) {
	int pid = fork();
	if (pid < 0) return;
	if (pid) {
		int status = 0;
		waitpid(pid, &status, 0);
		int exit_status = WEXITSTATUS(status);
		dbLog(ygl::LOG_DEBUG, "status: ", exit_status);
		if(exit_status == 0) return;
		if(exit_status == 1) {};
		if(exit_status == 2) exit(1);
	} else {
		execl("./msgBox", "./msgBox", msg.c_str(), desc.c_str(), NULL);
	}
}
#endif
