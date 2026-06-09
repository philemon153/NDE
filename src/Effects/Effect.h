#pragma once
#include "../VulkanContext.h"
#include <vulkan/vulkan.hpp>

class Effect {
public:
  virtual ~Effect() = default;
  virtual bool init(VulkanContext* ctx, VkRenderPass renderPass) = 0;
  virtual void update(float time, const std::vector<float>& fft) = 0;
  virtual void draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent) = 0;
  virtual void cleanup(VulkanContext* ctx) = 0;
  std::string name;
};