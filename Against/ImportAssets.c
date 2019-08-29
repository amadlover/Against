#include "ImportAssets.h"

#include <Windows.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

void ImportGLTF (const char* Filename, Mesh* Meshes, size_t* MeshCount)
{
	cgltf_options Options = { 0 };
	cgltf_data* Data = NULL;

	cgltf_result Result = cgltf_parse_file (&Options, Filename, &Data);

	if (Result == cgltf_result_success)
	{
		if (Meshes == NULL)
		{
			*MeshCount = Data->meshes_count;
		}
		else
		{
			cgltf_load_buffers (&Options, Data, Filename);

			for (cgltf_size m = 0; m < Data->meshes_count; m++)
			{
				cgltf_mesh* Mesh = Data->meshes + m;

				wchar_t Buffer[32];
				swprintf_s (Buffer, 32, L"Mesh Name: %hs\n", Mesh->name);
				OutputDebugString (Buffer);

				for (int p = 0; p < Mesh->primitives_count; p++)
				{
					cgltf_primitive* Primitive = Mesh->primitives + p;
					
					for (int a = 0; a < Primitive->attributes_count; a++)
					{
						cgltf_attribute* Attribute = Primitive->attributes + a;

						wchar_t Buffer[32];
						swprintf_s (Buffer, 32, L"Attribute: %hs\n", Attribute->name);
						OutputDebugString (Buffer);
					}
				}
			}
		}

		cgltf_free (Data);
	}
}

void ImportGLB (const char* Filename, Mesh* Meshes, size_t* MeshCount)
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