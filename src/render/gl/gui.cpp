#include "mineclonelib/render/gl/gui.h"

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "mineclonelib/render/gui.h"
#include <backends/imgui_impl_opengl3.h>

namespace mc
{
namespace render
{
gl_gui_context::gl_gui_context(context *ctx)
	: gui_context(ctx)
{
	ImGui_ImplOpenGL3_Init();
}

gl_gui_context::~gl_gui_context()
{
	ImGui_ImplOpenGL3_Shutdown();
}

void gl_gui_context::begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	gui_context::begin();
}

void gl_gui_context::present()
{
	gui_context::present();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO &io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow *ctx = glfwGetCurrentContext();

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		glfwMakeContextCurrent(ctx);
	}
}
}
}
