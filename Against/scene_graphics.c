#include "scene_graphics.h"
#include "common_graphics.h"
#include "gltf.h"
#include "utils.h"
#include "graphics_pipeline.h"

vk_skinned_opaque_graphics_pipeline* skinned_opaque_graphics_pipeline = NULL;

vk_command_pool* graphics_command_pools = NULL;
size_t num_graphics_command_pools = 0;

VkSemaphore* signal_semaphores = NULL;
VkSemaphore wait_semaphore = VK_NULL_HANDLE;
size_t num_signal_semaphores = 0;

VkRenderPass render_pass = VK_NULL_HANDLE;
VkFramebuffer* frame_buffers = NULL;
size_t num_frame_buffers = 0;

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

    num_frame_buffers = num_swapchain_image;
    frame_buffers = (VkFramebuffer*)utils_calloc (num_frame_buffers, sizeof (VkFramebuffer));

    VkFramebufferCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.width = surface_extent.width;
    create_info.height = surface_extent.height;
    create_info.layers = 1;
    create_info.renderPass = render_pass;

    for (size_t f = 0; f < num_frame_buffers; ++f)
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

    for (size_t cp = 0; cp < num_graphics_command_pools; ++cp)
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

AGAINST_RESULT scene_graphics_init (scene_graphics_obj* scene_graphics_data)
{
    AGAINST_RESULT result = AGAINST_SUCCESS;
    
    CHECK_AGAINST_RESULT (create_skinned_opaque_graphics_pipeline (scene_graphics_data, &skinned_opaque_graphics_pipeline), result);

    num_graphics_command_pools = num_swapchain_image;
    graphics_command_pools = (vk_command_pool*)utils_calloc (num_graphics_command_pools, sizeof (vk_command_pool));
    CHECK_AGAINST_RESULT (vk_utils_create_command_pools (graphics_device, graphics_queue_family_index, 3, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphics_command_pools), result);

    size_t* num_command_buffers_per_pool = (size_t*)utils_calloc (num_graphics_command_pools, sizeof (size_t));
    for (size_t bp = 0; bp < num_graphics_command_pools; ++bp)
    {
        num_command_buffers_per_pool[bp] = 1;
    }
    CHECK_AGAINST_RESULT (vk_utils_allocate_command_buffers (graphics_device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, num_graphics_command_pools, num_command_buffers_per_pool, graphics_command_pools), result);
    utils_free (num_command_buffers_per_pool);

    CHECK_AGAINST_RESULT (create_render_pass (), result);
    CHECK_AGAINST_RESULT (create_frame_buffers (), result);
    CHECK_AGAINST_RESULT (vk_utils_create_semaphores (graphics_device, 1, &wait_semaphore), result);
    CHECK_AGAINST_RESULT (vk_utils_create_semaphores_for_command_pools (graphics_device, num_graphics_command_pools, graphics_command_pools), result);
    CHECK_AGAINST_RESULT (record_command_buffers (), result);

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
    submit_info.commandBufferCount = current_command_pool.num_command_buffers;
    submit_info.pCommandBuffers = current_command_pool.command_buffers;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &wait_semaphore;
    submit_info.signalSemaphoreCount = current_command_pool.num_command_buffers;
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
    present_info.waitSemaphoreCount = current_command_pool.num_command_buffers;
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

AGAINST_RESULT scene_graphics_main_loop (void)
{
    AGAINST_RESULT result = AGAINST_SUCCESS;
    CHECK_AGAINST_RESULT (submit_present (), result);

    return AGAINST_SUCCESS;
}

void scene_graphics_shutdown (scene_graphics_obj* scene_graphics_data)
{
    vkQueueWaitIdle (graphics_queue);
    vkQueueWaitIdle (compute_queue);
    vkQueueWaitIdle (transfer_queue);

    destroy_skinned_opaque_graphics_pipeline (skinned_opaque_graphics_pipeline);

    vk_utils_destroy_command_pools_and_buffers (graphics_device, graphics_command_pools, num_graphics_command_pools);
    utils_free (graphics_command_pools);

    for (size_t s = 0; s < num_signal_semaphores; ++s)
    {
        vkDestroySemaphore (graphics_device, signal_semaphores[s], NULL);
    }
    utils_free (signal_semaphores);

    vkDestroyRenderPass (graphics_device, render_pass, NULL);

    for (size_t f = 0; f < num_frame_buffers; ++f)
    {
        vkDestroyFramebuffer (graphics_device, frame_buffers[f], NULL);
    }
    utils_free (frame_buffers);

    vkDestroySemaphore (graphics_device, wait_semaphore, NULL);
}
