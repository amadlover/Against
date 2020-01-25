#include "graphics.h"
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
	const VkDebugUtilsMessengerCreateInfoEXT* DebugUtilsMessengerCreateInfo,
	const VkAllocationCallbacks* AllocationCallbacks,
	VkDebugUtilsMessengerEXT* debug_utils_messenger)
{
	OutputDebugString (L"create_debug_utils_messenger\n");

	PFN_vkCreateDebugUtilsMessengerEXT Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");

	if (Func)
	{
		return Func (instance, DebugUtilsMessengerCreateInfo, AllocationCallbacks, debug_utils_messenger);
	}
	else
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
}

void destroy_debug_utils_messenger (VkInstance instance,
	VkDebugUtilsMessengerEXT debug_utils_messenger,
	const VkAllocationCallbacks* AllocationCallbacks)
{
	OutputDebugString (L"destroy_debug_utils_messenger\n");

	PFN_vkDestroyDebugUtilsMessengerEXT Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");

	if (Func)
	{
		Func (instance, debug_utils_messenger, AllocationCallbacks);
	}
	else
	{
		OutputDebugString (L"Could not Destroy Debug Utils Messenger\n");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback (
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (pCallbackData)
	{
		wchar_t buff[2048];
		swprintf_s (buff, 2048, L"Debug Messenger Callback: %hs\n", pCallbackData->pMessage);
		OutputDebugString (buff);
	}

	return 0;
}

int populate_instance_layers_and_extensions ()
{
	OutputDebugString (L"populate_instance_layers_and_extensions\n");

	if (is_validation_needed)
	{
		uint32_t LayerCount = 0;
		vkEnumerateInstanceLayerProperties (&LayerCount, NULL);
		VkLayerProperties* LayerProperties = (VkLayerProperties*)my_malloc (sizeof (VkLayerProperties) * LayerCount);
		vkEnumerateInstanceLayerProperties (&LayerCount, LayerProperties);

		for (uint32_t l = 0; l < LayerCount; l++)
		{
			if (strcmp (LayerProperties[l].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
			{
				requested_instance_layers[requested_instance_layer_count++] = ("VK_LAYER_LUNARG_standard_validation");
				break;
			}
		}

		my_free (LayerProperties);
	}

	uint32_t ExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties (NULL, &ExtensionCount, NULL);

	VkExtensionProperties* ExtensionProperties = (VkExtensionProperties*)my_malloc (sizeof (VkExtensionProperties) * ExtensionCount);
	vkEnumerateInstanceExtensionProperties (NULL, &ExtensionCount, ExtensionProperties);

	for (uint32_t e = 0; e < ExtensionCount; e++)
	{
		if (strcmp (ExtensionProperties[e].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			requested_instance_extensions[requested_instance_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}
		else if (strcmp (ExtensionProperties[e].extensionName, "VK_KHR_win32_surface") == 0)
		{
			requested_instance_extensions[requested_instance_extension_count++] = "VK_KHR_win32_surface";
		}

		if (is_validation_needed)
		{
			if (strcmp (ExtensionProperties[e].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			{
				requested_instance_extensions[requested_instance_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			}
		}
	}

	my_free (ExtensionProperties);

	return 0;
}

int create_instance ()
{
	OutputDebugString (L"Create instance\n");

	VkApplicationInfo ApplicationInfo;
	memset (&ApplicationInfo, 0, sizeof (VkApplicationInfo));

	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pEngineName = "AGE";
	ApplicationInfo.apiVersion = VK_API_VERSION_1_1;
	ApplicationInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
	ApplicationInfo.engineVersion = VK_MAKE_VERSION (1, 0, 0);
	ApplicationInfo.pApplicationName = "Against";

	VkInstanceCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkInstanceCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.enabledExtensionCount = requested_instance_extension_count;
	CreateInfo.ppEnabledExtensionNames = requested_instance_extensions;
	CreateInfo.enabledLayerCount = requested_instance_layer_count;
	CreateInfo.ppEnabledLayerNames = requested_instance_layers;
	CreateInfo.pApplicationInfo = &ApplicationInfo;
	CreateInfo.flags = 0;

	if (vkCreateInstance (&CreateInfo, NULL, &instance) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_INSTANCE;
	}

	return 0;
}

int setup_debug_utils_messenger ()
{
	OutputDebugString (L"setup_debug_utils_messenger\n");

	VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkDebugUtilsMessengerCreateInfoEXT));

	CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	CreateInfo.pfnUserCallback = debug_messenger_callback;
	CreateInfo.flags = 0;

	if (create_debug_utils_messenger (instance, &CreateInfo, NULL, &debug_utils_messenger) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER;
	}

	return 0;
}

int get_physical_device ()
{
	OutputDebugString (L"get_physical_device\n");

	uint32_t PhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices (instance, &PhysicalDeviceCount, NULL);

	VkPhysicalDevice* PhysicalDevices = (VkPhysicalDevice*)my_malloc (sizeof (VkPhysicalDevice) * PhysicalDeviceCount);
	vkEnumeratePhysicalDevices (instance, &PhysicalDeviceCount, PhysicalDevices);

	if (PhysicalDeviceCount == 0)
	{
		return AGAINST_ERROR_GRAPHICS_GET_PHYSICAL_DEVICE;
	}

	physical_device = PhysicalDevices[0];

	VkPhysicalDeviceFeatures DeviceFeatures;
	vkGetPhysicalDeviceFeatures (physical_device, &DeviceFeatures);

	uint32_t QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &QueueFamilyCount, NULL);
	VkQueueFamilyProperties* QueueFamilyProperties = (VkQueueFamilyProperties*)my_malloc (sizeof (VkQueueFamilyProperties) * QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &QueueFamilyCount, QueueFamilyProperties);

	for (uint32_t i = 0; i < QueueFamilyCount; i++)
	{
		if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (QueueFamilyProperties[i].queueCount > 1))
		{
			graphics_queue_family_index = i;
			break;
		}
	}

	vkGetPhysicalDeviceMemoryProperties (physical_device, &physical_device_memory_properties);

	VkPhysicalDeviceProperties DeviceProperties;
	vkGetPhysicalDeviceProperties (physical_device, &DeviceProperties);
	physical_device_limits = DeviceProperties.limits;

	my_free (PhysicalDevices);
	my_free (QueueFamilyProperties);

	return 0;
}

int create_surface (HINSTANCE HInstance, HWND HWnd)
{
	OutputDebugString (L"create_surface\n");

	VkWin32SurfaceCreateInfoKHR CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkWin32SurfaceCreateInfoKHR));

	CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	CreateInfo.hinstance = HInstance;
	CreateInfo.hwnd = HWnd;

	if (vkCreateWin32SurfaceKHR (instance, &CreateInfo, NULL, &surface) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SURFACE;
	}

	return 0;
}

int populate_graphics_device_extensions ()
{
	OutputDebugString (L"populate_graphics_device_extensions\n");

	uint32_t ExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &ExtensionCount, NULL);

	VkExtensionProperties* ExtensionProperties = (VkExtensionProperties*)my_malloc (sizeof (VkExtensionProperties) * ExtensionCount);
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &ExtensionCount, ExtensionProperties);

	for (uint32_t e = 0; e < ExtensionCount; e++)
	{
		if (strcmp (ExtensionProperties[e].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			requested_device_extensions[requested_device_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			break;
		}
	}

	my_free (ExtensionProperties);

	return 0;
}

int create_graphics_device ()
{
	OutputDebugString (L"create_graphics_device\n");

	float Priorities = 1.f;

	VkDeviceQueueCreateInfo QueueCreateInfo;
	memset (&QueueCreateInfo, 0, sizeof (VkDeviceQueueCreateInfo));

	QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	QueueCreateInfo.pNext = NULL;
	QueueCreateInfo.pQueuePriorities = &Priorities;
	QueueCreateInfo.queueCount = 1;
	QueueCreateInfo.queueFamilyIndex = graphics_queue_family_index;
	QueueCreateInfo.flags = 0;

	VkDeviceCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkDeviceCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	CreateInfo.pNext = NULL;
	CreateInfo.enabledExtensionCount = requested_device_extension_count;
	CreateInfo.ppEnabledExtensionNames = requested_device_extensions;
	CreateInfo.enabledLayerCount = 0;
	CreateInfo.ppEnabledLayerNames = NULL;
	CreateInfo.queueCreateInfoCount = 1;
	CreateInfo.pQueueCreateInfos = &QueueCreateInfo;
	CreateInfo.flags = 0;

	if (vkCreateDevice (physical_device, &CreateInfo, NULL, &graphics_device) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE;
	}

	vkGetDeviceQueue (graphics_device, graphics_queue_family_index, 0, &graphics_queue);

	return 0;
}

int create_swapchain ()
{
	OutputDebugString (L"create_swapchain\n");

	VkBool32 IsSurfaceSupported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, graphics_queue_family_index, surface, &IsSurfaceSupported);

	if (!IsSurfaceSupported)
	{
		return AGAINST_ERROR_GRAPHICS_SURFACE_SUPPORT;
	}

	VkSurfaceCapabilitiesKHR SurfaceCapabilites;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, surface, &SurfaceCapabilites);

	uint32_t SurfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &SurfaceFormatCount, NULL);

	VkSurfaceFormatKHR* SurfaceFormats = (VkSurfaceFormatKHR*)my_malloc (sizeof (VkSurfaceFormatKHR) * SurfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &SurfaceFormatCount, SurfaceFormats);

	for (uint32_t s = 0; s < SurfaceFormatCount; s++)
	{
		if (SurfaceFormats[s].format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			chosen_surface_format = SurfaceFormats[s];
			break;
		}
	}

	uint32_t PresentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &PresentModeCount, NULL);

	VkPresentModeKHR* PresentModes = (VkPresentModeKHR*)my_malloc (sizeof (VkPresentModeKHR) * PresentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &PresentModeCount, PresentModes);

	for (uint32_t p = 0; p < PresentModeCount; p++)
	{
		if (PresentModes[p] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			chosen_present_mode = PresentModes[p];
			break;
		}
	}

	surface_exten = SurfaceCapabilites.currentExtent;

	VkSwapchainCreateInfoKHR CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkSwapchainCreateInfoKHR));

	CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	CreateInfo.surface = surface;

	CreateInfo.clipped = 1;
	CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	CreateInfo.imageArrayLayers = 1;
	CreateInfo.imageColorSpace = chosen_surface_format.colorSpace;
	CreateInfo.imageExtent = SurfaceCapabilites.currentExtent;
	CreateInfo.imageFormat = chosen_surface_format.format;
	CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.imageUsage = SurfaceCapabilites.supportedUsageFlags;
	CreateInfo.minImageCount = SurfaceCapabilites.minImageCount + 1;
	CreateInfo.oldSwapchain = VK_NULL_HANDLE;
	CreateInfo.presentMode = chosen_present_mode;
	CreateInfo.preTransform = SurfaceCapabilites.currentTransform;

	if (vkCreateSwapchainKHR (graphics_device, &CreateInfo, NULL, &swapchain) != VK_SUCCESS)
	{
		my_free (SurfaceFormats);
		my_free (PresentModes);

		return AGAINST_ERROR_GRAPHICS_CREATE_SWAPCHAIN;
	}

	vkGetSwapchainImagesKHR (graphics_device, swapchain, &swapchain_image_count, NULL);
	swapchain_images = (VkImage*)my_malloc (sizeof (VkImage) * swapchain_image_count);

	vkGetSwapchainImagesKHR (graphics_device, swapchain, &swapchain_image_count, swapchain_images);

	swapchain_imageviews = (VkImageView*)my_malloc (sizeof (VkImageView) * swapchain_image_count);

	return 0;
}

