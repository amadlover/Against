#include "SplashScreen.h"
#include "Graphics.h"
#include "Error.h"
#include "Assets.h"
#include "ImportAssets.h"
#include "Utility.h"

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_ONLY_TGA
#include <stb_image.h>

#include <Shlwapi.h>
#include <strsafe.h>

VkBuffer SplashScreenUniformBuffer;
VkDeviceMemory SplashScreenUniformBufferMemory;
VkDescriptorSetLayout SplashScreenDescriptorSetLayout;
VkDescriptorPool SplashScreenDescriptorPool;
VkDescriptorSet SplashScreenDescriptorSet;
VkRenderPass SplashScreenRenderPass;
VkShaderModule SplashScreenVertexShaderModule;
VkShaderModule SplashScreenFragmentShaderModule;
VkPipelineShaderStageCreateInfo SplashScreenPipelineShaderStages[2];
VkFramebuffer* SwapchainFramebuffers;
VkCommandPool SplashScreenCommandPool;
VkCommandBuffer* SwapchainCommandBuffers;
VkPipelineLayout GraphicsPipelineLayout;
VkPipeline SplashScreenGraphicsPipeline;
VkFence* SplashScreenSwapchainFences;
VkSemaphore SplashScreenWaitSemaphore;
VkSemaphore SplashScreenSignalSemaphore;

VkSampler SplashScreenSampler;

VkBuffer SplashScreenHostVBIB;

VkImage SplashScreenTextureImage;

VkDeviceMemory SplasScreenHostVBIBMemory;
VkDeviceMemory SplashScreenTextureImageMemory;

VkImageView SplashScreenTextureImageView;

Mesh SplashScreenMesh;

//TODO: Use DEVICE_LOCAL memory where possible

int CreateSplashScreenMesh ()
{
	OutputDebugString (L"CreateSplashScreenMesh\n");

	memset (&SplashScreenMesh, 0, sizeof (Mesh));

	SplashScreenMesh.ID = 0;
	strcpy (SplashScreenMesh.Name, "SplashScreenMesh");
	SplashScreenMesh.PrimitiveCount = 1;
	SplashScreenMesh.Primitives = (Primitive*)malloc (sizeof (Primitive));

	SplashScreenMesh.Primitives[0].PositionSize = 4 * sizeof (float) * 3;

	SplashScreenMesh.Primitives[0].UV0Size = 4 * sizeof (float) * 2;

	SplashScreenMesh.Primitives[0].IndexCount = 6;
	SplashScreenMesh.Primitives[0].IndexSize = 6 * sizeof (uint32_t);

	SplashScreenMesh.Primitives[0].Indices = (uint32_t*)malloc (SplashScreenMesh.Primitives[0].IndexSize);
	SplashScreenMesh.Primitives[0].Indices[0] = 0; SplashScreenMesh.Primitives[0].Indices[1] = 1; SplashScreenMesh.Primitives[0].Indices[2] = 2;
	SplashScreenMesh.Primitives[0].Indices[3] = 0; SplashScreenMesh.Primitives[0].Indices[4] = 2; SplashScreenMesh.Primitives[0].Indices[5] = 3;

	SplashScreenMesh.Primitives[0].Positions = (float*)malloc (SplashScreenMesh.Primitives[0].PositionSize);

	SplashScreenMesh.Primitives[0].Positions[0] = 1; SplashScreenMesh.Primitives[0].Positions[1] = 1; SplashScreenMesh.Primitives[0].Positions[2] = 1;
	SplashScreenMesh.Primitives[0].Positions[3] = -1; SplashScreenMesh.Primitives[0].Positions[4] = 1; SplashScreenMesh.Primitives[0].Positions[5] = 1;
	SplashScreenMesh.Primitives[0].Positions[6] = -1; SplashScreenMesh.Primitives[0].Positions[7] = -1; SplashScreenMesh.Primitives[0].Positions[8] = 1;
	SplashScreenMesh.Primitives[0].Positions[9] = 1; SplashScreenMesh.Primitives[0].Positions[10] = -1; SplashScreenMesh.Primitives[0].Positions[11] = 1;

	SplashScreenMesh.Primitives[0].UV0s = (float*)malloc (SplashScreenMesh.Primitives[0].UV0Size);

	SplashScreenMesh.Primitives[0].UV0s[0] = 1; SplashScreenMesh.Primitives[0].UV0s[1] = 1;
	SplashScreenMesh.Primitives[0].UV0s[2] = 0; SplashScreenMesh.Primitives[0].UV0s[3] = 1;
	SplashScreenMesh.Primitives[0].UV0s[4] = 0; SplashScreenMesh.Primitives[0].UV0s[5] = 0;
	SplashScreenMesh.Primitives[0].UV0s[6] = 1; SplashScreenMesh.Primitives[0].UV0s[7] = 0;

	return 0;
}

