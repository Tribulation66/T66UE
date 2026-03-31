// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraUnrealRHI.h"

#include "DynamicRHI.h"
#include "GPUProfiler.h"
#include "GlobalShader.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/UnrealMemory.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "PixelShaderUtils.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "RenderGraphUtils.h"
#include "RenderingThread.h"
#include "ZibraUnrealShadersInterface.h"

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
#include "RHIUniformBufferLayoutInitializer.h"
#endif

namespace Zibra::RHI::APIUnrealRHI
{
	ZibraUnrealRHI::ZibraUnrealRHI()
	{
	}

	ZibraUnrealRHI::~ZibraUnrealRHI()
	{
	}

	ZibraUnrealRHI& ZibraUnrealRHI::Get()
	{
		static Zibra::RHI::APIUnrealRHI::ZibraUnrealRHI ZibraUnrealRHI;
		return ZibraUnrealRHI;
	}

	ReturnCode ZibraUnrealRHI::Initialize() noexcept
	{
		return ReturnCode::ZRHI_SUCCESS;
	}

	void ZibraUnrealRHI::Release() noexcept
	{
		if (this != &Get())
			delete this;
	}

	GFXAPI ZibraUnrealRHI::GetGFXAPI() const noexcept
	{
		return GFXAPI::Custom;
	}

	bool ZibraUnrealRHI::QueryFeatureSupport(Feature feature) const noexcept
	{
		return false;
	}

	ReturnCode ZibraUnrealRHI::SetStablePowerState(bool enable) noexcept
	{
		return ReturnCode::ZRHI_ERROR_NOT_SUPPORTED;
	}

	ReturnCode ZibraUnrealRHI::CompileComputePSO(const ComputePSODesc& Desc, ComputePSO** OutPSO) noexcept
	{
		FUnrealComputePSO* ComputePSO = new FUnrealComputePSO{};
		FString ShaderName = "F" + FString(Desc.CSSize, static_cast<const char*>(Desc.CS));
		ComputePSO->CS = ZIBRA_SHADERS_MODULE_INTERFACE::GetShaderByName(ShaderName);
		check(ComputePSO->CS.IsValid());

		ZIB_CHEAP_GPU_DEBUG_ONLY(ComputePSO->CSShaderName = MoveTemp(ShaderName));

		*OutPSO = ComputePSO;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::CompileGraphicPSO(const GraphicsPSODesc& Desc, GraphicsPSO** OutPSO) noexcept
	{
		check(Desc.VS && Desc.VSSize > 0);
		check(Desc.PS && Desc.PSSize > 0);

		FUnrealGraphicsPSO* GraphicsPSO = new FUnrealGraphicsPSO{};

		FString VSName = "F" + FString(Desc.VSSize, static_cast<const char*>(Desc.VS));
		FString PSName = "F" + FString(Desc.PSSize, static_cast<const char*>(Desc.PS));
		GraphicsPSO->VS = ZIBRA_SHADERS_MODULE_INTERFACE::GetShaderByName(VSName);
		GraphicsPSO->PS = ZIBRA_SHADERS_MODULE_INTERFACE::GetShaderByName(PSName);
		check(GraphicsPSO->VS.IsValid());
		check(GraphicsPSO->PS.IsValid());

		ZIB_CHEAP_GPU_DEBUG_ONLY(GraphicsPSO->VSName = MoveTemp(VSName));
		ZIB_CHEAP_GPU_DEBUG_ONLY(GraphicsPSO->PSName = MoveTemp(PSName));

		FDepthStencilStateInitializerRHI DepthStencilStateInitializer;
		DepthStencilStateInitializer.bEnableDepthWrite = Desc.depthStencil.depthWrite;
		DepthStencilStateInitializer.DepthTest = ConvertToUnrealComparisonFunc(Desc.depthStencil.depthComparison);
		DepthStencilStateInitializer.bEnableFrontFaceStencil = false;
		DepthStencilStateInitializer.bEnableBackFaceStencil = false;

		GraphicsPSO->DepthStencilState = RHICreateDepthStencilState(DepthStencilStateInitializer);
		check(GraphicsPSO->DepthStencilState.IsValid());

		FRasterizerStateInitializerRHI RasterizerStateInitializer;
		RasterizerStateInitializer.CullMode =
			ConvertToUnrealCullMode(Desc.rasterizer.cullMode, Desc.rasterizer.frontCounterClockwise);
		RasterizerStateInitializer.FillMode = ConvertToUnrealFillMode(Desc.rasterizer.wireframe);
		RasterizerStateInitializer.DepthBias = 0.0;
		RasterizerStateInitializer.SlopeScaleDepthBias = 0.0;
		RasterizerStateInitializer.bAllowMSAA = false;
		// Note: Desc.Rasterizer.FrontCounterClockwise ignored. Unreal always sets FrontCounterClockwise to true.

		GraphicsPSO->RasterState = RHICreateRasterizerState(RasterizerStateInitializer);
		check(GraphicsPSO->RasterState.IsValid());

		FVertexDeclarationElementList InputElements;
		InputElements.Reserve(Desc.inputLayout.inputElementsCount);
		for (uint32_t Index = 0; Index < Desc.inputLayout.inputElementsCount; ++Index)
		{
			const InputElementDesc& CurrentElementDesc = Desc.inputLayout.inputElements[Index];
			FVertexElement Element;

			Element.AttributeIndex = 0;
			Element.Type = ConvertToUnrealVertexInputFormat(CurrentElementDesc.format, CurrentElementDesc.elementCount);
			Element.StreamIndex = 0;
			Element.Offset = CurrentElementDesc.offset;
			Element.bUseInstanceIndex = false;
			Element.Stride = ConvertToUnrealVertexInputStride(CurrentElementDesc.format, CurrentElementDesc.elementCount);

			InputElements.Add(Element);
		}

		GraphicsPSO->InputLayout = RHICreateVertexDeclaration(InputElements);
		check(GraphicsPSO->InputLayout.IsValid());

		GraphicsPSO->PrimitiveTopology = ConvertToUnrealPrimitiveTopology(Desc.topology);

		GraphicsPSO->BlendState = RHICreateBlendState(FBlendStateInitializerRHI(
			FBlendStateInitializerRHI::FRenderTarget(BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero, CW_RGBA), false));

		*OutPSO = GraphicsPSO;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ReleaseComputePSO(ComputePSO* Pso) noexcept
	{
		delete static_cast<FUnrealComputePSO*>(Pso);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ReleaseGraphicPSO(GraphicsPSO* Pso) noexcept
	{
		delete static_cast<FUnrealGraphicsPSO*>(Pso);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::RegisterBuffer(void* ResourceHandle, const char* Name, Buffer** OutBuffer) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(ResourceHandle);

		FUnrealRHIBuffer* UnrealBuffer = new FUnrealRHIBuffer{};

		FBufferBridge* BufferBridge = static_cast<FBufferBridge*>(ResourceHandle);
		check(BufferBridge->Format == EBufferFormat::Buffer);

		Name = Name ? Name : "ZibraUnrealRHIResource";

		UnrealBuffer->Buffer = static_cast<FRHIBuffer*>(BufferBridge->Buffer);
		UnrealBuffer->BufferType = EUnrealBufferType::Buffer;
		UnrealBuffer->CurrentState = ERHIAccess::Unknown;

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 507
		ZIB_CHEAP_GPU_DEBUG_ONLY(FRHICommandListImmediate::Get().BindDebugLabelName(
			static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference()), UTF8_TO_TCHAR(Name)));
#else
		ZIB_CHEAP_GPU_DEBUG_ONLY(
			RHIBindDebugLabelName(static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference()), UTF8_TO_TCHAR(Name)));
#endif

		*OutBuffer = UnrealBuffer;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::RegisterTexture2D(void* ResourceHandle, const char* Name, Texture2D** OutTexture) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(ResourceHandle);

		FUnrealRHITexture2D* UnrealTexture = new FUnrealRHITexture2D{};

		Name = Name ? Name : "ZibraUnrealRHIResource";

		UnrealTexture->Texture = static_cast<FRHITexture*>(ResourceHandle);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 507
		ZIB_CHEAP_GPU_DEBUG_ONLY(FRHICommandListImmediate::Get().BindDebugLabelName(UnrealTexture->Texture, UTF8_TO_TCHAR(Name)));
#else
		ZIB_CHEAP_GPU_DEBUG_ONLY(RHIBindDebugLabelName(UnrealTexture->Texture, UTF8_TO_TCHAR(Name)));
#endif

		*OutTexture = UnrealTexture;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::RegisterTexture3D(void* ResourceHandle, const char* Name, Texture3D** OutTexture) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(ResourceHandle);

		FUnrealRHITexture3D* UnrealTexture = new FUnrealRHITexture3D{};

		Name = Name ? Name : "ZibraUnrealRHIResource";

		UnrealTexture->Texture = static_cast<FRHITexture*>(ResourceHandle);
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 507
		ZIB_CHEAP_GPU_DEBUG_ONLY(FRHICommandListImmediate::Get().BindDebugLabelName(UnrealTexture->Texture, UTF8_TO_TCHAR(Name)));
#else
		ZIB_CHEAP_GPU_DEBUG_ONLY(RHIBindDebugLabelName(UnrealTexture->Texture, UTF8_TO_TCHAR(Name)));
#endif

		*OutTexture = UnrealTexture;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::CreateBuffer(size_t Size, ResourceHeapType HeapType, ResourceUsage Usage, uint32_t StructuredStride,
		const char* Name, Buffer** OutBuffer) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHIBuffer* UnrealBuffer = new FUnrealRHIBuffer{};

		UnrealBuffer->CurrentState = ConvertToInitialState(HeapType, Usage);

		Name = Name ? Name : "ZibraUnrealRHIResource";

		if (EnumHasAnyFlags(Usage, ResourceUsage::ConstantBuffer))
		{
			UnrealBuffer->BufferType = EUnrealBufferType::UniformBuffer;

			FRHIUniformBufferLayoutInitializer UniformBufferLayoutInitializer{UTF8_TO_TCHAR(Name), static_cast<uint32>(Size)};
			UniformBufferLayoutInitializer.ComputeHash();

			UnrealBuffer->UniformBufferLayout = new FRHIUniformBufferLayout(UniformBufferLayoutInitializer);

			UnrealBuffer->CPUBufferData = TUniquePtr<unsigned char, FFreeDeleter>(
				static_cast<unsigned char*>(FMemory::Malloc(Size, SHADER_PARAMETER_STRUCT_ALIGNMENT)));
			check(UnrealBuffer->CPUBufferData);

			UnrealBuffer->Buffer = RHICreateUniformBuffer(UnrealBuffer->CPUBufferData.Get(),
				static_cast<const FRHIUniformBufferLayout*>(UnrealBuffer->UniformBufferLayout.GetReference()),
				EUniformBufferUsage::UniformBuffer_MultiFrame);
			check(UnrealBuffer->Buffer);
		}
		else if (EnumHasAnyFlags(HeapType, ResourceHeapType::Readback))
		{
			UnrealBuffer->BufferType = EUnrealBufferType::StaggingBuffer;
			UnrealBuffer->CPUBufferData = TUniquePtr<unsigned char, FFreeDeleter>(
				static_cast<unsigned char*>(FMemory::Malloc(Size, SHADER_PARAMETER_STRUCT_ALIGNMENT)));
			check(UnrealBuffer->CPUBufferData);
		}
		else
		{
			UnrealBuffer->BufferType = EUnrealBufferType::Buffer;

			EBufferUsageFlags UsageFlags = ConvertToERHIUsageFlags(HeapType, Usage);

			if (StructuredStride > 0)
			{
				// When structured buffer is requested
				// Create byte address buffer instead
				// This is required due to quirks of shader translation that translates structured buffers as byte address ones
				// This is also why we pass 0 instead of stride later on
				UsageFlags |= BUF_ByteAddressBuffer;
			}

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 506
			FRHIBufferCreateDesc CreateInfo(UTF8_TO_TCHAR(Name), Size, 0, UsageFlags);
			CreateInfo.SetInitialState(UnrealBuffer->CurrentState);

			UnrealBuffer->Buffer = RHICmdList.CreateBuffer(CreateInfo);
#else
			FRHIResourceCreateInfo CreateInfo(UTF8_TO_TCHAR(Name));

			UnrealBuffer->Buffer = RHICmdList.CreateBuffer(Size, UsageFlags, 0, UnrealBuffer->CurrentState, CreateInfo);
#endif

			check(UnrealBuffer->Buffer);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 507
			ZIB_CHEAP_GPU_DEBUG_ONLY(FRHICommandListImmediate::Get().BindDebugLabelName(
				static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference()), UTF8_TO_TCHAR(Name)));
#else
			ZIB_CHEAP_GPU_DEBUG_ONLY(
				RHIBindDebugLabelName(static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference()), UTF8_TO_TCHAR(Name)));
#endif
		}

