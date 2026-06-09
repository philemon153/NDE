#include "EmptyEffect.h"
#include "../VulkanContext.h"

bool EmptyEffect::init(VulkanContext* ctx, VkRenderPass renderPass)
{
    context = ctx;
    return true;
}

void EmptyEffect::draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent)
{
}

void EmptyEffect::cleanup()
{
}