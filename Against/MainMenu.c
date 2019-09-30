#include "MainMenu.h"

#include "Error.h"
#include "ImportAssets.h"
#include "Utility.h"
#include "Graphics.h"

#include <Windows.h>
#include <strsafe.h>

#include <stdlib.h>
#include <math.h>

#include <vulkan/vulkan.h>

Node* MainMenuNodes;
uint32_t MainMenuNodeCount;

Mesh* MainMenuMeshes;
uint32_t MainMenuMeshCount;

Material* MainMenuMaterials;
uint32_t MainMenuMaterialCount;

Texture* MainMenuTextures;
uint32_t MainMenuTextureCount;

Image* MainMenuImages;
uint32_t MainMenuImageCount;

Sampler* MainMenuSamplers;
uint32_t MainMenuSamplerCount;

VkBuffer MainMenuHostVBIB;
VkBuffer* MainMenuHostVBIBs;
uint32_t MainMenuBufferCount;

VkImage* MainMenuTImages;
VkImageView* MainMenuTImageViews;

VkDeviceMemory MainMenuHostVBIBMemory;
VkDeviceMemory MainMenuHostTImageMemory;

VkSampler MainMenuFallbackSampler;

VkDescriptorPool MainMenuDescriptorPool;
VkDescriptorSetLayout MainMenuDescriptorSetLayout;
VkDescriptorSet MainMenuDescriptorSet;

VkPipelineLayout MainMenuGraphicsPipelineLayout;

VkShaderModule MainMenuVertexShaderModule;
VkShaderModule MainMenuFragmentShaderModule;
VkPipelineShaderStageCreateInfo MainMenuPipelineShaderStages[2];

VkFramebuffer* MainMenuSwapchainFramebuffers;
VkRenderPass MainMenuRenderPass;

VkPipeline MainMenuGraphicsPipeline;

VkCommandPool MainMenuCommandPool;
VkCommandBuffer* MainMenuSwapchainCommandBuffers;

//TODO: Using only host memory now

int ImportMainMenuAssets ()
{
	OutputDebugString (L"ImportMainMenuAssets\n");

	TCHAR UIElementPath[MAX_PATH];
	GetApplicationFolder (UIElementPath);
	StringCchCat (UIElementPath, MAX_PATH, L"\\UIElements\\MainMenu\\MainMenu.gltf");

	char UIElementFile[MAX_PATH];
	wcstombs_s (NULL, UIElementFile, MAX_PATH, UIElementPath, MAX_PATH);

	int Result = ImportMainMenuGLTF (UIElementFile, &MainMenuNodes, &MainMenuNodeCount, &MainMenuMeshes, &MainMenuMeshCount, &MainMenuMaterials, &MainMenuMaterialCount, &MainMenuTextures, &MainMenuTextureCount, &MainMenuImages, &MainMenuImageCount, &MainMenuSamplers, &MainMenuSamplerCount);

	if (Result != 0)
	{
		return Result;
	}

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			++MainMenuBufferCount;
		}
	}

	return 0;
}

int CreateMainMenuUniformBuffer ()
{
	OutputDebugString (L"CreateMainScreenUniformBuffer\n");
	
	return 0;
}

