#pragma once

#include <Windows.h>

#include <vulkan/vulkan.h>

size_t graphics_queue_family_index;
size_t transfer_queue_family_index;
size_t compute_queue_family_index;

VkDevice graphics_device;
VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
VkPhysicalDeviceLimits physical_device_limits;
VkSurfaceFormatKHR chosen_surface_format;
VkExtent2D surface_extent;
VkSwapchainKHR swapchain;
VkImage* swapchain_images;
VkImageView* swapchain_imageviews;
size_t swapchain_image_count;
VkQueue graphics_queue;
VkQueue transfer_queue;
VkQueue compute_queue;
VkCommandPool common_command_pool;

int common_graphics_init (HINSTANCE HInstance, HWND WindowHandle);
void common_graphics_exit ();