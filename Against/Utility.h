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

void GetApplicationFolder (TCHAR* Path);

void SetupLinkedList (LinkedList SomeList, int NumElements);