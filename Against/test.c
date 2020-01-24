#include "list.h"
#include <Windows.h>

void list_test ()
{
	list list_obj = { 0 };
	list_init (&list_obj, e_list_uint32_t);
	uint32_t x = 34; uint32_t y = 994; uint32_t z = 475;
	VkImage image1 = (VkImage)777; VkImage image2 = (VkImage)666;
	list_insert (&list_obj, &x);
	list_insert (&list_obj, &y);
	list_insert (&list_obj, &z);
	list_insert (&list_obj, &image1);
	list_insert (&list_obj, &image2);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &x);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &image2);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &z);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &y);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &image1);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_destroy (&list_obj);
}