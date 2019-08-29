#include "ImportAssets.h"

#include <Windows.h>

void ImportGLTF (const char* Filename)
{
	cgltf_options Options = { 0 };
	cgltf_data* Data = NULL;
}

void ImportGLB (const char* Filename)
{
	cgltf_options Options = { 0 };
	cgltf_data* Data = NULL;

	cgltf_result Result = cgltf_parse_file (&Options, Filename, &Data);

	if (Result == cgltf_result_success)
	{
		for (cgltf_size m = 0; m < Data->meshes_count; m++)
		{
			cgltf_mesh* Mesh = Data->meshes + m;
			
			wchar_t Buffer[32];
			swprintf_s (Buffer, 32, L"%hs\n", Mesh->name);
			OutputDebugString (Buffer);			
		}

		cgltf_free (Data);
	}
}