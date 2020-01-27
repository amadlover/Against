#include "list.h"
#include "utils.h"
#include <stdio.h>

void list_init (list* list_ptr, e_list_data_type data_type)
{
	list_ptr->begin_node = (struct _list_node*)my_calloc (1, sizeof (struct _list_node));
	list_ptr->end_node = (struct _list_node*)my_calloc (1, sizeof (struct _list_node));
	list_ptr->data_type = data_type;
	list_ptr->list_iter = list_ptr->begin_node;
}

void list_insert (list* list_ptr, void* data)
{
	if (list_ptr->num_nodes == 0)
	{
		struct _list_node* new_node = (struct _list_node*)my_calloc (1, sizeof (struct _list_node));
		if (list_ptr->data_type == e_list_asset_image)
		{
			new_node->data.image_ptr = (asset_image*)my_calloc (1, sizeof (asset_image));
			memcpy (new_node->data.image_ptr, data, sizeof (asset_image));
		}
		else if (list_ptr->data_type == e_list_uint32)
		{
			new_node->data.uint32_ptr = (uint32_t*)my_calloc (1, sizeof (uint32_t));
			memcpy (new_node->data.uint32_ptr, data, sizeof (uint32_t));
		}

		list_ptr->begin_node->next_node = new_node;
		new_node->next_node = list_ptr->end_node;
		new_node->preview_node = list_ptr->begin_node;
		list_ptr->end_node->preview_node = new_node;

		++list_ptr->num_nodes;
	}
	else
	{
		struct _list_node* last_node = list_ptr->begin_node->next_node;

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
			struct _list_node* new_node = (struct _list_node*)my_calloc (1, sizeof (struct _list_node));
			if (list_ptr->data_type == e_list_asset_image)
			{
				new_node->data.image_ptr = (asset_image*)my_calloc (1, sizeof (asset_image));
				memcpy (new_node->data.image_ptr, data, sizeof (asset_image));
			}
			else if (list_ptr->data_type == e_list_uint32)
			{
				new_node->data.uint32_ptr = (uint32_t*)my_calloc (1, sizeof (uint32_t));
				memcpy (new_node->data.uint32_ptr, data, sizeof (uint32_t));
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

	struct _list_node* node = list_ptr->begin_node->next_node;

	while (node)
	{
		if (list_ptr->data_type == e_list_asset_image)
		{
			wchar_t Buff[64];
			swprintf (Buff, 64, L"Node has image %hs\n", node->data.image_ptr->name);
			OutputDebugString (Buff);
		}
		else if (list_ptr->data_type == e_list_uint32)
		{
			wchar_t Buff[64];
			swprintf (Buff, 64, L"Node has uint32 %d\n", *node->data.uint32_ptr);
			OutputDebugString (Buff);
		}
		else if (list_ptr->data_type == e_list_list)
		{
			list_print (node->data.list_ptr);
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

list_node* list_iter (list* list_ptr)
{
	if (list_ptr->num_nodes == 0)
	{
		return NULL;
	}

	if (list_ptr->list_iter->next_node != list_ptr->end_node)
	{
		list_node* tmp_out_node = list_ptr->list_iter->next_node;
		list_ptr->list_iter = list_ptr->list_iter->next_node;

		return tmp_out_node;
	}
	else
	{
		return NULL;
	}
}

void list_delete (list* list_ptr, void* data)
{
	if (list_ptr->num_nodes == 0)
	{
		return;
	}

	struct _list_node* node = list_ptr->begin_node->next_node;

	while (node)
	{
		if (list_ptr->data_type == e_list_asset_image)
		{
			if (strcmp (node->data.image_ptr->name, (*(asset_image*)data).name) == 0)
			{
				struct _list_node* next_node = node->next_node;
				struct _list_node* preview_node = node->preview_node;

				if (list_ptr->list_iter == node)
				{
					list_ptr->list_iter = next_node;
				}

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
		else if (list_ptr->data_type == e_list_list)
		{

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

	struct _list_node* node = list_ptr->begin_node->next_node;

	while (node != list_ptr->end_node)
	{
		if (list_ptr->data_type == e_list_asset_image)
		{
			my_free (node->data.image_ptr);
		}
		else if (list_ptr->data_type == e_list_list)
		{
			list_destroy (node->data.list_ptr);
		}
	
		struct _list_node* next_node = node->next_node;

		my_free (node);
		node = next_node;

		--list_ptr->num_nodes;
	}

	my_free (list_ptr->begin_node);
	my_free (list_ptr->end_node);
}

