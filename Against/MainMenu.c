#include "MainMenu.h"

#include "Error.h"
#include "ImportAssets.h"
#include "Utility.h"

#include <Windows.h>
#include <strsafe.h>

#include <stdlib.h>

#include <cgltf.h>

cgltf_data* MainMenuData = NULL;

int ImportMainMenuAssets ()
{
	OutputDebugString (L"ImportMainMenuAssets\n");
	
	TCHAR UIElementPath[MAX_PATH];
	GetApplicationFolder (UIElementPath);
	StringCchCat (UIElementPath, MAX_PATH, L"\\UIElements\\MainMenu\\MainMenu.gltf");

	char UIElementFile[MAX_PATH];
	wcstombs_s (NULL, UIElementFile, MAX_PATH, UIElementPath, MAX_PATH);

	MainMenuData = ImportGLTF (UIElementFile);

	if (MainMenuData == NULL)
	{
		return AGAINST_ERROR_GLTF_COULD_NOT_IMPORT;
	}

	for (cgltf_size n = 0; n < MainMenuData->nodes_count; n++)
	{
		cgltf_node* Node = MainMenuData->nodes + n;
		char* Name = Node->name;
	}

	return 0;
}

int CreateMainScreenUniformBuffer ()
{
	OutputDebugString (L"CreateMainScreenUniformBuffer\n");
	
	return 0;
}

int SetupMainMenu ()
{
	OutputDebugString (L"SetupMainMenu\n");

	int Result = ImportMainMenuAssets ();
	
	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainScreenUniformBuffer ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int DrawMainMenu ()
{
	return 0;
}

void DestroyMainMenu ()
{
	OutputDebugString (L"DestroyMainMenu\n");

	cgltf_free (MainMenuData);
}