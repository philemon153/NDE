#pragma once
#include "Effect.h"
#include <vector>

class EmptyEffect : public Effect {
public:
  EmptyEffect() { name = "Empty"; }
  bool init(VulkanContext* ctx) override;
  void update(float time, const std::vector<float>& fft) override;
  void draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent) override;
  void cleanup(VulkanContext* ctx) override;

private:
  VulkanContext* context = nullptr;
};