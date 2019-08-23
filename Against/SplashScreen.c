#include "SplashScreen.h"
#include "Graphics.h"
#include "Error.h"
#include "Assets.h"

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

typedef struct _CameraUBO
{
	Matrix4x4 ProjectionMatrix;
	Matrix4x4 ViewMatrix;
	Matrix4x4 ModelMatrix;
} CameraUBO;

VkBuffer UniformBuffer;
VkDeviceMemory UniformBufferMemory;
VkDescriptorSetLayout DescriptorSetLayout;
VkDescriptorPool DescriptorPool;
VkDescriptorSet DescriptorSet;
VkRenderPass RenderPass;
VkShaderModule VertexShaderModule;
VkShaderModule FragmentShaderModule;
VkPipelineShaderStageCreateInfo PipelineShaderStages[2];
VkFramebuffer* SwapchainFramebuffers;
VkCommandPool GraphicsDeviceCommandPool;
VkCommandBuffer* SwapchainCommandBuffers;
VkPipelineLayout GraphicsPipelineLayout;
VkPipeline GraphicsPipeline;
VkFence* SwapchainFences;
VkSemaphore WaitSemaphore;
VkSemaphore SignalSemaphore;

VkBuffer HostVertexBuffer;
VkBuffer HostIndexBuffer;
VkBuffer UniformBuffer;

VkImage TextureImage;

VkDeviceMemory HostVertexBufferMemory;
VkDeviceMemory HostIndexBufferMemory;
VkDeviceMemory UniformBufferMemory;

VkDeviceMemory TextureImageMemory;

Mesh SplashScreenMesh;
CameraUBO CUBO;

int CreateSplashScreenMesh ()
{
	OutputDebugString (L"CreateSplashScreenMesh\n");

	memset (&SplashScreenMesh, 0, sizeof (Mesh));

	SplashScreenMesh.ID = 0;
	SplashScreenMesh.VertexCount = 4;
	SplashScreenMesh.IndexCount = 6;

	SplashScreenMesh.VertexPositions = (float*)malloc (sizeof (float) * SplashScreenMesh.VertexCount * 3);

	SplashScreenMesh.VertexPositions[0] = 2; SplashScreenMesh.VertexPositions[1] = 2; SplashScreenMesh.VertexPositions[2] = 0;
	SplashScreenMesh.VertexPositions[3] = -2; SplashScreenMesh.VertexPositions[4] = 2; SplashScreenMesh.VertexPositions[5] = 0;
	SplashScreenMesh.VertexPositions[6] = -2; SplashScreenMesh.VertexPositions[7] = -2; SplashScreenMesh.VertexPositions[8] = 0;
	SplashScreenMesh.VertexPositions[9] = 2; SplashScreenMesh.VertexPositions[10] = -2; SplashScreenMesh.VertexPositions[11] = 0;

	SplashScreenMesh.Indices = (uint32_t*)malloc (sizeof (uint32_t) * SplashScreenMesh.IndexCount);
	SplashScreenMesh.Indices[0] = 0; SplashScreenMesh.Indices[1] = 1; SplashScreenMesh.Indices[2] = 2;
	SplashScreenMesh.Indices[3] = 0; SplashScreenMesh.Indices[4] = 2; SplashScreenMesh.Indices[5] = 3;

	return 0;
}

