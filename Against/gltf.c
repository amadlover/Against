#include "gltf.h"
#include "utils.h"
#include "error.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

int import_graphics_primitives (cgltf_data* data)
{
    return 0;
}

int import_meshes_from_nodes (cgltf_node* nodes, size_t nodes_count)
{
    for (size_t n = 0 ; n < nodes_count; ++n)
    {
        if (nodes[n].mesh == NULL)
        {
            continue;
        }

        wchar_t name[1024];
        swprintf (name, 1024, L"%hs\n", nodes[n].name);
        OutputDebugString (name);
    }

    return 0;
}

int import_materials ()
{
    return 0;
}

int import_textures ()
{
    return 0;
}

int import_images ()
{
    return 0;
}

int import_skins ()
{
    return 0;
}

int import_animations ()
{
    return 0;
}

int import_gltf_file (const char* partial_file_path, gltf_scene_data* gltf_data)
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

    CHECK_AGAINST_RESULT (import_materials (), result);
    CHECK_AGAINST_RESULT (import_textures (), result);
    CHECK_AGAINST_RESULT (import_images (), result);
    CHECK_AGAINST_RESULT (import_skins (), result);
    CHECK_AGAINST_RESULT (import_animations (), result);
    CHECK_AGAINST_RESULT (import_meshes_from_nodes (data->nodes, data->nodes_count), result);
    
    cgltf_free (data);

    return 0;
}

int import_gltf_files_from_folder (const char* partial_folder_path, gltf_scene_data* gltf_data)
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
        CHECK_AGAINST_RESULT (import_gltf_file (partial_file_path, gltf_data), result);
    }

    my_free (file_paths);

    return 0;
}