		*OutBuffer = UnrealBuffer;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode Zibra::RHI::APIUnrealRHI::ZibraUnrealRHI::CreateTexture2D(size_t Width, size_t Height, size_t Mips,
		TextureFormat Format, ResourceUsage Usage, const char* Name, Texture2D** OutTexture) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FUnrealRHITexture2D* UnrealRHITexture2D = new FUnrealRHITexture2D{};
		UnrealRHITexture2D->CurrentState = ConvertToInitialState(ResourceHeapType::Default, Usage);
		const EPixelFormat UnrealFormat = ConvertToEPixelFormat(Format);
		const ETextureCreateFlags UsageFlags = ConvertToETextureCreateFlags(Usage);

		Name = Name ? Name : "ZibraUnrealRHIResource";

		const FRHITextureCreateDesc TextureDesc = FRHITextureCreateDesc::Create2D(UTF8_TO_TCHAR(Name))
													  .SetExtent(Width, Height)
													  .SetFormat(UnrealFormat)
													  .SetNumMips((uint8) Mips)
													  .SetFlags(UsageFlags)
													  .SetInitialState(UnrealRHITexture2D->CurrentState);

		UnrealRHITexture2D->Texture = RHICreateTexture(TextureDesc);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 507
		ZIB_CHEAP_GPU_DEBUG_ONLY(
			FRHICommandListImmediate::Get().BindDebugLabelName(UnrealRHITexture2D->Texture, UTF8_TO_TCHAR(Name)));
#else
		ZIB_CHEAP_GPU_DEBUG_ONLY(RHIBindDebugLabelName(UnrealRHITexture2D->Texture, UTF8_TO_TCHAR(Name)));
#endif

		*OutTexture = UnrealRHITexture2D;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode Zibra::RHI::APIUnrealRHI::ZibraUnrealRHI::CreateTexture3D(size_t Width, size_t Height, size_t Depth, size_t Mips,
		TextureFormat Format, ResourceUsage Usage, const char* Name, Texture3D** OutTexture) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FUnrealRHITexture3D* UnrealRHITexture3D = new FUnrealRHITexture3D{};
		UnrealRHITexture3D->CurrentState = ConvertToInitialState(ResourceHeapType::Default, Usage);
		const EPixelFormat UnrealFormat = ConvertToEPixelFormat(Format);
		const ETextureCreateFlags UsageFlags = ConvertToETextureCreateFlags(Usage);

		Name = Name ? Name : "ZibraUnrealRHIResource";

		const FRHITextureCreateDesc TextureDesc = FRHITextureCreateDesc::Create3D(UTF8_TO_TCHAR(Name))
													  .SetExtent(Width, Height)
													  .SetDepth(Depth)
													  .SetFormat(UnrealFormat)
													  .SetNumMips((uint8) Mips)
													  .SetFlags(UsageFlags)
													  .SetInitialState(UnrealRHITexture3D->CurrentState);

