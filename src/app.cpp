#include "app.h"
#include "GLFW/glfw3.h"
#include <cstdint>
#include <cstring>
#include <cstdint>
#include <optional>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

bool check_validation_layers(const char** required_val_layers, 
                             const uint32_t required_val_layer_count);

bool is_device_suitable(const VkPhysicalDevice &device, const VkSurfaceKHR &surface, sQueueFamilies* queues);

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
                                   &Vulkan.queues)) {
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
        VkPhysicalDeviceFeatures device_features{};

        // TODO: add the enabled layers for retorcompatibility
        VkDeviceCreateInfo device_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = NULL,
            .queueCreateInfoCount = 2,
            .pQueueCreateInfos = queues_creation_info,
            .enabledExtensionCount = 0,
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
    // SWAPCHAIN CREATION ================
    // ===================================
    {

    }
}





// ===================================
// HELPER FUNCTIONS
// ===================================
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


bool is_device_suitable(const VkPhysicalDevice &device, 
                        const VkSurfaceKHR &surface, 
                        sQueueFamilies* queues) {
    // TODO: check for VkPhyscallDeviceFeatures, for things like texture compression, multiviewport etc
    VkPhysicalDeviceProperties device_properties = {};
    vkGetPhysicalDeviceProperties(device, 
                                  &device_properties);


    if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        return false;
    }

    // Now check the the needed queue families
    uint32_t queue_family_count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_properties = (VkQueueFamilyProperties*) malloc(sizeof(VkQueueFamilyProperties) * queue_family_count); 

    for(uint32_t i = 0; i < queue_family_count; i++) {
        // Check for support of a graphics capabale vulkan device
        if (queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queues->graphics_family_id = i;
            queues->has_found_graphics_family = true;
        }

        // Check for support for being able to present to the current VkSurface type
        VkBool32 surface_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &surface_support);
        if (surface_support) {
            queues->has_found_presenting_familiy = true;
            queues->presenting_family_id = i;
        }
    }

    free(queue_properties);

    return queues->has_found_graphics_family && queues->has_found_presenting_familiy;
}


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