#include "main_menu.h"

#include "error.h"
#include "import_assets.h"
#include "utils.h"
#include "graphics.h"
#include "maths.hpp"
#include "enums.h"
#include "event.h"

#include <Windows.h>
#include <strsafe.h>

#include <stdlib.h>
#include <math.h>

#include <vulkan/vulkan.h>
#include <stb_image.h>

Node_Orig* MainMenuNodes;
uint32_t MainMenuNodeCount;

Mesh_Orig* MainMenuMeshes;
uint32_t MainMenuMeshCount;

Material_Orig* MainMenuMaterials;
uint32_t MainMenuMaterialCount;

Texture_Orig* MainMenuTextures;
uint32_t MainMenuTextureCount;

Image_Orig* MainMenuImages;
uint32_t MainMenuImageCount;

Sampler_Orig* MainMenuSamplers;
uint32_t MainMenuSamplerCount;

VkBuffer MainMenuHostVBIB;
VkBuffer* MainMenuHostVBIBs;
uint32_t MainMenuBufferCount;

VkBuffer MainMenuUniformBuffer;
VkDeviceMemory MainMenuUniformBufferMemory;

VkImage* MainMenuTImages;
VkImageView* MainMenuTImageViews;

VkDeviceMemory MainMenuHostVBIBMemory;
VkDeviceMemory MainMenuHostTImageMemory;

VkSampler MainMenuFallbackSampler;

VkDescriptorPool MainMenuDescriptorPool;
VkDescriptorSetLayout MainMenuUniformBufferDescriptorSetLayout;
VkDescriptorSetLayout MainMenuColorTextureDescriptorSetLayout;
VkDescriptorSet MainMenuUniformBufferDescriptorSet;

VkPipelineLayout MainMenuGraphicsPipelineLayout;

VkShaderModule MainMenuVertexShaderModule;
VkShaderModule MainMenuFragmentShaderModule;
VkPipelineShaderStageCreateInfo MainMenuPipelineShaderStages[2];

VkImage MainMenuDepthImage;
VkDeviceMemory MainMenuDepthImageMemory;
VkImageView MainMenuDepthImageView;

VkFramebuffer* MainMenuSwapchainFramebuffers;
VkRenderPass MainMenuRenderPass;

VkPipeline MainMenuGraphicsPipeline;

VkCommandPool MainMenuCommandPool;
VkCommandBuffer* MainMenuSwapchainCommandBuffers;

VkSemaphore MainMenuImageAcquiredSemaphore;
VkSemaphore MainMenuImageRenderedSemaphore;

VkFence* MainMenuSwapchainFences;

float MainMenuCameraViewMatrix[16];
float MainMenuCameraProjectionMatrix[16];

float Glow;
uint32_t TotalMouseDeltaX;
uint32_t TotalMouseDeltaY;

//TODO: Use DEVICE_LOCAL memory where possible

int ImportMainMenuAssets ()
{
	OutputDebugString (L"ImportMainMenuAssets\n");

	char PartialFilePath[] = "\\UIElements\\MainMenu\\MainMenu.gltf";
	char FullFilePath[MAX_PATH];
	get_full_file_path (FullFilePath, PartialFilePath);

	int Result = ImportGLTF (FullFilePath,
		&MainMenuNodes, &MainMenuNodeCount,
		&MainMenuMeshes, &MainMenuMeshCount,
		&MainMenuMaterials, &MainMenuMaterialCount,
		&MainMenuTextures, &MainMenuTextureCount,
		&MainMenuImages, &MainMenuImageCount,
		&MainMenuSamplers, &MainMenuSamplerCount);

	if (Result != 0)
	{
		return Result;
	}

	MainMenuBufferCount = 0;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			++MainMenuBufferCount;
		}
	}

	return 0;
}

//TODO: To be removed if not used. Using Push constants to transfer matrix data to the shaders.
int CreateMainMenuUniformBuffer ()
{
	OutputDebugString (L"CreateMainScreenUniformBuffer\n");

	VkBufferCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.size = sizeof (float) * 16 + sizeof (float) + sizeof (int);
	CreateInfo.queueFamilyIndexCount = 1;
	CreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &MainMenuUniformBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements MemoryRequirements = { 0 };

	vkGetBufferMemoryRequirements (GraphicsDevice, MainMenuUniformBuffer, &MemoryRequirements);

	VkMemoryAllocateInfo AllocateInfo = { 0 };

	AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	AllocateInfo.allocationSize = MemoryRequirements.size;

	uint32_t RequiredMemoryTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t t = 0; t < PhysicalDeviceMemoryProperties.memoryTypeCount; t++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << t) && RequiredMemoryTypes & PhysicalDeviceMemoryProperties.memoryTypes[t].propertyFlags)
		{
			AllocateInfo.memoryTypeIndex = t;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &AllocateInfo, NULL, &MainMenuUniformBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, MainMenuUniformBuffer, MainMenuUniformBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	return 0;
}

