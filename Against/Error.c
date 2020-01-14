#include "Error.h"

#include <Windows.h>

void LogError (AGAINSTRESULT Result)
{
	switch (Result)
	{
	case AGAINST_ERROR_GRAPHICS_POPULATE_INSTANCE_LAYERS_AND_EXTENSIONS:
		OutputDebugString (L"Graphics Error: Populating Instance Layers and Extensions\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_INSTANCE:
		OutputDebugString (L"Graphics Error: Create Instance\n");
		break;

	case AGAINST_ERROR_GRAPHICS_SETUP_DEBUG_UTILS_MESSENGER:
		OutputDebugString (L"Graphics Error: Setup Debug Utils Messenger\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER:
		OutputDebugString (L"Graphics Error: Create Debug Utils Messenger\n");
		break;

	case AGAINST_ERROR_GRAPHICS_DESTROY_DEBUG_UTILS_MESSENGER:
		OutputDebugString (L"Graphics Error: Destroy Debud Utils Messenger\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_SURFACE:
		OutputDebugString (L"Graphics Error: Create Surface\n");
		break;

	case AGAINST_ERROR_GRAPHICS_GET_PHYSICAL_DEVICE:
		OutputDebugString (L"Graphics Error: Get Physical Device\n");
		break;

	case AGAINST_ERROR_GRAPHICS_POPULATE_DEVICE_LAYERS_AND_EXTENSIONS:
		OutputDebugString (L"Graphics Error: Populate Device Layers and Extensions\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE:
		OutputDebugString (L"Graphics Error: Create Graphics Device\n");
		break;

	case AGAINST_ERROR_GRAPHICS_SURFACE_SUPPORT:
		OutputDebugString (L"Graphics Error: Surface Support\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_SWAPCHAIN:
		OutputDebugString (L"Graphics Error: Create Swapchain\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW:
		OutputDebugString (L"Graphics Error: Create Image_Orig View\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_BUFFER:
		OutputDebugString (L"Graphics Error: Create Buffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY:
		OutputDebugString (L"Graphics Error: Allocate Buffer Memory\n");
		break;

	case AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY:
		OutputDebugString (L"Graphics Error: Bind Buffer Memory\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_IMAGE:
		OutputDebugString (L"Graphics Error: Create Image_Orig\n");
		break;

	case AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY:
		OutputDebugString (L"Graphics Error: Bind Image_Orig Memory\n");
		break;

	case AGAINST_ERROR_GRAPHICS_MAP_MEMORY:
		OutputDebugString (L"Graphics Error: Map Image_Orig Memory\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT:
		OutputDebugString (L"Graphics Error: Create Descriptor Set Layout\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT:
		OutputDebugString (L"Graphics Error: Create Pipeline Layout\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL:
		OutputDebugString (L"Graphics Error: Create Descriptor Pool\n");
		break;

	case AGAINST_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SET:
		OutputDebugString (L"Graphics Error: Allocate Descriptor Set\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_RENDER_PASS:
		OutputDebugString (L"Graphics Error: Create Render Pass\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE:
		OutputDebugString (L"Graphics Error: Create Shader Module\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_FRAMEBUFFER:
		OutputDebugString (L"Graphics Error: Create Framebuffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER:
		OutputDebugString (L"Graphics Error: Begin Command Buffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER:
		OutputDebugString (L"Graphics Error: End Command Buffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL:
		OutputDebugString (L"Graphics Error: Create Command Pool\n");
		break;

	case AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER:
		OutputDebugString (L"Graphics Error: Allocate Command Buffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE:
		OutputDebugString (L"Graphics Error: Create Graphics Pipeline\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_SEMAPHORE:
		OutputDebugString (L"Graphics Error: Create Semaphore\n");
		break;

	case AGAINST_ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE:
		OutputDebugString (L"Graphics Error: Acquire Next Image_Orig\n");
		break;

	case AGAINST_ERROR_GRAPHICS_WAIT_FOR_FENCES:
		OutputDebugString (L"Graphics Error: Wait for Fence\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_FENCE:
		OutputDebugString (L"Graphics Error: Create Fence\n");
		break;

	case AGAINST_ERROR_GRAPHICS_RESET_FENCE:
		OutputDebugString (L"Graphics Error: Reset Fence\n");
		break;

	case AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT:
		OutputDebugString (L"Graphics Error: Queue Submit\n");
		break;

	case AGAINST_ERROR_GRAPHICS_QUEUE_PRESENT:
		OutputDebugString (L"Graphics Error: Queue Present\n");
		break;

	case AGAINST_ERROR_GRAPHICS_UPDATE_UNIFORM_BUFFER:
		OutputDebugString (L"Graphics Error: Update Uniform Buffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_SAMPLER:
		OutputDebugString (L"Graphics Error: Create Texture_Orig SplashScreenSampler\n");
		break;

	case AGAINST_ERROR_GLTF_IMPORT:
		OutputDebugString (L"GLTF Error: Import GLTF File\n");
		break;

	case AGAINST_ERROR_SYSTEM_ALLOCATE_MEMORY:
		OutputDebugString (L"System Error: Allocate Memory\n");
		break;

	case AGAINST_ERROR_SYSTEM_MORE_THAN_64_IMAGES_TO_PROCESS:
		OutputDebugString (L"System Error: More than 64 Images to Process\n");
		break;

	default:
		break;
	}
}