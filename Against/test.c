#include "list.h"
#include <Windows.h>

void list_test ()
{
	list list_obj = { 0 };
	uint32_t x = 34; uint32_t y = 994; uint32_t z = 475;
	VkImage image1 = (VkImage)777; VkImage image2 = (VkImage)666;
	list_insert (&list_obj, &x, e_list_uint32_t);
	list_insert (&list_obj, &y, e_list_uint32_t);
	list_insert (&list_obj, &z, e_list_uint32_t);
	list_insert (&list_obj, &image1, e_list_vk_image);
	list_insert (&list_obj, &image2, e_list_vk_image);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &x, e_list_uint32_t);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &image2, e_list_vk_image);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &z, e_list_uint32_t);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &y, e_list_uint32_t);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_delete (&list_obj, &image1, e_list_vk_image);
	list_print (&list_obj);
	OutputDebugString (L"===========\n");
	list_destroy (&list_obj);
}