#pragma once

#include "../../Interface/GpuPipelineState/GpuBlendState.hpp"
#include "../DirectX12Utility.hpp"

#ifdef __ENABLE__DIRECTX12__

namespace CodeRed {

	class DirectX12BlendState final : public GpuBlendState {
	public:
		explicit DirectX12BlendState(
			const std::shared_ptr<GpuLogicalDevice> &device,
			const BlendProperty& blend_property);

		~DirectX12BlendState() = default;

		auto constexpr state() const noexcept -> D3D12_BLEND_DESC { return mBlendState; }
	private:
		D3D12_BLEND_DESC mBlendState = {};
	};
	
}

#endif