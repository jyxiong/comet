#pragma once

#include "volk.h"
#include "comet/vulkan/device.h"
#include "comet/vulkan/image.h"

namespace comet
{
    class ImageView
    {
    public:
        ImageView(Image &image, VkImageViewType view_type, VkFormat format = VK_FORMAT_UNDEFINED,
                  uint32_t base_mip_level = 0, uint32_t base_array_layer = 0,
                  uint32_t n_mip_levels = 0, uint32_t n_array_layers = 0);

        ImageView(ImageView &) = delete;

        ImageView(ImageView &&other);

        ~ImageView();

        ImageView &operator=(const ImageView &) = delete;

        ImageView &operator=(ImageView &&) = delete;

        VkImageView get_handle() const;

        VkFormat get_format() const;

        VkImageSubresourceRange get_subresource_range() const;

        const Image& get_image() const;

    private:
        Device *m_device{};

        Image *m_image{};

        VkImageView m_handle{};

        VkFormat m_format{};

        VkImageSubresourceRange m_subresource_range{};
    };
} // namespace comet