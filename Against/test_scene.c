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

vk_command_pool* graphics_command_pools = NULL;
size_t graphics_command_pools_count = 0;

VkSemaphore* signal_semaphores = NULL;
VkSemaphore wait_semaphore = VK_NULL_HANDLE;
size_t signal_semaphores_count = 0;

VkRenderPass render_pass = VK_NULL_HANDLE;
VkFramebuffer* frame_buffers = NULL;
size_t frame_buffers_count = 0;

AGAINST_RESULT create_render_pass ()
{
    OutputDebugString (L"create_render_pass\n");

    VkAttachmentDescription attachment_description = { 0 };
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachment_description.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference color_attachment_reference = { 0 };
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = { 0 };
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachment_reference;

    VkRenderPassCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.subpassCount = 1;
    create_info.pAttachments = &attachment_description;
    create_info.pSubpasses = &subpass_description;
    
    if (vkCreateRenderPass (graphics_device, &create_info, NULL, &render_pass) != VK_SUCCESS)
    {
        return AGAINST_ERROR_GRAPHICS_CREATE_RENDER_PASS;
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT create_frame_buffers ()
{
    OutputDebugString (L"create_frame_buffers\n");

    frame_buffers_count = swapchain_image_count;
    frame_buffers = (VkFramebuffer*)utils_calloc (frame_buffers_count, sizeof (VkFramebuffer));

    VkFramebufferCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.width = surface_extent.width;
    create_info.height = surface_extent.height;
    create_info.layers = 1;
    create_info.renderPass = render_pass;

    for (size_t f = 0; f < frame_buffers_count; ++f)
    {
        create_info.pAttachments = swapchain_imageviews + f;
        if (vkCreateFramebuffer (graphics_device, &create_info, NULL, frame_buffers + f) != VK_SUCCESS)
        {
            return AGAINST_ERROR_GRAPHICS_CREATE_FRAMEBUFFER;
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT record_command_buffers ()
{
    OutputDebugString (L"record_command_buffers\n");

    VkCommandBufferBeginInfo cb_begin_info = { 0 };
    cb_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cb_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkClearValue clear_value;
    VkClearColorValue color_value = { 1,0,1,1 };
    clear_value.color = color_value;

    VkRenderPassBeginInfo rp_begin_info = { 0 };
    rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;
    rp_begin_info.renderArea.extent = surface_extent;
    rp_begin_info.renderPass = render_pass;

    for (size_t cp = 0; cp < graphics_command_pools_count; ++cp)
    {
        rp_begin_info.framebuffer = frame_buffers[cp];

        if (vkBeginCommandBuffer (graphics_command_pools[cp].command_buffers[0], &cb_begin_info) != VK_SUCCESS)
        {
            return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
        }

        vkCmdBeginRenderPass (graphics_command_pools[cp].command_buffers[0], &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass (graphics_command_pools[cp].command_buffers[0]);
      
        if (vkEndCommandBuffer (graphics_command_pools[cp].command_buffers[0]) != VK_SUCCESS)
        {
            return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT test_scene_init (HINSTANCE h_instnace, HWND h_wnd)
{
    OutputDebugString (L"test_scene_init\n");
    
    AGAINST_RESULT result;

    CHECK_AGAINST_RESULT (import_scene_data ("", &asset_data), result);
    CHECK_AGAINST_RESULT (create_skeletal_opaque_graphics_pipeline (asset_data, &skeletal_opaque_graphics_pipeline), result);
    
    graphics_command_pools_count = swapchain_image_count;
    graphics_command_pools = (vk_command_pool*)utils_calloc (graphics_command_pools_count, sizeof (vk_command_pool));
    CHECK_AGAINST_RESULT (vk_utils_create_command_pools (graphics_device, graphics_queue_family_index, 3, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphics_command_pools), result);

    size_t command_buffers_count[3] = { 1,1,1 };
    CHECK_AGAINST_RESULT (vk_utils_allocate_command_buffers (graphics_device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, graphics_command_pools_count, command_buffers_count, graphics_command_pools), result);

    CHECK_AGAINST_RESULT (create_render_pass (), result);
    CHECK_AGAINST_RESULT (create_frame_buffers (), result);
    CHECK_AGAINST_RESULT (vk_utils_create_semaphores (graphics_device, 1, &wait_semaphore), result);
    CHECK_AGAINST_RESULT (vk_utils_create_semaphores_for_command_pools (graphics_device, graphics_command_pools_count, graphics_command_pools), result);
    CHECK_AGAINST_RESULT (record_command_buffers (), result);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT test_scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    OutputDebugString (L"test_scene_process_keyboard_input\n");
    
    AGAINST_RESULT result;

    switch (w_param)
    {
    case VK_ESCAPE:
        CHECK_AGAINST_RESULT (game_set_current_scene (e_scene_type_test), result);
        break;

    default:
        break;
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT draw_skybox ()
{
    OutputDebugString (L"draw_skybox\n");
    return AGAINST_SUCCESS;
}

AGAINST_RESULT draw_opaque_skeletal_meshes ()
{
    OutputDebugString (L"draw_opaque_skeletal_meshes\n");
    return AGAINST_SUCCESS;
}

AGAINST_RESULT tick ()
{
    return AGAINST_SUCCESS;
}

AGAINST_RESULT submit_present ()
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

    VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    vk_command_pool current_command_pool = graphics_command_pools[image_index];

    VkSubmitInfo submit_info = { 0 };
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = current_command_pool.command_buffers_count;
    submit_info.pCommandBuffers = current_command_pool.command_buffers;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &wait_semaphore;
    submit_info.signalSemaphoreCount = current_command_pool.command_buffers_count;
    submit_info.pSignalSemaphores = current_command_pool.submit_signal_semaphores;
    submit_info.pWaitDstStageMask = &wait_stage_mask;

    if (vkQueueSubmit (graphics_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_NULL_HANDLE)
    {
        return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
    }

    VkPresentInfoKHR present_info = { 0 };
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain;
    present_info.waitSemaphoreCount = current_command_pool.command_buffers_count;
    present_info.pWaitSemaphores = current_command_pool.submit_signal_semaphores;
    present_info.pImageIndices = &image_index;

    result = vkQueuePresentKHR (graphics_queue, &present_info);
    if (result != VK_SUCCESS)
    {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
        {
            return AGAINST_ERROR_GRAPHICS_QUEUE_PRESENT;
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT test_scene_main_loop ()
{
    AGAINST_RESULT result;

    CHECK_AGAINST_RESULT (tick (), result);
    CHECK_AGAINST_RESULT (submit_present (), result);

    return AGAINST_SUCCESS;
}

void test_scene_exit ()
{
    OutputDebugString (L"test_scene_exit\n");

    vkQueueWaitIdle (graphics_queue);
    vkQueueWaitIdle (compute_queue);
    vkQueueWaitIdle (transfer_queue);

    destroy_skeletal_opaque_graphics_pipeline (skeletal_opaque_graphics_pipeline);
    cleanup_scene_data (asset_data);
    
    vk_utils_destroy_command_pools_and_buffers (graphics_device, graphics_command_pools, graphics_command_pools_count);
    utils_free (graphics_command_pools);

    for (size_t s = 0; s < signal_semaphores_count; ++s)
    {
        vkDestroySemaphore (graphics_device, signal_semaphores[s], NULL);
    }
    utils_free (signal_semaphores);

    vkDestroyRenderPass (graphics_device, render_pass, NULL);

    for (size_t f = 0; f < frame_buffers_count; ++f)
    {
        vkDestroyFramebuffer (graphics_device, frame_buffers[f], NULL);
    }
    utils_free (frame_buffers);

    vkDestroySemaphore (graphics_device, wait_semaphore, NULL);
}