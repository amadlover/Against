#pragma once

#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>

#include "Actor.h"

void GetFullTexturePathFromURI (const char* FilePath, const char* URI, char* OutFullTexturePath);
void GetFullFilePath (char* OutFilePath, char* PartialFilePath);

void* MyMalloc (size_t Size);
void* MyCalloc (size_t Count, size_t Size);
void* MyRealloc (void* Ptr, size_t Size);
void MyFree (void* Ptr);