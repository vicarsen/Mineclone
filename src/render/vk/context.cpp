#include "mineclonelib/render/vk/context.h"

#include "mineclonelib/cvar.h"
#include "mineclonelib/misc.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>

#ifndef NDEBUG
static mc::cvar<bool> vulkan_validation(
	true, "render/vulkan/validation",
	"Whether or not to enable validation layers in Vulkan");
#else
static mc::cvar<bool> vulkan_validation(
	false, "render/vulkan/validation",
	"Whether or not to enable validation layers in Vulkan");
#endif

static mc::cvar<const char *> vulkan_physical_device(
	nullptr, "render/vulkan/physical_device",
	"The name of the physical device to use for Vulkan");

static VKAPI_ATTR VkBool32
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	       VkDebugUtilsMessageTypeFlagsEXT type,
	       const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
	       void *p_user_data)
{
	const char *type_str;
	switch (type) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		type_str = "general";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		type_str = "validation";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		type_str = "performance";
		break;
	default:
		type_str = "unknown";
		break;
	}

	const char *message = p_callback_data->pMessage;

	if (severity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		LOG_DEBUG(Render, "Vulkan [{}] message: {}", type_str, message);
	} else if (severity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		LOG_INFO(Render, "Vulkan [{}] message: {}", type_str, message);
	} else if (severity <=
		   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		LOG_WARN(Render, "Vulkan [{}] message: {}", type_str, message);
	} else if (severity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		LOG_ERROR(Render, "Vulkan [{}] message: {}", type_str, message);
	} else {
		LOG_CRITICAL(Render, "Vulkan [{}] message: {}", type_str,
			     message);
	}

	return VK_FALSE;
}

namespace mc
{
namespace render
{
vk_context::vk_context(mc::window *window)
{
	m_window = window;
	window->add_input_handler(this);

	create_instance();
	create_messenger();
	create_surface();
	choose_gpu();
	create_device();
	create_swapchain();
	create_render_pass();
	create_swapchain_frames();
	create_command_pool();
	create_frames();
}

vk_context::~vk_context()
{
	if (m_device) {
		vkDeviceWaitIdle(m_device);
	}

	for (uint32_t i = 0; i < m_frames.size(); i++) {
		if (m_frames[i].in_flight) {
			vkDestroyFence(m_device, m_frames[i].in_flight, NULL);
		}

		if (m_frames[i].render_finished) {
			vkDestroySemaphore(m_device,
					   m_frames[i].render_finished, NULL);
		}

		if (m_frames[i].image_available) {
			vkDestroySemaphore(m_device,
					   m_frames[i].image_available, NULL);
		}
	}

	if (m_command_pool) {
		vkDestroyCommandPool(m_device, m_command_pool, NULL);
	}

	cleanup_swapchain();

	if (m_render_pass) {
		vkDestroyRenderPass(m_device, m_render_pass, NULL);
	}

	if (m_device) {
		vkDestroyDevice(m_device, NULL);
	}

	if (m_surface) {
		vkDestroySurfaceKHR(m_instance, m_surface, NULL);
	}

	if (m_messenger) {
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_messenger, NULL);
	}

	if (m_instance) {
		vkDestroyInstance(m_instance, NULL);
	}

	m_window->remove_input_handler(this);
}

void vk_context::begin()
{
	if (m_minimized) {
		return;
	}

	vkWaitForFences(m_device, 1, &m_frames[m_current_frame].in_flight,
			VK_TRUE, UINT64_MAX);

	VkResult result = VK_ERROR_OUT_OF_DATE_KHR;
	while (result == VK_ERROR_OUT_OF_DATE_KHR) {
		result = vkAcquireNextImageKHR(
			m_device, m_swapchain, UINT64_MAX,
			m_frames[m_current_frame].image_available,
			VK_NULL_HANDLE, &m_image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate_swapchain();
		} else {
			LOG_ASSERT(
				Render,
				result == VK_SUCCESS ||
					result == VK_SUBOPTIMAL_KHR,
				"Failed to acquire next Vulkan swapchain image");
		}
	}

	vkResetFences(m_device, 1, &m_frames[m_current_frame].in_flight);

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = NULL,
	};

	vkResetCommandBuffer(m_frames[m_current_frame].command_buffer, 0);

	LOG_ASSERT(
		Render,
		vkBeginCommandBuffer(m_frames[m_current_frame].command_buffer,
				     &begin_info) == VK_SUCCESS,
		"Failed to begin recording Vulkan command buffer");

	VkClearValue clear_color = { .color = { .float32 = { 0.1f, 0.2f, 0.8f,
							     1.0f } } };

	VkRenderPassBeginInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_render_pass,
		.framebuffer = m_swapchain_frames[m_image_index].framebuffer,
		.renderArea = { .offset = { 0, 0 }, .extent = m_extent },
		.clearValueCount = 1,
		.pClearValues = &clear_color,
	};

	vkCmdBeginRenderPass(m_frames[m_current_frame].command_buffer,
			     &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void vk_context::present()
{
	if (m_minimized) {
		return;
	}

	vkCmdEndRenderPass(m_frames[m_current_frame].command_buffer);

	LOG_ASSERT(
		Render,
		vkEndCommandBuffer(m_frames[m_current_frame].command_buffer) ==
			VK_SUCCESS,
		"Failed to record Vulkan command buffer");

	VkPipelineStageFlags wait_stages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_frames[m_current_frame].image_available,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_frames[m_current_frame].command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_frames[m_current_frame].render_finished,
	};

	LOG_ASSERT(Render,
		   vkQueueSubmit(m_graphics, 1, &submit_info,
				 m_frames[m_current_frame].in_flight) ==
			   VK_SUCCESS,
		   "Failed to submit Vulkan draw command buffer");

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_frames[m_current_frame].render_finished,
		.swapchainCount = 1,
		.pSwapchains = &m_swapchain,
		.pImageIndices = &m_image_index,
		.pResults = NULL,
	};

	VkResult result = vkQueuePresentKHR(m_graphics, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
	    m_framebuffer_resized) {
		recreate_swapchain();
	} else {
		LOG_ASSERT(Render, result == VK_SUCCESS,
			   "Failed to present Vulkan swapchain image");
	}

	m_current_frame = (m_current_frame + 1) % m_frames.size();
}

void vk_context::framebuffer_callback(int width, int height)
{
	m_framebuffer_resized = true;
}

static void
populate_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info)
{
	*create_info = {
		.sType =
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = 0,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debug_callback,
		.pUserData = nullptr
	};

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_DEBUG
	create_info->messageSeverity |=
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_INFO
	create_info->messageSeverity |=
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
#endif

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_WARN
	create_info->messageSeverity |=
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
#endif

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_ERROR
	create_info->messageSeverity |=
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#endif
}

void vk_context::create_instance()
{
	LOG_ASSERT(Render, gladLoaderLoadVulkan(NULL, NULL, NULL),
		   "Failed to load Vulkan");

	const char *app_name =
		mc::cvars<const char *>::get()->find("app/name")->get();

	mc::version app_version =
		mc::cvars<mc::version>::get()->find("app/version")->get();

	const char *engine_name =
		mc::cvars<const char *>::get()->find("engine/name")->get();

	mc::version engine_version =
		mc::cvars<mc::version>::get()->find("engine/version")->get();

	std::vector<const char *> layers;
	std::vector<const char *> extensions;

	if (vulkan_validation.get()) {
		layers.emplace_back("VK_LAYER_KHRONOS_validation");
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	uint32_t glfw_extension_count = 0;
	const char **glfw_extensions =
		glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	extensions.insert(extensions.end(), glfw_extensions,
			  glfw_extensions + glfw_extension_count);

	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = app_name,
		.applicationVersion = VK_MAKE_VERSION(app_version.major,
						      app_version.minor,
						      app_version.patch),
		.pEngineName = engine_name,
		.engineVersion = VK_MAKE_VERSION(engine_version.major,
						 engine_version.minor,
						 engine_version.patch),
		.apiVersion = VK_API_VERSION_1_0,
	};

	VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = static_cast<uint32_t>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount =
			static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};

	VkDebugUtilsMessengerCreateInfoEXT debug_info;
	if (vulkan_validation.get()) {
		populate_messenger_create_info(&debug_info);
		create_info.pNext = &debug_info;
	} else {
		create_info.pNext = NULL;
	}

	LOG_ASSERT(Render,
		   vkCreateInstance(&create_info, NULL, &m_instance) ==
			   VK_SUCCESS,
		   "Failed to create Vulkan instance");

	gladLoaderLoadVulkan(m_instance, NULL, NULL);
}

void vk_context::create_messenger()
{
	VkDebugUtilsMessengerCreateInfoEXT create_info;
	populate_messenger_create_info(&create_info);

	LOG_ASSERT(Render,
		   vkCreateDebugUtilsMessengerEXT(m_instance, &create_info,
						  NULL,
						  &m_messenger) == VK_SUCCESS,
		   "Failed to create Vulkan debug messenger");
}

void vk_context::create_surface()
{
	LOG_ASSERT(Render,
		   glfwCreateWindowSurface(m_instance, m_window->get_handle(),
					   nullptr, &m_surface) == VK_SUCCESS,
		   "Failed to create Vulkan surface using GLFW");
}

