#include "app.h"
#include "shader.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>
//https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
// Creating the synchronization objects
void sApp::_create_graphics_pipeline() {
    // ===================================
    // RENDER-PASS: COLOR ATTACH =========
    // ===================================
    VkAttachmentDescription color_attachments;
    {
        color_attachments = {
            .format = Vulkan.swapchain_info.selected_format.format,
            .samples = VK_SAMPLE_COUNT_1_BIT, // Multisampling
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // Clean the values from anoter pass on load
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // This is used the color of the swapchain
        };
    }

    // ===================================
    // RENDER-PASS: SUBPASSES ============
    // ===================================
    VkAttachmentReference color_attachment_ref;
    color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL //  layout(lcoation=0) out vec4 out_color;
    };
    VkSubpassDescription render_subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, // gracphis subpass, since it can be a compute
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref
    };

    // ===================================
    // RENDER-PASS: RENDERPASS CREATE ====
    // ===================================
    {
        VkRenderPassCreateInfo renderpass_create_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = NULL,
            .attachmentCount = 1,
            .pAttachments = &color_attachments,
            .subpassCount = 1,
            .pSubpasses = &render_subpass
        };

        VK_OK(vkCreateRenderPass(Vulkan.device, 
                                 &renderpass_create_info,
                                 NULL, 
                                 &Vulkan.render_pass), 
              "Create renderpass");
    }

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
    VkPipelineMultisampleStateCreateInfo multisampling_create_info;
    // Disable for now
    {
        multisampling_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = NULL,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = NULL,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };
    }

    // ===================================
    // DEPTH & STENCIL CONFIG ============
    // ===================================
    // N/A


    // ===================================
    // COLOR BLENDING CONFIG =============
    // ===================================
    VkPipelineColorBlendAttachmentState color_blend_state;
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info;
    {
        color_blend_state = {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        color_blend_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = NULL,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_state,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
        };
    }

    // ===================================
    // PIPELINE LAYOUT ===================
    // ===================================
    {
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = NULL,
            .setLayoutCount = 0,
            .pSetLayouts = NULL,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = NULL
        };

        VK_OK(vkCreatePipelineLayout(Vulkan.device, 
                                     &pipeline_layout_create_info,
                                     NULL,
                                     &Vulkan.pipeline_layout), 
              "Creating pipeline layout");
    }


    // ===================================
    // CREATE PIPELINE ===================
    // ===================================
    {
        VkGraphicsPipelineCreateInfo pipeline_create_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .stageCount = 2,
            .pStages = shader_stages_create_info,
            .pVertexInputState = &vertex_input_stage_create_info,
            .pInputAssemblyState = &input_assembly_stage_create_info,
            .pViewportState = &view_port_create_info,
            .pRasterizationState = &rasterizer_state_create_info,
            .pMultisampleState = &multisampling_create_info,
            .pDepthStencilState = NULL,
            .pColorBlendState = &color_blend_state_create_info,
            .pDynamicState = &dynamic_state_stage_create_info,
            .layout = Vulkan.pipeline_layout,
            .renderPass = Vulkan.render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE, // For creating a pipeline from another pipeline, in order to replace it
            .basePipelineIndex = -1
        };

        VK_OK(vkCreateGraphicsPipelines(Vulkan.device, 
                                        NULL, 
                                        1, 
                                        &pipeline_create_info, 
                                        NULL, 
                                        &Vulkan.graphics_pipeline),
              "Creating graphcis pipeline");
    }
}



