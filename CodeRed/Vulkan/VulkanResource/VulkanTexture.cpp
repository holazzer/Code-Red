#include "../VulkanLogicalDevice.hpp"
#include "VulkanTexture.hpp"

#ifdef __ENABLE__VULKAN__

using namespace CodeRed::Vulkan;

CodeRed::VulkanTexture::VulkanTexture(
	const std::shared_ptr<GpuLogicalDevice>& device, 
	const ResourceInfo& info) :
	GpuTexture(device, info)
{
	vk::ImageViewCreateInfo viewInfo = {};
	vk::MemoryAllocateInfo memoryInfo = {};
	vk::ImageCreateInfo imageInfo = {};

	const auto property = std::get<TextureProperty>(mInfo.Property);
	
	imageInfo
		.setPNext(nullptr)
		.setFlags(vk::ImageCreateFlags(0))
		.setImageType(enumConvert(property.Dimension))
		.setFormat(enumConvert(property.PixelFormat))
		.setExtent(vk::Extent3D(
			static_cast<uint32_t>(property.Width),
			static_cast<uint32_t>(property.Height),
			static_cast<uint32_t>(property.Depth)))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(enumConvert(mInfo.Usage).second)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setSharingMode(vk::SharingMode::eExclusive);

	const auto vkDevice = std::static_pointer_cast<VulkanLogicalDevice>(mDevice);

	mImage = vkDevice->device().createImage(imageInfo);

	const auto memoryRequirement = vkDevice->device().getImageMemoryRequirements(mImage);

	memoryInfo
		.setPNext(nullptr)
		.setAllocationSize(memoryRequirement.size)
		.setMemoryTypeIndex(
			vkDevice->getMemoryTypeIndex(memoryRequirement.memoryTypeBits,
				enumConvert(mInfo.Heap)));

	mMemory = vkDevice->device().allocateMemory(memoryInfo);

	vkDevice->device().bindImageMemory(mImage, mMemory, 0);

	auto imageAspectFlags = vk::ImageAspectFlags(0);

	if (enumHas(mInfo.Usage, ResourceUsage::DepthStencil))
		imageAspectFlags = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	else imageAspectFlags = vk::ImageAspectFlagBits::eColor;
	
	viewInfo
		.setPNext(nullptr)
		.setFlags(vk::ImageViewCreateFlags(0))
		.setImage(mImage)
		.setFormat(enumConvert(property.PixelFormat))
		.setComponents(vk::ComponentMapping(
			vk::ComponentSwizzle::eR,
			vk::ComponentSwizzle::eG,
			vk::ComponentSwizzle::eB,
			vk::ComponentSwizzle::eA))
		.setSubresourceRange(vk::ImageSubresourceRange(
			imageAspectFlags,
			0, 1, 0, 1))
		.setViewType(enumConvert1(property.Dimension));

	mImageView = vkDevice->device().createImageView(viewInfo);
}

CodeRed::VulkanTexture::VulkanTexture(
	const std::shared_ptr<GpuLogicalDevice>& device, 
	const ResourceInfo& info,
	const vk::Image image) :
	GpuTexture(device, info),
	mMemory(nullptr),
	mImage(image)
{
	//this ctor version is used for swapchain
	//we use this ctor to create GpuTexture in swap chain.
	const auto vkDevice = std::static_pointer_cast<VulkanLogicalDevice>(mDevice);
	const auto property = std::get<TextureProperty>(mInfo.Property);

	auto imageAspectFlags = vk::ImageAspectFlags(0);

	if (enumHas(mInfo.Usage, ResourceUsage::DepthStencil))
		imageAspectFlags = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	else imageAspectFlags = vk::ImageAspectFlagBits::eColor;


	vk::ImageViewCreateInfo viewInfo = {};
	
	viewInfo
		.setPNext(nullptr)
		.setFlags(vk::ImageViewCreateFlags(0))
		.setImage(mImage)
		.setFormat(enumConvert(property.PixelFormat))
		.setComponents(vk::ComponentMapping(
			vk::ComponentSwizzle::eR,
			vk::ComponentSwizzle::eG,
			vk::ComponentSwizzle::eB,
			vk::ComponentSwizzle::eA))
		.setSubresourceRange(vk::ImageSubresourceRange(
			imageAspectFlags,
			0, 1, 0, 1))
		.setViewType(enumConvert1(property.Dimension));

	mImageView = vkDevice->device().createImageView(viewInfo);
}

CodeRed::VulkanTexture::~VulkanTexture()
{
	const auto vkDevice = std::static_pointer_cast<VulkanLogicalDevice>(mDevice)->device();

	vkDevice.destroyImageView(mImageView);

	//vulkan texture for swapchain
	//so we do not need to destroy memory and image
	//we will do this when we destroy the swapchain
	if (mMemory) {
		vkDevice.freeMemory(mMemory);
		vkDevice.destroyImage(mImage);
	}
}

auto CodeRed::VulkanTexture::mapMemory() const -> void* 
{
	CODE_RED_THROW_IF(
		!mMemory,
		Exception("This texture is not support map memory.")
	);

	const auto vkDevice = std::static_pointer_cast<VulkanLogicalDevice>(mDevice)->device();

	return vkDevice.mapMemory(mMemory, 0, VK_WHOLE_SIZE);
}

void CodeRed::VulkanTexture::unmapMemory() const
{
	CODE_RED_THROW_IF(
		!mMemory,
		Exception("This texture is not support unmap memory.")
	);

	const auto vkDevice = std::static_pointer_cast<VulkanLogicalDevice>(mDevice)->device();

	vkDevice.unmapMemory(mMemory);
}

#endif