#pragma once

#include "../Interface/GpuCommandQueue.hpp"

#include "DirectX12Utility.hpp"

#ifdef __ENABLE__DIRECTX12__

namespace CodeRed {

	class DirectX12CommandQueue final : public GpuCommandQueue {
	public:
		explicit DirectX12CommandQueue(
			const std::shared_ptr<GpuLogicalDevice>& device);

		~DirectX12CommandQueue() = default;

		auto queue() const noexcept -> WRL::ComPtr<ID3D12CommandQueue> { return mCommandQueue; }
	protected:
		WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	};
	
}

#endif