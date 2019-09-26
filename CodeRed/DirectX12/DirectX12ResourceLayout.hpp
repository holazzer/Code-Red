#pragma once

#ifdef __CODE__RED__GLOBAL__INCLUDE__
#include <Interface/GpuResourceLayout.hpp>
#include <DirectX12/DirectX12Utility.hpp>
#else
#include "../Interface/GpuResourceLayout.hpp"
#include "DirectX12Utility.hpp"
#endif

#ifdef __ENABLE__DIRECTX12__

namespace CodeRed {

	class DirectX12ResourceLayout final : public GpuResourceLayout {
	public:
		explicit DirectX12ResourceLayout(
			const std::shared_ptr<GpuLogicalDevice>& device,
			const std::vector<ResourceLayoutElement>& elements,
			const std::vector<SamplerLayoutElement>& samplers);

		~DirectX12ResourceLayout() = default;

		void bindTexture(
			const size_t index, 
			const std::shared_ptr<GpuTexture>& resource) override;

		void bindBuffer(
			const size_t index, 
			const std::shared_ptr<GpuBuffer>& resource) override;
		
		auto rootSignature() const noexcept -> WRL::ComPtr<ID3D12RootSignature> { return mRootSignature; }

		auto descriptorHeap() const noexcept -> WRL::ComPtr<ID3D12DescriptorHeap> { return mDescriptorHeap; }
	protected:
		WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		WRL::ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;

		size_t mDescriptorSize = 0;
	};
	
}

#endif