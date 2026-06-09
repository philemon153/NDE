#include "PlasmaEffect.h"
#include "../VulkanContext.h"

VkShaderModule PlasmaEffect::loadFragmentShader()
{
  return Shader::load("shaders/plasma.frag.spv", context->device.device);
}