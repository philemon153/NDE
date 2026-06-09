#include "EmptyEffect.h"
#include "../VulkanContext.h"
#include <iostream>

bool EmptyEffect::init(VulkanContext* ctx)
{
    context = ctx;
    std::cout << "EmptyEffect: Initialized\n";
    return true;
}

void EmptyEffect::update(float time, const std::vector<float>& fft)
{
}

void EmptyEffect::draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent)
{
}

void EmptyEffect::cleanup(VulkanContext* ctx)
{
    std::cout << "EmptyEffect cleaned\n";
}