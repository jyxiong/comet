#include "comet/vulkan/image.h"

#include <cassert>
#include <stdexcept>

using namespace comet;

namespace
{
inline VkImageType find_image_type(VkExtent3D extent)
{
	VkImageType result{};

	uint32_t dim_num{0};

	if (extent.width >= 1)
	{
		dim_num++;
	}

	if (extent.height >= 1)
	{
		dim_num++;
	}

	if (extent.depth > 1)
	{
		dim_num++;
	}

	switch (dim_num)
	{
		case 1:
			result = VK_IMAGE_TYPE_1D;
			break;
		case 2:
			result = VK_IMAGE_TYPE_2D;
			break;
		case 3:
			result = VK_IMAGE_TYPE_3D;
			break;
		default:
			throw std::runtime_error("No image type found.");
			break;
	}

	return result;
}
}        // namespace

Image::Image(Device const &device,
             VkImage handle,
             const VkExtent3D &extent,
             VkFormat format,
             VkImageUsageFlags image_usage,
             VkSampleCountFlagBits sample_count)
    : m_device{const_cast<Device *>(&device)},
      m_handle{handle},
        m_type{find_image_type(extent)},
      m_format{format},
      m_extent{extent},
        m_usage{image_usage},
        m_sample_count{sample_count}

{
    m_subresource.mipLevel = 1;
    m_subresource.arrayLayer = 1;
}

Image::Image(Device const         &device,
             const VkExtent3D     &extent,
             VkFormat              format,
             VkImageUsageFlags     image_usage,
             VmaMemoryUsage        memory_usage,
             VkSampleCountFlagBits sample_count,
             const uint32_t        mip_levels,
             const uint32_t        array_layers,
             VkImageTiling         tiling,
             VkImageCreateFlags    flags,
             uint32_t              num_queue_families,
             const uint32_t       *queue_families) :
    m_device{const_cast<Device *>(&device)},
    m_type{find_image_type(extent)},
    m_extent{extent},
    m_format{format},
    m_usage{image_usage},
    m_sample_count{sample_count},
    m_tiling{tiling},
    m_array_layer_count{array_layers}
{
	assert(mip_levels > 0 && "Image should have at least one level");
	assert(array_layers > 0 && "Image should have at least one layer");

	m_subresource.mipLevel   = mip_levels;
	m_subresource.arrayLayer = array_layers;

	VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	image_info.flags       = flags;
	image_info.imageType   = m_type;
	image_info.format      = format;
	image_info.extent      = extent;
	image_info.mipLevels   = mip_levels;
	image_info.arrayLayers = array_layers;
	image_info.samples     = sample_count;
	image_info.tiling      = tiling;
	image_info.usage       = image_usage;

	if (num_queue_families != 0)
	{
		image_info.sharingMode           = VK_SHARING_MODE_CONCURRENT;
		image_info.queueFamilyIndexCount = num_queue_families;
		image_info.pQueueFamilyIndices   = queue_families;
	}

	VmaAllocationCreateInfo memory_info{};
	memory_info.usage = memory_usage;

	if (image_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
	{
		memory_info.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
	}

	if (vmaCreateImage(device.get_memory_allocator(),
	                             &image_info, &memory_info,
	                             &m_handle, &m_memory,
	                             nullptr))
	{
		throw std::runtime_error("Failed to create image");
	}
}

Device &Image::get_device() const
{
    return *m_device;
}

VkImage Image::get_handle() const
{
    return m_handle;
}

VkImageType Image::get_type() const
{
	return m_type;
}

const VkExtent3D &Image::get_extent() const
{
	return m_extent;
}

VkFormat Image::get_format() const
{
    return m_format;
}

VkImageSubresource Image::get_subresource() const
{
    return m_subresource;
}

std::unordered_set<ImageView *> &Image::get_views()
{
    return m_views;
}

VkSampleCountFlagBits Image::get_sample_count() const
{
	return m_sample_count;
}

VkImageUsageFlags Image::get_usage() const
{
	return m_usage;
}
