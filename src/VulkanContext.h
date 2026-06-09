#pragma once
#include <vulkan/vulkan.hpp>
#include <VkBootstrap.h>
#include <GLFW/glfw3.h>
#include <vector>

class VulkanContext {
public:
  bool init(GLFWwindow* window);
  void cleanup();

  vkb::Instance instance;
  vkb::PhysicalDevice physicalDevice;
  vkb::Device device;
  vkb::Swapchain swapchain;

  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkCommandPool commandPool = VK_NULL_HANDLE;
  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

private:
  GLFWwindow* window = nullptr;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
};