int CreateSplashScreenUniformBuffer ()
{
	OutputDebugString (L"CreateSplashScreenUniformBuffer\n");

	VkBufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkBufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.size = sizeof (CameraUBO);
	CreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &UniformBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_CREATE_BUFFER;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, UniformBuffer, &MemoryRequirements);

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

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &UniformBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, UniformBuffer, UniformBufferMemory, 0) != VK_SUCCESS)
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
	DescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
	memset (&DescriptorSetLayoutCreateInfo, 0, sizeof (VkDescriptorSetLayoutCreateInfo));

	DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCreateInfo.bindingCount = 1;
	DescriptorSetLayoutCreateInfo.pBindings = &DescriptorSetLayoutBinding;

	if (vkCreateDescriptorSetLayout (GraphicsDevice, &DescriptorSetLayoutCreateInfo, NULL, &DescriptorSetLayout) != VK_SUCCESS)
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

	DescriptorPoolSize.descriptorCount = 1;
	DescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo;
	memset (&DescriptorPoolCreateInfo, 0, sizeof (VkDescriptorPoolCreateInfo));

	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	DescriptorPoolCreateInfo.poolSizeCount = 1;
	DescriptorPoolCreateInfo.pPoolSizes = &DescriptorPoolSize;
	DescriptorPoolCreateInfo.maxSets = 1;

	if (vkCreateDescriptorPool (GraphicsDevice, &DescriptorPoolCreateInfo, NULL, &DescriptorPool) != VK_SUCCESS)
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
	DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;
	DescriptorSetAllocateInfo.descriptorSetCount = 1;
	DescriptorSetAllocateInfo.pSetLayouts = &DescriptorSetLayout;

	if (vkAllocateDescriptorSets (GraphicsDevice, &DescriptorSetAllocateInfo, &DescriptorSet) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SET;
	}

	VkDescriptorBufferInfo BufferInfo;
	memset (&BufferInfo, 0, sizeof (VkDescriptorBufferInfo));

	BufferInfo.buffer = UniformBuffer;
	BufferInfo.offset = 0;
	BufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet WriteDescriptorSet;
	memset (&WriteDescriptorSet, 0, sizeof (VkWriteDescriptorSet));

	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = DescriptorSet;
	WriteDescriptorSet.dstBinding = 0;
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	WriteDescriptorSet.pBufferInfo = &BufferInfo;

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

	if (vkCreateRenderPass (GraphicsDevice, &CreateInfo, NULL, &RenderPass) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_RENDER_PASS;
	}

	return 0;
}

