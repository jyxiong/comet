#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include "GLFW/glfw3.h"

struct QueueFamilyIndices
{
    std::optional<unsigned int> graphicsFamily;
    std::optional<unsigned int> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApplication
{

private:
    GLFWwindow *m_window;

    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    VkSurfaceKHR m_surface;

    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

public:
    void run();

private:
    void initWindow();

    void initVulkan();

    void mainLoop();

    void cleanup();

    //========================
    // 创建vulkan实例
    //------------------------
    void createInstance();
    //------------------------
    // 检查是否支持指定的校验层
    bool checkValidationLayersSupport();
    //------------------------
    // 获取所需的扩展
    std::vector<const char *> getRequiredExtensions();
    //------------------------
    // 创建调试信使
    void setupDebugMessenger();
    //------------------------
    // 填充调试信使的创建信息
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    //========================
    // 创建窗口表面
    void createSurface();
    //========================
    // 选择物理设备
    // -----------------------
    void pickPhysicalDevice();
    //------------------------
    // 检查物理设备是否符合要求
    bool isDeviceSuitable(VkPhysicalDevice device);
    //------------------------
    // 要求一：物理设备支持所需的队列族
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    //------------------------
    // 要求二：物理设备支持所需的扩展
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    //------------------------
    // 要求三：物理设备的窗口表面能够创建交换链
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    //========================
    // 创建逻辑设备
    void createLogicalDevice();
    //========================
    // 创建交换链
    void createSwapChain();
    //------------------------
    // 选择交换链表面格式
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    //------------------------
    // 选择交换链显示模式
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    //------------------------
    // 选择交换链分辨率
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
}; // class HelloTriangleApplication