#include "DirectX12TextureRef.hpp"

#include "DirectX12Resource/DirectX12Texture.hpp"

#ifdef __ENABLE__DIRECTX12__

using namespace CodeRed::DirectX12;

CodeRed::DirectX12TextureRef::DirectX12TextureRef(
	const std::shared_ptr<GpuTexture>& texture,
	const TextureRefInfo& info) : GpuTextureRef(texture, info)
{
	mDesc.Format = enumConvert(mInfo.Format == PixelFormat::Unknown ? mTexture->format() : mInfo.Format);
	mDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (mTexture->dimension()) {
	case Dimension::Dimension1D:
		{
			if (mInfo.Array.size() != 1) {
				
				mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				mDesc.Texture1DArray.FirstArraySlice = static_cast<UINT>(mInfo.Array.Start);
				mDesc.Texture1DArray.MostDetailedMip = static_cast<UINT>(mInfo.MipLevel.Start);
				mDesc.Texture1DArray.ArraySize = static_cast<UINT>(mInfo.Array.size());
				mDesc.Texture1DArray.MipLevels = static_cast<UINT>(mInfo.MipLevel.size());
				mDesc.Texture1DArray.ResourceMinLODClamp = 0;
				
			}else {
				
				mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				mDesc.Texture1D.MostDetailedMip = static_cast<UINT>(mInfo.MipLevel.Start);
				mDesc.Texture1D.MipLevels = static_cast<UINT>(mInfo.MipLevel.size());
				mDesc.Texture1D.ResourceMinLODClamp = 0.0f;
				
			}

			break;
		}
	case Dimension::Dimension2D:
		{
			if (mInfo.Array.size() != 1) {
				if (mInfo.Usage == TextureRefUsage::Common) {

					if (mTexture->sample() == MultiSample::Count1) {

						mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
						mDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(mInfo.Array.Start);
						mDesc.Texture2DArray.MostDetailedMip = static_cast<UINT>(mInfo.MipLevel.Start);
						mDesc.Texture2DArray.ArraySize = static_cast<UINT>(mInfo.Array.size());
						mDesc.Texture2DArray.MipLevels = static_cast<UINT>(mInfo.MipLevel.size());
						mDesc.Texture2DArray.ResourceMinLODClamp = 0;
						mDesc.Texture2DArray.PlaneSlice = 0;

					}else {

						mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
						mDesc.Texture2DMSArray.FirstArraySlice = static_cast<UINT>(mInfo.Array.Start);
						mDesc.Texture2DMSArray.ArraySize = static_cast<UINT>(mInfo.Array.size());
						
					}
				}
				else {

					if (mInfo.Array.size() > 6) {

						mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						mDesc.TextureCubeArray.MostDetailedMip = static_cast<UINT>(mInfo.MipLevel.Start);
						mDesc.TextureCubeArray.MipLevels = static_cast<UINT>(mInfo.MipLevel.size());
						mDesc.TextureCubeArray.First2DArrayFace = static_cast<UINT>(mInfo.Array.Start);
						mDesc.TextureCubeArray.NumCubes = static_cast<UINT>(mInfo.Array.size() / 6);
						mDesc.TextureCubeArray.ResourceMinLODClamp = 0;
						
					}
					else {

						mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						mDesc.TextureCube.MostDetailedMip = static_cast<UINT>(mInfo.MipLevel.Start);
						mDesc.TextureCube.MipLevels = static_cast<UINT>(mInfo.MipLevel.size());
						mDesc.TextureCube.ResourceMinLODClamp = 0;

					}
				}
			}else {

				if (mTexture->sample() == MultiSample::Count1) {

					mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					mDesc.Texture2D.MostDetailedMip = static_cast<UINT>(mInfo.MipLevel.Start);
					mDesc.Texture2D.MipLevels = static_cast<UINT>(mInfo.MipLevel.size());
					mDesc.Texture2D.ResourceMinLODClamp = 0;
					mDesc.Texture2D.PlaneSlice = 0;
					
				}else {

					mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
					
				}
			}

			break;
		}
	case Dimension::Dimension3D:
		{
			mDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			mDesc.Texture3D.MostDetailedMip = static_cast<UINT>(mInfo.MipLevel.Start);
			mDesc.Texture3D.MipLevels = static_cast<UINT>(mInfo.MipLevel.size());
			mDesc.Texture3D.ResourceMinLODClamp = 0;

			break;
		}
	}
}

#endif