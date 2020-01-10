#include "Asset.h"
#include "Error.h"

#include "GraphicsUtilities.h"

#include <Windows.h>
#include <Shlwapi.h>
#include <strsafe.h>

#include <string.h>

#include <cgltf.h>
#include <stb_image.h>

int GetFullTexturePath (const char* FilePath, const char* URI, char* FullFilePath)
{
	TCHAR TextureFile[MAX_PATH];
	mbstowcs (TextureFile, FilePath, MAX_PATH);

	PathRemoveFileSpec (TextureFile);
	TCHAR Uri[MAX_PATH];
	mbstowcs (Uri, URI, MAX_PATH);
	
	TCHAR URIPath[MAX_PATH];
	StringCchCopy (URIPath, MAX_PATH, L"\\");
	StringCchCat (URIPath, MAX_PATH, Uri);

	StringCchCat (TextureFile, MAX_PATH, URIPath);
	wcstombs (FullFilePath, TextureFile, MAX_PATH);

	return 0;
}

int ImportAssets (const char* FilePath, Asset** Assets, uint32_t* AssetCount)
{
	cgltf_options Options = { 0 };
	cgltf_data* Data = NULL;

	cgltf_result Result = cgltf_parse_file (&Options, FilePath, &Data);

	if (Result == cgltf_result_success)
	{
		Result = cgltf_load_buffers (&Options, Data, FilePath);

		if (Result == cgltf_result_success)
		{
			Result = cgltf_validate (Data);

			if (Result == cgltf_result_success)
			{
				for (uint32_t i = 0; i < Data->images_count; i++)
				{
					cgltf_image* Image = Data->images + i;
					char TexturePath[MAX_PATH];
					GetFullTexturePath (FilePath, (const char*)Image->uri, TexturePath);

					int W, H, BPP;
					stbi_load (TexturePath, &W, &H, &BPP, 4);
				}

				for (uint32_t n = 0; n < Data->nodes_count; n++)
				{
					cgltf_node* Node = Data->nodes + n;

					if (strstr (Node->name, "CS_") != NULL)
					{
						continue;
					}

					for (uint32_t p = 0; p < Node->mesh->primitives_count; p++)
					{
						cgltf_primitive* Primitive = Node->mesh->primitives + p;

						for (uint32_t a = 0; a < Primitive->attributes_count; a++)
						{
							cgltf_attribute* Attribute = Primitive->attributes + a;
						}
					}
				}

			}
		}
	}

	cgltf_free (Data);

	return 0;
}