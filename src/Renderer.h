#pragma once
#include "VulkanContext.h"
#include "AudioSystem.h"
#include "Effects/Effect.h"
#include <vector>
#include <memory>

class Renderer {
public:
  Renderer(VulkanContext* context, AudioSystem* audio);
  ~Renderer();

  bool init(GLFWwindow* window);
  void update();
  void draw();
  void cleanup();

private:
  bool createRenderPass();
  bool createFrameResources();
  void recordCommandBuffer(uint32_t imageIndex);
  void switchEffect();

  VulkanContext* ctx = nullptr;
  AudioSystem* audio = nullptr;
  GLFWwindow* window = nullptr;

  VkRenderPass renderPass = VK_NULL_HANDLE;

  std::vector<VkFramebuffer> framebuffers;
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkCommandBuffer> commandBuffers;   // ← One per frame

  size_t currentFrame = 0;
  uint32_t currentEffectIndex = 0;

  std::vector<std::unique_ptr<Effect>> effects;
  Effect* currentEffect = nullptr;

  float time = 0.0f;
  std::vector<float> currentFFT;

  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};