#pragma once

#include <Windows.h>

#include <vulkan/vulkan.h>

uint32_t GraphicsQueueFamilyIndex;

VkDevice GraphicsDevice;
VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
VkSurfaceFormatKHR ChosenSurfaceFormat;
VkExtent2D SurfaceExtent;
VkSwapchainKHR Swapchain;
VkImage* SwapchainImages;
VkImageView* SwapchainImageViews;
uint32_t SwapchainImageCount;
VkQueue GraphicsQueue;

int GraphicsInit (HINSTANCE HInstance, HWND WindowHandle);
void GraphicsShutdown ();