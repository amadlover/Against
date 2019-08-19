#pragma once

#include <Windows.h>

#include <vulkan/vulkan.h>

VkDevice GraphicsDevice;
VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
VkSurfaceFormatKHR ChosenSurfaceFormat;

int GraphicsInit (HINSTANCE HInstance, HWND HWnd);
void GraphicsShutdown ();