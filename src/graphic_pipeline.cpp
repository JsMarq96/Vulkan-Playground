#include "app.h"
#include "shader.h"

void sApp::_create_graphics_pipeline() {
    VkShaderModule vert_shader, frag_shader;
    VkPipelineShaderStageCreateInfo  shader_stages_create_info[2];

    // ===================================
    // LOAD SHADERS & CREATE STAGES ======
    // ===================================
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

    

}