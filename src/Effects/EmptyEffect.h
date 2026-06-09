#pragma once
#include "Effect.h"

class EmptyEffect : public Effect {
public:
  EmptyEffect() : Effect("Empty") {}
  bool init(VulkanContext* ctx, VkRenderPass renderPass) override;
  void draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent) override;
  void cleanup() override;
};