int CreateMainMenuHostVBIBs ()
{
	OutputDebugString (L"CreateMainMenuHostVBIBs\n");

	VkBufferCreateInfo VBIBCreateInfo;
	memset (&VBIBCreateInfo, 0, sizeof (VkBufferCreateInfo));

	VBIBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VBIBCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VBIBCreateInfo.queueFamilyIndexCount = 1;
	VBIBCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	
	MainMenuHostVBIBs = (VkBuffer*)malloc (sizeof (VkBuffer) * MainMenuBufferCount);
	VkMemoryRequirements* VBIBMemoryRequirements = (VkMemoryRequirements*)malloc (sizeof (VkMemoryRequirements) * MainMenuBufferCount);

	uint32_t CurrentBufferCount = 0;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			VBIBCreateInfo.size = MainMenuMeshes[m].Primitives[p].PositionSize + MainMenuMeshes[m].Primitives[p].UV0Size + MainMenuMeshes[m].Primitives[p].IndexSize;

			if (vkCreateBuffer (GraphicsDevice, &VBIBCreateInfo, NULL, &MainMenuHostVBIBs[CurrentBufferCount]) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
			}
	
			vkGetBufferMemoryRequirements (GraphicsDevice, MainMenuHostVBIBs[CurrentBufferCount], VBIBMemoryRequirements + m);
			++CurrentBufferCount;
		}

	}
	
	VkMemoryAllocateInfo VBIBMemoryAllocateInfo;
	memset (&VBIBMemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	VBIBMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	
	for (uint32_t m = 0; m < MainMenuBufferCount; m++)
	{
		VBIBMemoryAllocateInfo.allocationSize += VBIBMemoryRequirements[m].size;
	}

	uint32_t RequiredMemoryTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	uint32_t MemoryTypeIndex = 0;

	for (uint32_t m = 0; m < MainMenuBufferCount; m++)
	{
		for (uint32_t mt = 0; mt < PhysicalDeviceMemoryProperties.memoryTypeCount; mt++)
		{
			if (VBIBMemoryRequirements->memoryTypeBits & (1 << mt))
			{
				if (RequiredMemoryTypes & PhysicalDeviceMemoryProperties.memoryTypes[mt].propertyFlags)
				{
					MemoryTypeIndex = mt;
					break;
				}
			}
		}
 	}

	VBIBMemoryAllocateInfo.memoryTypeIndex = MemoryTypeIndex;

	if (vkAllocateMemory (GraphicsDevice, &VBIBMemoryAllocateInfo, NULL, &MainMenuHostVBIBMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_BUFFER_MEMORY;
	}

	VkDeviceSize BindMemoryOffset = 0;

	for (uint32_t m = 0; m < MainMenuBufferCount; m++)
	{
		if (vkBindBufferMemory (GraphicsDevice, MainMenuHostVBIBs[m], MainMenuHostVBIBMemory, BindMemoryOffset) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		}

		BindMemoryOffset += VBIBMemoryRequirements[m].size;
	}

	free (VBIBMemoryRequirements);

	CurrentBufferCount = 0;

	VkDeviceSize MapMemoryOffset = 0;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++) {
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			void* Data = NULL;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, MapMemoryOffset, MainMenuMeshes[m].Primitives[p].PositionSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].Positions, MainMenuMeshes[m].Primitives[p].PositionSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			MapMemoryOffset += MainMenuMeshes[m].Primitives[p].PositionSize;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, MapMemoryOffset, MainMenuMeshes[m].Primitives[p].UV0Size, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].UV0s, MainMenuMeshes[m].Primitives[p].UV0Size);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			MapMemoryOffset += MainMenuMeshes[m].Primitives[p].UV0Size;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, MapMemoryOffset, MainMenuMeshes[m].Primitives[p].IndexSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].Indices, MainMenuMeshes[m].Primitives[p].IndexSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			MapMemoryOffset += MainMenuMeshes[m].Primitives[p].IndexSize;

			++CurrentBufferCount;
		}
	}

	return 0;
}