int CreateMainMenuHostVBIBs ()
{
	OutputDebugString (L"CreateMainMenuHostVBIBs\n");

	VkBufferCreateInfo VBIBCreateInfo = { 0 };

	VBIBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VBIBCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VBIBCreateInfo.queueFamilyIndexCount = 1;
	VBIBCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;

	MainMenuHostVBIBs = (VkBuffer*)my_malloc (sizeof (VkBuffer) * MainMenuBufferCount);

	VkMemoryAllocateInfo VBIBMemoryAllocateInfo = { 0 };

	VBIBMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	uint32_t CurrentBufferCount = 0;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			VBIBCreateInfo.size = MainMenuMeshes[m].Primitives[p].PositionsSize + MainMenuMeshes[m].Primitives[p].UV0Size + MainMenuMeshes[m].Primitives[p].IndicesSize;

			if (vkCreateBuffer (GraphicsDevice, &VBIBCreateInfo, NULL, &MainMenuHostVBIBs[CurrentBufferCount]) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
			}

			vkGetBufferMemoryRequirements (GraphicsDevice, MainMenuHostVBIBs[CurrentBufferCount], &MainMenuMeshes[m].Primitives[p].VkHandles.MemoryRequirements);

			VBIBMemoryAllocateInfo.allocationSize += MainMenuMeshes[m].Primitives[p].VkHandles.MemoryRequirements.size;
			++CurrentBufferCount;
		}
	}

	uint32_t RequiredMemoryTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	uint32_t MemoryTypeIndex = 0;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			for (uint32_t mt = 0; mt < PhysicalDeviceMemoryProperties.memoryTypeCount; mt++)
			{
				if (MainMenuMeshes[m].Primitives[p].VkHandles.MemoryRequirements.memoryTypeBits & (1 << mt))
				{
					if (RequiredMemoryTypes & PhysicalDeviceMemoryProperties.memoryTypes[mt].propertyFlags)
					{
						MemoryTypeIndex = mt;
						break;
					}
				}
			}
		}
	}

	VBIBMemoryAllocateInfo.memoryTypeIndex = MemoryTypeIndex;

	if (vkAllocateMemory (GraphicsDevice, &VBIBMemoryAllocateInfo, NULL, &MainMenuHostVBIBMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	VkDeviceSize BindMemoryOffset = 0;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			if (vkBindBufferMemory (GraphicsDevice, MainMenuHostVBIBs[m], MainMenuHostVBIBMemory, BindMemoryOffset) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
			}

			BindMemoryOffset += MainMenuMeshes[m].Primitives[p].VkHandles.MemoryRequirements.size;
		}
	}

	CurrentBufferCount = 0;

	VkDeviceSize MapMemoryOffset = 0;
	BindMemoryOffset = 0;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			MapMemoryOffset = 0;

			void* Data = NULL;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, BindMemoryOffset + MapMemoryOffset, MainMenuMeshes[m].Primitives[p].PositionsSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].Positions, MainMenuMeshes[m].Primitives[p].PositionsSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			MapMemoryOffset += MainMenuMeshes[m].Primitives[p].PositionsSize;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, BindMemoryOffset + MapMemoryOffset, MainMenuMeshes[m].Primitives[p].UV0Size, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].UV0s, MainMenuMeshes[m].Primitives[p].UV0Size);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			MapMemoryOffset += MainMenuMeshes[m].Primitives[p].UV0Size;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, BindMemoryOffset + MapMemoryOffset, MainMenuMeshes[m].Primitives[p].IndicesSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].Indices, MainMenuMeshes[m].Primitives[p].IndicesSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			MapMemoryOffset += MainMenuMeshes[m].Primitives[p].IndicesSize;

			BindMemoryOffset += MainMenuMeshes[m].Primitives[p].VkHandles.MemoryRequirements.size;

			++CurrentBufferCount;
		}
	}

	return 0;
}

