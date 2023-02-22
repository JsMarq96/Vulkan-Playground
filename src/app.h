#pragma once

#include <stdint.h>
#include <vulkan/vulkan_core.h>
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define VK_USE_PLATFORM_XLIB_KHR
#define GLFW_EXPOSE_NATIVE_X11
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <GLFW/glfw3native.h>

#include <cassert>
#include <iostream>

#include "utils.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_NAME   "Vulkan test"
#define ENGINE_NAME   "No engine"

struct sQueueFamilies {
    uint32_t graphics_family_id;
    bool has_found_graphics_family = false;
    uint32_t presenting_family_id;
    bool has_found_presenting_familiy = false;
};

struct sSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilites;
    VkSurfaceFormatKHR  *formats = NULL;
    uint32_t format_count = 0;
    VkSurfaceFormatKHR selected_format;

    VkExtent2D swapchain_extent;

    VkPresentModeKHR  *present_modes = NULL;
    uint32_t present_modes_count = 0;
    VkPresentModeKHR  selected_present_mode;

    void clean() {
        free(formats);
        free(present_modes);
    }
};

// https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain All of the details are in the struct now, so let's extend isDeviceSuitable
struct sApp {
    GLFWwindow *window = NULL;

    // Vulkan data
    struct {
        VkInstance instance;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        sQueueFamilies queues;
        VkDevice device; // logical device
        VkQueue  graphics_queue;
        VkQueue  present_queue;
        VkSurfaceKHR surface;
        sSwapchainSupportInfo swapchain_info;
        VkSwapchainKHR swapchain;

        VkImage *swapchain_images;
        VkImageView *swapchain_image_views;
        uint32_t swapchain_images_count = 0;
        uint32_t swapchain_images_index = 0;

        // Validation layers
        const char* required_validation_layers[2] = {
            "VK_LAYER_KHRONOS_validation",
        };
        uint32_t required_validation_layer_count = 1;

        // Instance extentions
        const char* required_extensions[10] = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        };
        uint32_t required_extension_count = 1;

        // Device extensions
        const char* required_device_extensions[10] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        uint32_t required_device_extension_count = 1;

        // Debug messaegs
        VkDebugUtilsMessengerEXT debug_messenger;
    } Vulkan;

    void run() {
        _init_window();
        _init_vulkan();
        _create_graphics_pipeline();
        _main_loop();
        _clean_up();
    };

    // EVENT FUNCTIONS
    
    // LIFE CYCLE FUNCTIONS
    void _init_window() {
        assert_msg(glfwInit(),"Could init GLFW");

        glfwWindowHint(GLFW_CLIENT_API, 
                       GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, 
                       GLFW_FALSE);

        window = glfwCreateWindow(WINDOW_WIDTH, 
                                 WINDOW_HEIGHT, 
                                 WINDOW_NAME, 
                                 NULL, 
                                 NULL);

        assert_msg(window != NULL, "Could not create window");

        glfwSetKeyCallback(window, 
                           [](GLFWwindow *window, 
                              int key, 
                              int scancode, 
                              int action, 
                              int mods) {
                                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                                    glfwSetWindowShouldClose(window, 
                                                             GL_TRUE);
                                }
                            });
    }

    void _init_vulkan();

    void _create_graphics_pipeline();

    void _clean_up() {
        Vulkan.swapchain_info.clean();
        for(uint32_t i = 0; i < Vulkan.swapchain_images_count; i++) {
            vkDestroyImageView(Vulkan.device, Vulkan.swapchain_image_views[i], NULL);
        }
        vkDestroySwapchainKHR(Vulkan.device, Vulkan.swapchain, NULL);
        vkDestroyDevice(Vulkan.device, NULL);
        vkDestroySurfaceKHR(Vulkan.instance, Vulkan.surface, NULL);
        // TODO destroy the Utils messener: add it to the Vulkna struct
        free(Vulkan.swapchain_images);
        vkDestroyInstance(Vulkan.instance, NULL);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void _main_loop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
};