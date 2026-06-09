// src/Shader.cpp
#include "Shader.h"
#include <fstream>
#include <vector>
#include <iostream>

VkShaderModule Shader::load(const std::string& path, VkDevice device)
{
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open shader: " << path << std::endl;
    return VK_NULL_HANDLE;
  }

  size_t fileSize = file.tellg();
  if (fileSize % 4 != 0) {
    std::cerr << "Error: Shader file " << path << " has invalid size (" << fileSize
              << ") - must be multiple of 4 bytes for SPIR-V\n";
    return VK_NULL_HANDLE;
  }

  std::vector<uint32_t> buffer(fileSize / 4);   // Important: read as uint32_t

  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
  file.close();

  VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  createInfo.codeSize = fileSize;
  createInfo.pCode = buffer.data();

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    std::cerr << "Failed to create shader module: " << path << std::endl;
    return VK_NULL_HANDLE;
  }

  std::cout << "Loaded shader: " << path << " (" << fileSize << " bytes)\n";
  return shaderModule;
}