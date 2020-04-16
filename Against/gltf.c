#include "gltf.h"
#include "utils.h"
#include "error.h"
#include "common_graphics.h"
#include "graphics_utils.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <stb_image.h>

cgltf_data** gltf_datas = NULL;
size_t num_gltf_datas = 0;

cgltf_image** ref_cgltf_images = NULL;
size_t num_images = 0;

int import_images (const char* full_folder_path, cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    size_t total_image_size = 0;

    size_t* img_offsets = NULL;
    size_t* img_sizes = NULL;
    size_t* img_widths = NULL;
    size_t* img_heights = NULL;
    uint8_t** img_pixels = NULL;

    AGAINSTRESULT result;
    for (size_t d = 0; d < num_datas; ++d)
    {
        if (out_data->images == NULL)
        {
            out_data->images = (VkImage*)my_calloc (datas[d]->images_count, sizeof (VkImage));
        }
        else
        {
            out_data->images = (VkImage*)my_realloc (out_data->images, sizeof (VkImage) * (num_images + datas[d]->images_count));
        }

        if (out_data->image_views == NULL)
        {
            out_data->image_views = (VkImageView*)my_calloc (datas[d]->images_count, sizeof (VkImageView));
        }
        else
        {
            out_data->image_views = (VkImageView*)my_realloc (out_data->image_views, sizeof (VkImageView) * (num_images + datas[d]->images_count));
        }

        if (img_offsets == NULL)
        {
            img_offsets = (size_t*)my_calloc (datas[d]->images_count, sizeof (size_t));
        }
        else
        {
            img_offsets = (size_t*)my_realloc (img_offsets, sizeof (size_t) * (num_images + datas[d]->images_count));
        }

        if (img_sizes == NULL)
        {
            img_sizes = (size_t*)my_calloc (datas[d]->images_count, sizeof (size_t));
        }
        else
        {
            img_sizes = (size_t*)my_realloc (img_sizes, sizeof (size_t) * (num_images + datas[d]->images_count));
        }

        if (img_widths == NULL)
        {
            img_widths = (size_t*)my_calloc (datas[d]->images_count, sizeof (size_t));
        }
        else
        {
            img_widths = (size_t*)my_realloc (img_widths, sizeof (size_t) * (num_images + datas[d]->images_count));
        }

        if (img_heights == NULL)
        {
            img_heights = (size_t*)my_calloc (datas[d]->images_count, sizeof (size_t));
        }
        else
        {
            img_heights = (size_t*)my_realloc (img_heights, sizeof (size_t) * (num_images + datas[d]->images_count));
        }

        if (img_pixels == NULL)
        {
            img_pixels = (uint8_t**)my_calloc (datas[d]->images_count, sizeof (uint8_t*));
        }
        else
        {
            img_pixels = (uint8_t**)my_realloc (img_pixels, sizeof (uint8_t*) * (num_images + datas[d]->images_count));
        }

        if (ref_cgltf_images == NULL)
        {
            ref_cgltf_images = (cgltf_image**)my_calloc (datas[d]->images_count, sizeof (cgltf_image*));
        }
        else
        {
            ref_cgltf_images = (cgltf_image**)my_realloc (ref_cgltf_images, sizeof (cgltf_image*) * (datas[d]->images_count + num_images));
        }

        for (size_t i = 0; i < datas[d]->images_count; ++i)
        {
            size_t current_index = num_images + i;

            cgltf_image* image = datas[d]->images + i;
            char full_texture_path[MAX_PATH];
            get_full_texture_path_from_uri (full_folder_path, image->uri, full_texture_path);
            int bpp;
            img_pixels[current_index] = stbi_load (full_texture_path, img_widths + current_index, img_heights + current_index, &bpp, 4);

            VkExtent3D image_extent = { img_widths[current_index], img_heights[current_index], 1 };
            CHECK_AGAINST_RESULT (graphics_utils_create_image (graphics_device, graphics_queue_family_index, image_extent, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_SHARING_MODE_EXCLUSIVE, &out_data->images[current_index]), result);
            img_sizes[current_index] = img_widths[current_index] * img_heights[current_index] * 4;
            total_image_size += img_sizes[current_index];
            img_offsets[current_index] = current_index > 0 ? total_image_size - img_sizes[current_index] : 0;

            ref_cgltf_images[current_index] = image;
        }

        num_images += datas[d]->images_count;
    }

    VkBuffer staging_buffer; VkDeviceMemory staging_memory;
    CHECK_AGAINST_RESULT (graphics_utils_create_buffer (graphics_device, total_image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (graphics_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_memory), result);

    for (size_t i = 0; i < num_images; ++i)
    {
        CHECK_AGAINST_RESULT (graphics_utils_map_data_to_device_memory (graphics_device, staging_memory, img_offsets[i], img_sizes[i], img_pixels[i]), result);
        CHECK_AGAINST_RESULT (graphics_utils_change_image_layout (graphics_device, graphics_queue, command_pool, graphics_queue_family_index, out_data->images[i], 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT), result);
    }

    CHECK_AGAINST_RESULT (graphics_utils_allocate_bind_image_memory (graphics_device, out_data->images, num_images, physical_device_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &out_data->images_memory), result);

    for (size_t i = 0; i < num_images; ++i)
    {
        VkExtent3D img_extent = { img_widths[i], img_heights[i], 1 };
        CHECK_AGAINST_RESULT (graphics_utils_copy_buffer_to_image (graphics_device, command_pool, graphics_queue, img_offsets[i], staging_buffer, &out_data->images[i], img_extent, 1), result);
        CHECK_AGAINST_RESULT (graphics_utils_change_image_layout (graphics_device, graphics_queue, command_pool, graphics_queue_family_index, out_data->images[i], 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT), result);
        CHECK_AGAINST_RESULT (graphics_utils_create_image_view (graphics_device, out_data->images[i], &out_data->image_views[i]), result);
    }

    my_free (img_offsets);
    my_free (img_sizes);
    my_free (img_widths);
    my_free (img_heights);
    
    for (size_t i = 0; i < num_images; ++i)
    {
        stbi_image_free (img_pixels[i]);
    }

    graphics_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_memory);

    return 0;
}

