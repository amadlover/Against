#include "Graphics.h"
#include "Error.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <vulkan/vulkan.h>

bool IsValidationNeeded;

const char* RequestedInstanceLayers[16];
uint32_t RequestedInstanceLayerCount;

const char* RequestedInstanceExtensions[16];
uint32_t RequestedInstanceExtensionCount;

uint32_t GraphicsQueueFamilyIndex;

VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;

VkInstance Instance;
VkDebugUtilsMessengerEXT DebugUtilsMessenger;
VkPhysicalDevice PhysicalDevice;


VkResult CreateDebugUtilsMessenger (VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* DebugUtilsMessengerCreateInfo, const VkAllocationCallbacks* AllocationCallbacks, VkDebugUtilsMessengerEXT* DebugUtilsMessenger)
{
	OutputDebugString (L"CreateDebugUtilsMessenger\n");

	PFN_vkCreateDebugUtilsMessengerEXT Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (Instance, "vkCreateDebugUtilsMessengerEXT");

	if (Func)
	{
		return Func (Instance, DebugUtilsMessengerCreateInfo, AllocationCallbacks, DebugUtilsMessenger);
	}
	else
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
}

void DestroyDebugUtilsMessenger (VkInstance Instance, VkDebugUtilsMessengerEXT DebugUtilsMessenger, const VkAllocationCallbacks* AllocationCallbacks)
{
	OutputDebugString (L"DestroyDebugUtilsMessenger\n");

	PFN_vkDestroyDebugUtilsMessengerEXT Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (Instance, "vkDestroyDebugUtilsMessengerEXT");

	if (Func)
	{
		Func (Instance, DebugUtilsMessenger, AllocationCallbacks);
	}
	else
	{
		OutputDebugString (L"Could not Destroy Debug Utils Messenger\n");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (pCallbackData)
	{
		wchar_t buff[2048];
		swprintf_s (buff, 2048, L"Debug Messenger Callback %hs\n", pCallbackData->pMessage);
		OutputDebugString (buff);
	}

	return VK_FALSE;
}

int PopulateInstanceLayersAndExtensions ()
{
	OutputDebugString (L"PopulateInstanceLayersAndExtensions\n");

	if (IsValidationNeeded)
	{
		uint32_t LayerCount = 0;
		vkEnumerateInstanceLayerProperties (&LayerCount, NULL);
		VkLayerProperties* LayerProperties = (VkLayerProperties*)malloc (sizeof (VkLayerProperties) * LayerCount);
		vkEnumerateInstanceLayerProperties (&LayerCount, LayerProperties);

		for (uint32_t l = 0; l < LayerCount; l++)
		{
			if (strcmp (LayerProperties[l].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
			{
				RequestedInstanceLayers[RequestedInstanceLayerCount++] = ("VK_LAYER_LUNARG_standard_validation");
				break;
			}
		}

		free (LayerProperties);
	}

	uint32_t ExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties (NULL, &ExtensionCount, NULL);

	VkExtensionProperties* ExtensionProperties = (VkExtensionProperties*)malloc (sizeof (VkExtensionProperties) * ExtensionCount);
	vkEnumerateInstanceExtensionProperties (NULL, &ExtensionCount, ExtensionProperties);

	for (uint32_t e = 0; e < ExtensionCount; e++)
	{
		if (strcmp (ExtensionProperties[e].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			RequestedInstanceExtensions[RequestedInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}
		else if (strcmp (ExtensionProperties[e].extensionName, "VK_KHR_win32_surface") == 0)
		{
			RequestedInstanceExtensions[RequestedInstanceExtensionCount++] = "VK_KHR_win32_surface";
		}
		else if (strcmp (ExtensionProperties[e].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			RequestedInstanceExtensions[RequestedInstanceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}

		if (IsValidationNeeded)
		{
			if (strcmp (ExtensionProperties[e].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			{
				RequestedInstanceExtensions[RequestedInstanceExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			}
		}
	}

	free (ExtensionProperties);

	return 0;
}

int CreateInstance ()
{
	OutputDebugString (L"Create Instance\n");

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
	CreateInfo.enabledExtensionCount = RequestedInstanceExtensionCount;
	CreateInfo.ppEnabledExtensionNames = RequestedInstanceExtensions;
	CreateInfo.enabledLayerCount = RequestedInstanceLayerCount;
	CreateInfo.ppEnabledLayerNames = RequestedInstanceLayers;
	CreateInfo.pApplicationInfo = &ApplicationInfo;
	CreateInfo.flags = 0;

	if (vkCreateInstance (&CreateInfo, NULL, &Instance) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_INSTANCE;
	}

	return 0;
}

int SetupDebugUtilsMessenger ()
{
	OutputDebugString (L"SetupDebugUtilsMessenger\n");

	VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkDebugUtilsMessengerCreateInfoEXT));

	CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	CreateInfo.pfnUserCallback = DebugMessengerCallback;
	CreateInfo.flags = 0;

	if (CreateDebugUtilsMessenger (Instance, &CreateInfo, NULL, &DebugUtilsMessenger) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER;
	}

	return 0;
}

int GetPhysicalDevice ()
{
	OutputDebugString (L"GetPhysicalDevice\n");

	uint32_t PhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices (Instance, &PhysicalDeviceCount, NULL);

	VkPhysicalDevice* PhysicalDevices = (VkPhysicalDevice*)malloc (sizeof (VkPhysicalDevice) * PhysicalDeviceCount);
	vkEnumeratePhysicalDevices (Instance, &PhysicalDeviceCount, PhysicalDevices);

	PhysicalDevice = PhysicalDevices[0];

	VkPhysicalDeviceProperties DeviceProperties;
	vkGetPhysicalDeviceProperties (PhysicalDevice, &DeviceProperties);

	VkPhysicalDeviceFeatures DeviceFeatures;
	vkGetPhysicalDeviceFeatures (PhysicalDevice, &DeviceFeatures);

	uint32_t QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties (PhysicalDevice, &QueueFamilyCount, NULL);
	VkQueueFamilyProperties* QueueFamilyProperties = (VkQueueFamilyProperties*)malloc (sizeof (VkQueueFamilyProperties) * QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties (PhysicalDevice, &QueueFamilyCount, QueueFamilyProperties);

	for (uint32_t i = 0; i < QueueFamilyCount; i++)
	{
		if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (QueueFamilyProperties[i].queueCount > 1))
		{
			GraphicsQueueFamilyIndex = i;
			break;
		}
	}

	vkGetPhysicalDeviceMemoryProperties (PhysicalDevice, &PhysicalDeviceMemoryProperties);

	free (PhysicalDevices);
	free (QueueFamilyProperties);

	return 0;
}

int GraphicsInit (HINSTANCE HInstance, HWND HWnd)
{
#ifdef _DEBUG
	IsValidationNeeded = true;
#else
	IsValidationNeeded = false;
#endif

	int Result = PopulateInstanceLayersAndExtensions ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateInstance ();

	if (Result != 0)
	{
		return Result;
	}

	if (IsValidationNeeded)
	{
		Result = SetupDebugUtilsMessenger ();

		if (Result != 0)
		{
			return Result;
		}
	}

	Result = GetPhysicalDevice ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

void GraphicsShutdown ()
{
	OutputDebugString (L"GraphicsShutdown\n");

	if (IsValidationNeeded)
	{
		DestroyDebugUtilsMessenger (Instance, DebugUtilsMessenger, NULL);
	}

	vkDestroyInstance (Instance, NULL);
}