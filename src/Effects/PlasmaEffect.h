#pragma once
#include "Effect.h"
#include "../Shader.h"
#include <glm/glm.hpp>

class PlasmaEffect : public Effect {
public:
  bool init(VulkanContext* ctx, VkRenderPass renderPass) override;   // ← Updated
  void update(float time, const std::vector<float>& fft) override;
  void draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent) override;
  void cleanup(VulkanContext* ctx) override;

private:
  VulkanContext* context = nullptr;
  VkRenderPass renderPass = VK_NULL_HANDLE;   // Store it

  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

  VkBuffer uniformBuffer = VK_NULL_HANDLE;
  VkDeviceMemory uniformMemory = VK_NULL_HANDLE;

  struct Uniforms {
    float time;
    float bass;
    float intensity;
    float padding;
    glm::vec2 resolution;
  } uniforms{};
};