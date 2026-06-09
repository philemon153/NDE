#include "VulkanContext.h"
#include "Renderer.h"
#include "AudioSystem.h"
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(1280, 720, "DemoScene Testbed", nullptr, nullptr);

  VulkanContext ctx;
  if (!ctx.init(window)) {
    std::cerr << "Vulkan init failed\n";
    return -1;
  }

  AudioSystem audio;
  if (!audio.init("music/track.mp3")) {
    std::cerr << "Audio init failed\n";
  }

  Renderer renderer(&ctx, &audio);
  if (!renderer.init(window)) {        // ← Pass window here
    std::cerr << "Renderer init failed\n";
    return -1;
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    renderer.update();
    renderer.draw();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}