#include "app.h"
#include "GLFW/glfw3.h"
#include <cstdint>
#include <cstring>
#include <cstdint>
#include <optional>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

bool check_validation_layers(const char** required_val_layers, 
                             const uint32_t required_val_layer_count);

bool is_device_suitable(const VkPhysicalDevice &device, 
                        const VkSurfaceKHR &surface, 
                        const char** required_extensions,
                        const uint32_t required_extensions_count,
                        sQueueFamilies* queues, 
                        sSwapchainSupportInfo *swapchain_info);

void get_swapchain_info(const VkPhysicalDevice& device,
                        const VkSurfaceKHR &surface,
                        sSwapchainSupportInfo *swapchain_info);

void choose_swapchain_config(sSwapchainSupportInfo *swapchain_info);

VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &capabilities);

static VKAPI_ATTR VkBool32 
VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                          VkDebugUtilsMessageTypeFlagsEXT message_type,
                          const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                          void* user_data);


void sApp::_init_vulkan() {
    // ===================================
    // LOAD EXTENSIONLIST ================
    // ===================================
    {
        // Get the required GLFW extensions & add it to the list of the required exts.
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        for(uint16_t i = 0; i < glfw_extension_count; i++) {
            Vulkan.required_extensions[Vulkan.required_extension_count++] = glfw_extensions[i];
        }

        std::cout  << "Enabled extensions: " << std::endl;
        for(uint16_t i = 0; i < Vulkan.required_extension_count; i++) {
            std::cout  << " - " << (const char*)Vulkan.required_extensions[i] << std::endl;
        }
    }

    // ===================================
    // CREATE VULKAN INSTANCE ============
    // ===================================
    {
         VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = NULL,
            .pApplicationName = WINDOW_NAME,
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = ENGINE_NAME,
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0
        };

        VkInstanceCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = NULL,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = 0,
            .enabledExtensionCount = Vulkan.required_extension_count,
            .ppEnabledExtensionNames = (const char**) Vulkan.required_extensions,
        };

        // Enable validation layers on debug 
#ifndef NDEBUG
        assert_msg(check_validation_layers(Vulkan.required_validation_layers, 
                                           Vulkan.required_validation_layer_count), 
                   "Validation layers not found");
        create_info.enabledLayerCount = Vulkan.required_validation_layer_count;
        create_info.ppEnabledLayerNames = Vulkan.required_validation_layers;

        VkDebugUtilsMessengerCreateInfoEXT debug_utils_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = NULL,
            .messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_callback,
            .pUserData = NULL // Not needed
        };

        // Why??
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_utils_create_info;
#endif

        VK_OK(vkCreateInstance(&create_info, NULL, &Vulkan.instance), "Create instance");
    }

    // ===================================
    // ENABLE DEBUG MESSENGE =============
    // ===================================
#ifndef NDEBUG
    {
        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(Vulkan.instance, 
                                                                                                         "vkCreateDebugUtilsMessengerEXT");
        assert_msg(vkCreateDebugUtilsMessengerEXT != NULL,
                  "Could not load the extension for debug messenger");

        VkDebugUtilsMessengerCreateInfoEXT create_info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = NULL,
            .messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_callback,
            .pUserData = NULL // Not needed
        };

        vkCreateDebugUtilsMessengerEXT(Vulkan.instance, 
                                       &create_info, 
                                       NULL, 
                                       &Vulkan.debug_messenger);
    }
