#include "comet/vulkan/image_view.h"

#include <stdexcept>
#include "comet/vulkan/common.h"
#include "vulkan/vulkan_core.h"

using namespace comet;

ImageView::ImageView(Image &image, VkImageViewType view_type, VkFormat format,
                     uint32_t base_mip_level, uint32_t base_array_layer,
                     uint32_t n_mip_levels, uint32_t n_array_layers)
    : m_device{&image.get_device()}, m_image{&image}, m_format{format}
{
    if (format == VK_FORMAT_UNDEFINED)
    {
        this->m_format = format = m_image->get_format();
    }

    m_subresource_range.baseMipLevel = base_mip_level;
    m_subresource_range.baseArrayLayer = base_array_layer;
    m_subresource_range.levelCount = n_mip_levels == 0 ? m_image->get_subresource().mipLevel : n_mip_levels;
    m_subresource_range.layerCount = n_array_layers == 0 ? m_image->get_subresource().arrayLayer : n_array_layers;

    if (is_depth_format(format))
    {
        m_subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
    {
        m_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = m_image->get_handle();
    view_info.viewType = view_type;
    view_info.format = format;
    view_info.subresourceRange = m_subresource_range;

    if (vkCreateImageView(m_device->get_handle(), &view_info, nullptr, &m_handle) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image view");
    }

    // Register this image view to its image
    // in order to be notified when it gets moved
    m_image->get_views().emplace(this);
}

ImageView::ImageView(ImageView &&other)
    : m_device{other.m_device}, m_handle{other.m_handle}, m_image{other.m_image}, m_format{other.m_format}, m_subresource_range{other.m_subresource_range}
{
    // Remove old view from image set and add this new one
    auto &views = m_image->get_views();
    views.erase(&other);
    views.emplace(this);

    other.m_handle = VK_NULL_HANDLE;
}

ImageView::~ImageView()
{
    vkDestroyImageView(m_device->get_handle(), m_handle, nullptr);
}

VkImageView ImageView::get_handle() const
{
    return m_handle;
}

VkFormat ImageView::get_format() const
{
    return m_format;
}

VkImageSubresourceRange ImageView::get_subresource_range() const
{
    return m_subresource_range;
}

const Image &ImageView::get_image() const
{
    return *m_image;
}
