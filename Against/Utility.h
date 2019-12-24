#pragma once

#include <Windows.h>
#include <stdbool.h>

typedef struct _LinkedListNode
{
	struct _LinkedListNode* Previous;
	struct _LinkedListNode* Next;

	int x;
} LinkedListNode;

typedef struct _LinkedList
{
	int x;
	struct _LinkedListNode* Root;
} LinkedList;

void GetApplicationFolder (char* OutPath);
void GetFullFilePath (char* OutFilePath, char* PartialFilePath);
void SetupLinkedList (LinkedList SomeList, int NumElements);