#endif

    // ===================================
    // WINDOW SURFACE CREATION ===========
    // ===================================
    {
        VK_OK(glfwCreateWindowSurface(Vulkan.instance, 
                                     window, 
                                     NULL, 
                                     &Vulkan.surface), 
              "Failed to create surface");
    }

    // ===================================
    // PICK PHYSICAL DEVICE ==============
    // ===================================
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(Vulkan.instance, 
                                   &device_count, 
                                   NULL);

        assert_msg(device_count > 0, "There are no GPUs detected!");

        VkPhysicalDevice *device_list = (VkPhysicalDevice*) malloc(sizeof(VkPhysicalDevice) * device_count);
        vkEnumeratePhysicalDevices(Vulkan.instance, 
                                   &device_count, 
                                   device_list);
        
        // TODO: create a reting system for each GPU's capabilities
        for(uint32_t i = 0; i < device_count; i++) {
            if (is_device_suitable(device_list[i], 
                                   Vulkan.surface,
                                   Vulkan.required_device_extensions, 
                                   Vulkan.required_device_extension_count,
                                   &Vulkan.queues, 
                                   &Vulkan.swapchain_info)) {
                Vulkan.physical_device = device_list[i];
                break;
            }
        }
        assert_msg(Vulkan.physical_device != VK_NULL_HANDLE, "Could not find a suitable GPU");

        free(device_list);
    }


    // ===================================
    // CREATE LOGICAL DEVICE =============
    // ===================================
    {
        // Create the queues for interacting with the device
        float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo queues_creation_info[2] = {
            { // GRAPHICS QUEUE
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = NULL,
                .queueFamilyIndex = Vulkan.queues.graphics_family_id,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority
            },
            { // PRESENT QUEUE
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = NULL,
                .queueFamilyIndex = Vulkan.queues.presenting_family_id,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority
            }
        };

        // Set the device features: no need for now (thingslike geometry shaders and stuff)
        VkPhysicalDeviceFeatures device_features{
            .samplerAnisotropy = VK_TRUE,
        };

        // TODO: add the enabled layers for retorcompatibility
        VkDeviceCreateInfo device_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = NULL,
            .queueCreateInfoCount = 2,
            .pQueueCreateInfos = queues_creation_info,
            .enabledExtensionCount = Vulkan.required_device_extension_count,
            .ppEnabledExtensionNames = Vulkan.required_device_extensions,
            .pEnabledFeatures = &device_features,
        };

        VK_OK(vkCreateDevice(Vulkan.physical_device, 
                             &device_create_info, 
                             NULL, 
                             &Vulkan.device),
             "Create a logical device");
        
        // Get the queues of the newly created device
        vkGetDeviceQueue(Vulkan.device, 
                         Vulkan.queues.graphics_family_id, 
                         0, 
                         &Vulkan.graphics_queue);
        vkGetDeviceQueue(Vulkan.device, 
                         Vulkan.queues.presenting_family_id, 
                         0, 
                         &Vulkan.present_queue);
    }

    
    // ===================================
    // CREATE SWAPCHAIN ==================
    // ===================================
    {
        choose_swapchain_config(&Vulkan.swapchain_info);

        // One more, in order to avoid waiting for the driver in order to adquire the image
        uint32_t image_count = Vulkan.swapchain_info.capabilites.minImageCount + 1;

        // Check if we dont go over the max amount of images of the device
        if (Vulkan.swapchain_info.capabilites.maxImageCount > 0 && 
            image_count > Vulkan.swapchain_info.capabilites.maxImageCount) {
            image_count = Vulkan.swapchain_info.capabilites.minImageCount;
        }

        uint32_t queue_familiy_indices[2] = { 
            Vulkan.queues.graphics_family_id, 
            Vulkan.queues.presenting_family_id 
        };

        bool use_concurrent_mode = Vulkan.queues.graphics_family_id != Vulkan.queues.presenting_family_id;

        Vulkan.swapchain_info.swapchain_extent = choose_swapchain_extent(Vulkan.swapchain_info.capabilites);

        VkSwapchainCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = NULL,
            .surface = Vulkan.surface,
            .minImageCount = image_count,
            .imageFormat = Vulkan.swapchain_info.selected_format.format,
            .imageColorSpace = Vulkan.swapchain_info.selected_format.colorSpace,
            .imageExtent = Vulkan.swapchain_info.swapchain_extent, // size of the swapchain iamges
            .imageArrayLayers = 1, 
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = (use_concurrent_mode) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE, // share images between queues families
            .queueFamilyIndexCount = (uint32_t) ((use_concurrent_mode) ? 2 : 0),
            .pQueueFamilyIndices = (use_concurrent_mode) ? queue_familiy_indices : NULL,
            .preTransform = Vulkan.swapchain_info.capabilites.currentTransform, // Apply a transformation to the swapchian if wanted to
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // use alpha channel for blending
            .presentMode = Vulkan.swapchain_info.selected_present_mode,
            .clipped = VK_TRUE, // remove ocluded, on screen pixels (if a window overlaps for example),
            .oldSwapchain = VK_NULL_HANDLE // Only used if we need to re create the swapchan, for example a resize
        };

        VK_OK(vkCreateSwapchainKHR(Vulkan.device, 
                                   &create_info, 
                                   NULL, 
                                   &Vulkan.swapchain), 
              "Swapchain creation");
    }

    // ===================================
    // GET IMAGES FROM SWAPCHAIN =========
    // ===================================
    {
        vkGetSwapchainImagesKHR(Vulkan.device, 
                                Vulkan.swapchain, 
                                &Vulkan.swapchain_images_count, 
                                NULL);

        Vulkan.swapchain_images = (VkImage*) malloc(sizeof(VkImage) * Vulkan.swapchain_images_count);

        vkGetSwapchainImagesKHR(Vulkan.device, 
                                Vulkan.swapchain, 
                                &Vulkan.swapchain_images_count, 
                                Vulkan.swapchain_images);
    }


    // ===================================
    // CREATE IMAGE VIEWS ================
    // ===================================
    {
        Vulkan.swapchain_image_views = (VkImageView*) malloc(sizeof(VkImageView) * Vulkan.swapchain_images_count);

        for(uint32_t i = 0; i < Vulkan.swapchain_images_count; i++) {
            VkImageViewCreateInfo create_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = NULL,
                .image = Vulkan.swapchain_images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = Vulkan.swapchain_info.selected_format.format,
                .components = { // Alter the was the RGBA channels are stored. Leave it as it comes
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // Wich aspect of the iamge are used, in this case just the color
                    .baseMipLevel = 0,
                    .levelCount = 1, // No need for mipmaping (for now) on the swapchain
                    .baseArrayLayer = 0, // multiplelayers could be usefull for multiple perspectives rendered at the same time
                    .layerCount = 1
                }
            };

            VK_OK(vkCreateImageView(Vulkan.device, 
                                    &create_info, 
                                    NULL, 
                                    &Vulkan.swapchain_image_views[i]),
                 "Error creating image views of swapchain");
        }
    }
}





