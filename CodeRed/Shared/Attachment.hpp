#pragma once

#ifdef __CODE__RED__GLOBAL__INCLUDE__
#include <Shared/Enum/AttachmentStore.hpp>
#include <Shared/Enum/AttachmentLoad.hpp>
#include <Shared/Enum/ResourceLayout.hpp>
#include <Shared/Enum/PixelFormat.hpp>
#else
#include "Enum/AttachmentStore.hpp"
#include "Enum/AttachmentLoad.hpp"
#include "Enum/ResourceLayout.hpp"
#include "Enum/PixelFormat.hpp"
#endif

namespace CodeRed {

	struct Attachment {
		PixelFormat Format = PixelFormat::Unknown;

		ResourceLayout InitialLayout = ResourceLayout::RenderTarget;
		ResourceLayout FinalLayout = ResourceLayout::Present;

		AttachmentLoad Load = AttachmentLoad::Clear;
		AttachmentStore Store = AttachmentStore::Store;

		AttachmentLoad StencilLoad = AttachmentLoad::DontCare;
		AttachmentStore StencilStore = AttachmentStore::DontCare;

		Attachment() = default;

		Attachment(
			const PixelFormat format,
			const ResourceLayout initial_layout,
			const ResourceLayout final_layout,
			const AttachmentLoad load,
			const AttachmentStore store,
			const AttachmentLoad stencil_load,
			const AttachmentStore stencil_store) :
			Format(format),
			InitialLayout(initial_layout),
			FinalLayout(final_layout),
			Load(load), Store(store),
			StencilLoad(stencil_load),
			StencilStore(stencil_store) {}

		static Attachment RenderTarget(
			const PixelFormat format,
			const ResourceLayout initial_layout = ResourceLayout::RenderTarget,
			const ResourceLayout final_layout = ResourceLayout::Present,
			const AttachmentLoad load = AttachmentLoad::Clear,
			const AttachmentStore store = AttachmentStore::Store)
		{
			return Attachment(
				format,
				initial_layout,
				final_layout,
				load, store,
				AttachmentLoad::DontCare,
				AttachmentStore::DontCare
			);
		}

		static Attachment DepthStencil(
			const PixelFormat format,
			const ResourceLayout initial_layout = ResourceLayout::DepthStencil,
			const ResourceLayout final_layout = ResourceLayout::DepthStencil,
			const AttachmentLoad load = AttachmentLoad::DontCare,
			const AttachmentStore store = AttachmentStore::Store,
			const AttachmentLoad stencil_load = AttachmentLoad::DontCare,
			const AttachmentStore stencil_store = AttachmentStore::DontCare)
		{
			return Attachment(
				format,
				initial_layout,
				final_layout,
				load, store,
				stencil_load,
				stencil_store
			);
		}
	};
	
}