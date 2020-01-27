#include "common_graphics.h"
#include "splash_screen_graphics.h"
#include "asset.h"
#include "graphics_utils.h"

#include "utils.h"

VkBuffer vertex_index_buffer;
VkDeviceMemory vertex_index_buffer_memory;

int create_vulkan_handles_for_meshes (asset_mesh* meshes, uint32_t mesh_count)
{
    VkDeviceSize total_size = 0;

    for (uint32_t m = 0; m < mesh_count; m++)
    {
        asset_mesh* current_mesh = meshes + m;
        for (uint32_t p = 0; p < current_mesh->graphics_primitive_count; p++)
        {
            asset_mesh_graphics_primitive* current_gp = current_mesh->graphics_primitives + p;
            total_size += (VkDeviceSize)current_gp->positions_size +
                (VkDeviceSize)current_gp->uv0s_size +
                (VkDeviceSize)current_gp->uv1s_size +
                (VkDeviceSize)current_gp->normals_size +
                (VkDeviceSize)current_gp->indices_size;
        }
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    CHECK_AGAINST_RESULT (graphics_utils_create_buffer (
        graphics_device,
        total_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        graphics_queue_family_index,
        &staging_buffer
    ));

    CHECK_AGAINST_RESULT (graphics_utils_allocate_bind_buffer_memory (
        graphics_device,
        &staging_buffer,
        1,
        physical_device_memory_properties,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &staging_buffer_memory
    ));

    VkDeviceSize offset = 0;

    for (uint32_t m = 0; m < mesh_count; m++)
    {
        asset_mesh* current_mesh = meshes + m;
        for (uint32_t p = 0; p < current_mesh->graphics_primitive_count; p++)
        {
            asset_mesh_graphics_primitive* current_gp = current_mesh->graphics_primitives + p;

            if (current_gp->positions_size > 0)
            {
                CHECK_AGAINST_RESULT (graphics_utils_map_data_to_buffer (
                    graphics_device,
                    staging_buffer_memory,
                    offset,
                    current_gp->positions_size,
                    current_gp->positions
                ));
                current_gp->positions_offset = offset;
                offset += current_gp->positions_size;
            }

            if (current_gp->uv0s_size > 0)
            {
                CHECK_AGAINST_RESULT (graphics_utils_map_data_to_buffer (
                    graphics_device,
                    staging_buffer_memory,
                    offset,
                    current_gp->uv0s_size,
                    current_gp->uv0s
                ));
                current_gp->uv0s_offset = offset;
                offset += current_gp->uv0s_size;
            }

            if (current_gp->uv1s_size > 0)
            {
                CHECK_AGAINST_RESULT (graphics_utils_map_data_to_buffer (
                    graphics_device,
                    staging_buffer_memory,
                    offset,
                    current_gp->uv1s_size,
                    current_gp->uv1s
                ));
                current_gp->uv1s_offset = offset;
                offset += current_gp->uv1s_size;
            }

            if (current_gp->normals_size > 0)
            {
                CHECK_AGAINST_RESULT (graphics_utils_map_data_to_buffer (
                    graphics_device,
                    staging_buffer_memory,
                    offset,
                    current_gp->normals_size,
                    current_gp->normals
                ));
                current_gp->normals_offset = offset;
                offset += current_gp->normals_size;
            }

            if (current_gp->indices_size > 0)
            {
                CHECK_AGAINST_RESULT (graphics_utils_map_data_to_buffer (
                    graphics_device,
                    staging_buffer_memory,
                    offset,
                    current_gp->indices_size,
                    current_gp->indices
                ));

                current_gp->indices_offset = offset;
                offset += current_gp->indices_size;
            }
        }
    }

    CHECK_AGAINST_RESULT (graphics_utils_create_buffer (
        graphics_device,
        total_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        graphics_queue_family_index,
        &vertex_index_buffer
    ));

    CHECK_AGAINST_RESULT (graphics_utils_allocate_bind_buffer_memory (
        graphics_device,
        &vertex_index_buffer,
        1,
        physical_device_memory_properties,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vertex_index_buffer_memory
    ));

    CHECK_AGAINST_RESULT (graphics_utils_copy_buffer_to_buffer (
        graphics_device,
        command_pool,
        graphics_queue,
        staging_buffer,
        vertex_index_buffer,
        total_size
    ));

    vkDestroyBuffer (graphics_device, staging_buffer, NULL);
    vkFreeMemory (graphics_device, staging_buffer_memory, NULL);

    return 0;
}

int splash_screen_graphics_init (asset_mesh* meshes, uint32_t mesh_count)
{
    CHECK_AGAINST_RESULT (create_vulkan_handles_for_meshes (meshes, mesh_count));

    return 0;
}

int splash_screen_graphics_draw ()
{
    return 0;
}

int splash_screen_graphics_exit ()
{
    if (vertex_index_buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer (graphics_device, vertex_index_buffer, NULL);
    }

    if (vertex_index_buffer_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory (graphics_device, vertex_index_buffer_memory, NULL);
    }

    return 0;
}