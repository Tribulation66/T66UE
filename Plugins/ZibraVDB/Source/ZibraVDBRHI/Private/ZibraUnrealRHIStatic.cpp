// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "DynamicRHI.h"
#include "RHI.h"
#include "Runtime/Launch/Resources/Version.h"
#include "ZibraUnrealRHI.h"

namespace Zibra::RHI::APIUnrealRHI
{
	EBufferUsageFlags ZibraUnrealRHI::ConvertToERHIUsageFlags(ResourceHeapType HeapType, ResourceUsage Usage) noexcept
	{
		EBufferUsageFlags result = EBufferUsageFlags::None;

		switch (HeapType)
		{
			case ResourceHeapType::Default:
				result |= EBufferUsageFlags::Static;
				break;
			case ResourceHeapType::Upload:
				result |= EBufferUsageFlags::Dynamic;
				break;
			case ResourceHeapType::Readback:
				// Readback buffer should be created separately as a StagingBuffer.
				// Therefore, calling this method is forbitten for ResourceHeapType::Readback.
				checkNoEntry();
				break;
			default:
				checkNoEntry();
				break;
		}

		if (EnumHasAnyFlags(Usage, ResourceUsage::VertexBuffer))
		{
			result |= EBufferUsageFlags::VertexBuffer;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::IndexBuffer))
		{
			result |= EBufferUsageFlags::IndexBuffer;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::ConstantBuffer))
		{
			// Constant buffer should be created separately as a UniformBuffer in Unreal RHI.
			// Therefore, calling this method is forbitten for ResourceUsage::ConstantBuffer.
			checkNoEntry();
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::ShaderResource))
		{
			result |= EBufferUsageFlags::ShaderResource;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::UnorderedAccess))
		{
			result |= EBufferUsageFlags::UnorderedAccess;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::CopySource))
		{
			result |= EBufferUsageFlags::SourceCopy;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::CopyDest))
		{
			// nullopt.
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::Indirect))
		{
			result |= EBufferUsageFlags::DrawIndirect;
			// Unreal transitions indirect buffers to Shader Resource for no reason when doing indirect calls
			// So we need to actually allow then to be used as Shader Resource
			result |= EBufferUsageFlags::ShaderResource;
		}

		return result;
	}

	ERHIAccess ZibraUnrealRHI::ConvertToInitialState(ResourceHeapType HeapType, ResourceUsage Usage) noexcept
	{
		switch (HeapType)
		{
			case ResourceHeapType::Default:
				// Deduce based on Usage
				break;
			case ResourceHeapType::Upload:
				return ERHIAccess::CopySrc;
			case ResourceHeapType::Readback:
				return ERHIAccess::CopyDest;
			default:
				checkNoEntry();
				break;
		}

		if (EnumHasAnyFlags(Usage, ResourceUsage::VertexBuffer) || EnumHasAnyFlags(Usage, ResourceUsage::IndexBuffer))
		{
			return ERHIAccess::VertexOrIndexBuffer;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::ConstantBuffer))
		{
			return ERHIAccess::CopyDest;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::ShaderResource))
		{
			return ERHIAccess::SRVMask;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::UnorderedAccess))
		{
			return ERHIAccess::UAVMask;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::CopySource))
		{
			return ERHIAccess::CopySrc;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::CopyDest))
		{
			return ERHIAccess::CopyDest;
		}
		if (EnumHasAnyFlags(Usage, ResourceUsage::Indirect))
		{
			return ERHIAccess::IndirectArgs;
		}
		// Resource must have at least some usage flags for creation to make sense
		checkNoEntry();
		// Return "safe-ish" state if we don't know what to do
		return ERHIAccess::CopySrc;
	}

	ESamplerAddressMode ZibraUnrealRHI::ConvertToESamplerAddressMode(SamplerAddressMode AddressMode) noexcept
	{
		switch (AddressMode)
		{
			case SamplerAddressMode::Wrap:
				return ESamplerAddressMode::AM_Wrap;
			case SamplerAddressMode::Clamp:
				return ESamplerAddressMode::AM_Clamp;
			default:
				checkNoEntry();
				return ESamplerAddressMode::AM_Clamp;
		}
	}

	ESamplerFilter ZibraUnrealRHI::ConvertToESamplerFilter(SamplerFilter Filter) noexcept
	{
		switch (Filter)
		{
			case SamplerFilter::Point:
				return ESamplerFilter::SF_Point;
			case SamplerFilter::Trilinear:
				return ESamplerFilter::SF_Trilinear;
			case SamplerFilter::Aniso16x:
				return ESamplerFilter::SF_AnisotropicPoint;
			default:
				checkNoEntry();
				return ESamplerFilter::SF_Point;
		}
	}

	int32 ZibraUnrealRHI::GetMaxAnisotrophy(SamplerFilter Filter) noexcept
	{
		switch (Filter)
		{
			case SamplerFilter::Aniso16x:
				return 16;
			default:
				return 0;
		}
	}

	ECompareFunction ZibraUnrealRHI::ConvertToUnrealComparisonFunc(DepthComparisonFunc DepthComparison) noexcept
	{
		switch (DepthComparison)
		{
			case DepthComparisonFunc::Less:
				return ECompareFunction::CF_Less;
			case DepthComparisonFunc::LessEqual:
				return ECompareFunction::CF_LessEqual;
			case DepthComparisonFunc::Greater:
				return ECompareFunction::CF_Greater;
			case DepthComparisonFunc::GreaterEqual:
				return ECompareFunction::CF_GreaterEqual;
			case DepthComparisonFunc::Equal:
				return ECompareFunction::CF_Equal;
			case DepthComparisonFunc::Always:
				return ECompareFunction::CF_Always;
			default:
				checkNoEntry();
				return ECompareFunction::CF_Less;
		}
	}

	ERasterizerCullMode ZibraUnrealRHI::ConvertToUnrealCullMode(TriangleCullMode CullMode, bool bFrontCounterClockwise) noexcept
	{
		switch (CullMode)
		{
			case TriangleCullMode::None:
				return ERasterizerCullMode::CM_None;
			case TriangleCullMode::CullBack:
				return bFrontCounterClockwise ? ERasterizerCullMode::CM_CW : ERasterizerCullMode::CM_CCW;
			case TriangleCullMode::CullFront:
				return bFrontCounterClockwise ? ERasterizerCullMode::CM_CCW : ERasterizerCullMode::CM_CW;
			default:
				checkNoEntry();
				return ERasterizerCullMode::CM_None;
		}
	}

	ERasterizerFillMode ZibraUnrealRHI::ConvertToUnrealFillMode(bool IsWireframeMode) noexcept
	{
		return IsWireframeMode ? ERasterizerFillMode::FM_Wireframe : ERasterizerFillMode::FM_Solid;
	}

	EPrimitiveType ZibraUnrealRHI::ConvertToUnrealPrimitiveTopology(PrimitiveTopology Topology) noexcept
	{
		switch (Topology)
		{
			case PrimitiveTopology::TriangleList:
				return EPrimitiveType::PT_TriangleList;
			default:
				checkNoEntry();
				return EPrimitiveType::PT_TriangleList;
		}
	}

	EVertexElementType ZibraUnrealRHI::ConvertToUnrealVertexInputFormat(InputElementFormat Format, uint32_t ElementCount) noexcept
	{
		switch (Format)
		{
			case InputElementFormat::Float16:
				switch (ElementCount)
				{
					case 2:
						return VET_Half2;
					case 4:
						return VET_Half4;
					default:
						checkNoEntry();
						return VET_None;
				}
			case InputElementFormat::Float32:
				switch (ElementCount)
				{
					case 1:
						return VET_Float1;
					case 2:
						return VET_Float2;
					case 3:
						return VET_Float3;
					case 4:
						return VET_Float4;
					default:
						checkNoEntry();
						return VET_None;
				}
			case InputElementFormat::Uint16:
				switch (ElementCount)
				{
					case 2:
						return VET_UShort2;
					case 4:
						return VET_UShort4;
					default:
						checkNoEntry();
						return VET_None;
				}
			case InputElementFormat::Uint32:
				switch (ElementCount)
				{
					case 1:
						return VET_UInt;
					default:
						checkNoEntry();
						return VET_None;
				}
			default:
				checkNoEntry();
				return VET_None;
		}
	}

	uint32_t ZibraUnrealRHI::ConvertToUnrealVertexInputStride(InputElementFormat format, uint32_t ElementCount) noexcept
	{
		switch (format)
		{
			case InputElementFormat::Float16:
				switch (ElementCount)
				{
					case 2:
						return sizeof(float);
					case 4:
						return sizeof(float) * 2;
					default:
						checkNoEntry();
						return 0;
				}
			case InputElementFormat::Float32:
			{
				check(ElementCount <= 4);
				return sizeof(float) * ElementCount;
			}
			case InputElementFormat::Uint16:
				switch (ElementCount)
				{
					case 2:
						return sizeof(short) * 2;
					case 4:
						return sizeof(short) * 4;
					default:
						checkNoEntry();
						return 0;
				}
			case InputElementFormat::Uint32:
				switch (ElementCount)
				{
					case 1:
						return sizeof(uint32_t);
					default:
						checkNoEntry();
						return 0;
				}
			default:
				checkNoEntry();
				return 0;
		}
	}

	uint32_t ZibraUnrealRHI::ConvertToNumVertices(uint32_t VertexBufferSize, uint32_t VertexBufferStride) noexcept
	{
		check(VertexBufferStride > 0);
		return VertexBufferSize / VertexBufferStride;
	}

	uint32_t ZibraUnrealRHI::ConvertToNumPrimitives(uint32_t IndexCount, EPrimitiveType Topology) noexcept
	{
		switch (Topology)
		{
			case EPrimitiveType::PT_TriangleList:
				return IndexCount / 3;
			default:
				checkNoEntry();
				return 0u;
		}
	}

	EPixelFormat ZibraUnrealRHI::ConvertToEPixelFormat(TextureFormat Format) noexcept
	{
		switch (Format)
		{
			case TextureFormat::R8G8B8A8_UNorm:
				return PF_R8G8B8A8;	   // Standard RGBA format, 8 bits per channel, mapped to normalized format.
			case TextureFormat::R16G16B16A16_SFloat:
				return PF_FloatRGBA;	// 16-bit floating point per channel RGBA
			case TextureFormat::R32G32B32A32_SFloat:
				return PF_A32B32G32R32F;
			case TextureFormat::R16_SFloat:
				return PF_R16F;
			case TextureFormat::R32_SFloat:
				return PF_R32_FLOAT;
			case TextureFormat::D32_SFloat_S8_UInt:
				return PF_DepthStencil;
			default:
				checkNoEntry();
				return PF_Unknown;
		}
	}

	EPixelFormat ZibraUnrealRHI::ConvertToEPixelFormat(BufferFormat Format) noexcept
	{
		switch (Format)
		{
			case BufferFormat::R16_SFloat:
				return PF_R16F;
			case BufferFormat::R32_UINT:
				return PF_R32_UINT;
			default:
				checkNoEntry();
			return PF_Unknown;
		}
	}

	ETextureCreateFlags ZibraUnrealRHI::ConvertToETextureCreateFlags(ResourceUsage Usage) noexcept
	{
		ETextureCreateFlags Flags = TexCreate_None;

		if (EnumHasAnyFlags(Usage, ResourceUsage::ShaderResource))
		{
			Flags |= TexCreate_ShaderResource;
		}

		if (EnumHasAnyFlags(Usage, ResourceUsage::UnorderedAccess))
		{
			Flags |= TexCreate_UAV;
		}

		return Flags;
	}

}	 // namespace Zibra::RHI::APIUnrealRHI