		UnrealRHITexture3D->Texture = RHICreateTexture(TextureDesc);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 507
		ZIB_CHEAP_GPU_DEBUG_ONLY(
			FRHICommandListImmediate::Get().BindDebugLabelName(UnrealRHITexture3D->Texture, UTF8_TO_TCHAR(Name)));
#else
		ZIB_CHEAP_GPU_DEBUG_ONLY(RHIBindDebugLabelName(UnrealRHITexture3D->Texture, UTF8_TO_TCHAR(Name)));
#endif

		*OutTexture = UnrealRHITexture3D;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::CreateSampler(SamplerAddressMode AddressMode, SamplerFilter Filter, size_t DescriptorLocationsCount,
		const DescriptorLocation* DescriptorLocations, const char* Name, Sampler** OutSampler) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FSamplerStateInitializerRHI SamplerStateInitializer;
		ESamplerAddressMode SamplerAdressMode = ConvertToESamplerAddressMode(AddressMode);
		SamplerStateInitializer.AddressU = SamplerAdressMode;
		SamplerStateInitializer.AddressV = SamplerAdressMode;
		SamplerStateInitializer.AddressW = SamplerAdressMode;

		SamplerStateInitializer.Filter = ConvertToESamplerFilter(Filter);
		SamplerStateInitializer.MaxAnisotropy = GetMaxAnisotrophy(Filter);

		SamplerStateInitializer.MaxMipLevel = 0.0f;
		SamplerStateInitializer.MaxMipLevel = FLT_MAX;

		FUnrealRHISampler* UnrealSampler = new FUnrealRHISampler{};
		UnrealSampler->Sampler = RHICreateSamplerState(SamplerStateInitializer);
		check(UnrealSampler->Sampler.IsValid());

		*OutSampler = UnrealSampler;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ReleaseBuffer(Buffer* Buffer) noexcept
	{
		delete static_cast<FUnrealRHIBuffer*>(Buffer);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ReleaseTexture2D(Texture2D* Texture) noexcept
	{
		delete static_cast<FUnrealRHITexture2D*>(Texture);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ReleaseTexture3D(Texture3D* Texture) noexcept
	{
		delete static_cast<FUnrealRHITexture3D*>(Texture);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ReleaseSampler(Sampler* Sampler) noexcept
	{
		delete static_cast<FUnrealRHISampler*>(Sampler);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::InitializeSRV(Buffer* Buffer, InitializeViewDesc Desc, size_t DescriptorLocationsCount,
		const DescriptorLocation* DescriptorLocations) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Buffer);

		FUnrealRHIBuffer* UnrealBuffer = static_cast<FUnrealRHIBuffer*>(Buffer);

		switch (Desc.type)
		{
			case ViewDataType::FormattedBuffer:
				InitializeFormattedSRV(UnrealBuffer, ConvertToEPixelFormat(Desc.rawBufferViewDescr.format));
				break;
			case ViewDataType::StructuredBuffer:
				InitializeSRV(UnrealBuffer);
				break;
			default:
				checkNoEntry();
		}
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::InitializeSRV(
		Texture2D* Texture, size_t DescriptorLocationsCount, const DescriptorLocation* DescriptorLocations) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Texture);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHITexture2D* UnrealTexture = static_cast<FUnrealRHITexture2D*>(Texture);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		UnrealTexture->SRV = RHICmdList.CreateShaderResourceView(UnrealTexture->Texture,
			FRHIViewDesc::CreateTextureSRV().SetDimensionFromTexture(UnrealTexture->Texture).SetMipRange(0, 1));
#else
		UnrealTexture->SRV = RHICmdList.CreateShaderResourceView(UnrealTexture->Texture, 0);
#endif

		check(UnrealTexture->SRV);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::InitializeSRV(
		Texture3D* Texture, size_t DescriptorLocationsCount, const DescriptorLocation* DescriptorLocations) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Texture);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHITexture3D* UnrealTexture = static_cast<FUnrealRHITexture3D*>(Texture);
		check(UnrealTexture->Texture);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		UnrealTexture->SRV = RHICmdList.CreateShaderResourceView(UnrealTexture->Texture,
			FRHIViewDesc::CreateTextureSRV().SetDimensionFromTexture(UnrealTexture->Texture).SetMipRange(0, 1));
#else
		UnrealTexture->SRV = RHICmdList.CreateShaderResourceView(UnrealTexture->Texture, 0);
#endif
		check(UnrealTexture->SRV);
		return ReturnCode::ZRHI_SUCCESS;
	}

	void ZibraUnrealRHI::InitializeSRV(FUnrealRHIBuffer* Buffer) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Buffer);
		check(Buffer->BufferType == EUnrealBufferType::Buffer);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FRHIBuffer* RHIBuffer = static_cast<FRHIBuffer*>(Buffer->Buffer.GetReference());
		check((RHIBuffer->GetUsage() & EBufferUsageFlags::ByteAddressBuffer) == EBufferUsageFlags::ByteAddressBuffer);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		Buffer->SRV = RHICmdList.CreateShaderResourceView(RHIBuffer, FRHIViewDesc::CreateBufferSRV()
																		 .SetType(FRHIViewDesc::EBufferType::Raw)
																		 .SetNumElements(RHIBuffer->GetSize() / sizeof(uint32_t)));
#else
		Buffer->SRV = RHICmdList.CreateShaderResourceView(RHIBuffer);
#endif
		check(Buffer->SRV);
	}

	void ZibraUnrealRHI::InitializeFormattedSRV(FUnrealRHIBuffer* Buffer, EPixelFormat Format) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Buffer);
		check(Buffer->BufferType == EUnrealBufferType::Buffer);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FRHIBuffer* RHIBuffer = static_cast<FRHIBuffer*>(Buffer->Buffer.GetReference());

		check(EnumHasAnyFlags(RHIBuffer->GetUsage(), EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::IndexBuffer) ||
			  RHIBuffer->GetStride() == sizeof(uint32_t));

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		Buffer->SRV = RHICmdList.CreateShaderResourceView(
			RHIBuffer, FRHIViewDesc::CreateBufferSRV().SetType(FRHIViewDesc::EBufferType::Typed).SetFormat(Format));
#else
		Buffer->SRV = RHICmdList.CreateShaderResourceView(RHIBuffer, sizeof(uint32_t), UE_PIXELFORMAT_TO_UINT8(Format));
#endif

		check(Buffer->SRV);
	}

	void ZibraUnrealRHI::InitializeUAV(FUnrealRHIBuffer* Buffer) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Buffer);
		check(Buffer->BufferType == EUnrealBufferType::Buffer);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FRHIBuffer* RHIBuffer = static_cast<FRHIBuffer*>(Buffer->Buffer.GetReference());

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		Buffer->UAV = RHICmdList.CreateUnorderedAccessView(RHIBuffer, FRHIViewDesc::CreateBufferUAV()
																		  .SetType(FRHIViewDesc::EBufferType::Raw)
																		  .SetNumElements(RHIBuffer->GetSize() / sizeof(uint32_t)));
#else
		Buffer->UAV = RHICmdList.CreateUnorderedAccessView(RHIBuffer, UE_PIXELFORMAT_TO_UINT8(EPixelFormat::PF_Unknown));
#endif
		check(Buffer->UAV);
	}

	ReturnCode ZibraUnrealRHI::InitializeUAV(Buffer* Buffer, InitializeViewDesc Desc, size_t DescriptorLocationsCount,
		const DescriptorLocation* DescriptorLocations) noexcept
	{
		FUnrealRHIBuffer* UnrealBuffer = static_cast<FUnrealRHIBuffer*>(Buffer);

		switch (Desc.type)
		{
			case RHI::ViewDataType::StructuredBuffer:
				InitializeUAV(UnrealBuffer);
				break;
			case RHI::ViewDataType::FormattedBuffer:
				InitializeFormattedUAV(UnrealBuffer, ConvertToEPixelFormat(Desc.rawBufferViewDescr.format));
				break;
			default:
				checkNoEntry();
		}
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::InitializeUAV(
		Texture2D* Texture, size_t DescriptorLocationsCount, const DescriptorLocation* DescriptorLocations) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Texture);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHITexture2D* UnrealTexture = static_cast<FUnrealRHITexture2D*>(Texture);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		UnrealTexture->UAV = RHICmdList.CreateUnorderedAccessView(UnrealTexture->Texture,
			FRHIViewDesc::CreateTextureUAV().SetDimensionFromTexture(UnrealTexture->Texture).SetMipLevel(0).SetArrayRange(0, 0));