int CreateMainMenuTextureImages ()
{
	OutputDebugString (L"CreateMainMenuTextureImages\n");

	VkBufferCreateInfo BufferCreateInfo = { 0 };

	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.queueFamilyIndexCount = 1;
	BufferCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer* StagingBuffers = (VkBuffer*)my_malloc (sizeof (VkBuffer) * MainMenuImageCount);
	VkMemoryRequirements* StagingBufferMemoryRequirements = (VkMemoryRequirements*)my_malloc (sizeof (VkMemoryRequirements) * MainMenuImageCount);

	VkMemoryAllocateInfo StagingMemoryAllocateInfo = { 0 };
	StagingMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		BufferCreateInfo.size = MainMenuImages[i].Size;

		if (vkCreateBuffer (GraphicsDevice, &BufferCreateInfo, NULL, &StagingBuffers[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
		}

		vkGetBufferMemoryRequirements (GraphicsDevice, StagingBuffers[i], &StagingBufferMemoryRequirements[i]);
		StagingMemoryAllocateInfo.allocationSize += StagingBufferMemoryRequirements[i].size;
	}

	VkDeviceMemory StagingBufferMemory;

	uint32_t MemoryTypeIndex = 0;
	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		for (uint32_t mt = 0; mt < PhysicalDeviceMemoryProperties.memoryTypeCount; mt++)
		{
			if (StagingBufferMemoryRequirements[i].memoryTypeBits & (1 << mt))
			{
				if (RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[mt].propertyFlags)
				{
					MemoryTypeIndex = mt;
					break;
				}
			}
		}
	}

	StagingMemoryAllocateInfo.memoryTypeIndex = MemoryTypeIndex;

	if (vkAllocateMemory (GraphicsDevice, &StagingMemoryAllocateInfo, NULL, &StagingBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	VkDeviceSize BindMemoryOffset = 0;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		if (vkBindBufferMemory (GraphicsDevice, StagingBuffers[i], StagingBufferMemory, BindMemoryOffset) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		}

		BindMemoryOffset += StagingBufferMemoryRequirements[i].size;
	}

	VkDeviceSize MapMemoryOffset = 0;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		void* Data = NULL;

		if (vkMapMemory (GraphicsDevice, StagingBufferMemory, MapMemoryOffset, MainMenuImages[i].Size, 0, &Data) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
		}

		memcpy (Data, MainMenuImages[i].Pixels, MainMenuImages[i].Size);
		vkUnmapMemory (GraphicsDevice, StagingBufferMemory);

		MapMemoryOffset += StagingBufferMemoryRequirements[i].size;
	}

	VkImageCreateInfo ImageCreateInfo = { 0 };

	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.queueFamilyIndexCount = 1;
	ImageCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	MainMenuTImages = (VkImage*)my_malloc (sizeof (VkImage) * MainMenuImageCount);
	MainMenuTImageViews = (VkImageView*)my_malloc (sizeof (VkImageView) * MainMenuImageCount);

	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		ImageCreateInfo.extent.width = MainMenuImages[i].Width;
		ImageCreateInfo.extent.height = MainMenuImages[i].Height;
		ImageCreateInfo.extent.depth = 1;

		if (vkCreateImage (GraphicsDevice, &ImageCreateInfo, NULL, &MainMenuTImages[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
		}

		vkGetImageMemoryRequirements (GraphicsDevice, MainMenuTImages[i], &MainMenuImages[i].VkHandles.MemoryRequirements);
		MemoryAllocateInfo.allocationSize += MainMenuImages[i].VkHandles.MemoryRequirements.size;
	}

	MemoryTypeIndex = 0;
	RequiredTypes = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		for (uint32_t mt = 0; mt < PhysicalDeviceMemoryProperties.memoryTypeCount; mt++)
		{
			if (MainMenuImages[i].VkHandles.MemoryRequirements.memoryTypeBits & (1 << mt))
			{
				if (RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[mt].propertyFlags)
				{
					MemoryTypeIndex = mt;
					break;
				}
			}
		}
	}

	MemoryAllocateInfo.memoryTypeIndex = MemoryTypeIndex;

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &MainMenuHostTImageMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	BindMemoryOffset = 0;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		if (vkBindImageMemory (GraphicsDevice, MainMenuTImages[i], MainMenuHostTImageMemory, BindMemoryOffset) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		}

		BindMemoryOffset += MainMenuImages[i].VkHandles.MemoryRequirements.size;
	}

	VkCommandPool MainMenuImageOpsCommandPool;

	VkCommandPoolCreateInfo CommandPoolCreateInfo = { 0 };

	CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCreateInfo.queueFamilyIndex = GraphicsQueueFamilyIndex;
	CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool (GraphicsDevice, &CommandPoolCreateInfo, NULL, &MainMenuImageOpsCommandPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	VkCommandBuffer LayoutChangeCommandBuffer;
	VkCommandBufferAllocateInfo LayoutChangeCommandBufferAllocateInfo = { 0 };

	LayoutChangeCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	LayoutChangeCommandBufferAllocateInfo.commandBufferCount = 1;
	LayoutChangeCommandBufferAllocateInfo.commandPool = MainMenuImageOpsCommandPool;

	if (vkAllocateCommandBuffers (GraphicsDevice, &LayoutChangeCommandBufferAllocateInfo, &LayoutChangeCommandBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkImageMemoryBarrier MemoryBarrier = { 0 };

	MemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	MemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	MemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	MemoryBarrier.srcAccessMask = 0;
	MemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	MemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	MemoryBarrier.subresourceRange.baseMipLevel = 0;
	MemoryBarrier.subresourceRange.levelCount = 1;
	MemoryBarrier.subresourceRange.layerCount = 1;

	VkCommandBufferBeginInfo LayoutChangeCmdBufferBeginInfo = { 0 };

	LayoutChangeCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	LayoutChangeCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer (LayoutChangeCommandBuffer, &LayoutChangeCmdBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		MemoryBarrier.image = MainMenuTImages[i];
		vkCmdPipelineBarrier (LayoutChangeCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &MemoryBarrier);
	}

	if (vkEndCommandBuffer (LayoutChangeCommandBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
	}

	VkSubmitInfo SubmitInfo = { 0 };

	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &LayoutChangeCommandBuffer;

	VkFence Fence;
	VkFenceCreateInfo FenceCreateInfo = { 0 };

	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if (vkCreateFence (GraphicsDevice, &FenceCreateInfo, NULL, &Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_FENCE;
	}

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &Fence, 1, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	if (vkResetFences (GraphicsDevice, 1, &Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	VkCommandBuffer CopyBufferToImageCmdBuffer;
	VkCommandBufferAllocateInfo CopyBufferToImageCmdBufferAllocateInfo = { 0 };

	CopyBufferToImageCmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CopyBufferToImageCmdBufferAllocateInfo.commandBufferCount = 1;
	CopyBufferToImageCmdBufferAllocateInfo.commandPool = MainMenuImageOpsCommandPool;

	if (vkAllocateCommandBuffers (GraphicsDevice, &CopyBufferToImageCmdBufferAllocateInfo, &CopyBufferToImageCmdBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkCommandBufferBeginInfo CopyBufferToImageCmdBufferBeginInfo = { 0 };

	CopyBufferToImageCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CopyBufferToImageCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer (CopyBufferToImageCmdBuffer, &CopyBufferToImageCmdBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	VkBufferImageCopy BufferImageCopy = { 0 };

	BufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	BufferImageCopy.imageSubresource.layerCount = 1;
	BufferImageCopy.bufferOffset = 0;
	BufferImageCopy.bufferImageHeight = 0;
	BufferImageCopy.bufferRowLength = 0;
	BufferImageCopy.imageOffset.x = 0; BufferImageCopy.imageOffset.y = 0; BufferImageCopy.imageOffset.z = 0;
	BufferImageCopy.imageExtent.depth = 1;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		BufferImageCopy.imageExtent.height = MainMenuImages[i].Height;
		BufferImageCopy.imageExtent.width = MainMenuImages[i].Width;

		vkCmdCopyBufferToImage (CopyBufferToImageCmdBuffer, StagingBuffers[i], MainMenuTImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
	}

	if (vkEndCommandBuffer (CopyBufferToImageCmdBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
	}

	SubmitInfo.pCommandBuffers = &CopyBufferToImageCmdBuffer;

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &Fence, 1, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	if (vkResetFences (GraphicsDevice, 1, &Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	if (vkResetCommandBuffer (LayoutChangeCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_COMMAND_BUFFER;
	}

	if (vkBeginCommandBuffer (LayoutChangeCommandBuffer, &LayoutChangeCmdBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	MemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	MemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	MemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	MemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		MemoryBarrier.image = MainMenuTImages[i];
		vkCmdPipelineBarrier (LayoutChangeCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &MemoryBarrier);
	}

	if (vkEndCommandBuffer (LayoutChangeCommandBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
	}

	SubmitInfo.pCommandBuffers = &LayoutChangeCommandBuffer;

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &Fence, 1, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	vkDestroyFence (GraphicsDevice, Fence, NULL);

	vkFreeCommandBuffers (GraphicsDevice, MainMenuImageOpsCommandPool, 1, &LayoutChangeCommandBuffer);
	vkFreeCommandBuffers (GraphicsDevice, MainMenuImageOpsCommandPool, 1, &CopyBufferToImageCmdBuffer);

	vkDestroyCommandPool (GraphicsDevice, MainMenuImageOpsCommandPool, NULL);

	vkFreeMemory (GraphicsDevice, StagingBufferMemory, NULL);

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		vkDestroyBuffer (GraphicsDevice, StagingBuffers[i], NULL);
	}

	my_free (StagingBuffers);
	my_free (StagingBufferMemoryRequirements);

	VkImageViewCreateInfo ImageViewCreateInfo = { 0 };

	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		ImageViewCreateInfo.image = MainMenuTImages[i];

		if (vkCreateImageView (GraphicsDevice, &ImageViewCreateInfo, NULL, MainMenuTImageViews + i) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		}
	}

	VkSamplerCreateInfo SamplerCreateInfo = { 0 };

	SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.mipLodBias = 0;
	SamplerCreateInfo.compareEnable = 1;
	SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

	if (vkCreateSampler (GraphicsDevice, &SamplerCreateInfo, NULL, &MainMenuFallbackSampler) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SAMPLER;
	}

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			for (uint32_t i = 0; i < MainMenuImageCount; i++)
			{
				if (MainMenuMeshes[m].Primitives[p].Material->BaseColorTexture->image == MainMenuImages + i)
				{
					MainMenuMeshes[m].Primitives[p].VkHandles.image = MainMenuTImages + i;
					MainMenuMeshes[m].Primitives[p].VkHandles.ImageViews = MainMenuTImageViews + i;
				}
			}
		}
	}

	return 0;
}

int CreateMainMenuDepthImage ()
{
	OutputDebugString (L"CreateMainMenuDepthImage\n");

	VkImageCreateInfo DepthImageCreateInfo = { 0 };

	DepthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	DepthImageCreateInfo.extent.width = SurfaceExtent.width;
	DepthImageCreateInfo.extent.height = SurfaceExtent.height;
	DepthImageCreateInfo.extent.depth = 1;
	DepthImageCreateInfo.arrayLayers = 1;
	DepthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	DepthImageCreateInfo.format = VK_FORMAT_D16_UNORM;
	DepthImageCreateInfo.mipLevels = 1;
	DepthImageCreateInfo.queueFamilyIndexCount = 1;
	DepthImageCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	DepthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	DepthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	DepthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	if (vkCreateImage (GraphicsDevice, &DepthImageCreateInfo, NULL, &MainMenuDepthImage) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
	}

	VkMemoryRequirements MemoryRequirements = { 0 };

	vkGetImageMemoryRequirements (GraphicsDevice, MainMenuDepthImage, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &MainMenuDepthImageMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindImageMemory (GraphicsDevice, MainMenuDepthImage, MainMenuDepthImageMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
	}

	VkImageViewCreateInfo DepthImageViewCreateInfo = { 0 };

	DepthImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	DepthImageViewCreateInfo.image = MainMenuDepthImage;
	DepthImageViewCreateInfo.format = VK_FORMAT_D16_UNORM;
	DepthImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	DepthImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	DepthImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	DepthImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	DepthImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

	DepthImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	DepthImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	DepthImageViewCreateInfo.subresourceRange.layerCount = 1;
	DepthImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	DepthImageViewCreateInfo.subresourceRange.levelCount = 1;

	if (vkCreateImageView (GraphicsDevice, &DepthImageViewCreateInfo, NULL, &MainMenuDepthImageView) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
	}

	return 0;
}

int CreateMainMenuShaders ()
{
	OutputDebugString (L"CreateMainMenuShaders\n");

	char VertPartialFilePath[] = "\\Shaders\\MainMenu\\UI.vert.spv";
	char VertFilename[MAX_PATH];

	get_full_file_path (VertFilename, VertPartialFilePath);

	FILE* VertFile = NULL;
	errno_t Err = fopen_s (&VertFile, VertFilename, "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (VertFile, 0, SEEK_END);

	uint32_t FileSize = (uint32_t)ftell (VertFile) / sizeof (uint32_t);
	rewind (VertFile);

	char* Buffer = (char*)my_malloc (sizeof (uint32_t) * FileSize);
	fread (Buffer, sizeof (uint32_t), FileSize, VertFile);
	fclose (VertFile);

	VkShaderModuleCreateInfo VertexShaderModuleCreateInfo = { 0 };

	VertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	VertexShaderModuleCreateInfo.pCode = (uint32_t*)Buffer;
	VertexShaderModuleCreateInfo.codeSize = sizeof (uint32_t) * FileSize;

	if (vkCreateShaderModule (GraphicsDevice, &VertexShaderModuleCreateInfo, NULL, &MainMenuVertexShaderModule) != VK_SUCCESS)
	{
		my_free (Buffer);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	my_free (Buffer);

	char FragPartialFilePath[] = "\\Shaders\\MainMenu\\UI.frag.spv";
	char FragFilename[MAX_PATH];

	get_full_file_path (FragFilename, FragPartialFilePath);


	FILE* FragFile = NULL;
	Err = fopen_s (&FragFile, FragFilename, "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (FragFile, 0, SEEK_END);

	FileSize = (uint32_t)ftell (FragFile) / sizeof (uint32_t);
	rewind (FragFile);

	Buffer = (char*)my_malloc (sizeof (uint32_t) * FileSize);
	fread (Buffer, sizeof (uint32_t), FileSize, FragFile);
	fclose (FragFile);

	VkShaderModuleCreateInfo FragmentShaderModuleCreateInfo = { 0 };

	FragmentShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	FragmentShaderModuleCreateInfo.pCode = (uint32_t*)Buffer;
	FragmentShaderModuleCreateInfo.codeSize = sizeof (uint32_t) * FileSize;

	if (vkCreateShaderModule (GraphicsDevice, &FragmentShaderModuleCreateInfo, NULL, &MainMenuFragmentShaderModule) != VK_SUCCESS)
	{
		my_free (Buffer);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	my_free (Buffer);

	VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo = { 0 };

	VertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCreateInfo.module = MainMenuVertexShaderModule;
	VertexShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageCreateInfo = { 0 };

	FragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageCreateInfo.module = MainMenuFragmentShaderModule;
	FragmentShaderStageCreateInfo.pName = "main";

	MainMenuPipelineShaderStages[0] = VertexShaderStageCreateInfo;
	MainMenuPipelineShaderStages[1] = FragmentShaderStageCreateInfo;

	return 0;
}

int CreateMainMenuRenderPass ()
{
	OutputDebugString (L"CreateMainMenuRenderPass\n");

	VkAttachmentDescription AttachmentDescriptions[2] = { 0 };

	AttachmentDescriptions[0].format = ChosenSurfaceFormat.format;
	AttachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	AttachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	AttachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	AttachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	AttachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	AttachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	AttachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;

	AttachmentDescriptions[1].format = VK_FORMAT_D16_UNORM;
	AttachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	AttachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	AttachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	AttachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	AttachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	AttachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	AttachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference ColorReference;
	ColorReference.attachment = 0;
	ColorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference DepthReference;
	DepthReference.attachment = 1;
	DepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription SubpassDescription = { 0 };

	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.inputAttachmentCount = 0;
	SubpassDescription.preserveAttachmentCount = 0;
	SubpassDescription.colorAttachmentCount = 1;
	SubpassDescription.pColorAttachments = &ColorReference;
	SubpassDescription.pDepthStencilAttachment = &DepthReference;

	VkRenderPassCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	CreateInfo.subpassCount = 1;
	CreateInfo.pSubpasses = &SubpassDescription;
	CreateInfo.attachmentCount = 2;
	CreateInfo.pAttachments = AttachmentDescriptions;

	if (vkCreateRenderPass (GraphicsDevice, &CreateInfo, NULL, &MainMenuRenderPass) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_RENDER_PASS;
	}

	return 0;
}

int CreateMainMenuFBs ()
{
	OutputDebugString (L"CreateMainMenuFBs\n");

	VkFramebufferCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	CreateInfo.renderPass = MainMenuRenderPass;
	CreateInfo.attachmentCount = 2;
	CreateInfo.width = SurfaceExtent.width;
	CreateInfo.height = SurfaceExtent.height;
	CreateInfo.layers = 1;

	MainMenuSwapchainFramebuffers = (VkFramebuffer*)my_malloc (sizeof (VkFramebuffer) * SwapchainImageCount);

	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		VkImageView Attachments[2] = { SwapchainImageViews[i] , MainMenuDepthImageView };
		CreateInfo.pAttachments = Attachments;

		if (vkCreateFramebuffer (GraphicsDevice, &CreateInfo, NULL, &MainMenuSwapchainFramebuffers[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_FRAMEBUFFER;
		}
	}

	return 0;
}

int CreateMainMenuDescriptorPool ()
{
	OutputDebugString (L"CreateMainMenuDescriptorPool\n");

	VkDescriptorPoolSize PoolSizes[2] = { 0 };

	PoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	PoolSizes[0].descriptorCount = 1;

	PoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	PoolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = { 0 };

	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	DescriptorPoolCreateInfo.poolSizeCount = 2;
	DescriptorPoolCreateInfo.pPoolSizes = PoolSizes;
	DescriptorPoolCreateInfo.maxSets = 1 + MainMenuMeshCount;

	if (vkCreateDescriptorPool (GraphicsDevice, &DescriptorPoolCreateInfo, NULL, &MainMenuDescriptorPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL;
	}

	return 0;
}

int CreateMainMenuDescriptorSetLayout ()
{
	OutputDebugString (L"CreateMainMenuDescriptorSetLayout\n");

	VkDescriptorSetLayoutBinding LayoutBinding = { 0 };

	LayoutBinding.binding = 0;
	LayoutBinding.descriptorCount = 1;
	LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	LayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = { 0 };

	DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCreateInfo.bindingCount = 1;
	DescriptorSetLayoutCreateInfo.pBindings = &LayoutBinding;

	if (vkCreateDescriptorSetLayout (GraphicsDevice, &DescriptorSetLayoutCreateInfo, NULL, &MainMenuUniformBufferDescriptorSetLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
	}

	LayoutBinding.binding = 0;
	LayoutBinding.descriptorCount = 1;
	LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	DescriptorSetLayoutCreateInfo.bindingCount = 1;
	DescriptorSetLayoutCreateInfo.pBindings = &LayoutBinding;

	if (vkCreateDescriptorSetLayout (GraphicsDevice, &DescriptorSetLayoutCreateInfo, NULL, &MainMenuColorTextureDescriptorSetLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
	}

	return 0;
}

int CreateMainMenuDescriptorSet ()
{
	OutputDebugString (L"CreateMainMenuDescriptorSet\n");

	VkDescriptorSetAllocateInfo AllocateInfo = { 0 };

	AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocateInfo.descriptorPool = MainMenuDescriptorPool;
	AllocateInfo.descriptorSetCount = 1;
	AllocateInfo.pSetLayouts = &MainMenuUniformBufferDescriptorSetLayout;

	if (vkAllocateDescriptorSets (GraphicsDevice, &AllocateInfo, &MainMenuUniformBufferDescriptorSet) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SET;
	}

	VkDescriptorBufferInfo MatBufferInfo = { 0 };

	MatBufferInfo.buffer = MainMenuUniformBuffer;
	MatBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet WriteDescriptorValues = { 0 };

	WriteDescriptorValues.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorValues.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	WriteDescriptorValues.dstSet = MainMenuUniformBufferDescriptorSet;
	WriteDescriptorValues.descriptorCount = 1;
	WriteDescriptorValues.dstBinding = 0;
	WriteDescriptorValues.pBufferInfo = &MatBufferInfo;

	vkUpdateDescriptorSets (GraphicsDevice, 1, &WriteDescriptorValues, 0, NULL);

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			AllocateInfo.pSetLayouts = &MainMenuColorTextureDescriptorSetLayout;

			MainMenuMeshes[m].Primitives[p].VkHandles.DescriptorSet = (VkDescriptorSet*)my_malloc (sizeof (VkDescriptorSet));

			if (vkAllocateDescriptorSets (GraphicsDevice, &AllocateInfo, MainMenuMeshes[m].Primitives[p].VkHandles.DescriptorSet) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SET;
			}

			VkDescriptorImageInfo ImageInfo = { 0 };
			ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			ImageInfo.sampler = MainMenuFallbackSampler;
			ImageInfo.imageView = *MainMenuMeshes[m].Primitives[p].VkHandles.ImageViews;

			VkWriteDescriptorSet DescriptorWrite = { 0 };

			DescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			DescriptorWrite.dstSet = *MainMenuMeshes[m].Primitives[p].VkHandles.DescriptorSet;
			DescriptorWrite.dstBinding = 0;
			DescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			DescriptorWrite.descriptorCount = 1;

			DescriptorWrite.pImageInfo = &ImageInfo;
			vkUpdateDescriptorSets (GraphicsDevice, 1, &DescriptorWrite, 0, NULL);
		}
	}

	return 0;
}

int CreateMainMenuGraphicsPipelineLayout ()
{
	OutputDebugString (L"CreateMainMenuGraphicsPipelineLayout\n");

	VkDescriptorSetLayout SetLayouts[2] = { MainMenuUniformBufferDescriptorSetLayout, MainMenuColorTextureDescriptorSetLayout };

	VkPushConstantRange PushConstantRange = { 0 };

	PushConstantRange.offset = 0;
	PushConstantRange.size = (sizeof (float) * 16) + sizeof (float) + sizeof (int);
	PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo PipelineCreateInfo = { 0 };

	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineCreateInfo.setLayoutCount = 2;
	PipelineCreateInfo.pSetLayouts = SetLayouts;
	PipelineCreateInfo.pushConstantRangeCount = 1;
	PipelineCreateInfo.pPushConstantRanges = &PushConstantRange;

	if (vkCreatePipelineLayout (GraphicsDevice, &PipelineCreateInfo, NULL, &MainMenuGraphicsPipelineLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
	}

	return 0;
}

int CreateMainMenuGraphicsPipeline ()
{
	OutputDebugString (L"CreateMainMenuGraphicsPipeline\n");

	VkVertexInputBindingDescription VertexInputBindingDescription[2] = { 0 };

	VertexInputBindingDescription[0].binding = 0;
	VertexInputBindingDescription[0].stride = sizeof (float) * 3;
	VertexInputBindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VertexInputBindingDescription[1].binding = 1;
	VertexInputBindingDescription[1].stride = sizeof (float) * 2;
	VertexInputBindingDescription[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription VertexInputAttributeDescriptions[2] = { 0 };

	VertexInputAttributeDescriptions[0].binding = 0;
	VertexInputAttributeDescriptions[0].location = 0;
	VertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescriptions[0].offset = 0;

	VertexInputAttributeDescriptions[1].binding = 1;
	VertexInputAttributeDescriptions[1].location = 1;
	VertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	VertexInputAttributeDescriptions[1].offset = 0;

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = { 0 };

	VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputStateCreateInfo.vertexBindingDescriptionCount = 2;
	VertexInputStateCreateInfo.pVertexBindingDescriptions = VertexInputBindingDescription;
	VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
	VertexInputStateCreateInfo.pVertexAttributeDescriptions = VertexInputAttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = { 0 };

	InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	InputAssemblyCreateInfo.primitiveRestartEnable = 0;

	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo = { 0 };

	RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	RasterizationStateCreateInfo.rasterizerDiscardEnable = 0;
	RasterizationStateCreateInfo.depthClampEnable = 0;
	RasterizationStateCreateInfo.depthBiasEnable = 0;
	RasterizationStateCreateInfo.depthBiasClamp = 0;
	RasterizationStateCreateInfo.depthBiasSlopeFactor = 0;
	RasterizationStateCreateInfo.depthBiasConstantFactor = 0;
	RasterizationStateCreateInfo.lineWidth = 1;

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = { 0 };

	ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachmentState.blendEnable = 0;

	VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo = { 0 };

	ColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendStateCreateInfo.attachmentCount = 1;
	ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachmentState;
	ColorBlendStateCreateInfo.logicOpEnable = 0;
	ColorBlendStateCreateInfo.blendConstants[0] = 1.f;
	ColorBlendStateCreateInfo.blendConstants[1] = 1.f;
	ColorBlendStateCreateInfo.blendConstants[2] = 1.f;
	ColorBlendStateCreateInfo.blendConstants[3] = 1.f;

	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo = { 0 };

	DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencilStateCreateInfo.depthTestEnable = 1;
	DepthStencilStateCreateInfo.depthWriteEnable = 1;
	DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	DepthStencilStateCreateInfo.depthBoundsTestEnable = 0;
	DepthStencilStateCreateInfo.stencilTestEnable = 0;

	VkViewport Viewport;
	Viewport.x = 0;
	Viewport.y = (float)SurfaceExtent.height;
	Viewport.width = (float)SurfaceExtent.width;
	Viewport.height = -(float)SurfaceExtent.height;
	Viewport.minDepth = 0;
	Viewport.maxDepth = 1;

	VkRect2D Scissors;
	Scissors.offset.x = 0;
	Scissors.offset.y = 0;
	Scissors.extent = SurfaceExtent;

	VkPipelineViewportStateCreateInfo ViewportStateCreateInfo = { 0 };

	ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportStateCreateInfo.viewportCount = 1;
	ViewportStateCreateInfo.pViewports = &Viewport;
	ViewportStateCreateInfo.scissorCount = 1;
	ViewportStateCreateInfo.pScissors = &Scissors;

	VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo = { 0 };

	MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	MultisampleStateCreateInfo.sampleShadingEnable = 0;
	MultisampleStateCreateInfo.alphaToOneEnable = 0;
	MultisampleStateCreateInfo.alphaToCoverageEnable = 0;

	VkGraphicsPipelineCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	CreateInfo.layout = MainMenuGraphicsPipelineLayout;
	CreateInfo.stageCount = 2;
	CreateInfo.pStages = MainMenuPipelineShaderStages;
	CreateInfo.renderPass = MainMenuRenderPass;
	CreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
	CreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
	CreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
	CreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
	CreateInfo.pViewportState = &ViewportStateCreateInfo;
	CreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
	CreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;

	if (vkCreateGraphicsPipelines (GraphicsDevice, VK_NULL_HANDLE, 1, &CreateInfo, NULL, &MainMenuGraphicsPipeline) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE;
	}

	return 0;
}

int CreateMainMenuCommandPool ()
{
	OutputDebugString (L"CreateMainMenuCommandPool\n");

	VkCommandPoolCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CreateInfo.queueFamilyIndex = GraphicsQueueFamilyIndex;

	if (vkCreateCommandPool (GraphicsDevice, &CreateInfo, NULL, &MainMenuCommandPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	VkCommandBufferAllocateInfo AllocateInfo = { 0 };

	AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocateInfo.commandPool = MainMenuCommandPool;
	AllocateInfo.commandBufferCount = SwapchainImageCount;
	AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	MainMenuSwapchainCommandBuffers = (VkCommandBuffer*)my_malloc (sizeof (VkCommandBuffer) * SwapchainImageCount);

	if (vkAllocateCommandBuffers (GraphicsDevice, &AllocateInfo, MainMenuSwapchainCommandBuffers) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	return 0;
}

int CreateMainMenuImageTransferCommandPool ()
{
	OutputDebugString (L"CreateMainMenuImageTransferCommandPool\n");

	return 0;
}

int CreateMainMenuCommandBuffers ()
{
	OutputDebugString (L"CreateMainMenuCommandBuffers\n");

	VkClearValue ClearValues[2];
	ClearValues[0].color.float32[0] = 0;
	ClearValues[0].color.float32[1] = 0;
	ClearValues[0].color.float32[2] = 0;
	ClearValues[0].color.float32[3] = 1;

	ClearValues[1].depthStencil.depth = 1;
	ClearValues[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo RenderPassBeginInfo = { 0 };

	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.renderPass = MainMenuRenderPass;
	RenderPassBeginInfo.renderArea.extent = SurfaceExtent;
	RenderPassBeginInfo.clearValueCount = 2;
	RenderPassBeginInfo.pClearValues = ClearValues;

	VkCommandBufferBeginInfo CommandBufferBeginInfo = { 0 };

	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		RenderPassBeginInfo.framebuffer = MainMenuSwapchainFramebuffers[i];

		if (vkBeginCommandBuffer (MainMenuSwapchainCommandBuffers[i], &CommandBufferBeginInfo) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		}

		vkCmdBeginRenderPass (MainMenuSwapchainCommandBuffers[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline (MainMenuSwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, MainMenuGraphicsPipeline);
		vkCmdBindDescriptorSets (MainMenuSwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, MainMenuGraphicsPipelineLayout, 0, 1, &MainMenuUniformBufferDescriptorSet, 0, NULL);

		uint32_t MeshCounter = 0;

		for (uint32_t n = 0; n < MainMenuNodeCount; n++)
		{
			if (MainMenuNodes[n].Mesh_Orig)
			{
				float ModelMatrix[16];
				CreateTransformationMatrixGLM (MainMenuNodes[n].Translation, MainMenuNodes[n].Rotation, MainMenuNodes[n].Scale, ModelMatrix);

				float ModelViewProjMatrix[16];

				CreateModelViewProjectinMatrix (MainMenuCameraProjectionMatrix, MainMenuCameraViewMatrix, ModelMatrix, ModelViewProjMatrix);

				vkCmdPushConstants (MainMenuSwapchainCommandBuffers[i], MainMenuGraphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof (float) * 16, ModelViewProjMatrix);

				if (strcmp (MainMenuNodes[n].Name, "Background") == 0)
				{
					int BackgroundPlane = 1;
					vkCmdPushConstants (MainMenuSwapchainCommandBuffers[i], MainMenuGraphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof (float) * 16 + sizeof (float), sizeof (int), &BackgroundPlane);
				}
				else
				{
					int BackgroundPlane = 0;
					vkCmdPushConstants (MainMenuSwapchainCommandBuffers[i], MainMenuGraphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof (float) * 16 + sizeof (float), sizeof (int), &BackgroundPlane);
				}

				for (uint32_t p = 0; p < MainMenuNodes[n].Mesh_Orig->PrimitiveCount; p++)
				{
					vkCmdBindDescriptorSets (MainMenuSwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, MainMenuGraphicsPipelineLayout, 1, 1, MainMenuNodes[n].Mesh_Orig->Primitives[p].VkHandles.DescriptorSet, 0, NULL);

					VkDeviceSize Offsets[3] = { 0, MainMenuNodes[n].Mesh_Orig->Primitives[p].PositionsSize, MainMenuNodes[n].Mesh_Orig->Primitives[p].PositionsSize + MainMenuNodes[n].Mesh_Orig->Primitives[p].UV0Size };
					vkCmdBindVertexBuffers (MainMenuSwapchainCommandBuffers[i], 0, 1, MainMenuHostVBIBs + MeshCounter, Offsets);
					vkCmdBindVertexBuffers (MainMenuSwapchainCommandBuffers[i], 1, 1, MainMenuHostVBIBs + MeshCounter, Offsets + 1);
					vkCmdBindIndexBuffer (MainMenuSwapchainCommandBuffers[i], MainMenuHostVBIBs[MeshCounter], Offsets[2], VK_INDEX_TYPE_UINT32);

					vkCmdDrawIndexed (MainMenuSwapchainCommandBuffers[i], MainMenuNodes[n].Mesh_Orig->Primitives[p].IndexCount, 1, 0, 0, 0);

					++MeshCounter;
				}
			}
		}

		vkCmdEndRenderPass (MainMenuSwapchainCommandBuffers[i]);

		if (vkEndCommandBuffer (MainMenuSwapchainCommandBuffers[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
		}
	}

	return 0;
}

int CreateMainMenuSyncObjects ()
{
	OutputDebugString (L"CreateMainMenuSyncObjects\n");

	VkSemaphoreCreateInfo SemaphoreCreateInfo = { 0 };

	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore (GraphicsDevice, &SemaphoreCreateInfo, NULL, &MainMenuImageAcquiredSemaphore) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE;
	}

	if (vkCreateSemaphore (GraphicsDevice, &SemaphoreCreateInfo, NULL, &MainMenuImageRenderedSemaphore) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE;
	}

	VkFenceCreateInfo FenceCreateInfo = { 0 };

	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	MainMenuSwapchainFences = (VkFence*)my_malloc (sizeof (VkFence) * SwapchainImageCount);

	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		if (vkCreateFence (GraphicsDevice, &FenceCreateInfo, NULL, &MainMenuSwapchainFences[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_FENCE;
		}
	}

	return 0;
}

int UpdateMainMenuUniformBufferViewProjMatrix ()
{
	CreatePerspectiveProjectionMatrixGLM (45.f, (float)SurfaceExtent.width / (float)SurfaceExtent.height, 0.1f, 50.f, MainMenuCameraProjectionMatrix);

	float Eye[3] = { 0,0,10 };
	float Center[3] = { (float)TotalMouseDeltaX,(float)TotalMouseDeltaY,0 };
	float Up[3] = { 0,1,0 };

	CreateLookatMatrixGLM (Eye, Center, Up, MainMenuCameraViewMatrix);

	return 0;
}

int InitMainMenuGraphics ()
{
	OutputDebugString (L"SetupMainMenu\n");

	int Result = ImportMainMenuAssets ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuUniformBuffer ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuShaders ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuHostVBIBs ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuImageTransferCommandPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuCommandPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuTextureImages ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuDepthImage ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuDescriptorPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuDescriptorSetLayout ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuDescriptorSet ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuRenderPass ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuFBs ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuGraphicsPipelineLayout ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuGraphicsPipeline ();

	if (Result != 0)
	{
		return Result;
	}

	Result = UpdateMainMenuUniformBufferViewProjMatrix ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuCommandBuffers ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuSyncObjects ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

void MainMenuProcessMouseMovement (uint32_t X, uint32_t Y, uint32_t DeltaX, uint32_t DeltaY)
{
	if (DeltaX != 0 && DeltaY != 0)
	{
		TotalMouseDeltaX += DeltaX;
		TotalMouseDeltaY += DeltaY;

		wchar_t Buff[64];
		swprintf (Buff, 64, L"%d %d\n", TotalMouseDeltaX, TotalMouseDeltaY);
		OutputDebugString (Buff);
	}
}

int main_menu_process_keyboard_input (WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:
		go_to_scene_fp (e_scene_type_splash_screen);
		break;
	default:
		break;
	}
	return 0;
}

int main_menu_draw (uint64_t ElapsedTime)
{
	UpdateMainMenuUniformBufferViewProjMatrix ();

	uint32_t ImageIndex = 0;

	VkResult Result = vkAcquireNextImageKHR (GraphicsDevice, Swapchain, UINT64_MAX, MainMenuImageAcquiredSemaphore, VK_NULL_HANDLE, &ImageIndex);

	if (Result != VK_SUCCESS)
	{
		if (Result == VK_SUBOPTIMAL_KHR || Result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			return 0;
		}
		else
		{
			return AGAINST_ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE;
		}
	}

	VkPipelineStageFlags WaitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo SubmitInfo = { 0 };

	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.pWaitDstStageMask = &WaitStageMask;
	SubmitInfo.pWaitSemaphores = &MainMenuImageAcquiredSemaphore;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = &MainMenuImageRenderedSemaphore;
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pCommandBuffers = &MainMenuSwapchainCommandBuffers[ImageIndex];
	SubmitInfo.commandBufferCount = 1;

	if (vkResetFences (GraphicsDevice, 1, &MainMenuSwapchainFences[ImageIndex]) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, MainMenuSwapchainFences[ImageIndex]) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &MainMenuSwapchainFences[ImageIndex], 1, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	VkPresentInfoKHR PresentInfo = { 0 };

	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &Swapchain;
	PresentInfo.pImageIndices = &ImageIndex;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = &MainMenuImageRenderedSemaphore;

	Result = vkQueuePresentKHR (GraphicsQueue, &PresentInfo);

	if (Result != VK_SUCCESS)
	{
		if (Result == VK_SUBOPTIMAL_KHR || Result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			return 0;
		}
		else
		{
			return AGAINST_ERROR_GRAPHICS_QUEUE_PRESENT;
		}
	}

	return 0;
}

void DestroyMainMenuGraphics ()
{
	OutputDebugString (L"DestroyMainMenu\n");

	vkWaitForFences (GraphicsDevice, SwapchainImageCount, MainMenuSwapchainFences, 1, UINT64_MAX);

	if (MainMenuDepthImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView (GraphicsDevice, MainMenuDepthImageView, NULL);
	}

	if (MainMenuDepthImage != VK_NULL_HANDLE)
	{
		vkDestroyImage (GraphicsDevice, MainMenuDepthImage, NULL);
	}

	vkFreeMemory (GraphicsDevice, MainMenuDepthImageMemory, NULL);

	if (MainMenuSwapchainFences)
	{
		vkWaitForFences (GraphicsDevice, SwapchainImageCount, MainMenuSwapchainFences, 1, UINT64_MAX);

		for (uint32_t i = 0; i < SwapchainImageCount; i++)
		{
			if (MainMenuSwapchainFences[i] != VK_NULL_HANDLE)
			{
				vkDestroyFence (GraphicsDevice, MainMenuSwapchainFences[i], NULL);
			}
		}

		my_free (MainMenuSwapchainFences);
	}

	if (MainMenuImageAcquiredSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (GraphicsDevice, MainMenuImageAcquiredSemaphore, NULL);
	}

	if (MainMenuImageRenderedSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (GraphicsDevice, MainMenuImageRenderedSemaphore, NULL);
	}

	if (MainMenuMeshes)
	{
		for (uint32_t m = 0; m < MainMenuMeshCount; m++)
		{
			for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
			{
				if (MainMenuMeshes[m].Primitives[p].VkHandles.DescriptorSet)
				{
					if (MainMenuMeshes[m].Primitives[p].VkHandles.DescriptorSet != VK_NULL_HANDLE)
					{
						vkFreeDescriptorSets (GraphicsDevice, MainMenuDescriptorPool, 1, MainMenuMeshes[m].Primitives[p].VkHandles.DescriptorSet);
					}

					my_free (MainMenuMeshes[m].Primitives[p].VkHandles.DescriptorSet);
				}
			}
		}
	}

	if (MainMenuUniformBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, MainMenuUniformBuffer, NULL);
	}

	if (MainMenuUniformBufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, MainMenuUniformBufferMemory, NULL);
	}

	if (MainMenuSwapchainCommandBuffers)
	{
		vkFreeCommandBuffers (GraphicsDevice, MainMenuCommandPool, SwapchainImageCount, MainMenuSwapchainCommandBuffers);

		my_free (MainMenuSwapchainCommandBuffers);
	}

	if (MainMenuCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (GraphicsDevice, MainMenuCommandPool, NULL);
	}

	if (MainMenuUniformBufferDescriptorSet != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (GraphicsDevice, MainMenuDescriptorPool, 1, &MainMenuUniformBufferDescriptorSet);
	}

	if (MainMenuUniformBufferDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (GraphicsDevice, MainMenuUniformBufferDescriptorSetLayout, NULL);
	}

	if (MainMenuColorTextureDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (GraphicsDevice, MainMenuColorTextureDescriptorSetLayout, NULL);
	}

	if (MainMenuDescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool (GraphicsDevice, MainMenuDescriptorPool, NULL);
	}

	if (MainMenuGraphicsPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline (GraphicsDevice, MainMenuGraphicsPipeline, NULL);
	}

	if (MainMenuGraphicsPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout (GraphicsDevice, MainMenuGraphicsPipelineLayout, NULL);
	}

	if (MainMenuSwapchainFramebuffers)
	{
		for (uint32_t s = 0; s < SwapchainImageCount; s++)
		{
			if (MainMenuSwapchainFramebuffers[s] != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer (GraphicsDevice, MainMenuSwapchainFramebuffers[s], NULL);
			}
		}

		my_free (MainMenuSwapchainFramebuffers);
	}

	if (MainMenuRenderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass (GraphicsDevice, MainMenuRenderPass, NULL);
	}

	if (MainMenuVertexShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (GraphicsDevice, MainMenuVertexShaderModule, NULL);
	}

	if (MainMenuFragmentShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (GraphicsDevice, MainMenuFragmentShaderModule, NULL);
	}

	if (MainMenuFallbackSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler (GraphicsDevice, MainMenuFallbackSampler, NULL);
	}

	if (MainMenuHostVBIB != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, MainMenuHostVBIB, NULL);
	}

	if (MainMenuHostVBIBs)
	{
		for (uint32_t b = 0; b < MainMenuBufferCount; b++)
		{
			if (MainMenuHostVBIBs[b] != VK_NULL_HANDLE)
			{
				vkDestroyBuffer (GraphicsDevice, MainMenuHostVBIBs[b], NULL);
			}
		}
	}

	if (MainMenuHostTImageMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, MainMenuHostTImageMemory, NULL);
	}

	if (MainMenuHostVBIBMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, MainMenuHostVBIBMemory, NULL);
	}

	if (MainMenuTImages)
	{
		for (uint32_t i = 0; i < MainMenuImageCount; i++)
		{
			if (MainMenuTImages[i] != VK_NULL_HANDLE)
			{
				vkDestroyImage (GraphicsDevice, MainMenuTImages[i], NULL);
			}
		}

		my_free (MainMenuTImages);
	}

	if (MainMenuTImageViews)
	{
		for (uint32_t i = 0; i < MainMenuImageCount; i++)
		{
			if (MainMenuTImageViews[i] != VK_NULL_HANDLE)
			{
				vkDestroyImageView (GraphicsDevice, MainMenuTImageViews[i], NULL);
			}
		}

		my_free (MainMenuTImageViews);
	}

	if (MainMenuNodes)
	{
		my_free (MainMenuNodes);
	}

	if (MainMenuMeshes)
	{
		for (uint32_t m = 0; m < MainMenuMeshCount; m++)
		{
			if (MainMenuMeshes[m].Primitives)
			{
				for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
				{
					if (MainMenuMeshes[m].Primitives[p].Positions)
					{
						my_free (MainMenuMeshes[m].Primitives[p].Positions);
					}

					if (MainMenuMeshes[m].Primitives[p].UV0s)
					{
						my_free (MainMenuMeshes[m].Primitives[p].UV0s);
					}

					if (MainMenuMeshes[m].Primitives[p].Indices)
					{
						my_free (MainMenuMeshes[m].Primitives[p].Indices);
					}
				}

				my_free (MainMenuMeshes[m].Primitives);
			}
		}

		my_free (MainMenuMeshes);
	}

	if (MainMenuImages)
	{
		for (uint32_t i = 0; i < MainMenuImageCount; i++)
		{
			if (MainMenuImages[i].Pixels)
			{
				stbi_image_free (MainMenuImages[i].Pixels);
			}
		}

		my_free (MainMenuImages);
	}

	if (MainMenuTextures)
	{
		my_free (MainMenuTextures);
	}

	if (MainMenuMaterials)
	{
		my_free (MainMenuMaterials);
	}

	OutputDebugString (L"Finished DestroyMainMenu\n");
}