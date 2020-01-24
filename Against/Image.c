#include "Image.h"
#include "utils.h"

#include <stb_image.h>
#include <cgltf.h>

int ImportImages (const char* FilePath, Image** Images, uint32_t* ImageCount)
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
				*ImageCount = Data->images_count;
				*Images = (Image*)MyCalloc (Data->images_count, sizeof (Image));

				for (uint32_t i = 0; i < Data->images_count; i++)
				{
					cgltf_image* I = Data->images + i;
					Image* CurrentImage = (*Images) + i;

					strcpy (CurrentImage->Name, I->name);

					char FullTexturePath[256];

					GetFullTexturePathFromURI (FilePath, I->uri, FullTexturePath);
					CurrentImage->Pixels = stbi_load (FullTexturePath, (int*)&CurrentImage->Width, (int*)&CurrentImage->Height, (int*)&CurrentImage->BPP, 4);
					CurrentImage->Size = CurrentImage->Width * CurrentImage->Height * CurrentImage->BPP;
				}
			}
		}
	}

	cgltf_free (Data);

	return 0;
}