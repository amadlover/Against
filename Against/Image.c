#include "Image.h"
#include "utils.h"

#include <stb_image.h>
#include <cgltf.h>

int import_images (const char* file_path, image** images, uint32_t* image_count)
{
	cgltf_options pptions = { 0 };
	cgltf_data* data = NULL;

	cgltf_result result = cgltf_parse_file (&pptions, file_path, &data);

	if (result == cgltf_result_success)
	{
		result = cgltf_load_buffers (&pptions, data, file_path);

		if (result == cgltf_result_success)
		{
			result = cgltf_validate (data);

			if (result == cgltf_result_success)
			{
				*image_count = data->images_count;
				*images = (image*)my_calloc (data->images_count, sizeof (image));

				for (uint32_t i = 0; i < data->images_count; i++)
				{
					cgltf_image* I = data->images + i;
					image* current_image = (*images) + i;

					strcpy (current_image->Name, I->name);

					char full_texture_path[256];

					get_full_texture_path_from_uri (file_path, I->uri, full_texture_path);
					current_image->Pixels = stbi_load (full_texture_path, (int*)&current_image->Width, (int*)&current_image->Height, (int*)&current_image->BPP, 4);
					current_image->Size = current_image->Width * current_image->Height * current_image->BPP;
				}
			}
		}
	}

	cgltf_free (data);

	return 0;
}