void vk_context::choose_gpu()
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(m_instance, &count, NULL);

	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(m_instance, &count, devices.data());

	VkPhysicalDevice discrete_gpu = VK_NULL_HANDLE;
	VkPhysicalDevice integrated_gpu = VK_NULL_HANDLE;
	VkPhysicalDevice cpu = VK_NULL_HANDLE;

	for (VkPhysicalDevice device : devices) {
		uint32_t queue_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count,
							 NULL);

		std::vector<VkQueueFamilyProperties> families(queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count,
							 families.data());

		uint32_t graphics_idx = queue_count;
		for (uint32_t i = 0; i < queue_count; i++) {
			VkBool32 present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(
				device, i, m_surface, &present);

			if (graphics_idx == queue_count &&
			    (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			    present == VK_TRUE) {
				graphics_idx = i;
			}
		}

		if (graphics_idx == queue_count) {
			continue;
		}

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		switch (properties.deviceType) {
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			if (discrete_gpu == VK_NULL_HANDLE) {
				discrete_gpu = device;
			}
			break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			if (integrated_gpu == VK_NULL_HANDLE) {
				integrated_gpu = device;
			}
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			if (cpu == VK_NULL_HANDLE) {
				cpu = device;
			}
			break;
		default:
			break;
		}
	}

	if (discrete_gpu != VK_NULL_HANDLE) {
		m_gpu = discrete_gpu;
	} else if (integrated_gpu != VK_NULL_HANDLE) {
		m_gpu = integrated_gpu;
	} else {
		m_gpu = cpu;
	}

	LOG_ASSERT(Render, m_gpu != VK_NULL_HANDLE,
		   "Failed to find a Vulkan physical device");

	LOG_ASSERT(Render, gladLoaderLoadVulkan(m_instance, m_gpu, NULL),
		   "Failed to reload Vulkan with instance");
}

void vk_context::create_device()
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &count, NULL);

	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &count,
						 families.data());

	m_graphics_idx = count;
	for (uint32_t i = 0; i < count; i++) {
		VkBool32 present = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_gpu, i, m_surface,
						     &present);

		if (m_graphics_idx == count &&
		    (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
		    present == VK_TRUE) {
			m_graphics_idx = i;
		}
	}

	LOG_ASSERT(Render, m_graphics_idx != count,
		   "Failed to find a Vulkan graphics queue");

	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

	float priority = 1.0f;
	queue_create_infos.emplace_back(VkDeviceQueueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = m_graphics_idx,
		.queueCount = 1,
		.pQueuePriorities = &priority });

	VkPhysicalDeviceFeatures features = {};

	std::vector<const char *> layers;
	std::vector<const char *> extensions;

	if (vulkan_validation.get()) {
		layers.emplace_back("VK_LAYER_KHRONOS_validation");
	}

	extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount =
			static_cast<uint32_t>(queue_create_infos.size()),
		.pQueueCreateInfos = queue_create_infos.data(),
		.enabledLayerCount = static_cast<uint32_t>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount =
			static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
		.pEnabledFeatures = &features,
	};

	LOG_ASSERT(Render,
		   vkCreateDevice(m_gpu, &create_info, NULL, &m_device) ==
			   VK_SUCCESS,
		   "Failed to create Vulkan device");

	LOG_ASSERT(Render, gladLoaderLoadVulkan(m_instance, m_gpu, m_device),
		   "Failed to reload Vulkan with instance and device");

	vkGetDeviceQueue(m_device, m_graphics_idx, 0, &m_graphics);
}

void vk_context::create_swapchain()
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface,
						  &capabilities);

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &format_count,
					     NULL);

	std::vector<VkSurfaceFormatKHR> formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &format_count,
					     formats.data());

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface,
						  &present_mode_count, NULL);

	std::vector<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		m_gpu, m_surface, &present_mode_count, present_modes.data());

	VkSurfaceFormatKHR format = formats[0];
	for (uint32_t i = 0; i < format_count; i++) {
		if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
		    formats[i].colorSpace ==
			    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			format = formats[i];
		}
	}

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < present_mode_count; i++) {
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = present_modes[i];
		}
	}

	VkExtent2D extent;
	if (capabilities.currentExtent.width !=
	    std::numeric_limits<uint32_t>::max()) {
		extent = capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(m_window->get_handle(), &width, &height);

		extent.width = std::clamp(static_cast<uint32_t>(width),
					  capabilities.minImageExtent.width,
					  capabilities.maxImageExtent.width);
		extent.height = std::clamp(static_cast<uint32_t>(height),
					   capabilities.minImageExtent.height,
					   capabilities.maxImageExtent.height);
	}

	uint32_t image_count = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 &&
	    image_count > capabilities.maxImageCount) {
		image_count = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_surface,
		.minImageCount = image_count,
		.imageFormat = format.format,
		.imageColorSpace = format.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.pQueueFamilyIndices = NULL,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	LOG_ASSERT(Render,
		   vkCreateSwapchainKHR(m_device, &create_info, NULL,
					&m_swapchain) == VK_SUCCESS,
		   "Failed to create Vulkan swapchain");

	m_format = format.format;
	m_extent = extent;
}

