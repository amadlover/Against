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
				for (uint32_t n = 0; n < Data->nodes_count; n++)
				{
					cgltf_node* Node = Data->nodes + n;

					if (Node->mesh == NULL) continue;

					++(*AssetCount);
				}

				*Assets = (Asset*)calloc ((size_t)*AssetCount, sizeof (Asset));

				for (uint32_t n = 0; n < Data->nodes_count; n++)
				{
					uint32_t CurrentAssetCount = 0;

					cgltf_node* Node = Data->nodes + n;

					if (Node->mesh == NULL) continue;

					if (strstr (Node->name, "CS_") != NULL)
					{
						(*Assets + CurrentAssetCount)->PhysicsPrimitiveCount = Node->mesh->primitives_count;
						(*Assets + CurrentAssetCount)->GraphicsPrimitiveCount = 0;
					}
					else
					{
						(*Assets + CurrentAssetCount)->PhysicsPrimitiveCount = 0;
						(*Assets + CurrentAssetCount)->GraphicsPrimitiveCount = Node->mesh->primitives_count;
					}

					++CurrentAssetCount;
				}
			}
		}
	}

	cgltf_free (Data);

	return 0;
}