#include "common_graphics.h"
#include "error.h"
#include "utils.h"
#include "vk_utils.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <vulkan/vulkan_win32.h>

bool is_validation_needed;

const char* requested_instance_layers[16];
size_t num_requested_instance_layer;

const char* requested_instance_extensions[16];
size_t num_requested_instance_extension;

const char* requested_device_extensions[16];
size_t num_requested_device_extension;

VkInstance instance;
VkDebugUtilsMessengerEXT debug_utils_messenger;
VkPhysicalDevice physical_device;
VkSurfaceKHR surface;
VkPresentModeKHR chosen_present_mode;
VkSurfaceCapabilitiesKHR surface_capabilites;

VkResult create_debug_utils_messenger (VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* debug_utils_messenger_create_info,
	const VkAllocationCallbacks* allocation_callbacks,
	VkDebugUtilsMessengerEXT* debug_utils_messenger)
{
	OutputDebugString (L"create_debug_utils_messenger\n");

	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");

	if (func)
	{
		return func (instance, debug_utils_messenger_create_info, allocation_callbacks, debug_utils_messenger);
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

	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func)
	{
		func (instance, debug_utils_messenger, allocation_callbacks);
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

	return AGAINST_SUCCESS;
}

AGAINST_RESULT populate_instance_layers_and_extensions ()
{
	OutputDebugString (L"populate_instance_layers_and_extensions\n");

	if (is_validation_needed)
	{
		size_t num_layers = 0;
		vkEnumerateInstanceLayerProperties (&num_layers, NULL);
		VkLayerProperties* layer_properties = (VkLayerProperties*)utils_malloc (sizeof (VkLayerProperties) * num_layers);
		vkEnumerateInstanceLayerProperties (&num_layers, layer_properties);

		for (size_t l = 0; l < num_layers; l++)
		{
			if (strcmp (layer_properties[l].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
			{
				requested_instance_layers[num_requested_instance_layer++] = ("VK_LAYER_LUNARG_standard_validation");
				break;
			}
		}

		utils_free (layer_properties);
	}

	size_t num_extensions = 0;
	vkEnumerateInstanceExtensionProperties (NULL, &num_extensions, NULL);

	VkExtensionProperties* extension_properties = (VkExtensionProperties*)utils_malloc (sizeof (VkExtensionProperties) * num_extensions);
	vkEnumerateInstanceExtensionProperties (NULL, &num_extensions, extension_properties);

	for (size_t e = 0; e < num_extensions; e++)
	{
		if (strcmp (extension_properties[e].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			requested_instance_extensions[num_requested_instance_extension++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}
		else if (strcmp (extension_properties[e].extensionName, "VK_KHR_win32_surface") == 0)
		{
			requested_instance_extensions[num_requested_instance_extension++] = "VK_KHR_win32_surface";
		}

		if (is_validation_needed)
		{
			if (strcmp (extension_properties[e].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			{
				requested_instance_extensions[num_requested_instance_extension++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			}
		}
	}

	utils_free (extension_properties);

	return AGAINST_SUCCESS;
}

AGAINST_RESULT create_instance ()
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
	create_info.enabledExtensionCount = num_requested_instance_extension;
	create_info.ppEnabledExtensionNames = requested_instance_extensions;
	create_info.enabledLayerCount = num_requested_instance_layer;
	create_info.ppEnabledLayerNames = requested_instance_layers;
	create_info.pApplicationInfo = &application_info;
	create_info.flags = 0;

	if (vkCreateInstance (&create_info, NULL, &instance) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_INSTANCE;
	}

	return AGAINST_SUCCESS;
}

AGAINST_RESULT setup_debug_utils_messenger ()
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

	return AGAINST_SUCCESS;
}

AGAINST_RESULT get_physical_device ()
{
	OutputDebugString (L"get_physical_device\n");

	size_t num_physical_devices = 0;
	vkEnumeratePhysicalDevices (instance, &num_physical_devices, NULL);

	VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)utils_malloc (sizeof (VkPhysicalDevice) * num_physical_devices);
	vkEnumeratePhysicalDevices (instance, &num_physical_devices, physical_devices);

	if (num_physical_devices == 0)
	{
		return AGAINST_ERROR_GRAPHICS_GET_PHYSICAL_DEVICE;
	}

	physical_device = physical_devices[0];

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures (physical_device, &device_features);

	size_t num_queue_families = 0;
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &num_queue_families, NULL);
	VkQueueFamilyProperties* queue_family_properties = (VkQueueFamilyProperties*)utils_malloc (sizeof (VkQueueFamilyProperties) * num_queue_families);
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &num_queue_families, queue_family_properties);

	for (size_t i = 0; i < num_queue_families; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics_queue_family_index = i;
			break;
		}
	}

	for (size_t i = 0; i < num_queue_families; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && (i != graphics_queue_family_index))
		{
			compute_queue_family_index = i;
			break;
		}
	}

	if (compute_queue_family_index == -1)
	{
		for (size_t i = 0; i < num_queue_families; ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				compute_queue_family_index = i;
				break;
			}
		}	
	}

	for (size_t i = 0; i < num_queue_families; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && (i != graphics_queue_family_index) && (i != compute_queue_family_index))
		{
			transfer_queue_family_index = i;
			break;
		}
	}

	if (transfer_queue_family_index == -1)
	{
		for (size_t i = 0; i < num_queue_families; ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				transfer_queue_family_index = i;
				break;
			}
		}	
	}
	
	vkGetPhysicalDeviceMemoryProperties (physical_device, &physical_device_memory_properties);

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties (physical_device, &device_properties);
	physical_device_limits = device_properties.limits;

	utils_free (physical_devices);
	utils_free (queue_family_properties);

	return AGAINST_SUCCESS;
}

