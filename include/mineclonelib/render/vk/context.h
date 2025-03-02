#pragma once

#include "mineclonelib/io/input.h"
#include "mineclonelib/render/context.h"

#include <glad/vulkan.h>

namespace mc
{
namespace render
{
class vk_context : public context, public input_handler {
    public:
	vk_context(mc::window *window);
	virtual ~vk_context();

	virtual void begin() override;
	virtual void present() override;

	virtual void framebuffer_callback(int width, int height) override;

    private:
	void create_instance();
	void create_messenger();
	void create_surface();
	void choose_gpu();
	void create_device();
	void create_swapchain();
	void cleanup_swapchain();
	void recreate_swapchain();
	void create_render_pass();
	void create_swapchain_frames();
	void create_command_pool();
	void create_frames();

    private:
	const int m_frames_in_flight = 2;

	window *m_window;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_messenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_gpu;
	VkDevice m_device;

	uint32_t m_graphics_idx;
	VkQueue m_graphics;

	VkSwapchainKHR m_swapchain;
	VkFormat m_format;
	VkExtent2D m_extent;

	VkRenderPass m_render_pass;

	VkCommandPool m_command_pool;

	struct swapchain_frame {
		VkImage image;
		VkImageView view;
		VkFramebuffer framebuffer;
	};

	std::vector<swapchain_frame> m_swapchain_frames;
	uint32_t m_image_index;

	struct frame {
		VkCommandBuffer command_buffer;
		VkSemaphore image_available;
		VkSemaphore render_finished;
		VkFence in_flight;
	};

	std::vector<frame> m_frames;
	uint32_t m_current_frame;

	bool m_framebuffer_resized;
	bool m_minimized;
};
}
}
