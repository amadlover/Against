#include <stdio.h>
#include "log.h"
#include <Windows.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

char file_name_base[256];
char* file_name_suffix = ".txt";
char file_name_timestamp[256];
char old_file_name[256];

FILE* log_file;

int log_init ()
{
	strcpy (file_name_base, getenv ("TMP"));
	strcat (file_name_base, "\\Against\\log");
	
	strcpy (old_file_name, file_name_base);
	strcat (old_file_name, "_live");
	strcat (old_file_name, file_name_suffix);
	
	log_file = fopen (old_file_name, "w");

	if (log_file != NULL)
	{
		return AGAINST_ERROR_SYSTEM_TMP_FILE;
	}

	return 0;
}

int log_info (const char* category, const char* message)
{
	if (log_file == NULL)
	{
		return AGAINST_ERROR_SYSTEM_TMP_FILE;
	}

	char buffer[256];
	strcpy (buffer, category);
	strcat (buffer, ":");
	strcat (buffer, message);
	strcat (buffer, "\n");
	fwrite (buffer, sizeof (char), strlen (buffer), log_file);

	wchar_t buff[256];
	swprintf (buff, 256, L"%hs", buffer);
	OutputDebugString (buff);

	return 0;
}

int log_error (AGAINSTRESULT Result)
{
	switch (Result)
	{
	case AGAINST_ERROR_GRAPHICS_POPULATE_INSTANCE_LAYERS_AND_EXTENSIONS:
		OutputDebugString (L"Graphics Error: Populating instance Layers and Extensions\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_INSTANCE:
		OutputDebugString (L"Graphics Error: Create instance\n");
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
		OutputDebugString (L"Graphics Error: Create surface\n");
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
		OutputDebugString (L"Graphics Error: surface Support\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_SWAPCHAIN:
		OutputDebugString (L"Graphics Error: Create swapchain\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW:
		OutputDebugString (L"Graphics Error: Create Image_Orig View\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_BUFFER:
		OutputDebugString (L"Graphics Error: Create buffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY:
		OutputDebugString (L"Graphics Error: Allocate buffer Memory\n");
		break;

	case AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY:
		OutputDebugString (L"Graphics Error: Bind buffer Memory\n");
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
		OutputDebugString (L"Graphics Error: Begin Command buffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER:
		OutputDebugString (L"Graphics Error: End Command buffer\n");
		break;

	case AGAINST_ERROR_GRAPHICS_CREATE_COMMAND_POOL:
		OutputDebugString (L"Graphics Error: Create Command Pool\n");
		break;

	case AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER:
		OutputDebugString (L"Graphics Error: Allocate Command buffer\n");
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
		OutputDebugString (L"Graphics Error: Update Uniform buffer\n");
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

	case AGAINST_ERROR_SYSTEM_TMP_FILE:
		OutputDebugString (L"System Error: Create TMP File\n");
		break;

	default:
		break;
	}

	return 0;
}

int log_exit ()
{
	if (log_file == NULL)
	{
		return AGAINST_ERROR_SYSTEM_TMP_FILE;
	}

	fclose (log_file);

	strcpy (file_name_timestamp, "_sometime");
	char new_file_name[256];
	strcpy (new_file_name, file_name_base);
	strcat (new_file_name, file_name_timestamp);
	strcat (new_file_name, file_name_suffix);

	rename (old_file_name, new_file_name);

	return 0;
}