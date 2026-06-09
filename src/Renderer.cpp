#include "Renderer.h"
#include "Effects/EmptyEffect.h"
#include "Effects/PlasmaEffect.h"
//#include "Effects/TunnelEffect.h"
//#include "Effects/ParticlesEffect.h"
#include <iostream>

Renderer::Renderer(VulkanContext* context, AudioSystem* audioSystem)
    : ctx(context), audio(audioSystem) {}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::init(GLFWwindow* w)
{
    if (!w) {
        std::cerr << "Renderer::init: Invalid window passed!\n";
        return false;
    }

    window = w;

    if (!createRenderPass()) return false;
    if (!createFrameResources()) return false;

    //effects.push_back(std::make_unique<EmptyEffect>());
    effects.push_back(std::make_unique<PlasmaEffect>());
    //effects.push_back(std::make_unique<TunnelEffect>());
    //effects.push_back(std::make_unique<ParticlesEffect>());

    for (auto& effect : effects) {
        if (!effect->init(ctx, renderPass)) {
            std::cerr << "Failed to initialize one or more effects\n";
            return false;
        }
    }

    currentEffect = effects[0].get();
    currentEffectIndex = 0;

    glfwSetKeyCallback(window, keyCallback);
    glfwSetWindowUserPointer(window, this);

    std::cout << "Renderer initialized successfully!\n";
    std::cout << "Press SPACE to switch effects | ESC to quit\n";
    return true;
}

bool Renderer::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = ctx->swapchain.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(ctx->device.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        std::cerr << "Failed to create render pass!\n";
        return false;
    }
    return true;
}

bool Renderer::createFrameResources()
{
    const int MAX_FRAMES = 3;

    imageAvailableSemaphores.resize(MAX_FRAMES);
    renderFinishedSemaphores.resize(MAX_FRAMES);
    inFlightFences.resize(MAX_FRAMES);
    commandBuffers.resize(MAX_FRAMES);

    VkSemaphoreCreateInfo semInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES; ++i) {
        vkCreateSemaphore(ctx->device.device, &semInfo, nullptr, &imageAvailableSemaphores[i]);
        vkCreateSemaphore(ctx->device.device, &semInfo, nullptr, &renderFinishedSemaphores[i]);
        vkCreateFence(ctx->device.device, &fenceInfo, nullptr, &inFlightFences[i]);
    }

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = ctx->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES;
    vkAllocateCommandBuffers(ctx->device.device, &allocInfo, commandBuffers.data());

    framebuffers.resize(ctx->swapchainImages.size());
    for (size_t i = 0; i < ctx->swapchainImages.size(); ++i) {
        VkFramebufferCreateInfo fbInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fbInfo.renderPass = renderPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &ctx->swapchainImageViews[i];
        fbInfo.width = ctx->swapchain.extent.width;
        fbInfo.height = ctx->swapchain.extent.height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(ctx->device.device, &fbInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create framebuffer!\n";
            return false;
        }
    }
    return true;
}

void Renderer::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    Renderer* r = static_cast<Renderer*>(glfwGetWindowUserPointer(win));
    if (!r) return;

    if (key == GLFW_KEY_SPACE) r->switchEffect();
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(win, GLFW_TRUE);
}

void Renderer::switchEffect()
{
    currentEffectIndex = (currentEffectIndex + 1) % effects.size();
    currentEffect = effects[currentEffectIndex].get();
    std::cout << "Switched to effect " << currentEffectIndex << std::endl;
}

void Renderer::update()
{
    if (audio) {
        audio->update();
        time = audio->getTime();
        audio->getFFT(currentFFT);
    } else {
        time += 0.016f;
        currentFFT.assign(64, 0.3f);
    }

    if (currentEffect) currentEffect->update(time, currentFFT);
}

void Renderer::draw()
{
    if (!currentEffect) return;

    vkWaitForFences(ctx->device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(ctx->device.device, ctx->swapchain.swapchain,
        UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) return;
    if (result != VK_SUCCESS) return;

    vkResetFences(ctx->device.device, 1, &inFlightFences[currentFrame]);

    VkCommandBuffer cmd = commandBuffers[currentFrame];
    vkResetCommandBuffer(cmd, 0);

    recordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

    vkQueueSubmit(ctx->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

    VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &ctx->swapchain.swapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(ctx->graphicsQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % 3;
}

void Renderer::recordCommandBuffer(uint32_t imageIndex)
{
    VkCommandBuffer cmd = commandBuffers[currentFrame];

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkRenderPassBeginInfo rpInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    rpInfo.renderPass = renderPass;
    rpInfo.framebuffer = framebuffers[imageIndex];
    rpInfo.renderArea.offset = {0, 0};
    rpInfo.renderArea.extent = ctx->swapchain.extent;

    VkClearValue clearColor = {{{0.01f, 0.01f, 0.03f, 1.0f}}};
    rpInfo.clearValueCount = 1;
    rpInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    if (currentEffect) {
        currentEffect->draw(cmd, ctx->swapchainImageViews[imageIndex], ctx->swapchain.extent);
    }

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);
}

void Renderer::cleanup()
{
    if (!ctx || !ctx->device.device) {
        ctx = nullptr;
        return;
    }

    std::cout << "Renderer: Starting cleanup...\n";
    vkDeviceWaitIdle(ctx->device.device);

    // Effects
    for (auto& effect : effects) {
        if (effect) effect->cleanup(ctx);
    }
    effects.clear();

    // Framebuffers
    for (auto& fb : framebuffers) {
        if (fb != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(ctx->device.device, fb, nullptr);
        }
    }
    framebuffers.clear();

    // Semaphores
    for (auto s : imageAvailableSemaphores) {
        if (s != VK_NULL_HANDLE) vkDestroySemaphore(ctx->device.device, s, nullptr);
    }
    imageAvailableSemaphores.clear();

    for (auto s : renderFinishedSemaphores) {
        if (s != VK_NULL_HANDLE) vkDestroySemaphore(ctx->device.device, s, nullptr);
    }
    renderFinishedSemaphores.clear();

    // Fences
    for (auto f : inFlightFences) {
        if (f != VK_NULL_HANDLE) vkDestroyFence(ctx->device.device, f, nullptr);
    }
    inFlightFences.clear();

    // Render Pass
    if (renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(ctx->device.device, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
    }

    ctx = nullptr;
    std::cout << "Renderer cleaned up successfully\n";
}