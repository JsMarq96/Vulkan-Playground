#include "app.h"
#include "shader.h"
#include <vulkan/vulkan_core.h>
//https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
void sApp::_create_graphics_pipeline() {
    // ===================================
    // LOAD SHADERS & CREATE STAGES ======
    // ===================================
    VkShaderModule vert_shader, frag_shader;
    VkPipelineShaderStageCreateInfo  shader_stages_create_info[2];
    {
        // Create shader modules
        create_shader_module(Vulkan.device, 
                        "resources/shaders/vertex.spv", 
                        &vert_shader);

        create_shader_module(Vulkan.device, 
                            "resources/shaders/frag.spv", 
                            &frag_shader);

        shader_stages_create_info[0] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = NULL,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_shader,
            .pName = "main"
        };

        shader_stages_create_info[1] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = NULL,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_shader,
            .pName = "main"
        };
    }

    
    // ===================================
    // VERTEX INPUT STAGE ================
    // ===================================
    VkPipelineVertexInputStateCreateInfo vertex_input_stage_create_info;
    {
        // TODO: no vertecies for now, so this is kinda empty
        vertex_input_stage_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = NULL,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = NULL,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = NULL
        };
    }

    // ===================================
    // INPUT ASSEMBLY STAGE ==============
    // ===================================
    VkPipelineInputAssemblyStateCreateInfo input_assembly_stage_create_info;
    {
        input_assembly_stage_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = NULL,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE // TODO ????
        };
    }

    // ===================================
    // CONFIG VIWPORT & SCISSORS =========
    // ===================================
    VkViewport viewport;
    VkRect2D scissor;
    {
        viewport = {
            .x = 0, 
            .y = 0,
            .width = (float) Vulkan.swapchain_info.swapchain_extent.width,
            .height = (float) Vulkan.swapchain_info.swapchain_extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        scissor = { // Scissor the size of the 
            .offset = {0, 0},
            .extent = Vulkan.swapchain_info.swapchain_extent
        };
    }

    // ===================================
    // SET DYNAMIC STATES ================
    // ===================================
    VkDynamicState dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state_stage_create_info;
    VkPipelineViewportStateCreateInfo view_port_create_info;
    {
        dynamic_state_stage_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = NULL,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamic_states
        };

        view_port_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = NULL,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };
    }

    // ===================================
    // RASTERIZER CONFIG =================
    // ===================================
    VkPipelineRasterizationStateCreateInfo rasterizer_state_create_info;
    {
        rasterizer_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = NULL,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f, // ???
        };
    }

    // ===================================
    // MUTISAMPLING CONFIG ===============
    // ===================================
    {

    }
}//