AGAINST_RESULT create_surface (HINSTANCE HInstance, HWND HWnd)
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

	return AGAINST_SUCCESS;
}

AGAINST_RESULT populate_graphics_device_extensions ()
{
	OutputDebugString (L"populate_graphics_device_extensions\n");

	size_t num_extensions = 0;
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &num_extensions, NULL);

	VkExtensionProperties* extension_properties = (VkExtensionProperties*)utils_malloc (sizeof (VkExtensionProperties) * num_extensions);
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &num_extensions, extension_properties);

	for (size_t e = 0; e < num_extensions; e++)
	{
		if (strcmp (extension_properties[e].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			requested_device_extensions[num_requested_device_extension++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			break;
		}
	}

	utils_free (extension_properties);

	return AGAINST_SUCCESS;
}

AGAINST_RESULT get_surface_properties ()
{
	OutputDebugString (L"get_surface_properties\n");

	VkBool32 is_surface_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, graphics_queue_family_index, surface, &is_surface_supported);

	if (!is_surface_supported)
	{
		return AGAINST_ERROR_GRAPHICS_SURFACE_SUPPORT;
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, surface, &surface_capabilites);

	size_t num_surface_formats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &num_surface_formats, NULL);

	VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)utils_malloc (sizeof (VkSurfaceFormatKHR) * num_surface_formats);
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &num_surface_formats, surface_formats);

	for (size_t s = 0; s < num_surface_formats; s++)
	{
		if (surface_formats[s].format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			chosen_surface_format = surface_formats[s];
			break;
		}
	}

	size_t num_present_modes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &num_present_modes, NULL);

	VkPresentModeKHR* present_modes = (VkPresentModeKHR*)utils_malloc (sizeof (VkPresentModeKHR) * num_present_modes);
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &num_present_modes, present_modes);

	for (size_t p = 0; p < num_present_modes; p++)
	{
		if (present_modes[p] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			chosen_present_mode = present_modes[p];
			break;
		}
	}

	surface_extent = surface_capabilites.currentExtent;

	utils_free (surface_formats);
	utils_free (present_modes);

	return AGAINST_SUCCESS;
}

AGAINST_RESULT create_graphics_device ()
{
	OutputDebugString (L"create_graphics_device\n");

	float priorities = 1.f;

	VkDeviceQueueCreateInfo queue_create_infos[3] = { 0,0,0 };
	size_t unique_queue_family_indices[3] = { 0,0,0 };
	size_t num_unique_queues[3] = { 1,1,1 };
	size_t num_unique_queue_family_indices = 0;

	if (graphics_queue_family_index == compute_queue_family_index)
	{
		unique_queue_family_indices[0] = graphics_queue_family_index;
		++num_unique_queue_family_indices;
		++num_unique_queues[0];
	}
	else
	{
		unique_queue_family_indices[0] = graphics_queue_family_index;
		unique_queue_family_indices[1] = compute_queue_family_index;
		num_unique_queue_family_indices += 2;
	}

	if (graphics_queue_family_index != transfer_queue_family_index)
	{
		unique_queue_family_indices[num_unique_queue_family_indices] = transfer_queue_family_index;
		++num_unique_queue_family_indices;
	}

	for (size_t ui = 0; ui < num_unique_queue_family_indices; ++ui)
	{
		queue_create_infos[ui].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[ui].pNext = NULL;
		queue_create_infos[ui].pQueuePriorities = &priorities;
		queue_create_infos[ui].queueCount = num_unique_queues[ui];
		queue_create_infos[ui].queueFamilyIndex = unique_queue_family_indices[ui];
		queue_create_infos[ui].flags = 0;
	}

	VkDeviceCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.enabledExtensionCount = num_requested_device_extension;
	create_info.ppEnabledExtensionNames = requested_device_extensions;
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = NULL;
	create_info.queueCreateInfoCount = num_unique_queue_family_indices;
	create_info.pQueueCreateInfos = queue_create_infos;
	create_info.flags = 0;

	if (vkCreateDevice (physical_device, &create_info, NULL, &graphics_device) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE;
	}

	return AGAINST_SUCCESS;
}

