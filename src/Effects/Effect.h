#pragma once
#include "../VulkanContext.h"
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

class Effect {
protected:
  Effect(const char* name) { effectName = name;}
public:
  virtual ~Effect() = default;
  virtual bool init(VulkanContext* ctx, VkRenderPass renderPass) = 0;
  virtual void update(float time, const std::vector<float>& fft);
  virtual void draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent) = 0;
  virtual void cleanup() = 0;
  std::string effectName;

protected:
  VulkanContext* context = nullptr;
  VkRenderPass renderPass = VK_NULL_HANDLE;

  // sound
  float smoothedBass = 0.0f;
  float bassTrigger = 0.0f;

  struct Uniforms {
    float time;
    float bass;
    float intensity;
    float padding;
    glm::vec2 resolution;
  } uniforms{};
};