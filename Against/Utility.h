#pragma once

#include <Windows.h>
#include <stdbool.h>

void GetFullFilePath (char* OutFilePath, char* PartialFilePath);

void* MyMalloc (size_t Size);
void* MyCalloc (size_t Count, size_t Size);
void MyFree (void* Ptr);