#else
		UnrealTexture->UAV = RHICmdList.CreateUnorderedAccessView(UnrealTexture->Texture);
#endif
		check(UnrealTexture->UAV);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::InitializeUAV(
		Texture3D* Texture, size_t DescriptorLocationsCount, const DescriptorLocation* DescriptorLocations) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Texture);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHITexture3D* UnrealTexture = static_cast<FUnrealRHITexture3D*>(Texture);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		UnrealTexture->UAV = RHICmdList.CreateUnorderedAccessView(UnrealTexture->Texture,
			FRHIViewDesc::CreateTextureUAV().SetDimensionFromTexture(UnrealTexture->Texture).SetMipLevel(0).SetArrayRange(0, 0));
#else
		UnrealTexture->UAV = RHICmdList.CreateUnorderedAccessView(UnrealTexture->Texture);
#endif
		check(UnrealTexture->UAV);
		return ReturnCode::ZRHI_ERROR_NOT_SUPPORTED;
	}

	void ZibraUnrealRHI::InitializeFormattedUAV(FUnrealRHIBuffer* UnrealBuffer, EPixelFormat Format) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(UnrealBuffer);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		check(UnrealBuffer->BufferType == EUnrealBufferType::Buffer);

		FRHIBuffer* RHIBuffer = static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference());
		check(UnrealBuffer);

		EBufferUsageFlags BufferUsageFlags = RHIBuffer->GetUsage();

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		UnrealBuffer->UAV = RHICmdList.CreateUnorderedAccessView(
			RHIBuffer, FRHIViewDesc::CreateBufferUAV().SetType(FRHIViewDesc::EBufferType::Typed).SetFormat(Format));
#else
		UnrealBuffer->UAV = RHICmdList.CreateUnorderedAccessView(RHIBuffer, Format);
#endif
		check(UnrealBuffer->UAV);
	}

	ReturnCode ZibraUnrealRHI::InitializeCBV(
		Buffer* Buffer, size_t DescriptorLocationsCount, const DescriptorLocation* DescriptorLocations) noexcept
	{
		// NOOP
		return ReturnCode::ZRHI_ERROR_NOT_SUPPORTED;
	}

	ReturnCode ZibraUnrealRHI::StartRecording() noexcept
	{
		// TODO Check whether we need to get non-immediate command list and cache it here
		// NOOP
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::StopRecording() noexcept
	{
		// NOOP
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::StartDebugRegion(const char* RegionName) noexcept
	{
		// NOOP
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::EndDebugRegion() noexcept
	{
		// NOOP
		return ReturnCode::ZRHI_SUCCESS;
	}

	void ZibraUnrealRHI::StartFrameCapture(const char* CaptureName) noexcept
	{
		// Not supported in Unreal Engine
	}

	void ZibraUnrealRHI::StopFrameCapture() noexcept
	{
		// Not supported in Unreal Engine
	}

	ReturnCode ZibraUnrealRHI::UploadBuffer(Buffer* Buffer, const void* Data, uint32_t Size, uint32_t Offset) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Buffer);
		check(Data);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHIBuffer* UnrealBuffer = static_cast<FUnrealRHIBuffer*>(Buffer);

		switch (UnrealBuffer->BufferType)
		{
			case EUnrealBufferType::Buffer:
			{
				FRHIBuffer* RHIBuffer = static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference());

				void* BufferData = RHICmdList.LockBuffer(RHIBuffer, 0, Size, EResourceLockMode::RLM_WriteOnly);
				check(BufferData);

				FMemory::Memcpy(static_cast<uint8*>(BufferData) + Offset, Data, Size);

				RHICmdList.UnlockBuffer(RHIBuffer);
				break;
			}
			case EUnrealBufferType::UniformBuffer:
			{
				FRHIUniformBuffer* UniformBuffer = static_cast<FRHIUniformBuffer*>(UnrealBuffer->Buffer.GetReference());
				FMemory::Memcpy(UnrealBuffer->CPUBufferData.Get() + Offset, Data, Size);
				RHICmdList.UpdateUniformBuffer(UniformBuffer, UnrealBuffer->CPUBufferData.Get());

				break;
			}
			default:
				check(!"ZibraUnrealRHI: UploadBuffer is not supported for other buffer types.");
				break;
		}
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::UploadBufferSlow(Buffer* Buffer, const void* Data, uint32_t Size, uint32_t Offset) noexcept
	{
		// TODO: check implementation UploadBuffer vs UploadBufferSlow.
		check(IsInRenderingThread() || IsInRHIThread());
		check(Buffer);
		check(Data);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHIBuffer* UnrealBuffer = static_cast<FUnrealRHIBuffer*>(Buffer);

		switch (UnrealBuffer->BufferType)
		{
			case EUnrealBufferType::Buffer:
			{
				FRHIBuffer* RHIBuffer = static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference());

				void* BufferData = RHICmdList.LockBuffer(RHIBuffer, 0, Size, EResourceLockMode::RLM_WriteOnly);
				check(BufferData);

				FMemory::Memcpy(BufferData, Data, Size);

				RHICmdList.UnlockBuffer(RHIBuffer);
				break;
			}
			case EUnrealBufferType::UniformBuffer:
			{
				FRHIUniformBuffer* UniformBuffer = static_cast<FRHIUniformBuffer*>(UnrealBuffer->Buffer.GetReference());
				FMemory::Memcpy(UnrealBuffer->CPUBufferData.Get(), Data, Size);
				RHICmdList.UpdateUniformBuffer(UniformBuffer, UnrealBuffer->CPUBufferData.Get());

				break;
			}
			default:
				check(!"ZibraUnrealRHI: UploadBuffer is not supported for other buffer types.");
				break;
		}
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::UploadTextureSlow(Texture3D* Texture, const TextureData& UploadData) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Texture);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHITexture3D* UnrealTexture = static_cast<FUnrealRHITexture3D*>(Texture);
		FUpdateTextureRegion3D TextureRegion{FIntVector{0, 0, 0}, FIntVector{0, 0, 0},
			FIntVector{UploadData.dimensionX, UploadData.dimensionY, UploadData.dimensionZ}};
		RHICmdList.UpdateTexture3D(UnrealTexture->Texture, 0, TextureRegion, UploadData.rowPitch,
			UploadData.rowPitch * UploadData.dimensionY, static_cast<const uint8*>(UploadData.data));
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::GetBufferData(Buffer* Buffer, void* Destination, size_t Size, size_t SrcOffset) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Buffer);
		check(Destination);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHIBuffer* UnrealBuffer = static_cast<FUnrealRHIBuffer*>(Buffer);

		switch (UnrealBuffer->BufferType)
		{
			case EUnrealBufferType::Buffer:
			{
				FRHIBuffer* RHIBuffer = static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference());

				// To read the data from the buffer (from CPU), it cannot be Dynamic (must be Static).
				check(EnumHasAnyFlags(RHIBuffer->GetUsage(), EBufferUsageFlags::Static));
				const void* BufferData = RHICmdList.LockBuffer(RHIBuffer, 0, Size, EResourceLockMode::RLM_ReadOnly);
				check(BufferData);

				FMemory::Memcpy(Destination, static_cast<const uint8*>(BufferData) + SrcOffset, Size);

				RHICmdList.UnlockBuffer(RHIBuffer);
				break;
			}
			case EUnrealBufferType::StaggingBuffer:
			{
				if (UnrealBuffer->CPUBufferData)
				{
					FMemory::Memcpy(
						Destination, static_cast<const unsigned char*>(UnrealBuffer->CPUBufferData.Get()) + SrcOffset, Size);
				}
				break;
			}
			default:
				checkNoEntry();
				break;
		}
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::GetBufferDataImmediately(Buffer* Buffer, void* Destination, size_t Size, size_t SrcOffset) noexcept
	{
		GetBufferData(Buffer, Destination, Size, SrcOffset);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::Dispatch(const DispatchDesc& Desc) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Desc.PSO);

		FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

		ValidateGroupCount(Desc);

		FUnrealComputePSO* ComputePSO = static_cast<FUnrealComputePSO*>(Desc.PSO);

		FRHIComputeShader* ShaderRHI = ComputePSO->CS.GetComputeShader();
		check(ShaderRHI);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		SetComputePipelineState(RHICmdList, ShaderRHI);
