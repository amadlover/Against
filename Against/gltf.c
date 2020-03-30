#include "gltf.h"
#include "utils.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void import_gltf_file (const char* partial_file_path)
{

}

void import_gltf_files_from_folder (const char* partial_folder_path)
{
    char full_path[MAX_PATH];
    get_full_file_path (partial_folder_path, full_path);

    return;
}