#pragma once
#include "ShaderEffect.h"

class PlasmaEffect : public ShaderEffect {
public:
  PlasmaEffect() : ShaderEffect("Plasma") {}
protected:
  VkShaderModule loadFragmentShader() override;
};