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
			Result = cgltf_load_buffers (&Options, Data, Filename);

			if (Result == cgltf_result_success)
			{

				Result = cgltf_validate (Data);

				if (Result == cgltf_result_success)
				{
					for (int n = 0; n < Data->nodes_count; n++)
					{
						cgltf_node* Node = Data->nodes + n;

						if (Node->has_matrix)
						{
							Mesh* Mesh = Meshes + n;
							Mesh->TransformationMatrix = MatrixCreateF (Node->matrix);
						}
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