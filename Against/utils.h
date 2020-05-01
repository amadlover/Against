#pragma once

#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>

#define CHECK_AGAINST_RESULT(func, result) result = func; if (result != 0) return result;

typedef struct
{
    char path[MAX_PATH];
} file_path;

void utils_get_full_texture_path_from_uri (const char* file_path, const char* uri, char* out_full_texture_path);
void utils_get_full_file_path (const char* partial_file_path, char* out_file_path);
void utils_get_files_in_folder (const char* partial_folder_path, file_path** out_file_paths, size_t* out_file_count);

void utils_read_image_from_uri (const char* file_path, const char* uri, int* width, int* height, int* bpp, uint8_t* pixels);

void* utils_malloc (size_t size);
void* utils_malloc_zero (size_t size);
void* utils_aligned_malloc (size_t size, size_t alignment);
void* utils_aligned_malloc_zero (size_t size, size_t alignment);
void* utils_calloc (size_t count, size_t size);
void* utils_aligned_calloc (size_t count, size_t size, size_t alignment);
void* utils_realloc_zero (void*ptr, size_t old_size, size_t new_size);
void* utils_aligned_realloc_zero (void* ptr, size_t old_size, size_t new_size, size_t alignment);
void utils_free (void* ptr);
void utils_aligned_free (void* ptr);
void utils_free_image_data (uint8_t* pixels);