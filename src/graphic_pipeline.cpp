#include "app.h"
#include "shader.h"
#include <cstddef>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "mesh.h"

//TODO: clean the Vertex descriptors 

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
         // Subpass transition (??)
        VkSubpassDependency dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        };

        VkRenderPassCreateInfo renderpass_create_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = NULL,
            .attachmentCount = 1,
            .pAttachments = &color_attachments,
            .subpassCount = 1,
            .pSubpasses = &render_subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
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
        uint32_t biding_descr, attribute_descr;
        vertex_input_stage_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = NULL,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = Geometry::get_2D_biding_description(&biding_descr),
            .vertexAttributeDescriptionCount = 2,
            .pVertexAttributeDescriptions = Geometry::get_2D_attribute_description(&attribute_descr)
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
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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
            .setLayoutCount = 1,
            .pSetLayouts = &Vulkan.descriptor_set_layout,
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

uint32_t find_memmory_type(const VkPhysicalDevice &phys_device,
                           const  uint32_t type_filter, 
                           const VkMemoryPropertyFlags &properties);

void sApp::_create_vertex_buffer() {
    VkDeviceSize buffer_size = sizeof(Geometry::Meshes::Quad::vertices);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    create_buffer(buffer_size, 
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &staging_buffer, 
                  &staging_buffer_memory);

    // Upload Data to the stating buffer's memory
    void *upload_data;
    VK_OK(vkMapMemory(Vulkan.device, 
                     staging_buffer_memory,
                     0, // Offset on the memory
                     buffer_size, 
                     0, // Flags, but there are none on the api
                     &upload_data), 
         "Mapping memory");
    
    memcpy(upload_data, 
           (void*) Geometry::Meshes::Quad::vertices, 
           buffer_size);
    
    vkUnmapMemory(Vulkan.device, 
                  staging_buffer_memory);
    
    
    create_buffer(buffer_size, 
                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // More optimal layout in memory
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                  &Vulkan.vertex_buffer, 
                  &Vulkan.vertex_buffer_memmory);

    copy_buffer(staging_buffer, 
                Vulkan.vertex_buffer, 
                buffer_size);

    vkDestroyBuffer(Vulkan.device, 
                    staging_buffer, 
                    NULL);
    vkFreeMemory(Vulkan.device, 
                 staging_buffer_memory, 
                 NULL);
}

void sApp::_create_index_buffer() {
    VkDeviceSize buffer_size = sizeof(Geometry::Meshes::Quad::indices);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    create_buffer(buffer_size, 
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &staging_buffer, 
                  &staging_buffer_memory);

    // Upload Data to the stating buffer's memory
    void *upload_data;
    VK_OK(vkMapMemory(Vulkan.device, 
                     staging_buffer_memory,
                     0, // Offset on the memory
                     buffer_size, 
                     0, // Flags, but there are none on the api
                     &upload_data), 
         "Mapping memory");
    
    memcpy(upload_data, 
           (void*) Geometry::Meshes::Quad::indices, 
           buffer_size);
    
    vkUnmapMemory(Vulkan.device, 
                  staging_buffer_memory);
    
    
    create_buffer(buffer_size, 
                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, // More optimal layout in memory
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                  &Vulkan.index_buffer, 
                  &Vulkan.index_buffer_memory);

    copy_buffer(staging_buffer, 
                Vulkan.index_buffer, 
                buffer_size);

    vkDestroyBuffer(Vulkan.device, 
                    staging_buffer, 
                    NULL);
    vkFreeMemory(Vulkan.device, 
                 staging_buffer_memory, 
                 NULL);
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
    // Vertex & indices buffer ===========
    // ===================================
    sApp::_create_vertex_buffer();
    sApp::_create_index_buffer();
    sApp::_create_uniform_buffers();
    sApp::_create_descriptor_pool_and_set();

    // ===================================
    // CREATE CMD BUFFER =================
    // ===================================
    {
        VkCommandBufferAllocateInfo command_buff_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = Vulkan.command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // Sbumited directly, not from other command buffers (?)
            .commandBufferCount = MAX_FRAMES_IN_FLIGHT
        };

        VK_OK(vkAllocateCommandBuffers(Vulkan.device, 
                                       &command_buff_alloc_info, 
                                       Vulkan.command_buffers),
              "Command buffer creation");
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

    // Bind the vertex buffer =======================
    {
        vkCmdBindPipeline(command_buffer, 
                         VK_PIPELINE_BIND_POINT_GRAPHICS, 
                         Vulkan.graphics_pipeline);
        
        VkBuffer vertex_buffers[] = {Vulkan.vertex_buffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer, 
                               0, 
                               1, 
                               vertex_buffers, 
                               offsets);

        vkCmdBindIndexBuffer(command_buffer, 
                            Vulkan.index_buffer, 
                            0, // offset
                            VK_INDEX_TYPE_UINT32);
    }

    vkCmdBindDescriptorSets(command_buffer, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            Vulkan.pipeline_layout, 
                            0, 
                            1, 
                            &Vulkan.descriptor_sets[Vulkan.current_frame], 
                            0, 
                            NULL);

    vkCmdDrawIndexed(command_buffer, 
                     Geometry::Meshes::Quad::indices_count, // Vertex count 
                     1, // instance count instanced rendering
                     0, // first vertex
                     0,
                     0); // first isntance
            
    vkCmdEndRenderPass(command_buffer);

    VK_OK(vkEndCommandBuffer(command_buffer), 
          "End Command buffer");
}


void sApp::_create_sync_objects() {
    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
    };

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_OK(vkCreateSemaphore(Vulkan.device, &semaphore_create_info, NULL, &Vulkan.image_available_semaphore[i]), "Create semaphore");
        VK_OK(vkCreateSemaphore(Vulkan.device, &semaphore_create_info, NULL, &Vulkan.render_finished_semaphore[i]), "Create semaphore");
        VK_OK(vkCreateFence(Vulkan.device, &fence_create_info, NULL, &Vulkan.in_flight_fence[i]), "Create fence");
    }
}