#pragma once

#include <stdint.h>
#include "actor.h"
#include "asset.h"

#include <vulkan/vulkan.h>

typedef enum
{
	e_list_uint32,
	e_list_asset_image,
	e_list_list
} e_list_data_type;

typedef struct _list_node
{
	struct _list_node* preview_node;
	struct _list_node* next_node;

	union
	{
		uint32_t* uint32_ptr;
		asset_image* image_ptr;
		struct _list* list_ptr;
	} data;
} list_node;

typedef struct _list
{
	struct _list_node* begin_node;
	struct _list_node* end_node;

	list_node* list_iter;

	e_list_data_type data_type;
	uint32_t num_nodes;
} list;

void list_init (list* list_ptr, e_list_data_type data_type);
void list_insert (list* list_ptr, void* data);
void list_delete (list* list_ptr, void* data);
list_node* list_iter (list* list_ptr);
void list_print (const list* list_ptr);
void list_destroy (list* list_ptr);