int CreateSplashScreenUniformBuffer ()
{
	OutputDebugString (L"CreateSplashScreenUniformBuffer\n");

	VkBufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkBufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.size = sizeof (float) * 16;
	CreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &SplashScreenUniformBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, SplashScreenUniformBuffer, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &SplashScreenUniformBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, SplashScreenUniformBuffer, SplashScreenUniformBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	return 0;
}

int CreateSplashScreenDescriptorSetLayout ()
{
	OutputDebugString (L"CreateDescriptorSetLayout\n");

	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding;
	memset (&DescriptorSetLayoutBinding, 0, sizeof (VkDescriptorSetLayoutBinding));

	DescriptorSetLayoutBinding.binding = 0;
	DescriptorSetLayoutBinding.descriptorCount = 1;
	DescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
	memset (&DescriptorSetLayoutCreateInfo, 0, sizeof (VkDescriptorSetLayoutCreateInfo));

	DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCreateInfo.bindingCount = 1;
	DescriptorSetLayoutCreateInfo.pBindings = &DescriptorSetLayoutBinding;

	if (vkCreateDescriptorSetLayout (GraphicsDevice, &DescriptorSetLayoutCreateInfo, NULL, &SplashScreenDescriptorSetLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
	}

	return 0;
}

int CreateSplashScreenDescriptorPool ()
{
	OutputDebugString (L"CreateSplashScreenDescriptorPool\n");

	VkDescriptorPoolSize DescriptorPoolSize;
	memset (&DescriptorPoolSize, 0, sizeof (VkDescriptorPoolSize));

	DescriptorPoolSize.descriptorCount = SwapchainImageCount;
	DescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo;
	memset (&DescriptorPoolCreateInfo, 0, sizeof (VkDescriptorPoolCreateInfo));

	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	DescriptorPoolCreateInfo.poolSizeCount = 1;
	DescriptorPoolCreateInfo.pPoolSizes = &DescriptorPoolSize;
	DescriptorPoolCreateInfo.maxSets = SwapchainImageCount;

	if (vkCreateDescriptorPool (GraphicsDevice, &DescriptorPoolCreateInfo, NULL, &SplashScreenDescriptorPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL;
	}

	return 0;
}

int CreateSplashScreenDescriptorSet ()
{
	OutputDebugString (L"CreateSplashScreenDescriptorSet\n");

	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo;
	memset (&DescriptorSetAllocateInfo, 0, sizeof (VkDescriptorSetAllocateInfo));

	DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocateInfo.descriptorPool = SplashScreenDescriptorPool;
	DescriptorSetAllocateInfo.descriptorSetCount = 1;
	DescriptorSetAllocateInfo.pSetLayouts = &SplashScreenDescriptorSetLayout;

	if (vkAllocateDescriptorSets (GraphicsDevice, &DescriptorSetAllocateInfo, &SplashScreenDescriptorSet) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SET;
	}

	VkDescriptorImageInfo ImageInfo;
	ImageInfo.sampler = SplashScreenSampler;
	ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ImageInfo.imageView = SplashScreenTextureImageView;

	VkWriteDescriptorSet WriteDescriptorSet;
	memset (&WriteDescriptorSet, 0, sizeof (VkWriteDescriptorSet));

	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = SplashScreenDescriptorSet;
	WriteDescriptorSet.dstBinding = 0;
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	WriteDescriptorSet.pImageInfo = &ImageInfo;

	vkUpdateDescriptorSets (GraphicsDevice, 1, &WriteDescriptorSet, 0, NULL);

	return 0;
}

int CreateSplashScreenRenderPass ()
{
	OutputDebugString (L"CreateSplashScreenRenderPass\n");

	VkAttachmentDescription AttachmentDescription;
	memset (&AttachmentDescription, 0, sizeof (VkAttachmentDescription));

	AttachmentDescription.format = ChosenSurfaceFormat.format;
	AttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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

	VkRenderPassCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkRenderPassCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	CreateInfo.subpassCount = 1;
	CreateInfo.pSubpasses = &SubpassDescription;
	CreateInfo.attachmentCount = 1;
	CreateInfo.pAttachments = &AttachmentDescription;

	if (vkCreateRenderPass (GraphicsDevice, &CreateInfo, NULL, &SplashScreenRenderPass) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_RENDER_PASS;
	}

	return 0;
}

int CreateSplashScreenShaders ()
{
	OutputDebugString (L"CreateSplashScreenShaders\n");

	char VertPartialFile[] = "\\Shaders\\SplashScreen\\vert.spv";
	char VertFilename[MAX_PATH];
	GetFullFilePath (VertFilename, VertPartialFile);
	
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

	if (vkCreateShaderModule (GraphicsDevice, &VertexShaderModuleCreateInfo, NULL, &SplashScreenVertexShaderModule) != VK_SUCCESS)
	{
		free (Buffer);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	free (Buffer);

	char FragPartialFilePath[] = "\\Shaders\\SplashScreen\\frag.spv";
	char FragFilename[MAX_PATH];

	GetFullFilePath (FragFilename, FragPartialFilePath);

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

	if (vkCreateShaderModule (GraphicsDevice, &FragmentShaderModuleCreateInfo, NULL, &SplashScreenFragmentShaderModule) != VK_SUCCESS)
	{
		free (Buffer);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	free (Buffer);

	VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo;
	memset (&VertexShaderStageCreateInfo, 0, sizeof (VkPipelineShaderStageCreateInfo));

	VertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCreateInfo.module = SplashScreenVertexShaderModule;
	VertexShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageCreateInfo;
	memset (&FragmentShaderStageCreateInfo, 0, sizeof (VkPipelineShaderStageCreateInfo));

	FragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageCreateInfo.module = SplashScreenFragmentShaderModule;
	FragmentShaderStageCreateInfo.pName = "main";

	SplashScreenPipelineShaderStages[0] = VertexShaderStageCreateInfo;
	SplashScreenPipelineShaderStages[1] = FragmentShaderStageCreateInfo;

	return 0;
}

int CreateSplashScreenFBs ()
{
	OutputDebugString (L"CreateSplashScreenFramebuffers\n");

	VkFramebufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkFramebufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	CreateInfo.renderPass = SplashScreenRenderPass;
	CreateInfo.attachmentCount = 1;
	CreateInfo.width = SurfaceExtent.width;
	CreateInfo.height = SurfaceExtent.height;
	CreateInfo.layers = 1;

	SwapchainFramebuffers = (VkFramebuffer*)malloc (sizeof (VkFramebuffer) * SwapchainImageCount);

	VkImageView Attachment;
	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		Attachment = SwapchainImageViews[i];
		CreateInfo.pAttachments = &Attachment;

		if (vkCreateFramebuffer (GraphicsDevice, &CreateInfo, NULL, &SwapchainFramebuffers[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_FRAMEBUFFER;
		}
	}

	return 0;
}

int CreateSplashScreenCommandPool ()
{
	OutputDebugString (L"CreateSplashScreenCommandPool\n");

	VkCommandPoolCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkCommandPoolCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CreateInfo.queueFamilyIndex = GraphicsQueueFamilyIndex;
	CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool (GraphicsDevice, &CreateInfo, NULL, &SplashScreenCommandPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	VkCommandBufferAllocateInfo AllocateInfo;
	memset (&AllocateInfo, 0, sizeof (VkCommandBufferAllocateInfo));

	AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocateInfo.commandPool = SplashScreenCommandPool;
	AllocateInfo.commandBufferCount = SwapchainImageCount;
	AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	SwapchainCommandBuffers = (VkCommandBuffer*)malloc (sizeof (VkCommandBuffer) * SwapchainImageCount);

	if (vkAllocateCommandBuffers (GraphicsDevice, &AllocateInfo, SwapchainCommandBuffers) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	return 0;
}

int CreateSplashScreenGraphicsPipelineLayout ()
{
	OutputDebugString (L"CreateSplashScreenGraphicsPipelineLayout\n");

	VkPipelineLayoutCreateInfo PipelineCreateInfo;
	memset (&PipelineCreateInfo, 0, sizeof (VkPipelineLayoutCreateInfo));

	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineCreateInfo.setLayoutCount = 1;
	PipelineCreateInfo.pSetLayouts = &SplashScreenDescriptorSetLayout;

	if (vkCreatePipelineLayout (GraphicsDevice, &PipelineCreateInfo, NULL, &GraphicsPipelineLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
	}

	return 0;
}

int CreateSplashScreenGraphicsPipeline ()
{
	OutputDebugString (L"CreateSplashScreenGraphicsPipeline\n");

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
	CreateInfo.layout = GraphicsPipelineLayout;
	CreateInfo.stageCount = 2;
	CreateInfo.pStages = SplashScreenPipelineShaderStages;
	CreateInfo.renderPass = SplashScreenRenderPass;
	CreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
	CreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
	CreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
	CreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
	CreateInfo.pViewportState = &ViewportStateCreateInfo;
	CreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
	CreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;

	if (vkCreateGraphicsPipelines (GraphicsDevice, VK_NULL_HANDLE, 1, &CreateInfo, NULL, &SplashScreenGraphicsPipeline) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE;
	}

	return 0;
}

int CreateSplashScreenCommandBuffer ()
{
	OutputDebugString (L"CreateSplashScreenCommandBuffer\n");

	VkCommandBufferBeginInfo CommandBufferBeginInfo;
	memset (&CommandBufferBeginInfo, 0, sizeof (VkCommandBufferBeginInfo));

	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

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
	RenderPassBeginInfo.renderPass = SplashScreenRenderPass;
	RenderPassBeginInfo.clearValueCount = 2;
	RenderPassBeginInfo.pClearValues = ClearValues;
	RenderPassBeginInfo.renderArea.extent = SurfaceExtent;
	RenderPassBeginInfo.renderArea.offset.x = 0;
	RenderPassBeginInfo.renderArea.offset.y = 0;

	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		RenderPassBeginInfo.framebuffer = SwapchainFramebuffers[i];

		if (vkBeginCommandBuffer (SwapchainCommandBuffers[i], &CommandBufferBeginInfo) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		}

		vkCmdBeginRenderPass (SwapchainCommandBuffers[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindDescriptorSets (SwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipelineLayout, 0, 1, &SplashScreenDescriptorSet, 0, NULL);
		vkCmdBindPipeline (SwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SplashScreenGraphicsPipeline);

		VkDeviceSize Offsets[3] = { 0, SplashScreenMesh.Primitives[0].PositionSize,  SplashScreenMesh.Primitives[0].PositionSize + SplashScreenMesh.Primitives[0].UV0Size };

		vkCmdBindVertexBuffers (SwapchainCommandBuffers[i], 0, 1, &SplashScreenHostVBIB, &Offsets[0]);
		vkCmdBindVertexBuffers (SwapchainCommandBuffers[i], 1, 1, &SplashScreenHostVBIB, &Offsets[1]);
		vkCmdBindIndexBuffer (SwapchainCommandBuffers[i], SplashScreenHostVBIB, Offsets[2], VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed (SwapchainCommandBuffers[i], SplashScreenMesh.Primitives[0].IndexCount, 1, 0, 0, 0);

		vkCmdEndRenderPass (SwapchainCommandBuffers[i]);

		if (vkEndCommandBuffer (SwapchainCommandBuffers[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
		}
	}

	return 0;
}

int CreateSplashScreenSyncObjects ()
{
	OutputDebugString (L"CreateSplashScreenSyncObjects\n");

	VkSemaphoreCreateInfo SemaphoreCreateInfo;
	memset (&SemaphoreCreateInfo, 0, sizeof (VkSemaphoreCreateInfo));

	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore (GraphicsDevice, &SemaphoreCreateInfo, NULL, &SplashScreenWaitSemaphore) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE;
	}

	if (vkCreateSemaphore (GraphicsDevice, &SemaphoreCreateInfo, NULL, &SplashScreenSignalSemaphore) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE;
	}

	VkFenceCreateInfo FenceCreateInfo;
	memset (&FenceCreateInfo, 0, sizeof (VkFenceCreateInfo));

	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	SplashScreenSwapchainFences = (VkFence*)malloc (sizeof (VkFence) * SwapchainImageCount);

	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		if (vkCreateFence (GraphicsDevice, &FenceCreateInfo, NULL, &SplashScreenSwapchainFences[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_FENCE;
		}
	}

	return 0;
}

int CreateSplashScreenHostVBIB ()
{
	OutputDebugString (L"CreateSplashScreenHostVertexBuffers\n");

	VkBufferCreateInfo VBCreateInfo;
	memset (&VBCreateInfo, 0, sizeof (VkBufferCreateInfo));

	VBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VBCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VBCreateInfo.queueFamilyIndexCount = 1;
	VBCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	VBCreateInfo.size = SplashScreenMesh.Primitives[0].PositionSize + SplashScreenMesh.Primitives[0].UV0Size + SplashScreenMesh.Primitives[0].IndexSize;

	if (vkCreateBuffer (GraphicsDevice, &VBCreateInfo, NULL, &SplashScreenHostVBIB) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements VBMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, SplashScreenHostVBIB, &VBMemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = VBMemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (VBMemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &SplasScreenHostVBIBMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, SplashScreenHostVBIB, SplasScreenHostVBIBMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	void* Data;

	if (vkMapMemory (GraphicsDevice, SplasScreenHostVBIBMemory, 0, SplashScreenMesh.Primitives[0].PositionSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, SplashScreenMesh.Primitives[0].Positions, SplashScreenMesh.Primitives[0].PositionSize);
	vkUnmapMemory (GraphicsDevice, SplasScreenHostVBIBMemory);

	if (vkMapMemory (GraphicsDevice, SplasScreenHostVBIBMemory, SplashScreenMesh.Primitives[0].PositionSize, SplashScreenMesh.Primitives[0].UV0Size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, SplashScreenMesh.Primitives[0].UV0s, SplashScreenMesh.Primitives[0].UV0Size);
	vkUnmapMemory (GraphicsDevice, SplasScreenHostVBIBMemory);

	if (vkMapMemory (GraphicsDevice, SplasScreenHostVBIBMemory, SplashScreenMesh.Primitives[0].PositionSize + SplashScreenMesh.Primitives[0].UV0Size, SplashScreenMesh.Primitives[0].IndexSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, SplashScreenMesh.Primitives[0].Indices, SplashScreenMesh.Primitives[0].IndexSize);
	vkUnmapMemory (GraphicsDevice, SplasScreenHostVBIBMemory);

	return 0;
}

int CreateSplashScreenDeviceTextureImage ()
{
	OutputDebugString (L"CreateSplashScreenDeviceTextureImage\n");

	char PartialFilePath[] = "\\Images\\SplashScreen\\SplashScreen.tga";
	char FullFilePath[MAX_PATH];

	GetFullFilePath (FullFilePath, PartialFilePath);

	int Width, Height, BPP;
	
	uint8_t* Pixels = stbi_load ((const char*)FullFilePath, &Width, &Height, &BPP, 0);

	VkBuffer StagingBuffer;

	VkBufferCreateInfo StagingBufferCreateInfo;
	memset (&StagingBufferCreateInfo, 0, sizeof (VkBufferCreateInfo));

	StagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	StagingBufferCreateInfo.queueFamilyIndexCount = 1;
	StagingBufferCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	StagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	StagingBufferCreateInfo.size = Width * Height * BPP * sizeof (float);

	if (vkCreateBuffer (GraphicsDevice, &StagingBufferCreateInfo, NULL, &StagingBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements StagingBufferMemoryRequirments;
	memset (&StagingBufferMemoryRequirments, 0, sizeof (VkMemoryRequirements));

	vkGetBufferMemoryRequirements (GraphicsDevice, StagingBuffer, &StagingBufferMemoryRequirments);

	VkDeviceMemory StagingBufferMemory;
	VkMemoryAllocateInfo StagingBufferAllocateInfo;
	memset (&StagingBufferAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	StagingBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	StagingBufferAllocateInfo.allocationSize = StagingBufferMemoryRequirments.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (StagingBufferMemoryRequirments.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			StagingBufferAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &StagingBufferAllocateInfo, NULL, &StagingBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, StagingBuffer, StagingBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
	}

	void* Data = NULL;
	if (vkMapMemory (GraphicsDevice, StagingBufferMemory, 0, StagingBufferMemoryRequirments.size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}
	memcpy (Data, Pixels, Width * Height * BPP * sizeof (uint8_t));

	vkUnmapMemory (GraphicsDevice, StagingBufferMemory);

	stbi_image_free (Pixels);

	VkImageCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkImageCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	CreateInfo.arrayLayers = 1;

	CreateInfo.extent.width = Width;
	CreateInfo.extent.height = Height;
	CreateInfo.extent.depth = 1;

	CreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	CreateInfo.imageType = VK_IMAGE_TYPE_2D;
	CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	CreateInfo.mipLevels = 1;
	CreateInfo.queueFamilyIndexCount = 1;
	CreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	CreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	if (vkCreateImage (GraphicsDevice, &CreateInfo, NULL, &SplashScreenTextureImage) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetImageMemoryRequirements (GraphicsDevice, SplashScreenTextureImage, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

	RequiredTypes = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &SplashScreenTextureImageMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindImageMemory (GraphicsDevice, SplashScreenTextureImage, SplashScreenTextureImageMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
	}

	VkCommandBuffer LayoutChangeCmdBuffer;

	VkCommandBufferAllocateInfo LayoutChangeCmdBufferAllocateInfo;
	memset (&LayoutChangeCmdBufferAllocateInfo, 0, sizeof (VkCommandBufferAllocateInfo));

	LayoutChangeCmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	LayoutChangeCmdBufferAllocateInfo.commandPool = SplashScreenCommandPool;
	LayoutChangeCmdBufferAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers (GraphicsDevice, &LayoutChangeCmdBufferAllocateInfo, &LayoutChangeCmdBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkCommandBufferBeginInfo LayoutCmdBufferBeginInfo;
	memset (&LayoutCmdBufferBeginInfo, 0, sizeof (VkCommandBufferBeginInfo));

	LayoutCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	LayoutCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer (LayoutChangeCmdBuffer, &LayoutCmdBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	VkImageSubresourceRange SubresourceRange;
	memset (&SubresourceRange, 0, sizeof (VkImageSubresource));

	SubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	SubresourceRange.baseMipLevel = 0;
	SubresourceRange.layerCount = 1;
	SubresourceRange.levelCount = 1;

	VkImageMemoryBarrier MemoryBarrier;
	memset (&MemoryBarrier, 0, sizeof (VkImageMemoryBarrier));

	MemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	MemoryBarrier.image = SplashScreenTextureImage;
	MemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	MemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	MemoryBarrier.srcAccessMask = 0;
	MemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	MemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	MemoryBarrier.subresourceRange.baseMipLevel = 0;
	MemoryBarrier.subresourceRange.levelCount = 1;
	MemoryBarrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier (LayoutChangeCmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &MemoryBarrier);
	vkEndCommandBuffer (LayoutChangeCmdBuffer);

	VkSubmitInfo SubmitInfo;
	memset (&SubmitInfo, 0, sizeof (VkSubmitInfo));

	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &LayoutChangeCmdBuffer;

	VkFence Fence;
	VkFenceCreateInfo FenceCreateInfo;
	memset (&FenceCreateInfo, 0, sizeof (VkFenceCreateInfo));

	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	
	if (vkCreateFence (GraphicsDevice, &FenceCreateInfo, NULL, &Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_FENCE;
	}

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &Fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}
	
	if (vkResetFences (GraphicsDevice, 1, &Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	VkCommandBuffer CopyBufferToImageCmdBuffer;
	VkCommandBufferAllocateInfo CopyBufferToImageCmdBufferAllocateInfo;
	memset (&CopyBufferToImageCmdBufferAllocateInfo, 0, sizeof (VkCommandBufferAllocateInfo));

	CopyBufferToImageCmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CopyBufferToImageCmdBufferAllocateInfo.commandBufferCount = 1;
	CopyBufferToImageCmdBufferAllocateInfo.commandPool = SplashScreenCommandPool;

	if (vkAllocateCommandBuffers (GraphicsDevice, &CopyBufferToImageCmdBufferAllocateInfo, &CopyBufferToImageCmdBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkCommandBufferBeginInfo CopyBufferToImageCmdBufferBeginInfo;
	memset (&CopyBufferToImageCmdBufferBeginInfo, 0, sizeof (VkCommandBufferBeginInfo));

	CopyBufferToImageCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CopyBufferToImageCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer (CopyBufferToImageCmdBuffer, &CopyBufferToImageCmdBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	VkBufferImageCopy BufferImageCopy;
	memset (&BufferImageCopy, 0, sizeof (VkBufferImageCopy));

	BufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	BufferImageCopy.imageSubresource.layerCount = 1;
	BufferImageCopy.bufferOffset = 0;
	BufferImageCopy.bufferImageHeight = 0;
	BufferImageCopy.bufferRowLength = 0;
	BufferImageCopy.imageOffset.x = 0; BufferImageCopy.imageOffset.y = 0; BufferImageCopy.imageOffset.z = 0;
	BufferImageCopy.imageExtent.height = Height;
	BufferImageCopy.imageExtent.width = Width;
	BufferImageCopy.imageExtent.depth = 1;

	vkCmdCopyBufferToImage (CopyBufferToImageCmdBuffer, StagingBuffer, SplashScreenTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
	vkEndCommandBuffer (CopyBufferToImageCmdBuffer);

	SubmitInfo.pCommandBuffers = &CopyBufferToImageCmdBuffer;

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &Fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	if (vkResetFences (GraphicsDevice, 1, &Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	MemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	MemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	MemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	MemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	if (vkResetCommandBuffer (LayoutChangeCmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_COMMAND_BUFFER;
	}

	if (vkBeginCommandBuffer (LayoutChangeCmdBuffer, &LayoutCmdBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	vkCmdPipelineBarrier (LayoutChangeCmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &MemoryBarrier);
	vkEndCommandBuffer (LayoutChangeCmdBuffer);

	SubmitInfo.pCommandBuffers = &LayoutChangeCmdBuffer;

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &Fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	vkDestroyFence (GraphicsDevice, Fence, NULL);

	vkFreeCommandBuffers (GraphicsDevice, SplashScreenCommandPool, 1, &LayoutChangeCmdBuffer);
	vkFreeCommandBuffers (GraphicsDevice, SplashScreenCommandPool, 1, &CopyBufferToImageCmdBuffer);

	vkFreeMemory (GraphicsDevice, StagingBufferMemory, NULL);
	vkDestroyBuffer (GraphicsDevice, StagingBuffer, NULL);

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
	SamplerCreateInfo.compareEnable = VK_FALSE;
	SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

	if (vkCreateSampler (GraphicsDevice, &SamplerCreateInfo, NULL, &SplashScreenSampler) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SAMPLER;
	}

	VkImageViewCreateInfo ImageViewCreateInfo;
	memset (&ImageViewCreateInfo, 0, sizeof (VkImageViewCreateInfo));

	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.image = SplashScreenTextureImage;
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
	
	if (vkCreateImageView (GraphicsDevice, &ImageViewCreateInfo, NULL, &SplashScreenTextureImageView) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
	}

	return 0;
}

int InitSplashScreenGraphics ()
{
	OutputDebugString (L"SetupSplashScreen\n");

	int Result = CreateSplashScreenMesh ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenUniformBuffer ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenRenderPass ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenShaders ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenFBs ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenHostVBIB ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenCommandPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenDeviceTextureImage ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenDescriptorSetLayout ();
	
	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenDescriptorPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenDescriptorSet ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenGraphicsPipelineLayout ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenGraphicsPipeline ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenCommandBuffer ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenSyncObjects ();

	if (Result != 0)
	{
		return Result;
	}
	
	return 0;
}

int DrawSplashScreen (uint64_t ElapsedTime)
{
	uint32_t ImageIndex = 0;

	VkResult Result = vkAcquireNextImageKHR (GraphicsDevice, Swapchain, UINT64_MAX, SplashScreenWaitSemaphore, VK_NULL_HANDLE, &ImageIndex);

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

	VkSubmitInfo SubmitInfo;
	memset (&SubmitInfo, 0, sizeof (VkSubmitInfo));

	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.pWaitDstStageMask = &WaitStageMask;
	SubmitInfo.pWaitSemaphores = &SplashScreenWaitSemaphore;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = &SplashScreenSignalSemaphore;
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pCommandBuffers = &SwapchainCommandBuffers[ImageIndex];
	SubmitInfo.commandBufferCount = 1;

	if (vkResetFences (GraphicsDevice, 1, &SplashScreenSwapchainFences[ImageIndex]) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, SplashScreenSwapchainFences[ImageIndex]) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &SplashScreenSwapchainFences[ImageIndex], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	VkPresentInfoKHR PresentInfo;
	memset (&PresentInfo, 0, sizeof (VkPresentInfoKHR));

	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &Swapchain;
	PresentInfo.pImageIndices = &ImageIndex;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = &SplashScreenSignalSemaphore;

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

void DestroySplashScreenGraphics ()
{
	OutputDebugString (L"DestroySplashScreen\n");

	if (SplashScreenSwapchainFences)
	{
		vkWaitForFences (GraphicsDevice, SwapchainImageCount, SplashScreenSwapchainFences, VK_TRUE, UINT64_MAX);

		for (uint32_t i = 0; i < SwapchainImageCount; i++)
		{
			if (SplashScreenSwapchainFences[i] != VK_NULL_HANDLE)
			{
				vkDestroyFence (GraphicsDevice, SplashScreenSwapchainFences[i], NULL);
			}
		}

		free (SplashScreenSwapchainFences);
	}

	if (SplashScreenWaitSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (GraphicsDevice, SplashScreenWaitSemaphore, NULL);
	}

	if (SplashScreenSignalSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (GraphicsDevice, SplashScreenSignalSemaphore, NULL);
	}

	if (SwapchainCommandBuffers)
	{
		vkFreeCommandBuffers (GraphicsDevice, SplashScreenCommandPool, SwapchainImageCount, SwapchainCommandBuffers);
		free (SwapchainCommandBuffers);
	}

	if (SplashScreenCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (GraphicsDevice, SplashScreenCommandPool, NULL);
	}

	if (SplashScreenGraphicsPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline (GraphicsDevice, SplashScreenGraphicsPipeline, NULL);
	}

	if (GraphicsPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout (GraphicsDevice, GraphicsPipelineLayout, NULL);
	}
	
	if (SplashScreenVertexShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (GraphicsDevice, SplashScreenVertexShaderModule, NULL);
	}
	
	if (SplashScreenFragmentShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (GraphicsDevice, SplashScreenFragmentShaderModule, NULL);
	}
	
	if (SplashScreenRenderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass (GraphicsDevice, SplashScreenRenderPass, NULL);
	}
	
	if (SplasScreenHostVBIBMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, SplasScreenHostVBIBMemory, NULL);
	}

	if (SwapchainFramebuffers)
	{
		for (uint32_t i = 0; i < SwapchainImageCount; i++)
		{
			if (SwapchainFramebuffers[i] != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer (GraphicsDevice, SwapchainFramebuffers[i], NULL);
			}
		}
	}
	
	if (SplashScreenHostVBIB != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, SplashScreenHostVBIB, NULL);
	}
	
	if (SplashScreenTextureImageMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, SplashScreenTextureImageMemory, NULL);
	}

	if (SplashScreenUniformBufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, SplashScreenUniformBufferMemory, NULL);
	}

	if (SplashScreenUniformBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, SplashScreenUniformBuffer, NULL);
	}
	
	if (SplashScreenTextureImage != VK_NULL_HANDLE)
	{
		vkDestroyImage (GraphicsDevice, SplashScreenTextureImage, NULL);
	}
	
	if (SplashScreenSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler (GraphicsDevice, SplashScreenSampler, NULL);
	}

	if (SplashScreenTextureImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView (GraphicsDevice, SplashScreenTextureImageView, NULL);
	}
	
	if (SplashScreenDescriptorPool != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (GraphicsDevice, SplashScreenDescriptorPool, 1, &SplashScreenDescriptorSet);
		vkDestroyDescriptorPool (GraphicsDevice, SplashScreenDescriptorPool, NULL);
	}

	if (SplashScreenDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (GraphicsDevice, SplashScreenDescriptorSetLayout, NULL);
	}

	if (SplashScreenMesh.Primitives)
	{
		if (SplashScreenMesh.Primitives[0].Positions)
		{
			free (SplashScreenMesh.Primitives[0].Positions);
		}

		if (SplashScreenMesh.Primitives[0].UV0s)
		{
			free (SplashScreenMesh.Primitives[0].UV0s);
		}

		if (SplashScreenMesh.Primitives[0].Indices)
		{
			free (SplashScreenMesh.Primitives[0].Indices);
		}

		free (SplashScreenMesh.Primitives);
	}

	OutputDebugString (L"Finished DestroySplashScreen\n");
}