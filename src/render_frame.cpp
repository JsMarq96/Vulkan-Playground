#include "app.h"
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_core.h>


void sApp::_render_frame() {
    // Wait for the prev frame is finished
    vkWaitForFences(Vulkan.device,
                    1,
                    &Vulkan.in_flight_fence,
                    VK_TRUE,
                    UINT64_MAX);
    vkResetFences(Vulkan.device,
                  1,
                  &Vulkan.in_flight_fence);
    // Adquire swapchian image
    vkAcquireNextImageKHR(Vulkan.device,
                          Vulkan.swapchain,
                          UINT64_MAX,
                          Vulkan.image_available_semaphore,
                          VK_NULL_HANDLE,
                          &Vulkan.swapchain_images_index);

    // Add teh command buffer
    vkResetCommandBuffer(Vulkan.command_buffer,
                         0);

    record_command_buffer(Vulkan.command_buffer,
                          Vulkan.render_pass,
                          Vulkan.swapchain_images_index);

    // Submit the command buffer
    VkPipelineStageFlags wait_stagers[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Vulkan.image_available_semaphore,
        .pWaitDstStageMask = wait_stagers,
        .commandBufferCount = 1,
        .pCommandBuffers = &Vulkan.command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &Vulkan.render_finished_semaphore
    };

    VK_OK(vkQueueSubmit(Vulkan.graphics_queue, 1, &submit_info, Vulkan.in_flight_fence), "Submit Queue frame");

    // Presentation
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Vulkan.render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &Vulkan.swapchain,
        .pImageIndices = &Vulkan.swapchain_images_index,
        .pResults = NULL
    };

    VK_OK(vkQueuePresentKHR(Vulkan.graphics_queue, &present_info), "Presenting frame");
}
