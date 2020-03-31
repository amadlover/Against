#pragma once

#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>

#define CHECK_AGAINST_RESULT(result) if (result != 0) return result;

void get_full_texture_path_from_uri (const char* file_path, const char* uri, char* out_full_texture_path);
void get_full_file_path (const char* partial_file_path, char* out_file_path);
void get_files_in_folder (const char* partial_folder_path, char** out_file_paths);

void* my_malloc (size_t size);
void* my_calloc (size_t count, size_t size);
void* my_realloc (void* ptr, size_t size);
void my_free (void* ptr);