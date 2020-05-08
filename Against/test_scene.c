#include "test_scene.h"
#include "game.h"
#include "graphics_pipeline.h"
#include "common_graphics.h"
#include "asset.h"
#include "utils.h"
#include "scene.h"
#include "vk_utils.h"

scene_asset_data* asset_data = NULL;
vk_skeletal_opaque_graphics_pipeline* skeletal_opaque_graphics_pipeline = NULL;

VkCommandPool command_pool = VK_NULL_HANDLE;
VkCommandBuffer* command_buffers = NULL;
size_t command_buffers_count = 0;

VkSemaphore* signal_semaphores = NULL;
VkSemaphore wait_semaphore = VK_NULL_HANDLE;
size_t signal_semaphores_count = 0;

int create_command_pool ()
{
    OutputDebugString (L"create_command_pool\n");

    return 0;
}

int test_scene_init (HINSTANCE h_instnace, HWND h_wnd)
{
    OutputDebugString (L"test_scene_init\n");
    
    AGAINSTRESULT result;

    CHECK_AGAINST_RESULT (import_scene_data ("", &asset_data), result);
    CHECK_AGAINST_RESULT (create_skeletal_opaque_graphics_pipeline (asset_data, &skeletal_opaque_graphics_pipeline), result);
    CHECK_AGAINST_RESULT (vk_utils_create_command_pool (graphics_device, graphics_queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &command_pool), result);

    command_buffers_count = swapchain_image_count;
    command_buffers = (VkCommandBuffer*)utils_calloc (command_buffers_count, sizeof (VkCommandBuffer));
    CHECK_AGAINST_RESULT (vk_utils_allocate_command_buffers (graphics_device, command_pool, command_buffers_count, VK_COMMAND_BUFFER_LEVEL_PRIMARY, command_buffers), result);

    signal_semaphores_count = command_buffers_count;
    signal_semaphores = (VkSemaphore*)utils_calloc (command_buffers_count, sizeof (VkSemaphore));
    CHECK_AGAINST_RESULT (vk_utils_create_semaphores (graphics_device, signal_semaphores_count, signal_semaphores), result);

    return 0;
}

int test_scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    OutputDebugString (L"test_scene_process_keyboard_input\n");
    
    AGAINSTRESULT result;

    switch (w_param)
    {
    case VK_ESCAPE:
        CHECK_AGAINST_RESULT (game_set_current_scene (e_scene_type_test), result);
        break;

    default:
        break;
    }

    return 0;
}

int draw_skybox ()
{
    OutputDebugString (L"draw_skybox\n");
    return 0;
}

int draw_opaque_skeletal_meshes ()
{
    OutputDebugString (L"draw_opaque_skeletal_meshes\n");

    return 0;
}

int update_command_buffers ()
{
    OutputDebugString (L"test_scene_update_command_buffers\n");

    AGAINSTRESULT result;
    CHECK_AGAINST_RESULT (draw_skybox (), result);
    CHECK_AGAINST_RESULT (draw_opaque_skeletal_meshes (), result);

    return 0;
}

int tick ()
{
    return 0;
}

int submit_present ()
{
    size_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR (graphics_device, swapchain, UINT64_MAX, wait_semaphore, VK_NULL_HANDLE, &image_index);

    if (result != VK_SUCCESS)
    {
        if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            return AGAINST_SUCCESS;
        }
        else
        {
            return AGAINST_ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE;
        }
    }

    return 0;
}

int test_scene_main_loop ()
{
    AGAINSTRESULT result;

    CHECK_AGAINST_RESULT (tick (), result);
    CHECK_AGAINST_RESULT (submit_present (), result);

    return 0;
}

void test_scene_exit ()
{
    OutputDebugString (L"test_scene_exit\n");

    destroy_skeletal_opaque_graphics_pipeline (skeletal_opaque_graphics_pipeline);
    cleanup_scene_data (asset_data);
    
    vkFreeCommandBuffers (graphics_device, command_pool, command_buffers_count, command_buffers);
    vkDestroyCommandPool (graphics_device, command_pool, NULL);
    utils_free (command_buffers);

    for (size_t s = 0; s < signal_semaphores_count; ++s)
    {
        vkDestroySemaphore (graphics_device, signal_semaphores[s], NULL);
    }

    vkDestroySemaphore (graphics_device, wait_semaphore, NULL);
    utils_free (signal_semaphores);
}