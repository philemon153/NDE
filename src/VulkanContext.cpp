#include "VulkanContext.h"
#include <iostream>

bool VulkanContext::init(GLFWwindow* w)
{
    window = w;

    // Create Instance
    vkb::InstanceBuilder instanceBuilder;
    auto instanceResult = instanceBuilder
        .set_app_name("DemoScene Testbed")
        .request_validation_layers()
        .use_default_debug_messenger()
        .build();

    if (!instanceResult) {
        std::cerr << "Failed to create Vulkan instance: " << instanceResult.error().message() << std::endl;
        return false;
    }
    instance = instanceResult.value();

    // Create Surface
    if (glfwCreateWindowSurface(instance.instance, window, nullptr, &surface) != VK_SUCCESS) {
        std::cerr << "Failed to create window surface\n";
        return false;
    }

    // Select Physical Device
    vkb::PhysicalDeviceSelector selector{ instance };
    auto physResult = selector
        .set_surface(surface)
        .set_minimum_version(1, 3)
        .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)  // Fixed line
        .require_present(true)
        .select();

    if (!physResult) {
        std::cerr << "Failed to select physical device: " << physResult.error().message() << std::endl;
        return false;
    }
    physicalDevice = physResult.value();

    // Create Logical Device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    auto devResult = deviceBuilder.build();

    if (!devResult) {
        std::cerr << "Failed to create logical device: " << devResult.error().message() << std::endl;
        return false;
    }
    device = devResult.value();

    // Get Graphics Queue
    graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();

    // Create Swapchain
    vkb::SwapchainBuilder swapchainBuilder{ device };
    auto swapResult = swapchainBuilder
        .set_old_swapchain(swapchain)
        .build();

    if (!swapResult) {
        std::cerr << "Failed to create swapchain: " << swapResult.error().message() << std::endl;
        return false;
    }
    swapchain = swapResult.value();

    swapchainImages = swapchain.get_images().value();
    swapchainImageViews = swapchain.get_image_views().value();

    // Command Pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = device.get_queue_index(vkb::QueueType::graphics).value();

    if (vkCreateCommandPool(device.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        std::cerr << "Failed to create command pool\n";
        return false;
    }

    // Command Buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device.device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        std::cerr << "Failed to allocate command buffer\n";
        return false;
    }

    std::cout << "VulkanContext initialized successfully!\n";
    return true;
}

void VulkanContext::cleanup()
{
    if (device.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device.device);

        if (commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device.device, commandPool, nullptr);
        }

        for (auto view : swapchainImageViews) {
            if (view != VK_NULL_HANDLE)
                vkDestroyImageView(device.device, view, nullptr);
        }

        // vk-bootstrap handles most cleanup automatically
    }
}