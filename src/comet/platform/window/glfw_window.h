#pragma once

#include <string>

#include "volk.h"
#include "GLFW/glfw3.h"

#include "comet/core/window.h"

namespace comet
{

    class GlfwWindow : public Window
    {
    public:
        GlfwWindow(const Window::Properties &properties);

        ~GlfwWindow() override;

        bool should_close() override;

        void process_events() override;

        void close() override;

        std::vector<const char *> get_required_extensions() const override;

        VkSurfaceKHR create_surface(VkInstance instance, VkPhysicalDevice physical_device) override;

    private:
        GLFWwindow *m_handle{nullptr};
    };

} // namespace comet