void sApp::_create_framebuffers() {
    Vulkan.framebuffers = (VkFramebuffer*) malloc(sizeof(VkFramebuffer) * Vulkan.swapchain_images_count);
    Vulkan.framebuffers_count = Vulkan.swapchain_images_count;

    VkFramebufferCreateInfo frame_buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = NULL,
        .renderPass = Vulkan.render_pass,
        .attachmentCount = 1,
        .pAttachments = NULL,
        .width = Vulkan.swapchain_info.swapchain_extent.width,
        .height = Vulkan.swapchain_info.swapchain_extent.height,
        .layers = 1
    };

    // For each imageviewof the swapchain, create a framebuffer
    for(uint32_t i = 0; Vulkan.framebuffers_count > i; i++) {
        VkImageView attachments[1] = { Vulkan.swapchain_image_views[i] };

        frame_buffer_create_info.pAttachments = attachments;

        VK_OK(vkCreateFramebuffer(Vulkan.device, 
                                  &frame_buffer_create_info, 
                                  NULL, 
                                  &Vulkan.framebuffers[i]),
              "Error creating framebuffer");
    }
}


void sApp::_create_command_buffers() {
    // ===================================
    // CREATE CMD POOL ===================
    // ===================================
    {
        VkCommandPoolCreateInfo pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = NULL,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,// update the command bufferes individually
            .queueFamilyIndex = Vulkan.queues.graphics_family_id
        };

        VK_OK(vkCreateCommandPool(Vulkan.device, 
                                  &pool_create_info, 
                                  NULL, 
                                  &Vulkan.command_pool),
              "Create command pool");
    }

    // ===================================
    // CREATE CMD BUFFER =================
    // ===================================
    {
        VkCommandBufferAllocateInfo command_buff_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = Vulkan.command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // Sbumited directly, not from other command buffers (?)
            .commandBufferCount = 1
        };

        VK_OK(vkAllocateCommandBuffers(Vulkan.device, 
                                       &command_buff_alloc_info, 
                                       &Vulkan.command_buffer),
              "Command buffer creation");
    }

    // ===================================
    // RECORD CMD BUFFER =================
    // ===================================
    {
        //record_command_buffer(Vulkan.command_buffer, Vulkan.render_pass, )
    }
}


// ===================================
// COMMAND BUFFER FUNCS

void sApp::record_command_buffer(const VkCommandBuffer &command_buffer,
                                 const VkRenderPass &render_pass,
                                 const uint32_t image_index) {
    VkCommandBufferBeginInfo cmd_buff_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = 0,
        .pInheritanceInfo = NULL,
    };

    VK_OK(vkBeginCommandBuffer(command_buffer, 
                              &cmd_buff_begin_info), 
          "Begin recording of command buffer");

    // Config the render pass
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = render_pass,
        .framebuffer = Vulkan.framebuffers[image_index],
        .renderArea = { 
            .offset = {0, 0},
            .extent = Vulkan.swapchain_info.swapchain_extent
        },
        .clearValueCount = 1,
        .pClearValues = &clear_color
    };

    vkCmdBeginRenderPass(command_buffer, 
                         &render_pass_begin_info, 
                         VK_SUBPASS_CONTENTS_INLINE); // The commands will be embedded on the primery command buffer
    
    vkCmdBindPipeline(command_buffer, 
                      VK_PIPELINE_BIND_POINT_GRAPHICS, // Graphis pipeline, not compute
                      Vulkan.graphics_pipeline);

    // Set the viewport and the scissor
    {
        VkViewport viewport = {
            .x = 0.0f, .y = 0.0f,
            .width = (float) Vulkan.swapchain_info.swapchain_extent.width,
            .height = (float) Vulkan.swapchain_info.swapchain_extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = Vulkan.swapchain_info.swapchain_extent
        };

        vkCmdSetViewport(command_buffer, 
                         0, 
                         1, 
                         &viewport);
        vkCmdSetScissor(command_buffer, 
                        0, 
                        1, 
                        &scissor);
    }

    vkCmdDraw(command_buffer, 
              3, // Vertex count 
              1, // instance count instanced rendering
              0, // first vertex
              0); // first isntance
            
    vkCmdEndRenderPass(command_buffer);

    VK_OK(vkEndCommandBuffer(command_buffer), 
          "End Command buffer");
}
