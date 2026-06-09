#pragma once
#include "Effect.h"
#include "../Shader.h"
#include <glm/glm.hpp>

class ShaderEffect : public Effect {
protected:
  ShaderEffect(const char* name) : Effect(name) {}
public:
  bool init(VulkanContext* ctx, VkRenderPass renderPass) override;
  void draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent) override;
  void cleanup() override;

protected:
  virtual VkShaderModule loadFragmentShader() = 0;

private:

  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

  VkBuffer uniformBuffer = VK_NULL_HANDLE;
  VkDeviceMemory uniformMemory = VK_NULL_HANDLE;
};