#pragma once

#include <unordered_set>

#include "volk.h"
#include "vulkan/vulkan_core.h"
#include "vk_mem_alloc.h"

#include "comet/vulkan/device.h"

namespace comet
{

    class ImageView;

    class Image
    {
    public:
        Image(const Device &device,
              VkImage handle,
              const VkExtent3D &extent,
              VkFormat format,
              VkImageUsageFlags image_usage,
              VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);

	    Image(Device const &        device,
            const VkExtent3D &    extent,
            VkFormat              format,
            VkImageUsageFlags     image_usage,
            VmaMemoryUsage        memory_usage,
            VkSampleCountFlagBits sample_count       = VK_SAMPLE_COUNT_1_BIT,
            uint32_t              mip_levels         = 1,
            uint32_t              array_layers       = 1,
            VkImageTiling         tiling             = VK_IMAGE_TILING_OPTIMAL,
            VkImageCreateFlags    flags              = 0,
            uint32_t              num_queue_families = 0,
            const uint32_t *      queue_families     = nullptr);

        ~Image() = default;

        Device &get_device() const;

        VkImage get_handle() const;

        VkImageType get_type() const;

        const VkExtent3D& get_extent() const;

        VkFormat get_format() const;

        VkImageSubresource get_subresource() const;

        std::unordered_set<ImageView *> &get_views();

        VkSampleCountFlagBits get_sample_count() const;     

        VkImageUsageFlags get_usage() const;   

    private:
        Device *m_device{};

        VkImage m_handle{};

        VkImageType m_type{};

        VkExtent3D m_extent{};

        VkFormat m_format{};

        VkImageUsageFlags m_usage{};

	    VkSampleCountFlagBits m_sample_count{};

	    VkImageTiling m_tiling{};

	    VkImageSubresource m_subresource{};

	    uint32_t m_array_layer_count{0};

        /// Image views referring to this image
        std::unordered_set<ImageView *> m_views;

        VmaAllocation m_memory{VK_NULL_HANDLE};

        uint8_t *m_mapped_data{nullptr};

	    /// Whether it was mapped with vmaMapMemory
	    bool m_mapped{false};
    };
} // namespace comet