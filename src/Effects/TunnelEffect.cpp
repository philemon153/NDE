#include "TunnelEffect.h"
#include "../VulkanContext.h"
#include <iostream>

bool TunnelEffect::init(VulkanContext* ctx, VkRenderPass renderPass)
{
    context = ctx;
    return true;
}

void TunnelEffect::draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent)
{
}

void TunnelEffect::cleanup()
{
}