#else
		RHICmdList.SetComputeShader(ShaderRHI);
#endif

		const BindingsDescBase* CommonParams = static_cast<const BindingsDescBase*>(&Desc);
		check(CommonParams);

		BindResources(RHICmdList, *CommonParams, ShaderRHI, ShaderStage::CS);

		RHICmdList.DispatchComputeShader(Desc.dimensions.groupCountX, Desc.dimensions.groupCountY, Desc.dimensions.groupCountZ);

		UnbindResources(RHICmdList, *CommonParams, ShaderRHI, ShaderStage::CS);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::DrawIndexedInstanced(const DrawIndexedInstancedDesc& Desc) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

		FUnrealGraphicsPSO* GraphicsPSO = static_cast<FUnrealGraphicsPSO*>(Desc.PSO);

		const BindingsDescBase* CommonParams = static_cast<const BindingsDescBase*>(&Desc);
		check(CommonParams);
		const DrawDescBase* DrawParams = static_cast<const DrawDescBase*>(&Desc);
		check(DrawParams);

		FRHIRenderPassInfo RPInfo = BeginDraw(RHICmdList, *DrawParams);
		RHICmdList.BeginRenderPass(RPInfo, *GraphicsPSO->VSName);
		RHICmdList.BeginRenderPass(RPInfo, *GraphicsPSO->VSName);
		{
			PrepareDraw(RHICmdList, GraphicsPSO, *DrawParams, *CommonParams);

			FBufferRHIRef UnrealIndexBuffer = static_cast<FBufferRHIRef>(static_cast<FUnrealRHIBuffer*>(Desc.indexBuffer)->Buffer);
			FBufferRHIRef UnrealVertexBuffer =
				static_cast<FBufferRHIRef>(static_cast<FUnrealRHIBuffer*>(DrawParams->vertexBuffers[0])->Buffer);
			uint32 NumVertices = ConvertToNumVertices(DrawParams->vertexBufferSizes[0], DrawParams->vertexBufferStrides[0]);
			uint32 NumPrimitives = ConvertToNumPrimitives(Desc.indexCount, GraphicsPSO->PrimitiveTopology);
			RHICmdList.DrawIndexedPrimitive(
				UnrealIndexBuffer, Desc.vertexOffset, 0, NumVertices, Desc.firstIndex, NumPrimitives, Desc.instanceCount);

			UnbindResources(RHICmdList, *CommonParams, GraphicsPSO->VS.GetVertexShader(), ShaderStage::VS);
			UnbindResources(RHICmdList, *CommonParams, GraphicsPSO->PS.GetPixelShader(), ShaderStage::PS);
		}
		EndDraw(RHICmdList, *DrawParams);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::DrawInstanced(const DrawInstancedDesc& Desc) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

		FUnrealGraphicsPSO* GraphicsPSO = static_cast<FUnrealGraphicsPSO*>(Desc.PSO);
		const BindingsDescBase* CommonParams = static_cast<const BindingsDescBase*>(&Desc);
		check(CommonParams);
		const DrawDescBase* DrawParams = static_cast<const DrawDescBase*>(&Desc);
		check(DrawParams);

		FRHIRenderPassInfo RPInfo = BeginDraw(RHICmdList, *DrawParams);
		RHICmdList.BeginRenderPass(RPInfo, *GraphicsPSO->VSName);
		{
			PrepareDraw(RHICmdList, GraphicsPSO, *DrawParams, *CommonParams);

			uint32 NumPrimitives = ConvertToNumPrimitives(Desc.vertexCount, GraphicsPSO->PrimitiveTopology);
			RHICmdList.DrawPrimitive(Desc.firstVertex, NumPrimitives, Desc.instanceCount);
			UnbindResources(RHICmdList, *CommonParams, GraphicsPSO->VS.GetVertexShader(), ShaderStage::VS);
			UnbindResources(RHICmdList, *CommonParams, GraphicsPSO->PS.GetPixelShader(), ShaderStage::PS);
		}
		EndDraw(RHICmdList, *DrawParams);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::DispatchIndirect(const DispatchIndirectDesc& Desc) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

		FUnrealComputePSO* PSO = static_cast<FUnrealComputePSO*>(Desc.PSO);
		check(PSO);
		const BindingsDescBase* CommonParams = static_cast<const BindingsDescBase*>(&Desc);
		check(CommonParams);
		FRHIComputeShader* ShaderRHI = PSO->CS.GetComputeShader();
		check(ShaderRHI);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		SetComputePipelineState(RHICmdList, ShaderRHI);
#else
		RHICmdList.SetComputeShader(ShaderRHI);
