#pragma once
#include "Effect.h"
#include <glm/glm.hpp>
#include <vector>

class ParticlesEffect : public Effect {
public:
  ParticlesEffect() { name = "Particles"; }
  bool init(VulkanContext* ctx) override;
  void update(float time, const std::vector<float>& fft) override;
  void draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent) override;
  void cleanup(VulkanContext* ctx) override;

private:
  VulkanContext* context = nullptr;

  struct Particle {
    glm::vec4 position;
    glm::vec4 velocity;
  };

  std::vector<Particle> particles;
  glm::vec2 attractor{0.0f, 0.0f};
  float beatForce = 1.0f;
};