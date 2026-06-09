#include "PlasmaEffect.h"
#include "../VulkanContext.h"
#include <iostream>

bool PlasmaEffect::init(VulkanContext* ctx)
{
    context = ctx;

    // 1. Descriptor Set Layout
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;

    if (vkCreateDescriptorSetLayout(ctx->device.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        std::cerr << "PlasmaEffect: Failed to create descriptor set layout\n";
        return false;
    }

    // 2. Pipeline Layout
    VkPipelineLayoutCreateInfo plInfo{};
    plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plInfo.setLayoutCount = 1;
    plInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(ctx->device.device, &plInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        std::cerr << "PlasmaEffect: Failed to create pipeline layout\n";
        return false;
    }

    // 3. Load Shaders
    VkShaderModule vertModule = Shader::load("shaders/fullscreen.vert.spv", ctx->device.device);
    VkShaderModule fragModule = Shader::load("shaders/plasma.frag.spv", ctx->device.device);

    if (!vertModule || !fragModule) {
        std::cerr << "PlasmaEffect: Failed to load shaders\n";
        return false;
    }

    // 4. Shader stages
    VkPipelineShaderStageCreateInfo shaderStages[2]{};
    
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertModule;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragModule;
    shaderStages[1].pName = "main";

    // 5. Fixed pipeline states for fullscreen triangle
    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // 6. Graphics Pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
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
    //pipelineInfo.renderPass = /* Will be set from Renderer - for now we assume it's handled externally */;

    std::cerr << "Warning: PlasmaEffect::init() needs renderPass from Renderer.\n";

    vkDestroyShaderModule(ctx->device.device, vertModule, nullptr);
    vkDestroyShaderModule(ctx->device.device, fragModule, nullptr);

    std::cout << "PlasmaEffect initialized successfully\n";
    return true;
}

void PlasmaEffect::update(float time, const std::vector<float>& fft)
{
    uniforms.time = time;
    uniforms.bass = fft.empty() ? 0.0f : fft[0] * 2.0f;
    uniforms.mid  = fft.size() > 12 ? fft[12] * 1.5f : 0.5f;
}

void PlasmaEffect::draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent)
{
    uniforms.resolution = glm::vec2(static_cast<float>(extent.width), static_cast<float>(extent.height));

    // TODO: Update uniform buffer here (map memory + memcpy)

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &descriptorSet, 0, nullptr);

    // Draw fullscreen triangle
    vkCmdDraw(cmd, 3, 1, 0, 0);
}

void PlasmaEffect::cleanup(VulkanContext* ctx)
{
    if (!ctx) return;

    vkDeviceWaitIdle(ctx->device.device);

    vkDestroyPipeline(ctx->device.device, pipeline, nullptr);
    vkDestroyPipelineLayout(ctx->device.device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(ctx->device.device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(ctx->device.device, descriptorPool, nullptr);
    vkDestroyBuffer(ctx->device.device, uniformBuffer, nullptr);
    vkFreeMemory(ctx->device.device, uniformMemory, nullptr);
}