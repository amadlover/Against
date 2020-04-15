#include "gltf.h"
#include "utils.h"
#include "error.h"
#include "common_graphics.h"
#include "graphics_utils.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <stb_image.h>

cgltf_data** gltf_datas;
size_t num_gltf_datas;

int import_images (const char* file_path, cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    size_t total_size = 0;
    for (size_t i = 0; i < data->images_count; ++i)
    {
        cgltf_image* image = data->images + i;
        int width; int height; int bpp; uint8_t* pixels = NULL;
        read_image_from_uri (file_path, image->uri, &width, &height, &bpp, pixels);

        total_size += width * height * 4;

        stbi_image_free (pixels);
    }

    AGAINSTRESULT result;

    VkBuffer staging_buffer; VkDeviceMemory staging_buffer_memory;
    CHECK_AGAINST_RESULT (graphics_utils_create_buffer (graphics_device, total_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (graphics_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_buffer_memory), result);

    graphics_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_buffer_memory);
    return 0;
}

int import_textures (cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    return 0;
}

int import_materials (cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    return 0;
}

int import_skins (cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    return 0;
}

int import_animations (cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    return 0;
}

int import_graphics_primitives (cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    return 0;
}   

int import_meshes_from_nodes (cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    for (size_t n = 0 ; n < data->nodes_count; ++n)
    {
        if (data->nodes[n].mesh == NULL)
        {
            continue;
        }

        wchar_t name[1024];
        swprintf (name, 1024, L"%hs\n", data->nodes[n].name);
        OutputDebugString (name);
    }

    return 0;
}

int import_gltf_datas (cgltf_data** datas)
{
    return 0;
}

int gather_gltf_datas (const char* partial_file_path, cgltf_data** datas, size_t current_data_index)
{
    cgltf_options options = { 0 };
    cgltf_data* data = NULL;

    char full_file_path[MAX_PATH];
    get_full_file_path (partial_file_path, full_file_path);

    if (cgltf_parse_file (&options, full_file_path, &data) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    if (cgltf_load_buffers (&options, data, full_file_path) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    if (cgltf_validate (data) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    gltf_datas[current_data_index] = data;
    return 0;
}

int import_gltf_files_from_folder (const char* partial_folder_path, gltf_asset_data** out_gltf_data)
{
    file_path* file_paths;
    size_t num_files = 0;
    get_files_in_folder (partial_folder_path, &file_paths, &num_files);

    num_gltf_datas = num_files;
    gltf_datas = (cgltf_data**)my_calloc (num_files, sizeof (cgltf_data*));

    AGAINSTRESULT result;

    for (size_t f = 0 ; f < num_files; ++f)
    {
        char partial_file_path[MAX_PATH];
        strcpy (partial_file_path, partial_folder_path);
        strcat (partial_file_path, file_paths[f].path);
        CHECK_AGAINST_RESULT (gather_gltf_datas (partial_file_path, gltf_datas, f), result);
    }

    my_free (file_paths);

    import_gltf_datas (gltf_datas);

    for (size_t d = 0; d < num_gltf_datas; ++d)
    {
        my_free (gltf_datas[d]);
    }

    return 0;
}