// ===================================
// HELPER FUNCTIONS
// ===================================

// LAYERS FUNC =========================================

bool check_validation_layers(const char** required_val_layers, 
                             const uint32_t required_val_layer_count) {
    // Get the layer count
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties* available_layers = (VkLayerProperties*) malloc(sizeof(VkLayerProperties) * layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    for(uint32_t i = 0; i < required_val_layer_count; i++) {
        bool layer_found = false;

        for(uint32_t j = 0; j < layer_count; j++) {
            if (strcmp(required_val_layers[i], available_layers[j].layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            return false;
        }
    }
    
    free(available_layers);

    return true;
}

// SWAPCHAIN FUNCS ========================================

void get_swapchain_info(const VkPhysicalDevice& device,
                        const VkSurfaceKHR &surface,
                        sSwapchainSupportInfo *swapchain_info) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchain_info->capabilites);

    // Enumerate surface formats & store them
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &swapchain_info->format_count, NULL);

    assert_msg(swapchain_info->format_count > 0, "Not found any surface formats of the device");

    swapchain_info->formats = (VkSurfaceFormatKHR*) malloc(sizeof(VkSurfaceFormatKHR) * swapchain_info->format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &swapchain_info->format_count, swapchain_info->formats);

    // Repeat for presentation modes
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &swapchain_info->present_modes_count, NULL);

    assert_msg(swapchain_info->present_modes_count > 0, "Not found any presentation mode of the device");

    swapchain_info->present_modes = (VkPresentModeKHR*) malloc(sizeof(VkPresentModeKHR) * swapchain_info->present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &swapchain_info->present_modes_count, swapchain_info->present_modes);
}