int import_materials (cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    vk_material* materials = NULL;
    size_t num_materials = 0;

    for (size_t d = 0; d < num_datas; ++d)
    {
        if (materials == NULL)
        {
            materials = (vk_material*)my_calloc (datas[d]->materials_count, sizeof (vk_material));
        }
        else
        {
            materials = (vk_material*)my_realloc (materials, sizeof (vk_material) * (datas[d]->materials_count + num_materials));
        }

        for (size_t m = 0; m < datas[d]->materials_count; ++m)
        {
            size_t current_index = num_materials + m;
            cgltf_material* material = datas[d]->materials + m;

            for (size_t i = 0; i < num_images; ++i)
            {
            }
        }

        num_materials += datas[d]->materials_count;
    }

    return 0;
}

int import_gltf_datas (const char* full_folder_path, cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    AGAINSTRESULT result;
    CHECK_AGAINST_RESULT (import_images (full_folder_path, datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_materials (datas, num_datas, out_data), result);

    return 0;
}

int gather_gltf_datas (const char* full_file_path, cgltf_data** datas, size_t current_data_index)
{
    cgltf_options options = { 0 };
    cgltf_data* data = NULL;

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

    char full_folder_path[MAX_PATH];
    get_full_file_path (partial_folder_path, full_folder_path);

    AGAINSTRESULT result;

    for (size_t f = 0 ; f < num_files; ++f)
    {
        char current_full_file_path[MAX_PATH];
        strcpy (current_full_file_path, full_folder_path);
        strcat (current_full_file_path, file_paths[f].path);
        CHECK_AGAINST_RESULT (gather_gltf_datas (current_full_file_path, gltf_datas, f), result);
    }

    my_free (file_paths);

    import_gltf_datas (full_folder_path, gltf_datas, num_gltf_datas, *out_gltf_data);

    for (size_t d = 0; d < num_gltf_datas; ++d)
    {
        my_free (gltf_datas[d]);
    }

    return 0;
}