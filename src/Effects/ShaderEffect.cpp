#include "ShaderEffect.h"
#include "../VulkanContext.h"
#include <iostream>

bool ShaderEffect::init(VulkanContext* ctx, VkRenderPass rp)
{
    context = ctx;
    renderPass = rp;

    // 1. Descriptor Set Layout
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;

    vkCreateDescriptorSetLayout(ctx->device.device, &layoutInfo, nullptr, &descriptorSetLayout);

    // 2. Pipeline Layout
    VkPipelineLayoutCreateInfo plInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    plInfo.setLayoutCount = 1;
    plInfo.pSetLayouts = &descriptorSetLayout;
    vkCreatePipelineLayout(ctx->device.device, &plInfo, nullptr, &pipelineLayout);

    // ==================== 3. Uniform Buffer + Proper Host Visible Memory ====================
    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = sizeof(Uniforms);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkCreateBuffer(ctx->device.device, &bufferInfo, nullptr, &uniformBuffer);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(ctx->device.device, uniformBuffer, &memReq);

    // Get memory properties
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(ctx->physicalDevice, &memProps);  // This line was causing error

    // Find suitable memory type
    uint32_t memoryTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReq.memoryTypeBits & (1u << i)) &&
            (memProps.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
            memoryTypeIndex = i;
            break;
                                                      }
    }

    if (memoryTypeIndex == UINT32_MAX) {
        std::cerr << "PlasmaEffect: Failed to find suitable memory type!\n";
        return false;
    }

    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(ctx->device.device, &allocInfo, nullptr, &uniformMemory) != VK_SUCCESS) {
        std::cerr << "PlasmaEffect: Failed to allocate uniform memory\n";
        return false;
    }

    vkBindBufferMemory(ctx->device.device, uniformBuffer, uniformMemory, 0);
    // 4. Descriptor
    VkDescriptorPoolSize poolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 };
    VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    vkCreateDescriptorPool(ctx->device.device, &poolInfo, nullptr, &descriptorPool);

    VkDescriptorSetAllocateInfo setAlloc{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    setAlloc.descriptorPool = descriptorPool;
    setAlloc.descriptorSetCount = 1;
    setAlloc.pSetLayouts = &descriptorSetLayout;
    vkAllocateDescriptorSets(ctx->device.device, &setAlloc, &descriptorSet);

    VkDescriptorBufferInfo bufInfo{ uniformBuffer, 0, sizeof(Uniforms) };
    VkWriteDescriptorSet writeSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writeSet.dstSet = descriptorSet;
    writeSet.dstBinding = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeSet.pBufferInfo = &bufInfo;
    vkUpdateDescriptorSets(ctx->device.device, 1, &writeSet, 0, nullptr);

    // 5. Shaders
    VkShaderModule vertModule = Shader::load("shaders/fullscreen.vert.spv", ctx->device.device);
    VkShaderModule fragModule = loadFragmentShader();

    // 6. Pipeline
    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertModule;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragModule;
    shaderStages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertexInput{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;

    VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(ctx->device.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        std::cerr << "Failed to create graphics pipeline!\n";
        return false;
    }

    vkDestroyShaderModule(ctx->device.device, vertModule, nullptr);
    vkDestroyShaderModule(ctx->device.device, fragModule, nullptr);

    std::cout << "PlasmaEffect initialized successfully!\n";
    return true;
}

void ShaderEffect::draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent)
{
    uniforms.resolution = glm::vec2(static_cast<float>(extent.width), static_cast<float>(extent.height));

    // Update uniform buffer
    void* data;
    if (vkMapMemory(context->device.device, uniformMemory, 0, sizeof(Uniforms), 0, &data) == VK_SUCCESS) {
        memcpy(data, &uniforms, sizeof(Uniforms));
        vkUnmapMemory(context->device.device, uniformMemory);
    }

    // Set dynamic states
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    vkCmdDraw(cmd, 3, 1, 0, 0);
}

void ShaderEffect::cleanup()
{
    if (!context) return;
    vkDeviceWaitIdle(context->device.device);

    vkDestroyPipeline(context->device.device, pipeline, nullptr);
    vkDestroyPipelineLayout(context->device.device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(context->device.device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(context->device.device, descriptorPool, nullptr);
    vkDestroyBuffer(context->device.device, uniformBuffer, nullptr);
    vkFreeMemory(context->device.device, uniformMemory, nullptr);
    context = nullptr;
}