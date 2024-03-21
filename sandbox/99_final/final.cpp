#include "final.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <vector>

#include "comet/vulkan/image.h"
#include "volk.h"
#include "vulkan/vulkan_core.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// 指定实例支持的校验层
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

HelloTriangleApplication::HelloTriangleApplication()
{
    if(volkInitialize())
    {
        throw std::runtime_error("Failed to initialize volk.");
    }
}

void HelloTriangleApplication::run()
{
    initWindow();

    initVulkan();

    mainLoop();

    cleanup();
}

void HelloTriangleApplication::initWindow()
{
       Window::Properties properties;
    properties.extent = {WIDTH, HEIGHT};
    properties.title = "99_final";
    properties.resizable = false;
    m_window = std::make_unique<GlfwWindow>(properties);
}

void HelloTriangleApplication::initVulkan()
{
    // 创建instance
    m_instance = std::make_unique<Instance>("99_final", getRequiredInstanceExtensions(), validationLayers);

    // 选择physical device
    const auto &physical_device = m_instance->get_suitable_physical_device(nullptr);

    // 创建surface
    m_surface = m_window->create_surface(m_instance->get_handle(), physical_device.get_handle());

    // 创建logical device
    m_device = std::make_unique<Device>(physical_device, m_surface, getRequiredDeviceExtensions());

    // 创建交换链
    m_swapchain = std::make_unique<Swapchain>(*m_device, m_surface);

    createImageViews();

    createRenderPass();

    createGraphicsPipeline();

    createFramebuffers();

    createCommandPool();

    createCommandBuffer();

    createSyncObjects();
}

void HelloTriangleApplication::mainLoop()
{
    while (!m_window->should_close())
    {
        m_window->process_events();

        drawFrame();
    }

    vkDeviceWaitIdle(m_device->get_handle());
}

void HelloTriangleApplication::cleanup()
{
    // 销毁同步对象
    vkDestroySemaphore(m_device->get_handle(), m_renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(m_device->get_handle(), m_imageAvailableSemaphore, nullptr);
    vkDestroyFence(m_device->get_handle(), m_inFlightFence, nullptr);

    // 销毁命令池
    vkDestroyCommandPool(m_device->get_handle(), m_commandPool, nullptr);

    // 销毁帧缓冲
    for (auto framebuffer: m_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(m_device->get_handle(), framebuffer, nullptr);
    }

    // 销毁图形管线
    vkDestroyPipeline(m_device->get_handle(), m_graphicsPipeline, nullptr);

    // 销毁管线布局
    vkDestroyPipelineLayout(m_device->get_handle(), m_pipelineLayout, nullptr);

    // 销毁渲染通道
    vkDestroyRenderPass(m_device->get_handle(), m_renderPass, nullptr);

    m_swapchain_image_views.clear();

    m_swapchain.reset();

    m_device.reset();

    vkDestroySurfaceKHR(m_instance->get_handle(), m_surface, nullptr);

    m_instance.reset();

    m_window.reset();
}

void HelloTriangleApplication::createImageViews()
{
    auto& swapchain_images = m_swapchain->get_images();

    VkExtent3D extent{m_swapchain->get_extent().width, m_swapchain->get_extent().height, 1};

    // 便利交换链图像，创建图像视图
    for (unsigned int i = 0; i < swapchain_images.size(); i++)
    {
        auto image = Image(*m_device, swapchain_images[i], extent, m_swapchain->get_format(), m_swapchain->get_usage());
        m_swapchain_image_views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D, m_swapchain->get_format());
    }
}

