#include "ParticlesEffect.h"
#include "../VulkanContext.h"
#include <iostream>
#include <glm/gtc/random.hpp>

bool ParticlesEffect::init(VulkanContext* ctx, VkRenderPass renderPass)
{
    context = ctx;

    const int particleCount = 2048;
    particles.resize(particleCount);

    // Initialize particles randomly
    for (auto& p : particles) {
        p.position = glm::vec4(glm::linearRand(glm::vec2(-1.0f), glm::vec2(1.0f)), 0.0f, 1.0f);
        p.velocity = glm::vec4(glm::linearRand(glm::vec2(-0.02f), glm::vec2(0.02f)), 0.0f, 0.0f);
    }

    std::cout << "ParticlesEffect: Initialized with " << particleCount << " particles\n";
    return true;
}

void ParticlesEffect::update(float time, const std::vector<float>& fft)
{
    // Move attractor in a nice pattern
    attractor.x = std::sin(time * 0.8f) * 0.7f;
    attractor.y = std::cos(time * 1.1f) * 0.6f;

    // Beat force from bass
    beatForce = 1.0f + (fft.empty() ? 0.0f : fft[0] * 6.0f);

    // Update particles
    for (auto& p : particles) {
        glm::vec2 pos2D = glm::vec2(p.position);
        glm::vec2 toAttractor = attractor - pos2D;
        float dist = glm::length(toAttractor) + 0.001f;

        // Attraction force (stronger on beat)
        glm::vec2 force = (toAttractor / dist) * beatForce * 0.035f;

        glm::vec2 vel2D = glm::vec2(p.velocity);
        vel2D += force;
        vel2D *= 0.96f; // damping

        pos2D += vel2D * 0.018f;

        // Wrap around
        if (pos2D.x < -1.2f) pos2D.x = 1.2f;
        if (pos2D.x > 1.2f) pos2D.x = -1.2f;
        if (pos2D.y < -1.2f) pos2D.y = 1.2f;
        if (pos2D.y > 1.2f) pos2D.y = -1.2f;

        p.position = glm::vec4(pos2D, 0.0f, 1.0f);
        p.velocity = glm::vec4(vel2D, 0.0f, 0.0f);
    }
}

void ParticlesEffect::draw(VkCommandBuffer cmd, VkImageView currentImageView, VkExtent2D extent)
{
    // Placeholder for now
    std::cout << "ParticlesEffect draw called - " << particles.size() << " particles\n";
}

void ParticlesEffect::cleanup(VulkanContext* ctx)
{
    particles.clear();
    std::cout << "ParticlesEffect cleaned\n";
}