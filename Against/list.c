#include "list.h"
#include "utils.h"
#include <stdio.h>

void list_init (list* list_ptr, e_list_data_type data_type)
{
	list_ptr->begin_node = (struct list_node*)my_calloc (1, sizeof (struct list_node));
	list_ptr->end_node = (struct list_node*)my_calloc (1, sizeof (struct list_node));
	list_ptr->data_type = data_type;
}

void list_insert (list* list_ptr, void* data)
{
	if (list_ptr->num_nodes == 0)
	{
		struct list_node* new_node = (struct list_node*)my_calloc (1, sizeof (struct list_node));
		if (list_ptr->data_type == e_list_uint32_t)
		{
			new_node->data.i = *((uint32_t*)(data));
		}
		else if (list_ptr->data_type == e_list_vk_image)
		{
			new_node->data.image = *((VkImage*)(data));
		}

		list_ptr->begin_node->next_node = new_node;
		new_node->next_node = list_ptr->end_node;
		new_node->preview_node = list_ptr->begin_node;
		list_ptr->end_node->preview_node = new_node;

		++list_ptr->num_nodes;
	}
	else
	{
		struct list_node* last_node = list_ptr->begin_node->next_node;

		while (last_node)
		{
			if (last_node->next_node != list_ptr->end_node)
			{
				last_node = last_node->next_node;
			}
			else
			{
				break;
			}
		}

		if (last_node)
		{
			struct list_node* new_node = (struct list_node*)my_calloc (1, sizeof (struct list_node));
			if (list_ptr->data_type == e_list_uint32_t)
			{
				new_node->data.i = *((uint32_t*)(data));
			}
			else if (list_ptr->data_type == e_list_vk_image)
			{
				new_node->data.image = *((VkImage*)(data));
			}

			new_node->preview_node = last_node;
			new_node->next_node = list_ptr->end_node;
			last_node->next_node = new_node;
			list_ptr->end_node->preview_node = new_node;

			++list_ptr->num_nodes;
		}
	}
}

void list_print (const list* list_ptr)
{
	if (list_ptr->num_nodes == 0)
	{
		return;
	}

	struct list_node* node = list_ptr->begin_node->next_node;

	while (node)
	{
		if (list_ptr->data_type == e_list_uint32_t)
		{
			wchar_t Buff[32];
			swprintf (Buff, 32, L"Node has uint32_t %d\n", node->data.i);
			OutputDebugString (Buff);
		}
		else if (list_ptr->data_type == e_list_vk_image)
		{
			wchar_t Buff[32];
			swprintf (Buff, 32, L"Node has image %d\n", (uint32_t)node->data.i);
			OutputDebugString (Buff);
		}

		if (node->next_node != list_ptr->end_node)
		{
			node = node->next_node;
		}
		else
		{
			break;
		}
	}
}

void list_delete (list* list_ptr, void* data)
{
	if (list_ptr->num_nodes == 0)
	{
		return;
	}

	struct list_node* node = list_ptr->begin_node->next_node;

	while (node)
	{
		if (list_ptr->data_type == e_list_uint32_t)
		{
			if (node->data.i == *(uint32_t*)data)
			{
				struct list_node* next_node = node->next_node;
				struct list_node* preview_node = node->preview_node;

				my_free (node);
				--list_ptr->num_nodes;

				preview_node->next_node = next_node;
				next_node->preview_node = preview_node;

				if (next_node != list_ptr->end_node)
				{
					node = next_node;
					continue;
				}
				else
				{
					break;
				}
			}
		}
		else if (list_ptr->data_type == e_list_vk_image)
		{
			if (node->data.image == *(VkImage*)data)
			{
				struct list_node* next_node = node->next_node;
				struct list_node* preview_node = node->preview_node;

				my_free (node);
				--list_ptr->num_nodes;

				preview_node->next_node = next_node;
				next_node->preview_node = preview_node;

				if (next_node != list_ptr->end_node)
				{
					node = next_node;
					continue;
				}
				else
				{
					break;
				}
			}
		}
		if (node->next_node != list_ptr->end_node)
		{
			node = node->next_node;
		}
		else
		{
			break;
		}
	}
}

void list_destroy (list* list_ptr)
{
	if (list_ptr->num_nodes == 0)
	{
		return;
	}

	struct list_node* node = list_ptr->begin_node->next_node;

	while (node != list_ptr->end_node)
	{
		struct list_node* next_node = node->next_node;

		my_free (node);
		node = next_node;

		--list_ptr->num_nodes;
	}

	my_free (list_ptr->begin_node);
	my_free (list_ptr->end_node);
}

