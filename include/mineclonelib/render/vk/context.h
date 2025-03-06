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
	vk_context(window *wnd);
	virtual ~vk_context();

	virtual void begin(render_state *state) override;
	virtual void present() override;

	virtual void make_current() override;
	virtual void unmake_current() override;

	virtual void framebuffer_callback(int width, int height) override;

	inline VkInstance get_instance() const noexcept
	{
		return m_instance;
	}

	inline VkSurfaceKHR get_surface() const noexcept
	{
		return m_surface;
	}

	inline VkPhysicalDevice get_physical_device() const noexcept
	{
		return m_gpu;
	}

	inline VkDevice get_device() const noexcept
	{
		return m_device;
	}

	inline uint32_t get_graphics_idx() const noexcept
	{
		return m_graphics_idx;
	}

	inline VkQueue get_graphics_queue() const noexcept
	{
		return m_graphics;
	}

	inline VkSwapchainKHR get_swapchain() const noexcept
	{
		return m_swapchain;
	}

	inline VkRenderPass get_render_pass() const noexcept
	{
		return m_render_pass;
	}

	inline uint32_t get_image_count() const noexcept
	{
		return m_swapchain_frames.size();
	}

	inline VkCommandBuffer get_command_buffer() const noexcept
	{
		return m_frames[m_current_frame].command_buffer;
	}

	inline bool is_minimized() const noexcept
	{
		return m_minimized;
	}

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