void HelloTriangleApplication::createRenderPass()
{
    // 创建颜色附件描述信息
    VkAttachmentDescription colorAttachment{};
    // 附件的格式
    colorAttachment.format = m_swapchain->get_format();
    // 附件的采样数(MSAA)
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // 附件的加载操作，用于颜色和深度附件
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // 附件的存储操作，用于颜色和深度附件
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // 附件的加载操作，用于模板附件
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // 附件的存储操作，用于模板附件
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // 附件的初始布局
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // 附件的最终布局
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // 创建颜色附件引用
    VkAttachmentReference colorAttachmentRef{};
    // 附件的索引
    colorAttachmentRef.attachment = 0;
    // 附件的布局
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // 创建渲染子过程描述信息
    VkSubpassDescription subpass{};
    // 渲染子过程的类型
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // 渲染子过程的颜色附件引用
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // 创建子过程依赖
    VkSubpassDependency dependency{};
    // 依赖的源子过程索引
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    // 依赖的目标子过程索引
    dependency.dstSubpass = 0;
    // 依赖的源阶段掩码
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 依赖的源访问掩码
    dependency.srcAccessMask = 0;
    // 依赖的目标阶段掩码
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 依赖的目标访问掩码
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // 创建渲染过程描述信息
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // 渲染过程的颜色附件描述信息
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    // 渲染过程的渲染子过程描述信息
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    // 渲染过程的子过程依赖
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // 创建渲染过程
    if (vkCreateRenderPass(m_device->get_handle(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void HelloTriangleApplication::createGraphicsPipeline()
{
    // 读取着色器文件
    auto vertShaderCode = readFile("/home/jyxiong/Learnings/comet/build/bin/shaders/vert.spv");
    auto fragShaderCode = readFile("/home/jyxiong/Learnings/comet/build/bin/shaders/frag.spv");

    // 创建着色器模块
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // 创建顶点着色器阶段信息
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    // 创建片段着色器阶段信息
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    // 着色器阶段信息列表
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // 顶点输入阶段信息
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // 输入装配阶段信息
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 视口和裁剪信息
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_swapchain->get_extent().width;
    viewport.height = (float) m_swapchain->get_extent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapchain->get_extent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    // 视口信息
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    // 裁剪信息
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // 光栅化阶段信息
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // 指定是否clamp深度值，否则丢弃深度值超出范围的片段
    rasterizer.depthClampEnable = VK_FALSE;
    // 指定是否禁止光栅化阶段输出图元
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // 指定图元的填充模式
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    // 指定面剔除模式
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // 指定顶点的绘制顺序
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    // 指定是否开启深度偏移
    rasterizer.depthBiasEnable = VK_FALSE;

    // 多重采样阶段信息
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // 指定是否开启多重采样
    multisampling.sampleShadingEnable = VK_FALSE;
    // 指定多重采样的采样级别
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // 深度模板测试阶段信息

    // 颜色混合阶段信息方式一：单独指定每个颜色附件的混合状态
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    // 指定颜色混合附件的通道
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // 指定是否开启颜色混合
    colorBlendAttachment.blendEnable = VK_FALSE;
    // 指定颜色混合因子
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    // 指定颜色混合操作
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    // 指定alpha混合因子
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    // 指定alpha混合操作
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // 颜色混合阶段信息方式二：指定全局的混合状态
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // 指定是否开启全局颜色混合
    colorBlending.logicOpEnable = VK_FALSE;
    // 指定逻辑操作
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    // 指定混合附件的数量
    colorBlending.attachmentCount = 1;
    // 指定混合附件的状态
    colorBlending.pAttachments = &colorBlendAttachment;
    // 指定混合常量
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // 动态状态：指定可以在绘制命令中修改的状态
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    // 创建管线布局
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(m_device->get_handle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // 创建图形管线
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // 指定管线阶段的数量
    pipelineInfo.stageCount = 2;
    // 指定管线阶段的信息
    pipelineInfo.pStages = shaderStages;
    // 指定顶点输入阶段的信息
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    // 指定输入装配阶段的信息
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    // 指定视口和裁剪矩形阶段的信息
    pipelineInfo.pViewportState = &viewportState;
    // 指定光栅化阶段的信息
    pipelineInfo.pRasterizationState = &rasterizer;
    // 指定多重采样阶段的信息
    pipelineInfo.pMultisampleState = &multisampling;
    // 指定深度模板测试阶段的信息
    pipelineInfo.pDepthStencilState = nullptr;
    // 指定颜色混合阶段的信息
    pipelineInfo.pColorBlendState = &colorBlending;
    // 指定动态状态的信息
    pipelineInfo.pDynamicState = &dynamicState;
    // 指定管线布局
    pipelineInfo.layout = m_pipelineLayout;
    // 指定渲染目标
    pipelineInfo.renderPass = m_renderPass;
    // 指定子管线索引
    pipelineInfo.subpass = 0;
    // 指定基础管线句柄
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    // 指定基础管线索引
    pipelineInfo.basePipelineIndex = -1;
    // 创建图形管线
    if (vkCreateGraphicsPipelines(m_device->get_handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // 销毁着色器模块
    vkDestroyShaderModule(m_device->get_handle(), fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device->get_handle(), vertShaderModule, nullptr);
}

std::vector<char> HelloTriangleApplication::readFile(const std::filesystem::path &filename)
{
    if (!std::filesystem::exists(filename))
    {
        throw std::runtime_error("file not found!");
    }

    // 打开文件：从尾部开始读取，二进制格式
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    // 从尾部开始读取文件，能够获取文件大小
    auto fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    // 从文件头开始读取文件内容
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char> &code)
{
    // 创建着色器模块信息
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    // 着色器代码的字节码指针类型转换
    createInfo.pCode = reinterpret_cast<const unsigned int *>(code.data());

    // 创建着色器模块
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device->get_handle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void HelloTriangleApplication::createFramebuffers()
{
    m_swapChainFramebuffers.resize(m_swapchain_image_views.size());

    for (size_t i = 0; i < m_swapchain_image_views.size(); i++)
    {
        VkImageView attachments[] = { m_swapchain_image_views[i].get_handle()};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchain->get_extent().width;
        framebufferInfo.height = m_swapchain->get_extent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device->get_handle(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void HelloTriangleApplication::createCommandPool()
{
    auto queueFamilyIndices = m_device->find_queue_family();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // 指定命令池分配的命令缓冲区，对应的队列族
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    // 指令池分配的命令缓冲区互相独立
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    // 创建指令池
    if (vkCreateCommandPool(m_device->get_handle(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void HelloTriangleApplication::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // 指定指令池
    allocInfo.commandPool = m_commandPool;
    // 指定为主命令缓冲区
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device->get_handle(), &allocInfo, &m_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void HelloTriangleApplication::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


    if (vkCreateSemaphore(m_device->get_handle(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_device->get_handle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(m_device->get_handle(), &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }

}

void HelloTriangleApplication::drawFrame()
{
    // 等待GPU执行完毕
    vkWaitForFences(m_device->get_handle(), 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device->get_handle(), 1, &m_inFlightFence);

    // 获取当前可用的交换链图像
    unsigned int imageIndex;
    vkAcquireNextImageKHR(m_device->get_handle(), m_swapchain->get_handle(), UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    // 重置命令缓冲区
    vkResetCommandBuffer(m_commandBuffer, 0);
    // 记录命令缓冲区
    recordCommandBuffer(m_commandBuffer, imageIndex);

    // 提交命令缓冲区
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // 等待信号量
    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    // 等待管线阶段
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitDstStageMask = waitStages;
    // 提交的命令缓冲区
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    // 信号量
    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(m_device->get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0).get_handle(), 1, &submitInfo, m_inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // 呈现信息
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // 等待信号量
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // 交换链
    VkSwapchainKHR swapChains[] = { m_swapchain->get_handle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    // 交换链图像索引
    presentInfo.pImageIndices = &imageIndex;
    // 提交队列
    vkQueuePresentKHR(m_device->get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0).get_handle(), &presentInfo);
}

void HelloTriangleApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, unsigned int imageIndex)
{
    // 开始记录命令
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // 渲染流程信息
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapchain->get_extent();
    // 清除颜色
    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    // 开始渲染流程
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // 绑定图形管线
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    {
        // 动态视口
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_swapchain->get_extent().width;
        viewport.height = (float)m_swapchain->get_extent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // 动态裁剪
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapchain->get_extent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    // 绘制
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    // 结束渲染流程
    vkCmdEndRenderPass(commandBuffer);

    // 结束记录命令
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

std::vector<const char *> HelloTriangleApplication::getRequiredInstanceExtensions()
{
    std::vector<const char *> extensions;

    // 获取glfw需要的扩展
    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

    // debug需要的扩展
    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

std::vector<const char *> HelloTriangleApplication::getRequiredDeviceExtensions()
{
    std::vector<const char *> extensions;

    // swapchain需要的扩展
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    return extensions;
}