int CreateSplashScreenShaders ()
{
	OutputDebugString (L"CreateSplashScreenShaders\n");

	FILE* VertFile = NULL;
	errno_t Err = fopen_s (&VertFile, "C:/Users/Nihal Kenkre/Documents/Visual Studio 2019/NTKApps/Against/Shaders/vert.spv", "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (VertFile, 0, SEEK_END);

	uint32_t  FileSize = (uint32_t)ftell (VertFile) / sizeof (uint32_t);
	rewind (VertFile);

	char* Buffer = (char*)malloc (sizeof (uint32_t) * FileSize);
	fread (Buffer, sizeof (uint32_t), FileSize, VertFile);
	fclose (VertFile);

	VkShaderModuleCreateInfo VertexShaderModuleCreateInfo;
	memset (&VertexShaderModuleCreateInfo, 0, sizeof (VkShaderModuleCreateInfo));

	VertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	VertexShaderModuleCreateInfo.pCode = (uint32_t*)Buffer;
	VertexShaderModuleCreateInfo.codeSize = sizeof (uint32_t) * FileSize;

	if (vkCreateShaderModule (GraphicsDevice, &VertexShaderModuleCreateInfo, NULL, &VertexShaderModule) != VK_SUCCESS)
	{
		free (Buffer);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	free (Buffer);

	FILE* FragFile = NULL;
	Err = fopen_s (&FragFile, "C:/Users/Nihal Kenkre/Documents/Visual Studio 2019/NTKApps/Against/Shaders/frag.spv", "rb");

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

	if (vkCreateShaderModule (GraphicsDevice, &FragmentShaderModuleCreateInfo, NULL, &FragmentShaderModule) != VK_SUCCESS)
	{
		free (Buffer);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	free (Buffer);

	VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo;
	memset (&VertexShaderStageCreateInfo, 0, sizeof (VkPipelineShaderStageCreateInfo));

	VertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCreateInfo.module = VertexShaderModule;
	VertexShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageCreateInfo;
	memset (&FragmentShaderStageCreateInfo, 0, sizeof (VkPipelineShaderStageCreateInfo));

	FragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageCreateInfo.module = FragmentShaderModule;
	FragmentShaderStageCreateInfo.pName = "main";

	PipelineShaderStages[0] = VertexShaderStageCreateInfo;
	PipelineShaderStages[1] = FragmentShaderStageCreateInfo;

	return 0;
}

int CreateSplashScreenFramebuffers ()
{
	OutputDebugString (L"CreateSplashScreenFramebuffers\n");

	VkFramebufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkFramebufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	CreateInfo.renderPass = RenderPass;
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

	if (vkCreateCommandPool (GraphicsDevice, &CreateInfo, NULL, &GraphicsDeviceCommandPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	VkCommandBufferAllocateInfo AllocateInfo;
	memset (&AllocateInfo, 0, sizeof (VkCommandBufferAllocateInfo));

	AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocateInfo.commandPool = GraphicsDeviceCommandPool;
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
	PipelineCreateInfo.pSetLayouts = &DescriptorSetLayout;

	if (vkCreatePipelineLayout (GraphicsDevice, &PipelineCreateInfo, NULL, &GraphicsPipelineLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
	}

	return 0;
}

int CreateSplashScreenGraphicsPipeline ()
{
	OutputDebugString (L"CreateSplashScreenGraphicsPipeline\n");

	VkVertexInputBindingDescription VertexInputBindingDescription;
	memset (&VertexInputBindingDescription, 0, sizeof (VkVertexInputBindingDescription));

	VertexInputBindingDescription.binding = 0;
	VertexInputBindingDescription.stride = sizeof (float) * 3;
	VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription VertexInputAttributeDescriptions;
	memset (&VertexInputAttributeDescriptions, 0, sizeof (VkVertexInputAttributeDescription));

	VertexInputAttributeDescriptions.binding = 0;
	VertexInputAttributeDescriptions.location = 0;
	VertexInputAttributeDescriptions.format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescriptions.offset = 0;

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo;
	memset (&VertexInputStateCreateInfo, 0, sizeof (VkPipelineVertexInputStateCreateInfo));

	VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	VertexInputStateCreateInfo.pVertexBindingDescriptions = &VertexInputBindingDescription;
	VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
	VertexInputStateCreateInfo.pVertexAttributeDescriptions = &VertexInputAttributeDescriptions;

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
	RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

	VkPipelineDepthStencilStateCreateInfo DepthStencilStateInfo;
	memset (&DepthStencilStateInfo, 0, sizeof (VkPipelineDepthStencilStateCreateInfo));

	DepthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencilStateInfo.depthTestEnable = VK_TRUE;
	DepthStencilStateInfo.depthWriteEnable = VK_TRUE;
	DepthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	DepthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
	DepthStencilStateInfo.stencilTestEnable = VK_FALSE;

	VkViewport Viewport;
	Viewport.x = 0;
	Viewport.y = 0;
	Viewport.width = (float)SurfaceExtent.width;
	Viewport.height = (float)SurfaceExtent.height;
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

	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo;
	memset (&DepthStencilStateCreateInfo, 0, sizeof (VkPipelineDepthStencilStateCreateInfo));

	DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	DepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

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
	CreateInfo.pStages = PipelineShaderStages;
	CreateInfo.renderPass = RenderPass;
	CreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
	CreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
	CreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
	CreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
	CreateInfo.pViewportState = &ViewportStateCreateInfo;
	CreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
	CreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;

	if (vkCreateGraphicsPipelines (GraphicsDevice, VK_NULL_HANDLE, 1, &CreateInfo, NULL, &GraphicsPipeline) != VK_SUCCESS)
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
	RenderPassBeginInfo.renderPass = RenderPass;
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

		vkCmdBindDescriptorSets (SwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipelineLayout, 0, 1, &DescriptorSet, 0, NULL);
		vkCmdBindPipeline (SwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);

		VkDeviceSize Offset = { 0 };

		vkCmdBindVertexBuffers (SwapchainCommandBuffers[i], 0, 1, &HostVertexBuffer, &Offset);
		vkCmdBindIndexBuffer (SwapchainCommandBuffers[i], HostIndexBuffer, Offset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed (SwapchainCommandBuffers[i], SplashScreenMesh.IndexCount, 1, 0, 0, 0);

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

	if (vkCreateSemaphore (GraphicsDevice, &SemaphoreCreateInfo, NULL, &WaitSemaphore) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE;
	}

	if (vkCreateSemaphore (GraphicsDevice, &SemaphoreCreateInfo, NULL, &SignalSemaphore) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE;
	}

	VkFenceCreateInfo FenceCreateInfo;
	memset (&FenceCreateInfo, 0, sizeof (VkFenceCreateInfo));

	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	SwapchainFences = (VkFence*)malloc (sizeof (VkFence) * SwapchainImageCount);

	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		if (vkCreateFence (GraphicsDevice, &FenceCreateInfo, NULL, &SwapchainFences[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_FENCE;
		}
	}

	return 0;
}

int CreateSplashScreenHostVertexBuffer ()
{
	OutputDebugString (L"CreateSplashScreenHostVertexBuffer\n");

	VkBufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkBufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.size = (uint64_t)sizeof (float) * 3 * SplashScreenMesh.VertexCount;
	CreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &HostVertexBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, HostVertexBuffer, &MemoryRequirements);

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

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &HostVertexBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, HostVertexBuffer, HostVertexBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	void* Data;

	if (vkMapMemory (GraphicsDevice, HostVertexBufferMemory, 0, MemoryRequirements.size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, SplashScreenMesh.VertexPositions, sizeof (float) * SplashScreenMesh.VertexCount * 3);
	vkUnmapMemory (GraphicsDevice, HostVertexBufferMemory);

	return 0;
}

int CreateSplashScreenHostIndexBuffer ()
{
	OutputDebugString (L"CreateSplashScreenHostIndexBuffer\n");

	VkBufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkBufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.size = (uint64_t)sizeof (uint32_t) * SplashScreenMesh.IndexCount;
	CreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &HostIndexBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, HostIndexBuffer, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &HostIndexBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, HostIndexBuffer, HostIndexBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	void* Data;

	if (vkMapMemory (GraphicsDevice, HostIndexBufferMemory, 0, MemoryRequirements.size, 0, (void**)& Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, SplashScreenMesh.Indices, sizeof (uint32_t) * SplashScreenMesh.IndexCount);

	vkUnmapMemory (GraphicsDevice, HostIndexBufferMemory);

	return 0;
}

int CreateSplashScreenHostTextureImage ()
{
	OutputDebugString (L"CreateSplashScreenHostTextureBuffer\n");

	const char* Filename = "C:/Users/Nihal Kenkre/Documents/Visual Studio 2019/NTKApps/Debug/RawImages/bcci.tga.ntk";
	
	int Width, Height, BPP;
	
	FILE* TextureFile;
	fopen_s (&TextureFile, Filename, "rb");
	fread (&Width, sizeof (int), 1, TextureFile);
	fread (&Height, sizeof (int), 1, TextureFile);
	fread (&BPP, sizeof (int), 1, TextureFile);

	uint8_t* Pixels = (uint8_t*)malloc (Width * Height * BPP * sizeof (uint8_t));
	size_t BytesRead = fread (Pixels, sizeof (uint8_t), Width * Height * BPP, TextureFile);
	fclose (TextureFile);

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
	CreateInfo.pQueueFamilyIndices = NULL;
	CreateInfo.queueFamilyIndexCount = 0;
	CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	CreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

	if (vkCreateImage (GraphicsDevice, &CreateInfo, NULL, &TextureImage) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetImageMemoryRequirements (GraphicsDevice, TextureImage, &MemoryRequirements);

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

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &TextureImageMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_IMAGE_MEMORY;
	}

	if (vkBindImageMemory (GraphicsDevice, TextureImage, TextureImageMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
	}

	void* Data = NULL;
	if (vkMapMemory (GraphicsDevice, TextureImageMemory, 0, MemoryRequirements.size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_IMAGE_MEMORY;
	}
	memcpy_s (Data, (const rsize_t)MemoryRequirements.size, Pixels, Width * Height * BPP * sizeof (uint8_t));

	vkUnmapMemory (GraphicsDevice, TextureImageMemory);

	free (Pixels);

	VkCommandBuffer DataCopyCmdBuffer;

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo;
	memset (&CommandBufferAllocateInfo, 0, sizeof (VkCommandBufferAllocateInfo));

	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandPool = GraphicsDeviceCommandPool;
	CommandBufferAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers (GraphicsDevice, &CommandBufferAllocateInfo, &DataCopyCmdBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkCommandBufferBeginInfo DataCopyCmdBufferBeginInfo;
	memset (&DataCopyCmdBufferBeginInfo, 0, sizeof (VkCommandBufferBeginInfo));

	DataCopyCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	DataCopyCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkBeginCommandBuffer (DataCopyCmdBuffer, &DataCopyCmdBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	vkEndCommandBuffer (DataCopyCmdBuffer);

	return 0;
}

int UpdateCameraUniformBuffer ()
{
	OutputDebugString (L"UpdateCameraUniformBuffer\n");

	Matrix4x4 ProjectionMatrix = MatrixGetIdentity ();
	MatrixCreatePerspectiveProjection (60, (float)SurfaceExtent.width / (float)SurfaceExtent.height, 0.1f, 100.f, &ProjectionMatrix);

	CUBO.ProjectionMatrix = ProjectionMatrix;

	Matrix4x4 CameraModelMatrix = MatrixGetIdentity (); 
	
	Vector3 CameraTranslation = { 0,0,10 };
	Vector4 CameraRotation = { 0,0,0,1 };
	Vector3 CameraScale = { 1,1,1 };

	MatrixCreateModelFromQuaternion (CameraTranslation, CameraRotation, CameraScale, &CameraModelMatrix);

	Matrix4x4 ViewMatrix = MatrixGetIdentity ();
	MatrixInverse (CameraModelMatrix, &ViewMatrix);

	CUBO.ViewMatrix = ViewMatrix;

	CUBO.ModelMatrix = MatrixGetIdentity ();

	void* Data;

	if (vkMapMemory (GraphicsDevice, UniformBufferMemory, 0, sizeof (CameraUBO), 0, (void**)& Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, &CUBO, sizeof (CameraUBO));

	vkUnmapMemory (GraphicsDevice, UniformBufferMemory);

	return 0;
}

int SetupSplashScreen ()
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

	Result = CreateSplashScreenFramebuffers ();

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

	Result = CreateSplashScreenHostVertexBuffer ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenHostIndexBuffer ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenCommandPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenHostTextureImage ();

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

	Result = UpdateCameraUniformBuffer ();
	
	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int DrawSplashScreen ()
{
	uint32_t ImageIndex = 0;

	if (vkAcquireNextImageKHR (GraphicsDevice, Swapchain, UINT64_MAX, WaitSemaphore, VK_NULL_HANDLE, &ImageIndex) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE;
	}

	if (vkWaitForFences (GraphicsDevice, 1, &SwapchainFences[ImageIndex], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCE;
	}

	if (vkResetFences (GraphicsDevice, 1, &SwapchainFences[ImageIndex]) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	VkPipelineStageFlags WaitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo SubmitInfo;
	memset (&SubmitInfo, 0, sizeof (VkSubmitInfo));

	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.pWaitDstStageMask = &WaitStageMask;
	SubmitInfo.pWaitSemaphores = &WaitSemaphore;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = &SignalSemaphore;
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pCommandBuffers = &SwapchainCommandBuffers[ImageIndex];
	SubmitInfo.commandBufferCount = 1;

	VkResult Err = vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, SwapchainFences[ImageIndex]);

	if (Err != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	VkPresentInfoKHR PresentInfo;
	memset (&PresentInfo, 0, sizeof (VkPresentInfoKHR));

	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &Swapchain;
	PresentInfo.pImageIndices = &ImageIndex;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = &SignalSemaphore;

	if (vkQueuePresentKHR (GraphicsQueue, &PresentInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_PRESENT;
	}

	return 0;
}

void DestroySplashScreen ()
{
	OutputDebugString (L"DestroySplashScreen\n");

	vkWaitForFences (GraphicsDevice, SwapchainImageCount, SwapchainFences, VK_TRUE, UINT64_MAX);

	for (uint32_t i = 0; i < SwapchainImageCount; i++)
	{
		vkDestroyFence (GraphicsDevice, SwapchainFences[i], NULL);
	}

	free (SwapchainFences);	

	vkDestroySemaphore (GraphicsDevice, WaitSemaphore, NULL);
	vkDestroySemaphore (GraphicsDevice, SignalSemaphore, NULL);

	vkFreeCommandBuffers (GraphicsDevice, GraphicsDeviceCommandPool, SwapchainImageCount, SwapchainCommandBuffers);

	free (SwapchainCommandBuffers);

	vkDestroyCommandPool (GraphicsDevice, GraphicsDeviceCommandPool, NULL);

	vkDestroyPipeline (GraphicsDevice, GraphicsPipeline, NULL);

	vkDestroyPipelineLayout (GraphicsDevice, GraphicsPipelineLayout, NULL);

	vkDestroyShaderModule (GraphicsDevice, VertexShaderModule, NULL);
	vkDestroyShaderModule (GraphicsDevice, FragmentShaderModule, NULL);

	for (uint8_t i = 0; i < SwapchainImageCount; i++)
	{
		vkDestroyFramebuffer (GraphicsDevice, SwapchainFramebuffers[i], NULL);
	}

	free (SwapchainFramebuffers);

	vkDestroyRenderPass (GraphicsDevice, RenderPass, NULL);

	vkFreeMemory (GraphicsDevice, HostVertexBufferMemory, NULL);
	vkFreeMemory (GraphicsDevice, HostIndexBufferMemory, NULL);
	vkFreeMemory (GraphicsDevice, UniformBufferMemory, NULL);

	vkDestroyBuffer (GraphicsDevice, HostVertexBuffer, NULL);
	vkDestroyBuffer (GraphicsDevice, HostIndexBuffer, NULL);
	vkDestroyBuffer (GraphicsDevice, UniformBuffer, NULL);

	vkFreeDescriptorSets (GraphicsDevice, DescriptorPool, 1, &DescriptorSet);
	vkDestroyDescriptorPool (GraphicsDevice, DescriptorPool, NULL);
	vkDestroyDescriptorSetLayout (GraphicsDevice, DescriptorSetLayout, NULL);
}