void choose_swapchain_config(sSwapchainSupportInfo *swapchain_info) {
    // SWAPCHAIN FORMAT =================
    // TODO: rank all the different available formats, and pick the best
    swapchain_info->selected_format = swapchain_info->formats[0]; // Choose the first one by default
    for(uint32_t i = 0; i < swapchain_info->format_count; i++) {
        if (swapchain_info->formats[i].format == VK_FORMAT_B8G8R8_SRGB && 
            swapchain_info->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain_info->selected_format = swapchain_info->formats[i];
            break;
        }
    }

    // SWAPCHAIN PRESENT MODE ====
    swapchain_info->selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for(uint32_t i = 0; i < swapchain_info->present_modes_count; i++) {
        if (swapchain_info->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapchain_info->selected_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }
}

// TODO: VERY BAD NOT ACCOUNT FOR VERY HIGH PIXEL DENSITY SCREENS
VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &capabilities) {
    return capabilities.currentExtent;
}


// DEVICE FUNCS ============================================

inline bool check_device_extension_support(const VkPhysicalDevice &device, 
                                           const char** required_extensions,
                                           const uint32_t required_extensions_count) {
    // Get the device's extensions
    uint32_t device_extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &device_extension_count, NULL);

    VkExtensionProperties *device_extensions = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties) * device_extension_count);

    vkEnumerateDeviceExtensionProperties(device, NULL, &device_extension_count, device_extensions);

    // Check if all the required extensions are avaible via the count after the comparison
    uint32_t extensions_available_count = 0;
    for(uint32_t i = 0; i < required_extensions_count; i++) {
        for(uint32_t j = 0; j < device_extension_count; j++) {
            if (strcmp(device_extensions[j].extensionName, required_extensions[i]) == 0) {
                device_extensions[j].extensionName[0] = '\0'; // Early skips for next iterations
                extensions_available_count++;
                break;
            }
        }       
    }

    free(device_extensions);

    return extensions_available_count == required_extensions_count;
}

bool is_device_suitable(const VkPhysicalDevice &device, 
                        const VkSurfaceKHR &surface, 
                        const char** required_extensions,
                        const uint32_t required_extensions_count,
                        sQueueFamilies* queues, 
                        sSwapchainSupportInfo *swap_info) {
    // TODO: check for VkPhyscallDeviceFeatures, for things like texture compression, multiviewport etc
    VkPhysicalDeviceProperties device_properties = {};
    vkGetPhysicalDeviceProperties(device, 
                                  &device_properties);


    //if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    //    return false;
    //}

    // Now check the the needed queue families
    uint32_t queue_family_count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_properties = (VkQueueFamilyProperties*) malloc(sizeof(VkQueueFamilyProperties) * queue_family_count); 
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_properties);

    for(uint32_t i = 0; i < queue_family_count; i++) {
        // Check for support of a graphics capabale vulkan device
        if (queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queues->graphics_family_id = i;
            queues->has_found_graphics_family = true;
        }

        // Check for support for being able to present to the current VkSurface type
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            queues->has_found_presenting_familiy = true;
            queues->presenting_family_id = i;
        }
    }

    free(queue_properties);

    bool extension_support = check_device_extension_support(device, required_extensions, required_extensions_count);

    // Check Swapchain support
    bool is_swapchain_adequate = false;
    if (extension_support) {
        get_swapchain_info(device, surface, swap_info);

        is_swapchain_adequate = swap_info->format_count > 0 && swap_info->present_modes_count > 0;
    }

    return queues->has_found_graphics_family && queues->has_found_presenting_familiy && extension_support && is_swapchain_adequate;
}


// DEBUG FUNCS ==============================================

static VKAPI_ATTR VkBool32 
VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                          VkDebugUtilsMessageTypeFlagsEXT message_type,
                          const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                          void* user_data) {
    if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "Validation layer error: " << callback_data->pMessage << std::endl;
    }
    return VK_FALSE;
}
