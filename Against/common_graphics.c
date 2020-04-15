#include "common_graphics.h"
#include "error.h"
#include "utils.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <vulkan/vulkan_win32.h>

bool is_validation_needed;

const char* requested_instance_layers[16];
uint32_t requested_instance_layer_count;

const char* requested_instance_extensions[16];
uint32_t requested_instance_extension_count;

const char* requested_device_extensions[16];
uint32_t requested_device_extension_count;

VkInstance instance;
VkDebugUtilsMessengerEXT debug_utils_messenger;
VkPhysicalDevice physical_device;
VkSurfaceKHR surface;
VkPresentModeKHR chosen_present_mode;

VkResult create_debug_utils_messenger (VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* debug_utils_messenger_create_info,
	const VkAllocationCallbacks* allocation_callbacks,
	VkDebugUtilsMessengerEXT* debug_utils_messenger)
{
	OutputDebugString (L"create_debug_utils_messenger\n");

	PFN_vkCreateDebugUtilsMessengerEXT Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");

	if (Func)
	{
		return Func (instance, debug_utils_messenger_create_info, allocation_callbacks, debug_utils_messenger);
	}
	else
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
}

void destroy_debug_utils_messenger (VkInstance instance,
	VkDebugUtilsMessengerEXT debug_utils_messenger,
	const VkAllocationCallbacks* allocation_callbacks)
{
	OutputDebugString (L"destroy_debug_utils_messenger\n");

	PFN_vkDestroyDebugUtilsMessengerEXT Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");

	if (Func)
	{
		Func (instance, debug_utils_messenger, allocation_callbacks);
	}
	else
	{
		OutputDebugString (L"Could not Destroy Debug Utils Messenger\n");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback (
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* pUserData)
{
	if (callback_data)
	{
		wchar_t buff[2048];
		swprintf_s (buff, 2048, L"Debug Messenger Callback: %hs\n", callback_data->pMessage);
		OutputDebugString (buff);
	}

	return 0;
}

int populate_instance_layers_and_extensions ()
{
	OutputDebugString (L"populate_instance_layers_and_extensions\n");

	if (is_validation_needed)
	{
		uint32_t layer_count = 0;
		vkEnumerateInstanceLayerProperties (&layer_count, NULL);
		VkLayerProperties* layer_properties = (VkLayerProperties*)my_malloc (sizeof (VkLayerProperties) * layer_count);
		vkEnumerateInstanceLayerProperties (&layer_count, layer_properties);

		for (uint32_t l = 0; l < layer_count; l++)
		{
			if (strcmp (layer_properties[l].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
			{
				requested_instance_layers[requested_instance_layer_count++] = ("VK_LAYER_LUNARG_standard_validation");
				break;
			}
		}

		my_free (layer_properties);
	}

	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties (NULL, &extension_count, NULL);

	VkExtensionProperties* extension_properties = (VkExtensionProperties*)my_malloc (sizeof (VkExtensionProperties) * extension_count);
	vkEnumerateInstanceExtensionProperties (NULL, &extension_count, extension_properties);

	for (uint32_t e = 0; e < extension_count; e++)
	{
		if (strcmp (extension_properties[e].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			requested_instance_extensions[requested_instance_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}
		else if (strcmp (extension_properties[e].extensionName, "VK_KHR_win32_surface") == 0)
		{
			requested_instance_extensions[requested_instance_extension_count++] = "VK_KHR_win32_surface";
		}

		if (is_validation_needed)
		{
			if (strcmp (extension_properties[e].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			{
				requested_instance_extensions[requested_instance_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			}
		}
	}

	my_free (extension_properties);

	return 0;
}

int create_instance ()
{
	OutputDebugString (L"create_instance\n");

	VkApplicationInfo application_info = { 0 };

	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pEngineName = "AGE";
	application_info.apiVersion = VK_API_VERSION_1_1;
	application_info.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
	application_info.engineVersion = VK_MAKE_VERSION (1, 0, 0);
	application_info.pApplicationName = "Against";

	VkInstanceCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.enabledExtensionCount = requested_instance_extension_count;
	create_info.ppEnabledExtensionNames = requested_instance_extensions;
	create_info.enabledLayerCount = requested_instance_layer_count;
	create_info.ppEnabledLayerNames = requested_instance_layers;
	create_info.pApplicationInfo = &application_info;
	create_info.flags = 0;

	if (vkCreateInstance (&create_info, NULL, &instance) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_INSTANCE;
	}

	return 0;
}

int setup_debug_utils_messenger ()
{
	OutputDebugString (L"setup_debug_utils_messenger\n");

	VkDebugUtilsMessengerCreateInfoEXT create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debug_messenger_callback;
	create_info.flags = 0;

	if (create_debug_utils_messenger (instance, &create_info, NULL, &debug_utils_messenger) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER;
	}

	return 0;
}

int get_physical_device ()
{
	OutputDebugString (L"get_physical_device\n");

	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices (instance, &physical_device_count, NULL);

	VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)my_malloc (sizeof (VkPhysicalDevice) * physical_device_count);
	vkEnumeratePhysicalDevices (instance, &physical_device_count, physical_devices);

	if (physical_device_count == 0)
	{
		return AGAINST_ERROR_GRAPHICS_GET_PHYSICAL_DEVICE;
	}

	physical_device = physical_devices[0];

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures (physical_device, &device_features);

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_count, NULL);
	VkQueueFamilyProperties* queue_family_properties = (VkQueueFamilyProperties*)my_malloc (sizeof (VkQueueFamilyProperties) * queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_count, queue_family_properties);

	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_family_properties[i].queueCount > 1))
		{
			graphics_queue_family_index = i;
			break;
		}
	}

	vkGetPhysicalDeviceMemoryProperties (physical_device, &physical_device_memory_properties);

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties (physical_device, &device_properties);
	physical_device_limits = device_properties.limits;

	my_free (physical_devices);
	my_free (queue_family_properties);

	return 0;
}

int create_surface (HINSTANCE HInstance, HWND HWnd)
{
	OutputDebugString (L"create_surface\n");

	VkWin32SurfaceCreateInfoKHR create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hinstance = HInstance;
	create_info.hwnd = HWnd;

	if (vkCreateWin32SurfaceKHR (instance, &create_info, NULL, &surface) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SURFACE;
	}

	return 0;
}

int populate_graphics_device_extensions ()
{
	OutputDebugString (L"populate_graphics_device_extensions\n");

	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &extension_count, NULL);

	VkExtensionProperties* extension_properties = (VkExtensionProperties*)my_malloc (sizeof (VkExtensionProperties) * extension_count);
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &extension_count, extension_properties);

	for (uint32_t e = 0; e < extension_count; e++)
	{
		if (strcmp (extension_properties[e].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			requested_device_extensions[requested_device_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			break;
		}
	}

	my_free (extension_properties);

	return 0;
}

int create_graphics_device ()
{
	OutputDebugString (L"create_graphics_device\n");

	float priorities = 1.f;

	VkDeviceQueueCreateInfo queue_create_info = { 0 };

	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.pNext = NULL;
	queue_create_info.pQueuePriorities = &priorities;
	queue_create_info.queueCount = 1;
	queue_create_info.queueFamilyIndex = graphics_queue_family_index;
	queue_create_info.flags = 0;

	VkDeviceCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.enabledExtensionCount = requested_device_extension_count;
	create_info.ppEnabledExtensionNames = requested_device_extensions;
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = NULL;
	create_info.queueCreateInfoCount = 1;
	create_info.pQueueCreateInfos = &queue_create_info;
	create_info.flags = 0;

	if (vkCreateDevice (physical_device, &create_info, NULL, &graphics_device) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE;
	}

	vkGetDeviceQueue (graphics_device, graphics_queue_family_index, 0, &graphics_queue);

	return 0;
}

int create_swapchain ()
{
	OutputDebugString (L"create_swapchain\n");

	VkBool32 is_surface_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, graphics_queue_family_index, surface, &is_surface_supported);

	if (!is_surface_supported)
	{
		return AGAINST_ERROR_GRAPHICS_SURFACE_SUPPORT;
	}

	VkSurfaceCapabilitiesKHR surface_capabilites;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, surface, &surface_capabilites);

	uint32_t surface_format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &surface_format_count, NULL);

	VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)my_malloc (sizeof (VkSurfaceFormatKHR) * surface_format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &surface_format_count, surface_formats);

	for (uint32_t s = 0; s < surface_format_count; s++)
	{
		if (surface_formats[s].format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			chosen_surface_format = surface_formats[s];
			break;
		}
	}

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &present_mode_count, NULL);

	VkPresentModeKHR* present_modes = (VkPresentModeKHR*)my_malloc (sizeof (VkPresentModeKHR) * present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &present_mode_count, present_modes);

	for (uint32_t p = 0; p < present_mode_count; p++)
	{
		if (present_modes[p] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			chosen_present_mode = present_modes[p];
			break;
		}
	}

	surface_extent = surface_capabilites.currentExtent;

	VkSwapchainCreateInfoKHR create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface;

	create_info.clipped = 1;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.imageArrayLayers = 1;
	create_info.imageColorSpace = chosen_surface_format.colorSpace;
	create_info.imageExtent = surface_capabilites.currentExtent;
	create_info.imageFormat = chosen_surface_format.format;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.imageUsage = surface_capabilites.supportedUsageFlags;
	create_info.minImageCount = surface_capabilites.minImageCount + 1;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	create_info.presentMode = chosen_present_mode;
	create_info.preTransform = surface_capabilites.currentTransform;

	if (vkCreateSwapchainKHR (graphics_device, &create_info, NULL, &swapchain) != VK_SUCCESS)
	{
		my_free (surface_formats);
		my_free (present_modes);

		return AGAINST_ERROR_GRAPHICS_CREATE_SWAPCHAIN;
	}

	vkGetSwapchainImagesKHR (graphics_device, swapchain, &swapchain_image_count, NULL);
	swapchain_images = (VkImage*)my_malloc (sizeof (VkImage) * swapchain_image_count);

	vkGetSwapchainImagesKHR (graphics_device, swapchain, &swapchain_image_count, swapchain_images);

	swapchain_imageviews = (VkImageView*)my_malloc (sizeof (VkImageView) * swapchain_image_count);
	
	my_free (surface_formats);
	my_free (present_modes);

	return 0;
}

