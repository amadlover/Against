#pragma once

#include <Windows.h>

#include <vulkan/vulkan.h>

uint32_t GraphicsQueueFamilyIndex;

VkDevice GraphicsDevice;
VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
VkSurfaceFormatKHR ChosenSurfaceFormat;
VkExtent2D SurfaceExtent;
VkImage* SwapchainImages;
VkImageView* SwapchainImageViews;
uint32_t SwapchainImageCount;

int GraphicsInit (HINSTANCE HInstance, HWND HWnd);
void GraphicsShutdown ();