int CreateMainMenuTextureImages ()
{
	OutputDebugString (L"CreateMainMenuTextureImages\n");

	VkImageCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkImageCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	CreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	CreateInfo.arrayLayers = 1;
	CreateInfo.imageType = VK_IMAGE_TYPE_2D;
	CreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	CreateInfo.mipLevels = 1;
	CreateInfo.queueFamilyIndexCount = 1;
	CreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	CreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

	MainMenuTImages = (VkImage*)malloc (sizeof (VkImage) * MainMenuImageCount);
	MainMenuTImageViews = (VkImageView*)malloc (sizeof (VkImageView) * MainMenuImageCount);

	VkMemoryRequirements* TImageMemoryRequirements = (VkMemoryRequirements*)malloc (sizeof (VkMemoryRequirements) * MainMenuImageCount);

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		CreateInfo.extent.width = MainMenuImages[i].Width;
		CreateInfo.extent.height = MainMenuImages[i].Height;
		CreateInfo.extent.depth = 1;

		if (vkCreateImage (GraphicsDevice, &CreateInfo, NULL, &MainMenuTImages[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
		}

		vkGetImageMemoryRequirements (GraphicsDevice, MainMenuTImages[i], (TImageMemoryRequirements + i));
	}

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		MemoryAllocateInfo.allocationSize += TImageMemoryRequirements[i].size;
	}

	uint32_t MemoryTypeIndex = 0;
	uint32_t RequiredMemoryTypes = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		for (uint32_t mt = 0; mt < PhysicalDeviceMemoryProperties.memoryTypeCount; mt++)
		{
			if (TImageMemoryRequirements[i].memoryTypeBits & (1 << mt))
			{
				if (RequiredMemoryTypes & PhysicalDeviceMemoryProperties.memoryTypes[mt].propertyFlags)
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
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_IMAGE_MEMORY;
	}

	VkDeviceSize BindMemoryOffset = 0;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		if (vkBindImageMemory (GraphicsDevice, MainMenuTImages[i], MainMenuHostTImageMemory, BindMemoryOffset) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		}

		BindMemoryOffset += TImageMemoryRequirements[i].size;
	}

	uint32_t MapMemoryOffset = 0;

	for (uint32_t i = 0; i < MainMenuImageCount; i++)
	{
		void* Data = NULL;

		if (vkMapMemory (GraphicsDevice, MainMenuHostTImageMemory, MapMemoryOffset, MainMenuImages[i].PixelSize, 0, &Data) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_MAP_IMAGE_MEMORY;
		}

		memcpy (Data, MainMenuImages[i].Pixels, MainMenuImages[i].PixelSize);
		vkUnmapMemory (GraphicsDevice, MainMenuHostTImageMemory);
	}

	free (TImageMemoryRequirements);

	VkImageViewCreateInfo ImageViewCreateInfo;
	memset (&ImageViewCreateInfo, 0, sizeof (VkImageViewCreateInfo));

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

	VkSamplerCreateInfo SamplerCreateInfo;
	memset (&SamplerCreateInfo, 0, sizeof (VkSamplerCreateInfo));

	SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.mipLodBias = 0;
	SamplerCreateInfo.compareEnable = VK_TRUE;
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
				if (MainMenuMeshes[m].Primitives[p].Material->BaseColorTexture->Image == MainMenuImages + i)
				{
					MainMenuMeshes[m].Primitives[p].VkHandles.Image = MainMenuTImages + i;
					MainMenuMeshes[m].Primitives[p].VkHandles.ImageView = MainMenuTImageViews + i;
				}
			}
		}
	}

	return 0;
}

