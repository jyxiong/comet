#pragma once

#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "comet/platform/window/glfw_window.h"
#include "comet/vulkan/device.h"
#include "comet/vulkan/image_view.h"
#include "comet/vulkan/instance.h"
#include "comet/vulkan/physical_device.h"
#include "comet/vulkan/swapchain.h"

using namespace comet;

class HelloTriangleApplication {
private:
  std::unique_ptr<Window> m_window;
  VkSurfaceKHR m_surface;
  std::unique_ptr<Instance> m_instance;
  std::unique_ptr<Device> m_device;
  std::unique_ptr<Swapchain> m_swapchain;
  // 交换链中的图像视图
  std::vector<ImageView> m_swapchain_image_views;

  // 渲染通道
  VkRenderPass m_renderPass{};
  // 管线布局
  VkPipelineLayout m_pipelineLayout{};
  // 图形管线
  VkPipeline m_graphicsPipeline{};

  // 帧缓冲
  std::vector<VkFramebuffer> m_swapChainFramebuffers{};

  // 命令池
  VkCommandPool m_commandPool{};
  // 命令缓冲
  VkCommandBuffer m_commandBuffer{};

  // 同步对象
  VkSemaphore m_imageAvailableSemaphore{};
  VkSemaphore m_renderFinishedSemaphore{};
  VkFence m_inFlightFence{};

public:
  HelloTriangleApplication();

  ~HelloTriangleApplication() = default;

  void run();

private:
  void initWindow();

  void initVulkan();

  void mainLoop();

  void cleanup();

  std::vector<const char *> getRequiredInstanceExtensions();

  std::vector<const char *> getRequiredDeviceExtensions();

  //--------------------------------------------------
  // 创建图像视图
  void createImageViews();

  //--------------------------------------------------
  // 创建渲染通道
  void createRenderPass();

  //--------------------------------------------------
  // 创建图形管线
  void createGraphicsPipeline();

  // 读取文件
  static std::vector<char> readFile(const std::filesystem::path &filename);

  // 创建着色器模块
  VkShaderModule createShaderModule(const std::vector<char> &code);

  //--------------------------------------------------
  // 创建帧缓冲
  void createFramebuffers();

  //--------------------------------------------------
  // 创建命令池
  void createCommandPool();

  //--------------------------------------------------
  // 创建命令缓冲
  void createCommandBuffer();

  //--------------------------------------------------
  // 创建同步对象
  void createSyncObjects();

  //==================================================
  // 主循环
  //--------------------------------------------------
  void drawFrame();

  void recordCommandBuffer(VkCommandBuffer commandBuffer,
                           unsigned int imageIndex);

  //==================================================
}; // class HelloTriangleApplication
