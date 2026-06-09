#pragma once
#include <vulkan/vulkan.hpp>
#include <string>

class Shader {
public:
  static VkShaderModule load(const std::string& path, VkDevice device);
};