AGAINST_RESULT create_swapchain ()
{
	OutputDebugString (L"create_swapchain\n");

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
		return AGAINST_ERROR_GRAPHICS_CREATE_SWAPCHAIN;
	}

	vkGetSwapchainImagesKHR (graphics_device, swapchain, &num_swapchain_image, NULL);
	swapchain_images = (VkImage*)utils_calloc (num_swapchain_image, sizeof (VkImage));

	vkGetSwapchainImagesKHR (graphics_device, swapchain, &num_swapchain_image, swapchain_images);

	swapchain_imageviews = (VkImageView*)utils_calloc (num_swapchain_image, sizeof (VkImageView));

	return AGAINST_SUCCESS;
}

AGAINST_RESULT create_swapchain_imageviews ()
{
	OutputDebugString (L"create_swapchain_imageviews\n");

	VkComponentMapping components = { 0 };
	VkImageSubresourceRange subresource_range = { 0 };
	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range.levelCount = 1;
	subresource_range.layerCount = 1;

	VkImageViewCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.format = chosen_surface_format.format;
	create_info.components = components;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.subresourceRange = subresource_range;

	for (size_t i = 0; i < num_swapchain_image; i++)
	{
		create_info.image = swapchain_images[i];
		if (vkCreateImageView (graphics_device, &create_info, NULL, swapchain_imageviews + i) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		}
	}

	return AGAINST_SUCCESS;
}

AGAINST_RESULT get_device_queues ()
{
	OutputDebugString (L"get_device_queues\n");

	size_t graphics_queue_index = 0;
	size_t compute_queue_index = graphics_queue_family_index == compute_queue_family_index ? 1 : 0;
	size_t transfer_queue_index = transfer_queue_family_index == compute_queue_family_index ? compute_queue_index + 1 : 0;

	vkGetDeviceQueue (graphics_device, graphics_queue_family_index, graphics_queue_index, &graphics_queue);
	vkGetDeviceQueue (graphics_device, compute_queue_family_index, compute_queue_index, &compute_queue);
	vkGetDeviceQueue (graphics_device, transfer_queue_family_index, transfer_queue_index, &transfer_queue);

	return AGAINST_SUCCESS;
}

AGAINST_RESULT common_graphics_init (HINSTANCE HInstance, HWND HWnd)
{
	OutputDebugString (L"graphics_init\n");

#ifdef _DEBUG
	is_validation_needed = true;
#else
	is_validation_needed = false;
#endif

	AGAINST_RESULT result = AGAINST_SUCCESS;

	CHECK_AGAINST_RESULT (populate_instance_layers_and_extensions (), result);
	CHECK_AGAINST_RESULT (create_instance (), result);

	if (is_validation_needed)
	{
		CHECK_AGAINST_RESULT (setup_debug_utils_messenger (), result);
	}

	CHECK_AGAINST_RESULT (get_physical_device (), result);
	CHECK_AGAINST_RESULT (create_surface (HInstance, HWnd), result);
	CHECK_AGAINST_RESULT (populate_graphics_device_extensions (), result);
	CHECK_AGAINST_RESULT (get_surface_properties (), result);
	CHECK_AGAINST_RESULT (create_graphics_device (), result);
	CHECK_AGAINST_RESULT (create_swapchain (), result);
	CHECK_AGAINST_RESULT (create_swapchain_imageviews (), result);
	CHECK_AGAINST_RESULT (get_device_queues (), result);
	CHECK_AGAINST_RESULT (vk_utils_create_command_pools (graphics_device, transfer_queue_family_index, 1, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &transfer_command_pool), result);

	return AGAINST_SUCCESS;
}

void common_graphics_shutdown ()
{
	OutputDebugString (L"graphics_shutdown\n");

	vk_utils_destroy_command_pools_and_buffers (graphics_device, &transfer_command_pool, 1);

	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR (graphics_device, swapchain, NULL);
	}

	if (swapchain_imageviews)
	{
		for (size_t i = 0; i < num_swapchain_image; i++)
		{
			if (swapchain_imageviews[i] != VK_NULL_HANDLE)
			{
				vkDestroyImageView (graphics_device, swapchain_imageviews[i], NULL);
			}
		}

		utils_free (swapchain_imageviews);
	}

	utils_free (swapchain_images);

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

	OutputDebugString (L"common_graphics_shutdown\n");
}