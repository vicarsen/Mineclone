#include "mineclonelib/render/render.h"
#include "mineclonelib/render/gui.h"
#include "mineclonelib/render/gl/gui.h"
#include "mineclonelib/render/vk/gui.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

namespace mc
{
namespace render
{
gui_context::gui_context(context *ctx)
	: m_ctx(ctx)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	window *wnd = m_ctx->get_window();
	render::render_api api = wnd->get_api();

	switch (api) {
	case render::render_api::opengl:
		ImGui_ImplGlfw_InitForOpenGL(wnd->get_handle(), true);
		break;
	case render::render_api::vulkan:
		ImGui_ImplGlfw_InitForVulkan(wnd->get_handle(), true);
		break;
	default:
		ImGui_ImplGlfw_InitForOther(wnd->get_handle(), true);
		break;
	}
}

gui_context::~gui_context()
{
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void gui_context::begin()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void gui_context::present()
{
	ImGui::Render();
}

std::unique_ptr<gui_context> gui_context::create(context *ctx)
{
	render::render_api api = ctx->get_window()->get_api();

	switch (api) {
	case render::render_api::opengl:
		return std::make_unique<gl_gui_context>(ctx);
	case render::render_api::vulkan:
		return std::make_unique<vk_gui_context>(ctx);
	case render::render_api::none:
		LOG_CRITICAL(
			Render,
			"Failed to create context: render_api::none currently not supported");
		return nullptr;
	default:
		return nullptr;
	}
}
}
}