void vk_context::cleanup_swapchain()
{
	for (uint32_t i = 0; i < m_swapchain_frames.size(); i++) {
		if (m_swapchain_frames[i].framebuffer) {
			vkDestroyFramebuffer(m_device,
					     m_swapchain_frames[i].framebuffer,
					     NULL);
		}

		if (m_swapchain_frames[i].view) {
			vkDestroyImageView(m_device, m_swapchain_frames[i].view,
					   NULL);
		}
	}

	if (m_swapchain) {
		vkDestroySwapchainKHR(m_device, m_swapchain, NULL);
	}
}

void vk_context::recreate_swapchain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window->get_handle(), &width, &height);
	m_minimized = (width == 0 || height == 0);

	vkDeviceWaitIdle(m_device);

	cleanup_swapchain();

	if (!m_minimized) {
		create_swapchain();
		create_swapchain_frames();
	}
}

void vk_context::create_render_pass()
{
	VkAttachmentDescription color_attachment = {
		.format = m_format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference color_attachment_ref = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
	};

	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	VkRenderPassCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &color_attachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	LOG_ASSERT(Render,
		   vkCreateRenderPass(m_device, &create_info, NULL,
				      &m_render_pass) == VK_SUCCESS,
		   "Failed to create Vulkan render pass");
}

void vk_context::create_swapchain_frames()
{
	uint32_t image_count = 0;
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, NULL);

	std::vector<VkImage> images(image_count);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count,
				images.data());

	m_swapchain_frames.resize(image_count);
	for (uint32_t i = 0; i < image_count; i++) {
		m_swapchain_frames[i].image = images[i];

		VkImageViewCreateInfo view_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_format,
			.components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY },
			.subresourceRange = { .aspectMask =
						      VK_IMAGE_ASPECT_COLOR_BIT,
					      .baseMipLevel = 0,
					      .levelCount = 1,
					      .baseArrayLayer = 0,
					      .layerCount = 1 },
		};

		LOG_ASSERT(Render,
			   vkCreateImageView(m_device, &view_info, NULL,
					     &m_swapchain_frames[i].view) ==
				   VK_SUCCESS,
			   "Failed to create Vulkan image view");

		VkFramebufferCreateInfo framebuffer_info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = m_render_pass,
			.attachmentCount = 1,
			.pAttachments = &m_swapchain_frames[i].view,
			.width = m_extent.width,
			.height = m_extent.height,
			.layers = 1,
		};

		LOG_ASSERT(Render,
			   vkCreateFramebuffer(
				   m_device, &framebuffer_info, NULL,
				   &m_swapchain_frames[i].framebuffer) ==
				   VK_SUCCESS,
			   "Failed to create Vulkan framebuffer");
	}

	m_framebuffer_resized = false;
}

void vk_context::create_command_pool()
{
	VkCommandPoolCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = m_graphics_idx,
	};

	LOG_ASSERT(Render,
		   vkCreateCommandPool(m_device, &create_info, NULL,
				       &m_command_pool) == VK_SUCCESS,
		   "Failed to create Vulkan command pool");
}

void vk_context::create_frames()
{
	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkSemaphoreCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	m_frames.resize(m_frames_in_flight);
	for (uint32_t i = 0; i < m_frames.size(); i++) {
		LOG_ASSERT(Render,
			   vkAllocateCommandBuffers(
				   m_device, &allocate_info,
				   &m_frames[i].command_buffer) == VK_SUCCESS,
			   "Failed to allocate Vulkan command buffer");
		LOG_ASSERT(Render,
			   vkCreateSemaphore(m_device, &semaphore_info, NULL,
					     &m_frames[i].image_available) ==
				   VK_SUCCESS,
			   "Failed to create Vulkan semaphore");
		LOG_ASSERT(Render,
			   vkCreateSemaphore(m_device, &semaphore_info, NULL,
					     &m_frames[i].render_finished) ==
				   VK_SUCCESS,
			   "Failed to create Vulkan semaphore");
		LOG_ASSERT(Render,
			   vkCreateFence(m_device, &fence_info, NULL,
					 &m_frames[i].in_flight) == VK_SUCCESS,
			   "Failed to create Vulkan fence");
	}
}

}
}
