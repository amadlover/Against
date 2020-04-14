#include "gltf.h"
#include "utils.h"
#include "error.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

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

int import_images (const char* file_path, cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    out_gltf_data->images_count = data->images_count;
    out_gltf_data->images = (asset_image*)my_calloc (data->images_count, sizeof (asset_image));

    for (size_t i = 0; i < data->images_count; ++i)
    {
        cgltf_image* image = data->images + i;
        asset_image* ass_image = out_gltf_data->images + i;

        strcpy (ass_image->name, image->name);
        //read_image_from_uri (file_path, image->uri, &ass_image->width, &ass_image->height, &ass_image->bpp, ass_image->pixels);
    }

    return 0;
}

int import_textures (cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    return 0;
}

int import_materials (cgltf_data* data, gltf_asset_data* out_gltf_data)
{
    out_gltf_data->materials_count = data->materials_count;
    out_gltf_data->materials = (asset_material*)my_calloc (out_gltf_data->materials_count, sizeof (asset_material));

    for (size_t m = 0; m < data->materials_count; ++m)
    {
        cgltf_material* mat = data->materials + m;
        asset_material* ass_mat = out_gltf_data->materials;

        strcpy (ass_mat->name, mat->name);
        ass_mat->alpha_mode = mat->alpha_mode;
    }

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

int import_gltf_file (const char* partial_file_path, gltf_asset_data** out_gltf_data)
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

    AGAINSTRESULT result;

    CHECK_AGAINST_RESULT (import_images (full_file_path, data, *out_gltf_data), result);
    CHECK_AGAINST_RESULT (import_textures (data, *out_gltf_data), result);
    CHECK_AGAINST_RESULT (import_materials (data, *out_gltf_data), result);
    CHECK_AGAINST_RESULT (import_skins (data, *out_gltf_data), result);
    CHECK_AGAINST_RESULT (import_animations (data, *out_gltf_data), result);
    CHECK_AGAINST_RESULT (import_meshes_from_nodes (data, *out_gltf_data), result);
    
    cgltf_free (data);

    return 0;
}

int import_gltf_files_from_folder (const char* partial_folder_path, gltf_asset_data** out_gltf_data)
{
    file_path* file_paths;
    size_t num_files = 0;
    get_files_in_folder (partial_folder_path, &file_paths, &num_files);

    AGAINSTRESULT result;

    for (size_t f = 0 ; f < num_files; ++f)
    {
        char partial_file_path[MAX_PATH];
        strcpy (partial_file_path, partial_folder_path);
        strcat (partial_file_path, file_paths[f].path);
        CHECK_AGAINST_RESULT (import_gltf_file (partial_file_path, out_gltf_data), result);
    }

    my_free (file_paths);

    return 0;
}