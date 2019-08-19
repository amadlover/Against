#include "SplashScreen.h"
#include "Graphics.h"
#include "Error.h"
#include "UBO.h"

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

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

int CreateSplashScreenUniformBuffer ()
{
	OutputDebugString (L"CreateSplashScreenUniformBuffer\n");

	VkBufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkBufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.size = sizeof (MatricesUBO);
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
	errno_t Err = fopen_s (&VertFile, "C:/Users/Nihal Kenkre/Documents/VisualStudio2019/NTKApps/Against/Shaders/vert.spv", "rb");

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
	Err = fopen_s (&FragFile, "C:/Users/Nihal Kenkre/Documents/VisualStudio2019/NTKApps/Against/Shaders/frag.spv", "rb");

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

int SetupSplashScreen ()
{
	OutputDebugString (L"SetupSplashScreen\n");

	int Result = CreateSplashScreenUniformBuffer ();

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

	Result = CreateSplashScreenCommandPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenGraphicsPipelineLayout ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int DrawSplashScreen ()
{
	return 0;
}

void DestroySplashScreen ()
{
	OutputDebugString (L"DestroySplashScreen\n");

	vkDestroyPipelineLayout (GraphicsDevice, GraphicsPipelineLayout, NULL);

	vkFreeCommandBuffers (GraphicsDevice, GraphicsDeviceCommandPool, SwapchainImageCount, SwapchainCommandBuffers);

	vkDestroyCommandPool (GraphicsDevice, GraphicsDeviceCommandPool, NULL);

	vkDestroyShaderModule (GraphicsDevice, VertexShaderModule, NULL);
	vkDestroyShaderModule (GraphicsDevice, FragmentShaderModule, NULL);

	for (uint8_t i = 0; i < SwapchainImageCount; i++)
	{
		vkDestroyFramebuffer (GraphicsDevice, SwapchainFramebuffers[i], NULL);
	}

	vkDestroyRenderPass (GraphicsDevice, RenderPass, NULL);

	vkFreeDescriptorSets (GraphicsDevice, DescriptorPool, 1, &DescriptorSet);
	vkDestroyDescriptorPool (GraphicsDevice, DescriptorPool, NULL);
	vkDestroyDescriptorSetLayout (GraphicsDevice, DescriptorSetLayout, NULL);

	vkFreeMemory (GraphicsDevice, UniformBufferMemory, NULL);
	vkDestroyBuffer (GraphicsDevice, UniformBuffer, NULL);
}