int create_swapchain_imageviews ()
{
	OutputDebugString (L"create_swapchain_imageviews\n");

	VkComponentMapping components;

	components.a = VK_COMPONENT_SWIZZLE_A;
	components.b = VK_COMPONENT_SWIZZLE_B;
	components.g = VK_COMPONENT_SWIZZLE_G;
	components.r = VK_COMPONENT_SWIZZLE_R;

	VkImageSubresourceRange subresource_range;

	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range.baseMipLevel = 0;
	subresource_range.levelCount = 1;

	subresource_range.baseArrayLayer = 0;
	subresource_range.layerCount = 1;

	VkImageViewCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.format = chosen_surface_format.format;
	create_info.components = components;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.subresourceRange = subresource_range;

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		create_info.image = swapchain_images[i];
		if (vkCreateImageView (graphics_device, &create_info, NULL, &swapchain_imageviews[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		}
	}

	return 0;
}

int create_command_pool ()
{
	OutputDebugString (L"create_command_pool\n");

	VkCommandPoolCreateInfo command_pool_create_info = { 0 };
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.queueFamilyIndex = graphics_queue_family_index;
	//command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool (graphics_device, &command_pool_create_info, NULL, &command_pool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	return 0;
}

int common_graphics_init (HINSTANCE HInstance, HWND HWnd)
{
	OutputDebugString (L"graphics_init\n");

#ifdef _DEBUG
	is_validation_needed = true;
#else
	is_validation_needed = false;
#endif

	AGAINSTRESULT result;

	CHECK_AGAINST_RESULT (populate_instance_layers_and_extensions (), result);
	CHECK_AGAINST_RESULT (create_instance (), result);

	if (is_validation_needed)
	{
		CHECK_AGAINST_RESULT (setup_debug_utils_messenger (), result);
	}

	CHECK_AGAINST_RESULT (get_physical_device (), result);
	CHECK_AGAINST_RESULT (create_surface (HInstance, HWnd), result);
	CHECK_AGAINST_RESULT (populate_graphics_device_extensions (), result);
	CHECK_AGAINST_RESULT (create_graphics_device (), result);
	CHECK_AGAINST_RESULT (create_swapchain (), result);
	CHECK_AGAINST_RESULT (create_swapchain_imageviews (), result);
	CHECK_AGAINST_RESULT (create_command_pool (), result);

	return 0;
}

void common_graphics_exit ()
{
	OutputDebugString (L"graphics_exit\n");

	if (command_pool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (graphics_device, command_pool, NULL);
	}

	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR (graphics_device, swapchain, NULL);
	}

	if (swapchain_imageviews)
	{
		for (uint32_t i = 0; i < swapchain_image_count; i++)
		{
			if (swapchain_imageviews[i] != VK_NULL_HANDLE)
			{
				vkDestroyImageView (graphics_device, swapchain_imageviews[i], NULL);
			}
		}

		my_free (swapchain_imageviews);
	}

	if (swapchain_images)
	{
		my_free (swapchain_images);
	}

	if (graphics_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice (graphics_device, NULL);
	}

	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR (instance, surface, NULL);
	}

	if (is_validation_needed)
	{
		if (debug_utils_messenger != VK_NULL_HANDLE)
		{
			destroy_debug_utils_messenger (instance, debug_utils_messenger, NULL);
		}
	}

	if (instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance (instance, NULL);
	}

	OutputDebugString (L"Finished graphics_exit\n");
}