#pragma once

#include <Windows.h>
#include <stdbool.h>

typedef struct _LinkedList
{
	int x;
	struct _LinkedList* Next;
} LinkedList;

void GetApplicationFolder (TCHAR* Path);

void SetupLinkedList (int NumElements);