int CreateMainMenuShaders ()
{
	OutputDebugString (L"CreateMainMenuShaders\n");

	TCHAR VertPath[MAX_PATH];
	GetApplicationFolder (VertPath);
	StringCchCat (VertPath, MAX_PATH, L"\\Shaders\\MainMenu\\UI.vert.spv");

	char VertFilename[MAX_PATH];
	wcstombs_s (NULL, VertFilename, MAX_PATH, VertPath, MAX_PATH);

	FILE* VertFile = NULL;
	errno_t Err = fopen_s (&VertFile, VertFilename, "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (VertFile, 0, SEEK_END);

	uint32_t FileSize = (uint32_t)ftell (VertFile) / sizeof (uint32_t);
	rewind (VertFile);

	char* Buffer = (char*)malloc (sizeof (uint32_t) * FileSize);
	fread (Buffer, sizeof (uint32_t), FileSize, VertFile);
	fclose (VertFile);

	VkShaderModuleCreateInfo VertexShaderModuleCreateInfo;
	memset (&VertexShaderModuleCreateInfo, 0, sizeof (VkShaderModuleCreateInfo));

	VertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	VertexShaderModuleCreateInfo.pCode = (uint32_t*)Buffer;
	VertexShaderModuleCreateInfo.codeSize = sizeof (uint32_t) * FileSize;

	if (vkCreateShaderModule (GraphicsDevice, &VertexShaderModuleCreateInfo, NULL, &MainMenuVertexShaderModule) != VK_SUCCESS)
	{
		free (Buffer);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	free (Buffer);

	TCHAR FragPath[MAX_PATH];
	GetApplicationFolder (FragPath);
	StringCchCat (FragPath, MAX_PATH, L"\\Shaders\\MainMenu\\UI.frag.spv");

	char FragFilename[MAX_PATH];
	wcstombs_s (NULL, FragFilename, MAX_PATH, FragPath, MAX_PATH);

	FILE* FragFile = NULL;
	Err = fopen_s (&FragFile, FragFilename, "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (FragFile, 0, SEEK_END);

	FileSize = (uint32_t)ftell (FragFile) / sizeof (uint32_t);
	rewind (FragFile);

	Buffer = (char*)malloc (sizeof (uint32_t) * FileSize);
	fread (Buffer, sizeof (uint32_t), FileSize, FragFile);
	fclose (FragFile);

	VkShaderModuleCreateInfo FragmentShaderModuleCreateInfo;
	memset (&FragmentShaderModuleCreateInfo, 0, sizeof (VkShaderModuleCreateInfo));

	FragmentShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	FragmentShaderModuleCreateInfo.pCode = (uint32_t*)Buffer;
	FragmentShaderModuleCreateInfo.codeSize = sizeof (uint32_t) * FileSize;

	if (vkCreateShaderModule (GraphicsDevice, &FragmentShaderModuleCreateInfo, NULL, &MainMenuFragmentShaderModule) != VK_SUCCESS)
	{
		free (Buffer);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	free (Buffer);

	VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo;
	memset (&VertexShaderStageCreateInfo, 0, sizeof (VkPipelineShaderStageCreateInfo));

	VertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCreateInfo.module = MainMenuVertexShaderModule;
	VertexShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageCreateInfo;
	memset (&FragmentShaderStageCreateInfo, 0, sizeof (VkPipelineShaderStageCreateInfo));

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

	VkAttachmentDescription AttachmentDescription;
	memset (&AttachmentDescription, 0, sizeof (VkAttachmentDescription));

	AttachmentDescription.format = ChosenSurfaceFormat.format;
	AttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	AttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	AttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	AttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	AttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	AttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	AttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference ColorReference;
	ColorReference.attachment = 0;
	ColorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription SubpassDescription;
	memset (&SubpassDescription, 0, sizeof (VkSubpassDescription));

	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.inputAttachmentCount = 0;
	SubpassDescription.preserveAttachmentCount = 0;
	SubpassDescription.colorAttachmentCount = 1;
	SubpassDescription.pColorAttachments = &ColorReference;

	VkSubpassDependency SubpassDependencies[2];
	memset (&SubpassDependencies, 0, sizeof (VkSubpassDependency) * 2);

	SubpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependencies[0].dstSubpass = 0;
	SubpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	SubpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	SubpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	SubpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	SubpassDependencies[1].srcSubpass = 0;
	SubpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	SubpassDependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	SubpassDependencies[1].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	SubpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkRenderPassCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	CreateInfo.subpassCount = 1;
	CreateInfo.pSubpasses = &SubpassDescription;
	CreateInfo.attachmentCount = 1;
	CreateInfo.pAttachments = &AttachmentDescription;
	CreateInfo.dependencyCount = 2;
	CreateInfo.pDependencies = SubpassDependencies;

	if (vkCreateRenderPass (GraphicsDevice, &CreateInfo, NULL, &MainMenuRenderPass) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_RENDER_PASS;
	}

	return 0;
}

int CreateMainMenuFBs ()
{
	OutputDebugString (L"CreateMainMenuFBs\n");

	VkFramebufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkFramebufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	CreateInfo.renderPass = MainMenuRenderPass;
	CreateInfo.attachmentCount = 1;
	CreateInfo.width = SurfaceExtent.width;
	CreateInfo.height = SurfaceExtent.height;
	CreateInfo.layers = 1;

	MainMenuSwapchainFramebuffers = (VkFramebuffer*)malloc (sizeof (VkFramebuffer) * SwapchainImageCount);

	VkImageView Attachment;
	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		Attachment = SwapchainImageViews[i];
		CreateInfo.pAttachments = &Attachment;

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

	VkDescriptorPoolSize PoolSizes[2];
	memset (&PoolSizes, 0, sizeof (VkDescriptorPoolSize) * 2);

	PoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	PoolSizes[0].descriptorCount = 2;

	PoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	PoolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo;
	memset (&DescriptorPoolCreateInfo, 0, sizeof (VkDescriptorPoolCreateInfo));

	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	DescriptorPoolCreateInfo.poolSizeCount = 2;
	DescriptorPoolCreateInfo.pPoolSizes = PoolSizes;
	DescriptorPoolCreateInfo.maxSets = 1;
	
	if (vkCreateDescriptorPool (GraphicsDevice, &DescriptorPoolCreateInfo, NULL, &MainMenuDescriptorPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL;
	}

	return 0;
}

int CreateMainMenuDescriptorSetLayout ()
{
	OutputDebugString (L"CreateMainMenuDescriptorSetLayout\n");

	VkDescriptorSetLayoutBinding LayoutBinding[3];
	memset (&LayoutBinding, 0, sizeof (VkDescriptorSetLayoutBinding) * 3);

	LayoutBinding[0].binding = 0;
	LayoutBinding[0].descriptorCount = 1;
	LayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	LayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	LayoutBinding[1].binding = 1;
	LayoutBinding[1].descriptorCount = 1; 
	LayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	LayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	LayoutBinding[2].binding = 2;
	LayoutBinding[2].descriptorCount = 1;
	LayoutBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	LayoutBinding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
	memset (&DescriptorSetLayoutCreateInfo, 0, sizeof (VkDescriptorSetLayoutCreateInfo));
	DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCreateInfo.bindingCount = 3;
	DescriptorSetLayoutCreateInfo.pBindings = LayoutBinding;

	if (vkCreateDescriptorSetLayout (GraphicsDevice, &DescriptorSetLayoutCreateInfo, NULL, &MainMenuDescriptorSetLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
	}

	return 0;
}

int CreateMainMenuDescriptorSet ()
{
	OutputDebugString (L"CreateMainMenuDescriptorSet\n");

	VkDescriptorSetAllocateInfo AllocateInfo;
	memset (&AllocateInfo, 0, sizeof (VkDescriptorSetAllocateInfo));

	AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocateInfo.descriptorPool = MainMenuDescriptorPool;
	AllocateInfo.descriptorSetCount = 1;
	AllocateInfo.pSetLayouts = &MainMenuDescriptorSetLayout;

	if (vkAllocateDescriptorSets (GraphicsDevice, &AllocateInfo, &MainMenuDescriptorSet) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SET;
	}

	return 0;
}

int CreateMainMenuGraphicsPipelineLayout ()
{
	OutputDebugString (L"CreateMainMenuGraphicsPipelineLayout\n");

	VkPipelineLayoutCreateInfo PipelineCreateInfo;
	memset (&PipelineCreateInfo, 0, sizeof (VkPipelineLayoutCreateInfo));

	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineCreateInfo.setLayoutCount = 1;
	PipelineCreateInfo.pSetLayouts = &MainMenuDescriptorSetLayout;

	if (vkCreatePipelineLayout (GraphicsDevice, &PipelineCreateInfo, NULL, &MainMenuGraphicsPipelineLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
	}

	return 0;
}

int CreateMainMenuGraphicsPipeline ()
{
	OutputDebugString (L"CreateMainMenuGraphicsPipeline\n");

	VkVertexInputBindingDescription VertexInputBindingDescription[2];
	memset (&VertexInputBindingDescription, 0, sizeof (VkVertexInputBindingDescription) * 2);

	VertexInputBindingDescription[0].binding = 0;
	VertexInputBindingDescription[0].stride = sizeof (float) * 3;
	VertexInputBindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VertexInputBindingDescription[1].binding = 1;
	VertexInputBindingDescription[1].stride = sizeof (float) * 2;
	VertexInputBindingDescription[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription VertexInputAttributeDescriptions[2];
	memset (&VertexInputAttributeDescriptions, 0, sizeof (VkVertexInputAttributeDescription) * 2);

	VertexInputAttributeDescriptions[0].binding = 0;
	VertexInputAttributeDescriptions[0].location = 0;
	VertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescriptions[0].offset = 0;

	VertexInputAttributeDescriptions[1].binding = 1;
	VertexInputAttributeDescriptions[1].location = 1;
	VertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	VertexInputAttributeDescriptions[1].offset = 0;

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo;
	memset (&VertexInputStateCreateInfo, 0, sizeof (VkPipelineVertexInputStateCreateInfo));

	VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputStateCreateInfo.vertexBindingDescriptionCount = 2;
	VertexInputStateCreateInfo.pVertexBindingDescriptions = VertexInputBindingDescription;
	VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
	VertexInputStateCreateInfo.pVertexAttributeDescriptions = VertexInputAttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo;
	memset (&InputAssemblyCreateInfo, 0, sizeof (VkPipelineInputAssemblyStateCreateInfo));

	InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	InputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo;
	memset (&RasterizationStateCreateInfo, 0, sizeof (VkPipelineRasterizationStateCreateInfo));;

	RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	RasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	RasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	RasterizationStateCreateInfo.depthBiasClamp = 0;
	RasterizationStateCreateInfo.depthBiasSlopeFactor = 0;
	RasterizationStateCreateInfo.depthBiasConstantFactor = 0;
	RasterizationStateCreateInfo.lineWidth = 1;

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState;
	memset (&ColorBlendAttachmentState, 0, sizeof (VkPipelineColorBlendAttachmentState));

	ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo;
	memset (&ColorBlendStateCreateInfo, 0, sizeof (VkPipelineColorBlendStateCreateInfo));

	ColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendStateCreateInfo.attachmentCount = 1;
	ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachmentState;
	ColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	ColorBlendStateCreateInfo.blendConstants[0] = 1.f;
	ColorBlendStateCreateInfo.blendConstants[1] = 1.f;
	ColorBlendStateCreateInfo.blendConstants[2] = 1.f;
	ColorBlendStateCreateInfo.blendConstants[3] = 1.f;

	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo;
	memset (&DepthStencilStateCreateInfo, 0, sizeof (VkPipelineDepthStencilStateCreateInfo));

	DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	DepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

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

	VkPipelineViewportStateCreateInfo ViewportStateCreateInfo;
	memset (&ViewportStateCreateInfo, 0, sizeof (VkPipelineViewportStateCreateInfo));

	ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportStateCreateInfo.viewportCount = 1;
	ViewportStateCreateInfo.pViewports = &Viewport;
	ViewportStateCreateInfo.scissorCount = 1;
	ViewportStateCreateInfo.pScissors = &Scissors;

	VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo;
	memset (&MultisampleStateCreateInfo, 0, sizeof (VkPipelineMultisampleStateCreateInfo));

	MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
	MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkGraphicsPipelineCreateInfo));

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

	VkCommandPoolCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkCommandPoolCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CreateInfo.queueFamilyIndex = GraphicsQueueFamilyIndex;

	if (vkCreateCommandPool (GraphicsDevice, &CreateInfo, NULL, &MainMenuCommandPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	VkCommandBufferAllocateInfo AllocateInfo;
	memset (&AllocateInfo, 0, sizeof (VkCommandBufferAllocateInfo));

	AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocateInfo.commandPool = MainMenuCommandPool;
	AllocateInfo.commandBufferCount = SwapchainImageCount;
	AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	MainMenuSwapchainCommandBuffers = (VkCommandBuffer*)malloc (sizeof (VkCommandBuffer) * SwapchainImageCount);

	if (vkAllocateCommandBuffers (GraphicsDevice, &AllocateInfo, MainMenuSwapchainCommandBuffers) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

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

	VkRenderPassBeginInfo RenderPassBeginInfo;
	memset (&RenderPassBeginInfo, 0, sizeof (VkRenderPassBeginInfo));

	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.renderPass = MainMenuRenderPass;
	RenderPassBeginInfo.renderArea.extent = SurfaceExtent;
	RenderPassBeginInfo.clearValueCount = 2;
	RenderPassBeginInfo.pClearValues = ClearValues;

	VkCommandBufferBeginInfo CommandBufferBeginInfo;
	memset (&CommandBufferBeginInfo, 0, sizeof (VkCommandBufferBeginInfo));

	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkDescriptorImageInfo ImageInfo;
	memset (&ImageInfo, 0, sizeof (VkDescriptorImageInfo));
	ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ImageInfo.sampler = MainMenuFallbackSampler;

	VkWriteDescriptorSet DescriptorWrite;
	memset (&DescriptorWrite, 0, sizeof (VkWriteDescriptorSet));

	DescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	DescriptorWrite.dstSet = MainMenuDescriptorSet;
	DescriptorWrite.dstBinding = 1;
	DescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	DescriptorWrite.descriptorCount = 1;

	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		RenderPassBeginInfo.framebuffer = MainMenuSwapchainFramebuffers[i];

		if (vkBeginCommandBuffer (MainMenuSwapchainCommandBuffers[i], &CommandBufferBeginInfo) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		}

		vkCmdBeginRenderPass (MainMenuSwapchainCommandBuffers[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline (MainMenuSwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, MainMenuGraphicsPipeline);
		vkCmdBindDescriptorSets (MainMenuSwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, MainMenuGraphicsPipelineLayout, 0, 1, &MainMenuDescriptorSet, 0, NULL);

		uint32_t MeshCounter = 0;

		for (uint32_t m = 0; m < MainMenuMeshCount; m++)
		{
			for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
			{
				ImageInfo.imageView = *MainMenuMeshes[m].Primitives[p].VkHandles.ImageView;

				DescriptorWrite.pImageInfo = &ImageInfo;
				vkUpdateDescriptorSets (GraphicsDevice, 1, &DescriptorWrite, 0, NULL);

				VkDeviceSize Offsets[3] = { 0, MainMenuMeshes[m].Primitives[p].PositionSize, MainMenuMeshes[m].Primitives[p].PositionSize + MainMenuMeshes[m].Primitives[p].UV0Size };

				vkCmdBindVertexBuffers (MainMenuSwapchainCommandBuffers[i], 0, 1, MainMenuHostVBIBs + MeshCounter, Offsets);
				vkCmdBindVertexBuffers (MainMenuSwapchainCommandBuffers[i], 1, 1, MainMenuHostVBIBs + MeshCounter, Offsets + 1);
				vkCmdBindIndexBuffer (MainMenuSwapchainCommandBuffers[i], MainMenuHostVBIBs[MeshCounter], Offsets[2], VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed (MainMenuSwapchainCommandBuffers[i], MainMenuMeshes[m].Primitives[p].IndexCount, 1, 0, 0, 0);

				++MeshCounter;
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

int CreateMainMenuGraphics ()
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

	Result = CreateMainMenuTextureImages ();

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

	Result = CreateMainMenuCommandPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuCommandBuffers ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int DrawMainMenu ()
{
	return 0;
}

void DestroyMainMenuGraphics ()
{
	OutputDebugString (L"DestroyMainMenu\n");

	if (MainMenuSwapchainCommandBuffers)
	{
		vkFreeCommandBuffers (GraphicsDevice, MainMenuCommandPool, SwapchainImageCount, MainMenuSwapchainCommandBuffers);

		free (MainMenuSwapchainCommandBuffers);
	}

	if (MainMenuCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (GraphicsDevice, MainMenuCommandPool, NULL);
	}

	if (MainMenuDescriptorSet!= VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (GraphicsDevice, MainMenuDescriptorPool, 1, &MainMenuDescriptorSet);
	}

	if (MainMenuDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (GraphicsDevice, MainMenuDescriptorSetLayout, NULL);
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

		free (MainMenuSwapchainFramebuffers);
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

		free (MainMenuTImages);
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

		free (MainMenuTImageViews);
	}

	if (MainMenuNodes)
	{
		free (MainMenuNodes);
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
						free (MainMenuMeshes[m].Primitives[p].Positions);
					}

					if (MainMenuMeshes[m].Primitives[p].UV0s)
					{
						free (MainMenuMeshes[m].Primitives[p].UV0s);
					}

					if (MainMenuMeshes[m].Primitives[p].Indices)
					{
						free (MainMenuMeshes[m].Primitives[p].Indices);
					}
				}

				free (MainMenuMeshes[m].Primitives);
			}
		}

		free (MainMenuMeshes);
	}

	if (MainMenuImages)
	{
		for (uint32_t i = 0; i < MainMenuImageCount; i++)
		{
			if (MainMenuImages[i].Pixels)
			{
				free (MainMenuImages[i].Pixels);
			}
		}

		free (MainMenuImages);
	}

	if (MainMenuTextures)
	{
		free (MainMenuTextures);
	}

	if (MainMenuMaterials)
	{
		free (MainMenuMaterials);
	}
}