int create_swapchain_imageviews ()
{
	OutputDebugString (L"create_swapchain_imageviews\n");

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		VkImageViewCreateInfo CreateInfo;
		memset (&CreateInfo, 0, sizeof (VkImageViewCreateInfo));

		VkComponentMapping Components;

		Components.a = VK_COMPONENT_SWIZZLE_A;
		Components.b = VK_COMPONENT_SWIZZLE_B;
		Components.g = VK_COMPONENT_SWIZZLE_G;
		Components.r = VK_COMPONENT_SWIZZLE_R;

		VkImageSubresourceRange SubresourceRange;

		SubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		SubresourceRange.baseMipLevel = 0;
		SubresourceRange.levelCount = 1;

		SubresourceRange.baseArrayLayer = 0;
		SubresourceRange.layerCount = 1;

		CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		CreateInfo.image = swapchain_images[i];
		CreateInfo.format = chosen_surface_format.format;
		CreateInfo.components = Components;
		CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		CreateInfo.subresourceRange = SubresourceRange;

		if (vkCreateImageView (graphics_device, &CreateInfo, NULL, &swapchain_imageviews[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		}
	}

	return 0;
}

int graphics_init (HINSTANCE HInstance, HWND HWnd)
{
	OutputDebugString (L"graphics_init\n");

#ifdef _DEBUG
	is_validation_needed = true;
#else
	is_validation_needed = false;
#endif

	int Result = populate_instance_layers_and_extensions ();

	if (Result != 0)
	{
		return Result;
	}

	Result = create_instance ();

	if (Result != 0)
	{
		return Result;
	}

	if (is_validation_needed)
	{
		Result = setup_debug_utils_messenger ();

		if (Result != 0)
		{
			return Result;
		}
	}

	Result = get_physical_device ();

	if (Result != 0)
	{
		return Result;
	}

	Result = create_surface (HInstance, HWnd);

	if (Result != 0)
	{
		return Result;
	}

	Result = populate_graphics_device_extensions ();

	if (Result != 0)
	{
		return Result;
	}

	Result = create_graphics_device ();

	if (Result != 0)
	{
		return Result;
	}

	Result = create_swapchain ();

	if (Result != 0)
	{
		return Result;
	}

	Result = create_swapchain_imageviews ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

void graphics_exit ()
{
	OutputDebugString (L"graphics_exit\n");

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