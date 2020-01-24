#include "splash_screen_graphics.h"
#include "asset.h"
#include "actor.h"
#include "image.h"
#include "error.h"
#include "graphics_utils.h"
#include "utils.h"

#include <stdlib.h>

#include <stb_image.h>

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

Mesh_Orig SplashScreenMesh;

_SplashScreenGraphics* SplashScreenGraphics;

//TODO: Use DEVICE_LOCAL memory where possible

int CreateSplashScreenMesh ()
{
	OutputDebugString (L"CreateSplashScreenMesh\n");

	memset (&SplashScreenMesh, 0, sizeof (Mesh_Orig));

	SplashScreenMesh.ID = 0;
	strcpy (SplashScreenMesh.Name, "SplashScreenMesh");
	SplashScreenMesh.PrimitiveCount = 1;
	SplashScreenMesh.Primitives = (Primitive_Orig*)my_malloc (sizeof (Primitive_Orig));

	SplashScreenMesh.Primitives[0].PositionsSize = 4 * sizeof (float) * 3;

	SplashScreenMesh.Primitives[0].UV0Size = 4 * sizeof (float) * 2;

	SplashScreenMesh.Primitives[0].IndexCount = 6;
	SplashScreenMesh.Primitives[0].IndicesSize = 6 * sizeof (uint32_t);

	SplashScreenMesh.Primitives[0].Indices = (uint32_t*)my_malloc (SplashScreenMesh.Primitives[0].IndicesSize);
	SplashScreenMesh.Primitives[0].Indices[0] = 0; SplashScreenMesh.Primitives[0].Indices[1] = 1; SplashScreenMesh.Primitives[0].Indices[2] = 2;
	SplashScreenMesh.Primitives[0].Indices[3] = 0; SplashScreenMesh.Primitives[0].Indices[4] = 2; SplashScreenMesh.Primitives[0].Indices[5] = 3;

	SplashScreenMesh.Primitives[0].Positions = (float*)my_malloc (SplashScreenMesh.Primitives[0].PositionsSize);

	SplashScreenMesh.Primitives[0].Positions[0] = 1; SplashScreenMesh.Primitives[0].Positions[1] = 1; SplashScreenMesh.Primitives[0].Positions[2] = 1;
	SplashScreenMesh.Primitives[0].Positions[3] = -1; SplashScreenMesh.Primitives[0].Positions[4] = 1; SplashScreenMesh.Primitives[0].Positions[5] = 1;
	SplashScreenMesh.Primitives[0].Positions[6] = -1; SplashScreenMesh.Primitives[0].Positions[7] = -1; SplashScreenMesh.Primitives[0].Positions[8] = 1;
	SplashScreenMesh.Primitives[0].Positions[9] = 1; SplashScreenMesh.Primitives[0].Positions[10] = -1; SplashScreenMesh.Primitives[0].Positions[11] = 1;

	SplashScreenMesh.Primitives[0].UV0s = (float*)my_malloc (SplashScreenMesh.Primitives[0].UV0Size);

	SplashScreenMesh.Primitives[0].UV0s[0] = 1; SplashScreenMesh.Primitives[0].UV0s[1] = 1;
	SplashScreenMesh.Primitives[0].UV0s[2] = 0; SplashScreenMesh.Primitives[0].UV0s[3] = 1;
	SplashScreenMesh.Primitives[0].UV0s[4] = 0; SplashScreenMesh.Primitives[0].UV0s[5] = 0;
	SplashScreenMesh.Primitives[0].UV0s[6] = 1; SplashScreenMesh.Primitives[0].UV0s[7] = 0;

	return 0;
}


