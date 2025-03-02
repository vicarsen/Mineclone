#include "mineclonelib/render/gui.h"
#include "mineclonelib/render/vk/gui.h"
#include "mineclonelib/render/vk/context.h"

#include <imgui.h>
#include <backends/custom/imgui_impl_vulkan.h>

namespace mc
{
namespace render
{
static void check_vk_result(VkResult result)
{
	LOG_ASSERT(Render, result == VK_SUCCESS,
		   "ImGui_ImplVulkan internal error");
}

vk_gui_context::vk_gui_context(context *ctx)
	: gui_context(ctx)
{
	vk_context *vk_ctx = reinterpret_cast<vk_context *>(m_ctx);

	VkDevice device = vk_ctx->get_device();

	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		  IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE }
	};

	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
	};

	pool_info.maxSets = 0;
	for (VkDescriptorPoolSize pool_size : pool_sizes) {
		pool_info.maxSets += pool_size.descriptorCount;
	}

	pool_info.poolSizeCount =
		static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
	pool_info.pPoolSizes = pool_sizes;

	LOG_ASSERT(Render,
		   vkCreateDescriptorPool(device, &pool_info, NULL, &m_pool) ==
			   VK_SUCCESS,
		   "Failed to create Vulkan descriptor pool");

	ImGui_ImplVulkan_InitInfo info = {
		.Instance = vk_ctx->get_instance(),
		.PhysicalDevice = vk_ctx->get_physical_device(),
		.Device = vk_ctx->get_device(),
		.QueueFamily = vk_ctx->get_graphics_idx(),
		.Queue = vk_ctx->get_graphics_queue(),
		.DescriptorPool = m_pool,
		.RenderPass = vk_ctx->get_render_pass(),
		.MinImageCount = vk_ctx->get_image_count(),
		.ImageCount = vk_ctx->get_image_count(),
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.PipelineCache = VK_NULL_HANDLE,
		.Subpass = 0,
		.Allocator = NULL,
		.CheckVkResultFn = check_vk_result,
	};

	ImGui_ImplVulkan_Init(&info);
}

vk_gui_context::~vk_gui_context()
{
	vk_context *vk_ctx = reinterpret_cast<vk_context *>(m_ctx);
	VkDevice device = vk_ctx->get_device();

	vkDeviceWaitIdle(device);

	ImGui_ImplVulkan_Shutdown();

	if (m_pool) {
		vkDestroyDescriptorPool(device, m_pool, NULL);
	}
}

void vk_gui_context::begin()
{
	ImGui_ImplVulkan_NewFrame();
	gui_context::begin();
}

void vk_gui_context::present()
{
	gui_context::present();

	ImGuiIO &io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	vk_context *vk_ctx = reinterpret_cast<vk_context *>(m_ctx);

	if (!vk_ctx->is_minimized()) {
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
						vk_ctx->get_command_buffer());
	}
}
}
}
