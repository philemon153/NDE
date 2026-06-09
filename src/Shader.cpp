#include "Shader.h"
#include <fstream>
#include <vector>

VkShaderModule Shader::load(const std::string& path, VkDevice device) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) return VK_NULL_HANDLE;

  size_t size = file.tellg();
  std::vector<char> buffer(size);
  file.seekg(0);
  file.read(buffer.data(), size);

  VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  createInfo.codeSize = size;
  createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

  VkShaderModule module;
  vkCreateShaderModule(device, &createInfo, nullptr, &module);
  return module;
}