#include "TunnelEffect.h"
#include "../VulkanContext.h"
#include <iostream>

bool TunnelEffect::init(VulkanContext* ctx)
{
    context = ctx;
    std::cout << "TunnelEffect: Initialized (stub)\n";
    return true;
}

void TunnelEffect::update(float time, const std::vector<float>& fft)
{
    uniforms.time = time;
    uniforms.bass = fft.empty() ? 0.0f : fft[0] * 2.0f;
    uniforms.mid  = fft.size() > 8 ? fft[8] * 1.5f : 0.5f;
    uniforms.resolution = glm::vec2(1280.0f, 720.0f); // will be updated in draw
}

void TunnelEffect::draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent)
{
    // Placeholder - will be properly implemented later
    // For now just avoid crash
    std::cout << "TunnelEffect draw called\n";
}

void TunnelEffect::cleanup(VulkanContext* ctx)
{
    // Nothing to clean yet
    std::cout << "TunnelEffect cleaned\n";
}