int CreateSplashScreenCommandPool ()
{
	OutputDebugString (L"CreateSplashScreenCommandPool\n");

	VkCommandPoolCreateInfo CommandPoolCreateInfo = { 0 };

	CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCreateInfo.queueFamilyIndex = graphics_queue_family_index;
	CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool (graphics_device, &CommandPoolCreateInfo, NULL, &SplashScreenGraphics->CommandPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	return 0;
}

int CreateVulkanHandlesForMeshes (Asset* Assets, uint32_t AssetCount)
{
	OutputDebugString (L"CreateVulkanHandlesForMeshes\n");

	VkDeviceSize TotalSize = 0;

	for (uint32_t a = 0; a < AssetCount; a++)
	{
		Asset* CurrentAsset = Assets + a;

		for (uint32_t gp = 0; gp < CurrentAsset->GraphicsPrimitiveCount; gp++)
		{
			GraphicsPrimitive* CurrentGP = CurrentAsset->GraphicsPrimitives + gp;
			TotalSize += CurrentGP->IndicesSize + CurrentGP->PositionsSize + CurrentGP->UV0sSize + CurrentGP->NormalsSize;
		}
	}

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;

	CreateBuffer (graphics_device,
		TotalSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		graphics_queue_family_index,
		&StagingBuffer);

	AllocateBindBufferMemory (graphics_device, &StagingBuffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &StagingBufferMemory);

	VkDeviceSize Offset = 0;

	for (uint32_t a = 0; a < AssetCount; a++)
	{
		Asset* CurrentAsset = Assets + a;

		for (uint32_t gp = 0; gp < CurrentAsset->GraphicsPrimitiveCount; gp++)
		{
			GraphicsPrimitive* CurrentGP = CurrentAsset->GraphicsPrimitives + gp;

			if (CurrentGP->PositionsSize > 0)
			{
				MapDataToBuffer (graphics_device, StagingBufferMemory, Offset, CurrentGP->PositionsSize, CurrentGP->Positions);
				CurrentGP->PositionsOffset = Offset;
				Offset += CurrentGP->PositionsSize;
			}

			if (CurrentGP->UV0sSize > 0)
			{
				MapDataToBuffer (graphics_device, StagingBufferMemory, Offset, CurrentGP->UV0sSize, CurrentGP->UV0s);
				CurrentGP->UV0sOffset = Offset;
				Offset += CurrentGP->UV0sSize;
			}

			if (CurrentGP->IndicesSize > 0)
			{
				MapDataToBuffer (graphics_device, StagingBufferMemory, Offset, CurrentGP->IndicesSize, CurrentGP->Indices);
				CurrentGP->IndicesOffset = Offset;
				Offset += CurrentGP->IndicesSize;
			}

			if (CurrentGP->NormalsSize > 0)
			{
				MapDataToBuffer (graphics_device, StagingBufferMemory, Offset, CurrentGP->NormalsSize, CurrentGP->Normals);
				CurrentGP->NormalsOffset = Offset;
				Offset += CurrentGP->NormalsSize;
			}
		}
	}

	CreateBuffer (graphics_device,
		TotalSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		graphics_queue_family_index,
		&SplashScreenGraphics->GraphicsVBIBBuffer);

	AllocateBindBufferMemory (graphics_device,
		&SplashScreenGraphics->GraphicsVBIBBuffer,
		1,
		physical_device_memory_properties,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &SplashScreenGraphics->GraphicsVBIBBufferMemory);

	CopyBufferToBuffer (graphics_device,
		SplashScreenGraphics->CommandPool,
		graphics_queue,
		StagingBuffer,
		SplashScreenGraphics->GraphicsVBIBBuffer,
		TotalSize);

	DestroyBufferAndBufferMemory (graphics_device, StagingBuffer, StagingBufferMemory);

	return 0;
}

int CheckForSimilarImages (image* CurrentImage, image* Images, uint32_t ImageCount, uint32_t CheckStartIndex, uint32_t* SimilarImageIndices)
{
	uint32_t SimilarImageIndicesCounter = 1;

	while (CheckStartIndex < ImageCount)
	{
		image* ImageToBeChecked = Images + CheckStartIndex;

		if ((CurrentImage->Width == ImageToBeChecked->Width) && (CurrentImage->Height == ImageToBeChecked->Height))
		{
			SimilarImageIndices[SimilarImageIndicesCounter++] = CheckStartIndex;
		}

		++CheckStartIndex;
	}

	return 0;
}

int CreateVulkanHandlesForImages (image* Images, uint32_t ImageCount)
{
	OutputDebugString (L"CreateVulkanHandlesForImages\n");

	for (uint32_t i = 0; i < ImageCount; i++)
	{
		image* CurrentImage = Images + i;
		uint32_t CheckStartIndex = i + 1;
		uint32_t* SimilarImageIndices = (uint32_t*)my_malloc (ImageCount * sizeof (uint32_t));
		SimilarImageIndices[0] = i;

		CheckForSimilarImages (CurrentImage, Images, ImageCount, CheckStartIndex, SimilarImageIndices);

		my_free (SimilarImageIndices);
	}

	return 0;
}

int CreateVulkanHandles (Asset* Assets, uint32_t AssetCount, image* Images, uint32_t ImageCount)
{
	OutputDebugString (L"CreateVulkanHandles\n");

	CreateVulkanHandlesForMeshes (Assets, AssetCount);
	CreateVulkanHandlesForImages (Images, ImageCount);

	return 0;
}

int CreateSplashScreenUniformBuffer ()
{
	OutputDebugString (L"CreateSplashScreenUniformBuffer\n");

	VkBufferCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.size = sizeof (float) * 16;
	CreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	if (vkCreateBuffer (graphics_device, &CreateInfo, NULL, &SplashScreenUniformBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements (graphics_device, SplashScreenUniformBuffer, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (graphics_device, &MemoryAllocateInfo, NULL, &SplashScreenUniformBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (graphics_device, SplashScreenUniformBuffer, SplashScreenUniformBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	return 0;
}

int CreateSplashScreenDescriptorSetLayout ()
{
	OutputDebugString (L"CreateDescriptorSetLayout\n");

	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding = { 0 };

	DescriptorSetLayoutBinding.binding = 0;
	DescriptorSetLayoutBinding.descriptorCount = 1;
	DescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = { 0 };

	DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCreateInfo.bindingCount = 1;
	DescriptorSetLayoutCreateInfo.pBindings = &DescriptorSetLayoutBinding;

	if (vkCreateDescriptorSetLayout (graphics_device, &DescriptorSetLayoutCreateInfo, NULL, &SplashScreenDescriptorSetLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
	}

	return 0;
}

int CreateSplashScreenDescriptorPool ()
{
	OutputDebugString (L"CreateSplashScreenDescriptorPool\n");

	VkDescriptorPoolSize DescriptorPoolSize = { 0 };

	DescriptorPoolSize.descriptorCount = swapchain_image_count;
	DescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = { 0 };

	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	DescriptorPoolCreateInfo.poolSizeCount = 1;
	DescriptorPoolCreateInfo.pPoolSizes = &DescriptorPoolSize;
	DescriptorPoolCreateInfo.maxSets = swapchain_image_count;

	if (vkCreateDescriptorPool (graphics_device, &DescriptorPoolCreateInfo, NULL, &SplashScreenDescriptorPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL;
	}

	return 0;
}

int CreateSplashScreenDescriptorSet ()
{
	OutputDebugString (L"CreateSplashScreenDescriptorSet\n");

	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = { 0 };

	DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocateInfo.descriptorPool = SplashScreenDescriptorPool;
	DescriptorSetAllocateInfo.descriptorSetCount = 1;
	DescriptorSetAllocateInfo.pSetLayouts = &SplashScreenDescriptorSetLayout;

	if (vkAllocateDescriptorSets (graphics_device, &DescriptorSetAllocateInfo, &SplashScreenDescriptorSet) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SET;
	}

	VkDescriptorImageInfo ImageInfo;
	ImageInfo.sampler = SplashScreenSampler;
	ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ImageInfo.imageView = SplashScreenTextureImageView;

	VkWriteDescriptorSet WriteDescriptorSet = { 0 };

	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = SplashScreenDescriptorSet;
	WriteDescriptorSet.dstBinding = 0;
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	WriteDescriptorSet.pImageInfo = &ImageInfo;

	vkUpdateDescriptorSets (graphics_device, 1, &WriteDescriptorSet, 0, NULL);

	return 0;
}

int CreateSplashScreenRenderPass ()
{
	OutputDebugString (L"CreateSplashScreenRenderPass\n");

	VkAttachmentDescription AttachmentDescription = { 0 };

	AttachmentDescription.format = chosen_surface_format.format;
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

	VkSubpassDescription SubpassDescription = { 0 };

	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.inputAttachmentCount = 0;
	SubpassDescription.preserveAttachmentCount = 0;
	SubpassDescription.colorAttachmentCount = 1;
	SubpassDescription.pColorAttachments = &ColorReference;

	VkRenderPassCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	CreateInfo.subpassCount = 1;
	CreateInfo.pSubpasses = &SubpassDescription;
	CreateInfo.attachmentCount = 1;
	CreateInfo.pAttachments = &AttachmentDescription;

	if (vkCreateRenderPass (graphics_device, &CreateInfo, NULL, &SplashScreenRenderPass) != VK_SUCCESS)
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
	get_full_file_path (VertFilename, VertPartialFile);
	CreateShader (VertFilename, graphics_device, VK_SHADER_STAGE_VERTEX_BIT, &SplashScreenVertexShaderModule, &SplashScreenPipelineShaderStages[0]);

	char FragPartialFilePath[] = "\\Shaders\\SplashScreen\\frag.spv";
	char FragFilename[MAX_PATH];

	get_full_file_path (FragFilename, FragPartialFilePath);
	CreateShader (FragFilename, graphics_device, VK_SHADER_STAGE_FRAGMENT_BIT, &SplashScreenFragmentShaderModule, &SplashScreenPipelineShaderStages[1]);

	return 0;
}

int CreateSplashScreenFBs ()
{
	OutputDebugString (L"CreateSplashScreenFramebuffers\n");

	VkFramebufferCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	CreateInfo.renderPass = SplashScreenRenderPass;
	CreateInfo.attachmentCount = 1;
	CreateInfo.width = surface_exten.width;
	CreateInfo.height = surface_exten.height;
	CreateInfo.layers = 1;

	SwapchainFramebuffers = (VkFramebuffer*)my_malloc (sizeof (VkFramebuffer) * swapchain_image_count);

	VkImageView Attachment;
	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		Attachment = swapchain_imageviews[i];
		CreateInfo.pAttachments = &Attachment;

		if (vkCreateFramebuffer (graphics_device, &CreateInfo, NULL, &SwapchainFramebuffers[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_FRAMEBUFFER;
		}
	}

	return 0;
}

int CreateSplashScreenCommandPool_Orig ()
{
	OutputDebugString (L"CreateSplashScreenCommandPool_Orig\n");

	VkCommandPoolCreateInfo CreateInfo = { 0 };

	CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CreateInfo.queueFamilyIndex = graphics_queue_family_index;
	CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool (graphics_device, &CreateInfo, NULL, &SplashScreenCommandPool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	VkCommandBufferAllocateInfo AllocateInfo = { 0 };

	AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocateInfo.commandPool = SplashScreenCommandPool;
	AllocateInfo.commandBufferCount = swapchain_image_count;
	AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	SwapchainCommandBuffers = (VkCommandBuffer*)my_malloc (sizeof (VkCommandBuffer) * swapchain_image_count);

	if (vkAllocateCommandBuffers (graphics_device, &AllocateInfo, SwapchainCommandBuffers) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	return 0;
}

int CreateSplashScreenGraphicsPipelineLayout ()
{
	OutputDebugString (L"CreateSplashScreenGraphicsPipelineLayout\n");

	VkPipelineLayoutCreateInfo PipelineCreateInfo = { 0 };

	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineCreateInfo.setLayoutCount = 1;
	PipelineCreateInfo.pSetLayouts = &SplashScreenDescriptorSetLayout;

	if (vkCreatePipelineLayout (graphics_device, &PipelineCreateInfo, NULL, &GraphicsPipelineLayout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
	}

	return 0;
}

int CreateSplashScreenGraphicsPipeline ()
{
	OutputDebugString (L"CreateSplashScreenGraphicsPipeline\n");

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
	Viewport.y = (float)surface_exten.height;
	Viewport.width = (float)surface_exten.width;
	Viewport.height = -(float)surface_exten.height;
	Viewport.minDepth = 0;
	Viewport.maxDepth = 1;

	VkRect2D Scissors;
	Scissors.offset.x = 0;
	Scissors.offset.y = 0;
	Scissors.extent = surface_exten;

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

	if (vkCreateGraphicsPipelines (graphics_device, VK_NULL_HANDLE, 1, &CreateInfo, NULL, &SplashScreenGraphicsPipeline) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE;
	}

	return 0;
}

int CreateSplashScreenCommandBuffer ()
{
	OutputDebugString (L"CreateSplashScreenCommandBuffer\n");

	VkCommandBufferBeginInfo CommandBufferBeginInfo = { 0 };

	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkClearValue ClearValues[2];
	ClearValues[0].color.float32[0] = 0;
	ClearValues[0].color.float32[1] = 0;
	ClearValues[0].color.float32[2] = 0;
	ClearValues[0].color.float32[3] = 1;

	ClearValues[1].depthStencil.depth = 1;
	ClearValues[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo RenderPassBeginInfo = { 0 };

	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.renderPass = SplashScreenRenderPass;
	RenderPassBeginInfo.clearValueCount = 2;
	RenderPassBeginInfo.pClearValues = ClearValues;
	RenderPassBeginInfo.renderArea.extent = surface_exten;
	RenderPassBeginInfo.renderArea.offset.x = 0;
	RenderPassBeginInfo.renderArea.offset.y = 0;

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		RenderPassBeginInfo.framebuffer = SwapchainFramebuffers[i];

		if (vkBeginCommandBuffer (SwapchainCommandBuffers[i], &CommandBufferBeginInfo) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		}

		vkCmdBeginRenderPass (SwapchainCommandBuffers[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindDescriptorSets (SwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipelineLayout, 0, 1, &SplashScreenDescriptorSet, 0, NULL);
		vkCmdBindPipeline (SwapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SplashScreenGraphicsPipeline);

		VkDeviceSize Offsets[3] = { 0, SplashScreenMesh.Primitives[0].PositionsSize,  SplashScreenMesh.Primitives[0].PositionsSize + SplashScreenMesh.Primitives[0].UV0Size };

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

	VkSemaphoreCreateInfo SemaphoreCreateInfo = { 0 };

	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore (graphics_device, &SemaphoreCreateInfo, NULL, &SplashScreenWaitSemaphore) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE;
	}

	if (vkCreateSemaphore (graphics_device, &SemaphoreCreateInfo, NULL, &SplashScreenSignalSemaphore) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE;
	}

	VkFenceCreateInfo FenceCreateInfo = { 0 };

	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	SplashScreenSwapchainFences = (VkFence*)my_malloc (sizeof (VkFence) * swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		if (vkCreateFence (graphics_device, &FenceCreateInfo, NULL, &SplashScreenSwapchainFences[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_FENCE;
		}
	}

	return 0;
}

int CreateSplashScreenHostVBIB ()
{
	OutputDebugString (L"CreateSplashScreenHostVertexBuffers\n");

	VkBufferCreateInfo VBCreateInfo = { 0 };

	VBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VBCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VBCreateInfo.queueFamilyIndexCount = 1;
	VBCreateInfo.pQueueFamilyIndices = &graphics_queue_family_index;
	VBCreateInfo.size = SplashScreenMesh.Primitives[0].PositionsSize + SplashScreenMesh.Primitives[0].UV0Size + SplashScreenMesh.Primitives[0].IndicesSize;

	if (vkCreateBuffer (graphics_device, &VBCreateInfo, NULL, &SplashScreenHostVBIB) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements VBMemoryRequirements;
	vkGetBufferMemoryRequirements (graphics_device, SplashScreenHostVBIB, &VBMemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = VBMemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (VBMemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (graphics_device, &MemoryAllocateInfo, NULL, &SplasScreenHostVBIBMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (graphics_device, SplashScreenHostVBIB, SplasScreenHostVBIBMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	void* Data;

	if (vkMapMemory (graphics_device, SplasScreenHostVBIBMemory, 0, SplashScreenMesh.Primitives[0].PositionsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, SplashScreenMesh.Primitives[0].Positions, SplashScreenMesh.Primitives[0].PositionsSize);
	vkUnmapMemory (graphics_device, SplasScreenHostVBIBMemory);

	if (vkMapMemory (graphics_device, SplasScreenHostVBIBMemory, SplashScreenMesh.Primitives[0].PositionsSize, SplashScreenMesh.Primitives[0].UV0Size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, SplashScreenMesh.Primitives[0].UV0s, SplashScreenMesh.Primitives[0].UV0Size);
	vkUnmapMemory (graphics_device, SplasScreenHostVBIBMemory);

	if (vkMapMemory (graphics_device, SplasScreenHostVBIBMemory, SplashScreenMesh.Primitives[0].PositionsSize + SplashScreenMesh.Primitives[0].UV0Size, SplashScreenMesh.Primitives[0].IndicesSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, SplashScreenMesh.Primitives[0].Indices, SplashScreenMesh.Primitives[0].IndicesSize);
	vkUnmapMemory (graphics_device, SplasScreenHostVBIBMemory);

	return 0;
}

int CreateSplashScreenDeviceTextureImage ()
{
	OutputDebugString (L"CreateSplashScreenDeviceTextureImage\n");

	char PartialFilePath[] = "\\Images\\SplashScreen\\SplashScreen.tga";
	char FullFilePath[MAX_PATH];

	get_full_file_path (FullFilePath, PartialFilePath);

	int Width, Height, BPP;

	uint8_t* Pixels = stbi_load ((const char*)FullFilePath, &Width, &Height, &BPP, 0);

	VkBuffer StagingBuffer;

	VkBufferCreateInfo StagingBufferCreateInfo = { 0 };

	StagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	StagingBufferCreateInfo.queueFamilyIndexCount = 1;
	StagingBufferCreateInfo.pQueueFamilyIndices = &graphics_queue_family_index;
	StagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	StagingBufferCreateInfo.size = (VkDeviceSize)Width * (VkDeviceSize)Height * (VkDeviceSize)BPP;

	if (vkCreateBuffer (graphics_device, &StagingBufferCreateInfo, NULL, &StagingBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements StagingBufferMemoryRequirments = { 0 };

	vkGetBufferMemoryRequirements (graphics_device, StagingBuffer, &StagingBufferMemoryRequirments);

	VkDeviceMemory StagingBufferMemory;
	VkMemoryAllocateInfo StagingBufferAllocateInfo = { 0 };

	StagingBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	StagingBufferAllocateInfo.allocationSize = StagingBufferMemoryRequirments.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (StagingBufferMemoryRequirments.memoryTypeBits & (1 << i) && RequiredTypes & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			StagingBufferAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (graphics_device, &StagingBufferAllocateInfo, NULL, &StagingBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (graphics_device, StagingBuffer, StagingBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
	}

	void* Data = NULL;
	if (vkMapMemory (graphics_device, StagingBufferMemory, 0, StagingBufferMemoryRequirments.size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}
	memcpy (Data, Pixels, Width * Height * BPP);

	vkUnmapMemory (graphics_device, StagingBufferMemory);

	stbi_image_free (Pixels);

	VkImageCreateInfo CreateInfo = { 0 };

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
	CreateInfo.pQueueFamilyIndices = &graphics_queue_family_index;
	CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	CreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	if (vkCreateImage (graphics_device, &CreateInfo, NULL, &SplashScreenTextureImage) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetImageMemoryRequirements (graphics_device, SplashScreenTextureImage, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

	RequiredTypes = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (graphics_device, &MemoryAllocateInfo, NULL, &SplashScreenTextureImageMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindImageMemory (graphics_device, SplashScreenTextureImage, SplashScreenTextureImageMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
	}

	VkCommandBuffer LayoutChangeCmdBuffer;

	VkCommandBufferAllocateInfo LayoutChangeCmdBufferAllocateInfo = { 0 };

	LayoutChangeCmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	LayoutChangeCmdBufferAllocateInfo.commandPool = SplashScreenCommandPool;
	LayoutChangeCmdBufferAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers (graphics_device, &LayoutChangeCmdBufferAllocateInfo, &LayoutChangeCmdBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkCommandBufferBeginInfo LayoutCmdBufferBeginInfo = { 0 };

	LayoutCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	LayoutCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer (LayoutChangeCmdBuffer, &LayoutCmdBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	VkImageMemoryBarrier MemoryBarrier = { 0 };

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

	VkSubmitInfo SubmitInfo = { 0 };

	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &LayoutChangeCmdBuffer;

	VkFence Fence;
	VkFenceCreateInfo FenceCreateInfo = { 0 };

	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if (vkCreateFence (graphics_device, &FenceCreateInfo, NULL, &Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_FENCE;
	}

	if (vkQueueSubmit (graphics_queue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (graphics_device, 1, &Fence, 1, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	if (vkResetFences (graphics_device, 1, &Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	VkCommandBuffer CopyBufferToImageCmdBuffer;
	VkCommandBufferAllocateInfo CopyBufferToImageCmdBufferAllocateInfo = { 0 };

	CopyBufferToImageCmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CopyBufferToImageCmdBufferAllocateInfo.commandBufferCount = 1;
	CopyBufferToImageCmdBufferAllocateInfo.commandPool = SplashScreenCommandPool;

	if (vkAllocateCommandBuffers (graphics_device, &CopyBufferToImageCmdBufferAllocateInfo, &CopyBufferToImageCmdBuffer) != VK_SUCCESS)
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
	BufferImageCopy.imageExtent.height = Height;
	BufferImageCopy.imageExtent.width = Width;
	BufferImageCopy.imageExtent.depth = 1;

	vkCmdCopyBufferToImage (CopyBufferToImageCmdBuffer, StagingBuffer, SplashScreenTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
	vkEndCommandBuffer (CopyBufferToImageCmdBuffer);

	SubmitInfo.pCommandBuffers = &CopyBufferToImageCmdBuffer;

	if (vkQueueSubmit (graphics_queue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (graphics_device, 1, &Fence, 1, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	if (vkResetFences (graphics_device, 1, &Fence) != VK_SUCCESS)
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

	if (vkQueueSubmit (graphics_queue, 1, &SubmitInfo, Fence) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (graphics_device, 1, &Fence, 1, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	vkDestroyFence (graphics_device, Fence, NULL);

	vkFreeCommandBuffers (graphics_device, SplashScreenCommandPool, 1, &LayoutChangeCmdBuffer);
	vkFreeCommandBuffers (graphics_device, SplashScreenCommandPool, 1, &CopyBufferToImageCmdBuffer);

	vkFreeMemory (graphics_device, StagingBufferMemory, NULL);
	vkDestroyBuffer (graphics_device, StagingBuffer, NULL);

	VkSamplerCreateInfo SamplerCreateInfo = { 0 };

	SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.mipLodBias = 0;
	SamplerCreateInfo.compareEnable = 0;
	SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

	if (vkCreateSampler (graphics_device, &SamplerCreateInfo, NULL, &SplashScreenSampler) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SAMPLER;
	}

	VkImageViewCreateInfo ImageViewCreateInfo = { 0 };

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

	if (vkCreateImageView (graphics_device, &ImageViewCreateInfo, NULL, &SplashScreenTextureImageView) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
	}

	return 0;
}

int DrawSplashScreenGraphics ()
{
	uint32_t ImageIndex = 0;

	VkResult Result = vkAcquireNextImageKHR (graphics_device, swapchain, UINT64_MAX, SplashScreenWaitSemaphore, VK_NULL_HANDLE, &ImageIndex);

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
	SubmitInfo.pWaitSemaphores = &SplashScreenWaitSemaphore;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = &SplashScreenSignalSemaphore;
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pCommandBuffers = &SwapchainCommandBuffers[ImageIndex];
	SubmitInfo.commandBufferCount = 1;

	if (vkResetFences (graphics_device, 1, &SplashScreenSwapchainFences[ImageIndex]) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_RESET_FENCE;
	}

	if (vkQueueSubmit (graphics_queue, 1, &SubmitInfo, SplashScreenSwapchainFences[ImageIndex]) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	if (vkWaitForFences (graphics_device, 1, &SplashScreenSwapchainFences[ImageIndex], 1, UINT64_MAX) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	VkPresentInfoKHR PresentInfo = { 0 };

	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &swapchain;
	PresentInfo.pImageIndices = &ImageIndex;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = &SplashScreenSignalSemaphore;

	Result = vkQueuePresentKHR (graphics_queue, &PresentInfo);

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

int InitSplashScreenGraphics (Asset* Assets, uint32_t AssetCount, image* Images, uint32_t ImageCount)
{
	OutputDebugString (L"SetupSplashScreen\n");

	SplashScreenGraphics = (_SplashScreenGraphics*)my_calloc (1, sizeof (_SplashScreenGraphics));

	int Result = CreateSplashScreenMesh ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateSplashScreenCommandPool ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateVulkanHandles (Assets, AssetCount, Images, ImageCount);

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

	Result = CreateSplashScreenCommandPool_Orig ();

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


void DestroySplashScreenGraphics ()
{
	OutputDebugString (L"splash_screen_exit\n");

	if (SplashScreenSwapchainFences)
	{
		vkWaitForFences (graphics_device, swapchain_image_count, SplashScreenSwapchainFences, 1, UINT64_MAX);

		for (uint32_t i = 0; i < swapchain_image_count; i++)
		{
			if (SplashScreenSwapchainFences[i] != VK_NULL_HANDLE)
			{
				vkDestroyFence (graphics_device, SplashScreenSwapchainFences[i], NULL);
			}
		}

		my_free (SplashScreenSwapchainFences);
	}

	if (SplashScreenWaitSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (graphics_device, SplashScreenWaitSemaphore, NULL);
	}

	if (SplashScreenSignalSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (graphics_device, SplashScreenSignalSemaphore, NULL);
	}

	if (SwapchainCommandBuffers)
	{
		vkFreeCommandBuffers (graphics_device, SplashScreenCommandPool, swapchain_image_count, SwapchainCommandBuffers);
		my_free (SwapchainCommandBuffers);
	}

	if (SplashScreenCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (graphics_device, SplashScreenCommandPool, NULL);
	}

	if (SplashScreenGraphicsPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline (graphics_device, SplashScreenGraphicsPipeline, NULL);
	}

	if (GraphicsPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout (graphics_device, GraphicsPipelineLayout, NULL);
	}

	if (SplashScreenVertexShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (graphics_device, SplashScreenVertexShaderModule, NULL);
	}

	if (SplashScreenFragmentShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (graphics_device, SplashScreenFragmentShaderModule, NULL);
	}

	if (SplashScreenRenderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass (graphics_device, SplashScreenRenderPass, NULL);
	}

	if (SplasScreenHostVBIBMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (graphics_device, SplasScreenHostVBIBMemory, NULL);
	}

	if (SwapchainFramebuffers)
	{
		for (uint32_t i = 0; i < swapchain_image_count; i++)
		{
			if (SwapchainFramebuffers[i] != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer (graphics_device, SwapchainFramebuffers[i], NULL);
			}
		}
	}

	if (SplashScreenHostVBIB != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (graphics_device, SplashScreenHostVBIB, NULL);
	}

	if (SplashScreenTextureImageMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (graphics_device, SplashScreenTextureImageMemory, NULL);
	}

	if (SplashScreenUniformBufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (graphics_device, SplashScreenUniformBufferMemory, NULL);
	}

	if (SplashScreenUniformBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (graphics_device, SplashScreenUniformBuffer, NULL);
	}

	if (SplashScreenTextureImage != VK_NULL_HANDLE)
	{
		vkDestroyImage (graphics_device, SplashScreenTextureImage, NULL);
	}

	if (SplashScreenSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler (graphics_device, SplashScreenSampler, NULL);
	}

	if (SplashScreenTextureImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView (graphics_device, SplashScreenTextureImageView, NULL);
	}

	if (SplashScreenDescriptorPool != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (graphics_device, SplashScreenDescriptorPool, 1, &SplashScreenDescriptorSet);
		vkDestroyDescriptorPool (graphics_device, SplashScreenDescriptorPool, NULL);
	}

	if (SplashScreenDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (graphics_device, SplashScreenDescriptorSetLayout, NULL);
	}

	if (SplashScreenMesh.Primitives)
	{
		if (SplashScreenMesh.Primitives[0].Positions)
		{
			my_free (SplashScreenMesh.Primitives[0].Positions);
		}

		if (SplashScreenMesh.Primitives[0].UV0s)
		{
			my_free (SplashScreenMesh.Primitives[0].UV0s);
		}

		if (SplashScreenMesh.Primitives[0].Indices)
		{
			my_free (SplashScreenMesh.Primitives[0].Indices);
		}

		my_free (SplashScreenMesh.Primitives);
	}

	if (SplashScreenGraphics)
	{
		if (SplashScreenGraphics->CommandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool (graphics_device, SplashScreenGraphics->CommandPool, NULL);
		}

		if (SplashScreenGraphics->GraphicsVBIBBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer (graphics_device, SplashScreenGraphics->GraphicsVBIBBuffer, NULL);
		}

		if (SplashScreenGraphics->GraphicsVBIBBufferMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory (graphics_device, SplashScreenGraphics->GraphicsVBIBBufferMemory, NULL);
		}

		if (SplashScreenGraphics->GraphicsImageMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory (graphics_device, SplashScreenGraphics->GraphicsImageMemory, NULL);
		}

		for (uint32_t i = 0; i < SplashScreenGraphics->ImageCount; i++)
		{
			vkDestroyImage (graphics_device, SplashScreenGraphics->Images[i], NULL);
		}

		for (uint32_t i = 0; i < SplashScreenGraphics->ImageViewCount; i++)
		{
			vkDestroyImageView (graphics_device, SplashScreenGraphics->ImageViews[i], NULL);
		}

		my_free (SplashScreenGraphics);
	}

	OutputDebugString (L"Finished splash_screen_exit\n");
}