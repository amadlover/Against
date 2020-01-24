#pragma once

#include <stdint.h>
#include "Actor.h"

#include <vulkan/vulkan.h>

typedef enum
{
	e_list_uint32_t,
	e_list_actor,
	e_list_vk_image
}e_list_data_type;

struct list_node
{
	struct list_node* next_node;
	struct list_node* preview_node;

	union
	{
		uint32_t i;
		VkImage image;
		VkImageView image_view;
	} data;
};

typedef struct
{
	struct list_node* begin_node;
	struct list_node* end_node;

	e_list_data_type data_type;
	uint32_t num_nodes;
} list;

void list_init (list* list_ptr, e_list_data_type data_type);
void list_insert (list* list_ptr, void* data);
void list_delete (list* list_ptr, void* data);
void list_print (const list* list_ptr);
void list_destroy (list* list_ptr);