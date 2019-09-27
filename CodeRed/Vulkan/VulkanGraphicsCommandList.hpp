#pragma once

#include "../Interface/GpuGraphicsCommandList.hpp"
#include "../Shared/Attachment.hpp"
#include "VulkanUtility.hpp"

#include <optional>

#ifdef __ENABLE__VULKAN__

namespace CodeRed {

	class VulkanResourceLayout;
	class VulkanFrameBuffer;
	class VulkanRenderPass;
	
	class VulkanGraphicsCommandList final : public GpuGraphicsCommandList {
	public:
		explicit VulkanGraphicsCommandList(
			const std::shared_ptr<GpuLogicalDevice>& device,
			const std::shared_ptr<GpuCommandAllocator>& allocator);

		~VulkanGraphicsCommandList();

		void beginRecoding() override;

		void endRecoding() override;

		void beginRenderPass(
			const std::shared_ptr<GpuRenderPass>& render_pass, 
			const std::shared_ptr<GpuFrameBuffer>& frame_buffer) override;

		void endRenderPass() override;
		
		void setGraphicsPipeline(
			const std::shared_ptr<GpuGraphicsPipeline>& pipeline) override;

		void setResourceLayout(
			const std::shared_ptr<GpuResourceLayout>& layout) override;

		void setVertexBuffer(
			const std::shared_ptr<GpuBuffer>& buffer) override;

		void setIndexBuffer(
			const std::shared_ptr<GpuBuffer>& buffer) override;

		void setGraphicsConstantBuffer(
			const size_t index,
			const std::shared_ptr<GpuBuffer>& buffer) override;

		void setGraphicsTexture(
			const size_t index,
			const std::shared_ptr<GpuTexture>& texture) override;

		void setViewPort(
			const ViewPort& view_port) override;

		void setScissorRect(
			const ScissorRect& rect) override;

		void layoutTransition(
			const std::shared_ptr<GpuTexture>& texture,
			const ResourceLayout old_layout,
			const ResourceLayout new_layout) override;

		void layoutTransition(
			const std::shared_ptr<GpuBuffer>& buffer,
			const ResourceLayout old_layout,
			const ResourceLayout new_layout) override;

		void copyBuffer(
			const std::shared_ptr<GpuBuffer>& source,
			const std::shared_ptr<GpuBuffer>& destination,
			const size_t size,
			const size_t source_offset = 0,
			const size_t destination_offset = 0) override;

		void copyTexture(
			const std::shared_ptr<GpuTexture>& source,
			const std::shared_ptr<GpuTexture>& destination,
			const Extent3D<UInt32>& region,
			const size_t x = 0,
			const size_t y = 0,
			const size_t z = 0) override;

		void draw(
			const size_t vertex_count,
			const size_t instance_count = 1,
			const size_t start_vertex_location = 0,
			const size_t start_instance_location = 0) override;

		void drawIndexed(
			const size_t index_count,
			const size_t instance_count,
			const size_t start_index_location,
			const size_t base_vertex_location,
			const size_t start_instance_location) override;

		auto commandList() const noexcept -> vk::CommandBuffer { return mCommandBuffer; }
	private:
		void tryLayoutTransition(
			const std::shared_ptr<GpuTexture>& texture,
			const std::optional<Attachment>& attachment,
			const bool final = false);
	private:
		vk::CommandBuffer mCommandBuffer;

		std::shared_ptr<VulkanResourceLayout> mResourceLayout;
		std::shared_ptr<VulkanFrameBuffer> mFrameBuffer;
		std::shared_ptr<VulkanRenderPass> mRenderPass;
	};
	
}

#endif