#endif

		BindResources(RHICmdList, *CommonParams, ShaderRHI, ShaderStage::CS);

		FUnrealRHIBuffer* UnrealBuffer = static_cast<FUnrealRHIBuffer*>(Desc.indirectArgumentsBuffer);

		check(UnrealBuffer->BufferType == EUnrealBufferType::Buffer);
		FRHIBuffer* IndirectArgumentsRHI = static_cast<FRHIBuffer*>(UnrealBuffer->Buffer.GetReference());

		RHICmdList.DispatchIndirectComputeShader(IndirectArgumentsRHI, Desc.indirectArgumentsOffset);

		UnbindResources(RHICmdList, *CommonParams, ShaderRHI, ShaderStage::CS);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::DrawInstancedIndirect(const DrawInstancedIndirectDesc& Desc) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

		FUnrealGraphicsPSO* GraphicsPSO = static_cast<FUnrealGraphicsPSO*>(Desc.PSO);
		const BindingsDescBase* CommonParams = static_cast<const BindingsDescBase*>(&Desc);
		check(CommonParams);
		const DrawDescBase* DrawParams = static_cast<const DrawDescBase*>(&Desc);
		check(DrawParams);

		FRHIRenderPassInfo RPInfo = BeginDraw(RHICmdList, *DrawParams);
		RHICmdList.BeginRenderPass(RPInfo, *GraphicsPSO->VSName);
		{
			PrepareDraw(RHICmdList, GraphicsPSO, *DrawParams, *CommonParams);

			FBufferRHIRef UnrealIndirectArguments =
				static_cast<FBufferRHIRef>(static_cast<FUnrealRHIBuffer*>(Desc.indirectArgumentsBuffer)->Buffer);
			RHICmdList.DrawPrimitiveIndirect(UnrealIndirectArguments, Desc.indirectArgumentsOffset);

			UnbindResources(RHICmdList, *CommonParams, GraphicsPSO->VS.GetVertexShader(), ShaderStage::VS);
			UnbindResources(RHICmdList, *CommonParams, GraphicsPSO->PS.GetPixelShader(), ShaderStage::PS);
		}
		EndDraw(RHICmdList, *DrawParams);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::DrawIndexedInstancedIndirect(const DrawIndexedInstancedIndirectDesc& Desc) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());

		FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

		FUnrealGraphicsPSO* GraphicsPSO = static_cast<FUnrealGraphicsPSO*>(Desc.PSO);
		const BindingsDescBase* CommonParams = static_cast<const BindingsDescBase*>(&Desc);
		check(CommonParams);
		const DrawDescBase* DrawParams = static_cast<const DrawDescBase*>(&Desc);
		check(DrawParams);

		FRHIRenderPassInfo RPInfo = BeginDraw(RHICmdList, *DrawParams);
		RHICmdList.BeginRenderPass(RPInfo, *GraphicsPSO->VSName);
		{
			PrepareDraw(RHICmdList, GraphicsPSO, *DrawParams, *CommonParams);

			FBufferRHIRef UnrealIndexBuffer = static_cast<FBufferRHIRef>(static_cast<FUnrealRHIBuffer*>(Desc.indexBuffer)->Buffer);
			FBufferRHIRef UnrealIndirectArguments =
				static_cast<FBufferRHIRef>(static_cast<FUnrealRHIBuffer*>(Desc.indirectArgumentsBuffer)->Buffer);
			RHICmdList.DrawIndexedPrimitiveIndirect(UnrealIndexBuffer, UnrealIndirectArguments, Desc.indirectArgumentsOffset);

			UnbindResources(RHICmdList, *CommonParams, GraphicsPSO->VS.GetVertexShader(), ShaderStage::VS);
			UnbindResources(RHICmdList, *CommonParams, GraphicsPSO->PS.GetPixelShader(), ShaderStage::PS);
		}
		EndDraw(RHICmdList, *DrawParams);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::CopyBufferRegion(
		Buffer* DstBuffer, uint32_t DstOffset, Buffer* SrcBuffer, uint32_t SrcOffset, uint32_t Size) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(DstBuffer);
		check(SrcBuffer);

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();

		FUnrealRHIBuffer* UnrealDstBuffer = static_cast<FUnrealRHIBuffer*>(DstBuffer);
		FUnrealRHIBuffer* UnrealSrcBuffer = static_cast<FUnrealRHIBuffer*>(SrcBuffer);

		check(UnrealSrcBuffer->BufferType == EUnrealBufferType::Buffer);

		switch (UnrealDstBuffer->BufferType)
		{
			case EUnrealBufferType::Buffer:
			{
				check(UnrealSrcBuffer->Buffer.IsValid());
				check(UnrealDstBuffer->Buffer.IsValid());

				TArray<FRHITransitionInfo> RequiredTransitions;

				RequiredTransitions.Emplace(static_cast<FRHIBuffer*>(UnrealSrcBuffer->Buffer.GetReference()),
					UnrealSrcBuffer->CurrentState, ERHIAccess::CopySrc);
				UnrealSrcBuffer->CurrentState = ERHIAccess::CopySrc;
				RequiredTransitions.Emplace(static_cast<FRHIBuffer*>(UnrealDstBuffer->Buffer.GetReference()),
					UnrealDstBuffer->CurrentState, ERHIAccess::CopyDest);
				UnrealDstBuffer->CurrentState = ERHIAccess::CopyDest;

				RHICmdList.Transition(RequiredTransitions);

				RHICmdList.CopyBufferRegion(static_cast<FRHIBuffer*>(UnrealDstBuffer->Buffer.GetReference()), DstOffset,
					static_cast<FRHIBuffer*>(UnrealSrcBuffer->Buffer.GetReference()), SrcOffset, Size);
				break;
			}
			case EUnrealBufferType::StaggingBuffer:
			{
				check(UnrealSrcBuffer->Buffer.IsValid());
				check(UnrealDstBuffer->CPUBufferData);

				FRHIBuffer* BufferRHI = static_cast<FRHIBuffer*>(UnrealSrcBuffer->Buffer.GetReference());
				void* StagingBufferData = UnrealDstBuffer->CPUBufferData.Get();

				// TODO: Use real staging buffers (FRHIStagingBuffer).
				const void* BufferData = RHICmdList.LockBuffer(BufferRHI, SrcOffset, Size, EResourceLockMode::RLM_ReadOnly);

				FMemory::Memcpy(static_cast<unsigned char*>(StagingBufferData) + DstOffset, BufferData, Size);

				RHICmdList.UnlockBuffer(BufferRHI);
				break;
			}
			case EUnrealBufferType::UniformBuffer:
			{
				check(UnrealSrcBuffer->Buffer.IsValid());
				check(UnrealDstBuffer->Buffer.IsValid());

				// TODO: optimize later, since right now we have:
				// 1. UnrealSrcBuffer Copy -> StaggingBuffer.
				// 2. StaggingBuffer Copy -> UnrealDstBuffer->CPUBufferData.
				// 3. UnrealDstBuffer->CPUBufferData Copy -> UniformBufferRHI.
				//
				// Optimal solution: UnrealSrcBuffer Copy -> UniformBufferRHI (for instance, use custom shader).
				FRHIBuffer* BufferRHI = static_cast<FRHIBuffer*>(UnrealSrcBuffer->Buffer.GetReference());
				FRHIUniformBuffer* UniformBufferRHI = static_cast<FRHIUniformBuffer*>(UnrealDstBuffer->Buffer.GetReference());
				void* BufferData = RHICmdList.LockBuffer(BufferRHI, 0, Size, EResourceLockMode::RLM_ReadOnly);
				check(BufferData);

				FMemory::Memcpy(static_cast<unsigned char*>(UnrealDstBuffer->CPUBufferData.Get()) + DstOffset,
					static_cast<unsigned char*>(BufferData) + SrcOffset, Size);
				RHICmdList.UnlockBuffer(BufferRHI);

				RHICmdList.UpdateUniformBuffer(UniformBufferRHI, UnrealDstBuffer->CPUBufferData.Get());
				break;
			}
			default:
				break;
		}
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ClearFormattedBuffer(
		Buffer* Buffer, uint32_t BufferSize, uint8_t ClearValue, const DescriptorLocation& DescriptorLocation) noexcept
	{
		check(IsInRenderingThread() || IsInRHIThread());
		check(Buffer);

		FUnrealRHIBuffer* UnrealBuffer = static_cast<FUnrealRHIBuffer*>(Buffer);
		check(UnrealBuffer->BufferType == EUnrealBufferType::Buffer);

		const UINT CalculatedClearValue = ClearValue | ClearValue << 8 | ClearValue << 16 | ClearValue << 24;
		const FUintVector4 ClearValueVector{CalculatedClearValue};

		check(UnrealBuffer->UAV);
		FRHICommandListExecutor::GetImmediateCommandList().ClearUAVUint(UnrealBuffer->UAV, ClearValueVector);
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::SubmitQuery(QueryHeap* queryHeap, uint64_t queryIndex, const QueryDesc& queryDesc) noexcept
	{
		return ReturnCode::ZRHI_ERROR_NOT_SUPPORTED;
	}

	ReturnCode ZibraUnrealRHI::ResolveQuery(
		QueryHeap* queryHeap, QueryType queryType, uint64_t offset, uint64_t count, void* result, size_t size) noexcept
	{
		return ReturnCode::ZRHI_ERROR_NOT_SUPPORTED;
	}

	uint64_t ZibraUnrealRHI::GetTimestampFrequency() const noexcept
	{
		checkNoEntry();
		return 0;
	}

	TimestampCalibrationResult ZibraUnrealRHI::GetTimestampCalibration() const noexcept
	{
		checkNoEntry();
		return {};
	}

	uint64_t ZibraUnrealRHI::GetCPUTimestamp() const noexcept
	{
		checkNoEntry();
		return 0;
	}

	ReturnCode ZibraUnrealRHI::GarbageCollect() noexcept
	{
		// NOOP
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::CreateDescriptorHeap(DescriptorHeapType HeapType, const PipelineLayoutDesc& PipelineLayoutDesc,
		const char* Name, DescriptorHeap** OutHeap) noexcept
	{
		// NOOP for non bindless API
		auto* DescriptorHeap = new FUnrealRHIDescriptorHeap{};
		*OutHeap = DescriptorHeap;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::CloneDescriptorHeap(const DescriptorHeap* Src, const char* Name, DescriptorHeap** OutHeap) noexcept
	{
		// NOOP for non bindless API
		auto* DescriptorHeap = new FUnrealRHIDescriptorHeap{};
		*OutHeap = DescriptorHeap;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ReleaseDescriptorHeap(DescriptorHeap* Heap) noexcept
	{
		FUnrealRHIDescriptorHeap* DescriptorHeap = static_cast<FUnrealRHIDescriptorHeap*>(Heap);
		delete DescriptorHeap;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::CreateQueryHeap(
		QueryHeapType heapType, uint64_t queryCount, const char* name, QueryHeap** outHeap) noexcept
	{
		return ReturnCode::ZRHI_ERROR_NOT_SUPPORTED;
	}

	ReturnCode ZibraUnrealRHI::ReleaseQueryHeap(QueryHeap* heap) noexcept
	{
		return ReturnCode::ZRHI_ERROR_NOT_SUPPORTED;
	}

	void ZibraUnrealRHI::ValidateGroupCount(const DispatchDesc& Desc)
	{
		ensure(int(Desc.dimensions.groupCountX) <= GRHIMaxDispatchThreadGroupsPerDimension.X);
		ensure(int(Desc.dimensions.groupCountY) <= GRHIMaxDispatchThreadGroupsPerDimension.Y);
		ensure(int(Desc.dimensions.groupCountZ) <= GRHIMaxDispatchThreadGroupsPerDimension.Z);
	}

	void ZibraUnrealRHI::BindResources(
		FRHICommandListImmediate& RHICmdList, const BindingsDescBase& Desc, FRHIShader* ShaderRHI, ShaderStage Stage) noexcept
	{
		FUniformBufferRHIRef CBVs[Limits::MaxBoundCBVs] = {};
		FShaderResourceViewRHIRef SRVs[Limits::MaxBoundSRVs] = {};
		FUnorderedAccessViewRHIRef UAVs[Limits::MaxBoundUAVs] = {};
		FSamplerStateRHIRef Samplers[Limits::MaxBoundSamplers] = {};
		FTextureRHIRef Textures[Limits::MaxBoundSRVs] = {};

		TArray<FRHITransitionInfo> RequiredTransitions;

		for (size_t Index = 0; Index < Desc.UAVsCount; Index++)
		{
			if (!Desc.UAVs[Index])
				continue;

			switch (Desc.UAVs[Index]->GetType())
			{
				case Resource::Buffer:
				{
					auto* BufferResource = static_cast<FUnrealRHIBuffer*>(Desc.UAVs[Index]);
					UAVs[Index] = BufferResource->UAV;
					RequiredTransitions.Emplace(static_cast<FRHIBuffer*>(BufferResource->Buffer.GetReference()),
						BufferResource->CurrentState, ERHIAccess::UAVMask);
					BufferResource->CurrentState = ERHIAccess::UAVMask;
					break;
				}
				case Resource::Texture2D:
				{
					auto* TextureResource = static_cast<FUnrealRHITexture2D*>(Desc.UAVs[Index]);
					UAVs[Index] = TextureResource->UAV;
					RequiredTransitions.Emplace(TextureResource->Texture, TextureResource->CurrentState, ERHIAccess::UAVMask);
					TextureResource->CurrentState = ERHIAccess::UAVMask;
					break;
				}
				case Resource::Texture3D:
				{
					auto* TextureResource = static_cast<FUnrealRHITexture3D*>(Desc.UAVs[Index]);
					UAVs[Index] = TextureResource->UAV;
					RequiredTransitions.Emplace(TextureResource->Texture, TextureResource->CurrentState, ERHIAccess::UAVMask);
					TextureResource->CurrentState = ERHIAccess::UAVMask;
					break;
				}
				default:
					checkNoEntry();
			}

			check(UAVs[Index]);
		}

		for (size_t Index = 0; Index < Desc.SRVsCount; Index++)
		{
			if (!Desc.SRVs[Index])
				continue;

			switch (Desc.SRVs[Index]->GetType())
			{
				case Resource::Buffer:
				{
					auto* BufferResource = static_cast<FUnrealRHIBuffer*>(Desc.SRVs[Index]);
					SRVs[Index] = BufferResource->SRV;

					if (BufferResource->CurrentState != ERHIAccess::SRVMask)
					{
						RequiredTransitions.Emplace(static_cast<FRHIBuffer*>(BufferResource->Buffer.GetReference()),
							BufferResource->CurrentState, ERHIAccess::SRVMask);
						BufferResource->CurrentState = ERHIAccess::SRVMask;
					}
					break;
				}
				case Resource::Texture2D:
				{
					auto* TextureResource = static_cast<FUnrealRHITexture2D*>(Desc.SRVs[Index]);
					SRVs[Index] = TextureResource->SRV;
					Textures[Index] = TextureResource->Texture;

					if (TextureResource->CurrentState != ERHIAccess::SRVMask)
					{
						RequiredTransitions.Emplace(TextureResource->Texture, TextureResource->CurrentState, ERHIAccess::SRVMask);
						TextureResource->CurrentState = ERHIAccess::SRVMask;
					}
					break;
				}
				case Resource::Texture3D:
				{
					auto* TextureResource = static_cast<FUnrealRHITexture3D*>(Desc.SRVs[Index]);
					SRVs[Index] = TextureResource->SRV;
					Textures[Index] = TextureResource->Texture;

					if (TextureResource->CurrentState != ERHIAccess::SRVMask)
					{
						RequiredTransitions.Emplace(TextureResource->Texture, TextureResource->CurrentState, ERHIAccess::SRVMask);
						TextureResource->CurrentState = ERHIAccess::SRVMask;
					}
					break;
				}
				default:
					checkNoEntry();
			}
			check(SRVs[Index]);
		}

		for (size_t Index = 0; Index < Desc.samplersCount; Index++)
		{
			if (!Desc.samplers[Index])
				continue;
			Samplers[Index] = static_cast<FUnrealRHISampler*>(Desc.samplers[Index])->Sampler;
		}

		for (size_t Index = 0; Index < Desc.CBVsCount; Index++)
		{
			if (!Desc.CBVs[Index])
				continue;
			CBVs[Index] = static_cast<FUniformBufferRHIRef>(static_cast<FUnrealRHIBuffer*>(Desc.CBVs[Index])->Buffer);
		}

		RHICmdList.Transition(RequiredTransitions);

		switch (Stage)
		{
			case Zibra::RHI::ShaderStage::VS:
			case Zibra::RHI::ShaderStage::PS:
			{
				FRHIPixelShader* PixelShaderRHI = static_cast<FRHIPixelShader*>(ShaderRHI);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
				FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
#endif

				for (size_t Index = 0; Index < Desc.UAVsCount; Index++)
				{
					if (UAVs[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetUAVParameter(Index, UAVs[Index]);
#else
						RHICmdList.SetUAVParameter(PixelShaderRHI, Index, UAVs[Index]);
#endif
					}
				}
				// Bind SRVs after UAVs as a workaround for D3D11 RHI unbinding SRVs when binding a UAV on the same resource
				// even when the views don't overlap.
				for (size_t Index = 0; Index < Desc.SRVsCount; Index++)
				{
					if (SRVs[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetShaderResourceViewParameter(Index, SRVs[Index]);
#else
						RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI, Index, SRVs[Index]);
#endif
					}
				}

				for (size_t Index = 0; Index < Desc.SRVsCount; Index++)
				{
					if (Textures[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetShaderTexture(Index, Textures[Index]);
#else
						RHICmdList.SetShaderTexture(PixelShaderRHI, Index, Textures[Index]);
#endif
					}
				}
				for (size_t Index = 0; Index < Desc.samplersCount; Index++)
				{
					if (Samplers[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetShaderSampler(Index, Samplers[Index]);
#else
						RHICmdList.SetShaderSampler(PixelShaderRHI, Index, Samplers[Index]);
#endif
					}
				}
				for (size_t Index = 0; Index < Desc.CBVsCount; Index++)
				{
					if (CBVs[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetShaderUniformBuffer(Index, CBVs[Index]);
#else
						RHICmdList.SetShaderUniformBuffer(PixelShaderRHI, Index, CBVs[Index]);
#endif
					}
				}

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
				RHICmdList.SetBatchedShaderParameters(PixelShaderRHI, BatchedParameters);
#endif
				break;
			}
			case Zibra::RHI::ShaderStage::CS:
			{
				FRHIComputeShader* ComputeShaderRHI = static_cast<FRHIComputeShader*>(ShaderRHI);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
				FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
#endif

				for (size_t Index = 0; Index < Desc.UAVsCount; Index++)
				{
					if (UAVs[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetUAVParameter(Index, UAVs[Index]);
#else
						RHICmdList.SetUAVParameter(ComputeShaderRHI, Index, UAVs[Index]);
#endif
					}
				}
				// Bind SRVs after UAVs as a workaround for D3D11 RHI unbinding SRVs when binding a UAV on the same resource
				// even when the views don't overlap.
				for (size_t Index = 0; Index < Desc.SRVsCount; Index++)
				{
					if (SRVs[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetShaderResourceViewParameter(Index, SRVs[Index]);
#else
						RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, Index, SRVs[Index]);
#endif
					}
				}
				for (size_t Index = 0; Index < Desc.samplersCount; Index++)
				{
					if (Samplers[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetShaderSampler(Index, Samplers[Index]);
#else
						RHICmdList.SetShaderSampler(ComputeShaderRHI, Index, Samplers[Index]);
#endif
					}
				}
				for (size_t Index = 0; Index < Desc.CBVsCount; Index++)
				{
					if (CBVs[Index].IsValid())
					{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
						BatchedParameters.SetShaderUniformBuffer(Index, CBVs[Index]);
#else
						RHICmdList.SetShaderUniformBuffer(ComputeShaderRHI, Index, CBVs[Index]);
#endif
					}
				}
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
				RHICmdList.SetBatchedShaderParameters(ComputeShaderRHI, BatchedParameters);
#endif
				break;
			}
			default:
				checkNoEntry();
				break;
		}
	}

	void ZibraUnrealRHI::UnbindResources(
		FRHICommandListImmediate& RHICmdList, const BindingsDescBase& Desc, FRHIShader* ShaderRHI, ShaderStage Stage) noexcept
	{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
		FRHIBatchedShaderUnbinds& BatchedUnbinds = RHICmdList.GetScratchShaderUnbinds();
		for (size_t Index = 0; Index < Desc.UAVsCount; Index++)
		{
			BatchedUnbinds.UnsetUAV(Index);
		}
		for (size_t Index = 0; Index < Desc.SRVsCount; Index++)
		{
			BatchedUnbinds.UnsetSRV(Index);
		}
#endif

		switch (Stage)
		{
			case Zibra::RHI::ShaderStage::VS:
			case Zibra::RHI::ShaderStage::PS:
			{
				FRHIPixelShader* PixelShaderRHI = static_cast<FRHIPixelShader*>(ShaderRHI);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
				RHICmdList.SetBatchedShaderUnbinds(PixelShaderRHI, BatchedUnbinds);
#else
				for (size_t Index = 0; Index < Desc.UAVsCount; Index++)
				{
					RHICmdList.SetUAVParameter(PixelShaderRHI, Index, nullptr);
				}
				for (size_t Index = 0; Index < Desc.SRVsCount; Index++)
				{
					RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI, Index, nullptr);
				}
#endif
				break;
			}
			case Zibra::RHI::ShaderStage::CS:
			{
				FRHIComputeShader* ComputeShaderRHI = static_cast<FRHIComputeShader*>(ShaderRHI);
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
				RHICmdList.SetBatchedShaderUnbinds(ComputeShaderRHI, BatchedUnbinds);
#else
				for (size_t Index = 0; Index < Desc.UAVsCount; Index++)
				{
					RHICmdList.SetUAVParameter(ComputeShaderRHI, Index, nullptr);
				}
				for (size_t Index = 0; Index < Desc.SRVsCount; Index++)
				{
					RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, Index, nullptr);
				}
#endif
				break;
			}
			default:
				checkNoEntry();
				break;
		}
	}

	ReturnCode ZibraUnrealRHI::CreateFramebuffer(size_t RenderTargetsCount, Texture2D* const* RenderTargets,
		Texture2D* DepthStencil, const char* Name, Framebuffer** OutFramebuffer) noexcept
	{
		FUnrealFramebuffer* FrameBuffer = new FUnrealFramebuffer{};

		FrameBuffer->RTVs.Reserve(RenderTargetsCount);
		for (size_t Index = 0; Index < RenderTargetsCount; Index++)
		{
			FrameBuffer->RTVs.Add(static_cast<FUnrealRHITexture2D*>(RenderTargets[Index])->Texture);
		}

		if (DepthStencil)
		{
			FrameBuffer->DSV = static_cast<FUnrealRHITexture2D*>(DepthStencil)->Texture;
		}

		*OutFramebuffer = FrameBuffer;
		return ReturnCode::ZRHI_SUCCESS;
	}

	ReturnCode ZibraUnrealRHI::ReleaseFramebuffer(Framebuffer* Framebuffer) noexcept
	{
		FUnrealFramebuffer* UEFramebuffer = static_cast<FUnrealFramebuffer*>(Framebuffer);
		delete UEFramebuffer;
		return ReturnCode::ZRHI_SUCCESS;
	}

	FRHIRenderPassInfo ZibraUnrealRHI::BeginDraw(FRHICommandListImmediate& RHICmdList, const DrawDescBase& Desc)
	{
		FUnrealFramebuffer* FrameBuffer = static_cast<FUnrealFramebuffer*>(Desc.framebuffer);

		for (size_t Index = 0; Index < FrameBuffer->RTVs.Num(); Index++)
		{
			RHICmdList.Transition(FRHITransitionInfo(FrameBuffer->RTVs[Index], ERHIAccess::Unknown, ERHIAccess::RTV));
		}

		FRHIRenderPassInfo RPInfo(FrameBuffer->RTVs.Num(), FrameBuffer->RTVs.GetData(), ERenderTargetActions::Clear_DontStore,
			FrameBuffer->DSV, EDepthStencilTargetActions::ClearDepthStencil_DontStoreDepthStencil);

		return RPInfo;
	}

	void ZibraUnrealRHI::PrepareDraw(FRHICommandListImmediate& RHICmdList, FUnrealGraphicsPSO* GraphicsPSO,
		const DrawDescBase& DrawDesc, const BindingsDescBase& CommonDesc)
	{
		// Set the graphic pipeline state.
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = GraphicsPSO->DepthStencilState;
		GraphicsPSOInit.BlendState = GraphicsPSO->BlendState;
		GraphicsPSOInit.RasterizerState = GraphicsPSO->RasterState;
		GraphicsPSOInit.PrimitiveType = GraphicsPSO->PrimitiveTopology;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GraphicsPSO->InputLayout;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GraphicsPSO->VS.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GraphicsPSO->PS.GetPixelShader();
		GraphicsPSOInit.bDepthBounds = false;
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, false);

		// Set vertex buffers (must be after SetGraphicsPipelineState, since VertexDeclarationRHI(InputLayout) must be set
		// first).
		for (uint32_t StreamIndex = 0; StreamIndex < DrawDesc.vertexBuffersCount; ++StreamIndex)
		{
			if (DrawDesc.vertexBuffers[StreamIndex])
			{
				RHICmdList.SetStreamSource(StreamIndex,
					static_cast<FBufferRHIRef>(static_cast<FUnrealRHIBuffer*>(DrawDesc.vertexBuffers[StreamIndex])->Buffer), 0);
			}
		}

		BindResources(RHICmdList, CommonDesc, GraphicsPSO->VS.GetVertexShader(), ShaderStage::VS);
		BindResources(RHICmdList, CommonDesc, GraphicsPSO->PS.GetPixelShader(), ShaderStage::PS);
	}

	void ZibraUnrealRHI::EndDraw(FRHICommandListImmediate& RHICmdList, const DrawDescBase& Desc)
	{
		RHICmdList.EndRenderPass();
	}

}	 // namespace Zibra::RHI::APIUnrealRHI
