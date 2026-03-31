// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

// clang-format off
#include "ZIBRHIImpl.h"

#include "RHI.h"
#include "DynamicRHI.h"
#include "GlobalShader.h"
// clang-format on

namespace Zibra::RHI::APIUnrealRHI
{
	enum class EUnrealBufferType
	{
		Buffer,
		UniformBuffer,
		StaggingBuffer,
		Count
	};

	enum class EBufferFormat : int8_t
	{
		Buffer,
		StagingBuffer,
		UniformBuffer,
		VertexBuffer,
		IndexBuffer,
	};

	struct FBufferBridge
	{
		void* Buffer;
		EBufferFormat Format;
	};

	struct FFreeDeleter
	{
		void operator()(void* Pointer) noexcept
		{
			FMemory::Free(Pointer);
		}
	};

	class FUnrealRHIBuffer : public BufferInternal
	{
	public:
		TRefCountPtr<FRHIResource> Buffer;
		FShaderResourceViewRHIRef SRV;
		FUnorderedAccessViewRHIRef UAV;
		EUnrealBufferType BufferType;
		ERHIAccess CurrentState;

		/// Additional field used by EUnrealBufferType::UniformBuffer.
		TRefCountPtr<const FRHIResource> UniformBufferLayout;
		/// Additional field used by EUnrealBufferType::UniformBuffer and EUnrealBufferType::StaggingBuffer.
		TUniquePtr<unsigned char, FFreeDeleter> CPUBufferData;
	};

	class FUnrealRHITexture2D final : public Texture2DInternal
	{
	public:
		FTextureRHIRef Texture;
		FShaderResourceViewRHIRef SRV;
		FUnorderedAccessViewRHIRef UAV;
		ERHIAccess CurrentState;
	};

	class FUnrealRHITexture3D final : public Texture3DInternal
	{
	public:
		FTextureRHIRef Texture;
		FShaderResourceViewRHIRef SRV;
		FUnorderedAccessViewRHIRef UAV;
		ERHIAccess CurrentState;
	};

	class FUnrealComputePSO final : public ComputePSOInternal
	{
	public:
		TShaderRef<FShader> CS;
#ifdef ZIB_RHI_CHEAP_GPU_DEBUG
		FString CSShaderName;
#endif
	};

	class FUnrealGraphicsPSO final : public GraphicsPSOInternal
	{
	public:
		TShaderRef<FShader> VS;
#ifdef ZIB_RHI_CHEAP_GPU_DEBUG
		FString VSName;
#endif
		TShaderRef<FShader> PS;
#ifdef ZIB_RHI_CHEAP_GPU_DEBUG
		FString PSName;
#endif

		FDepthStencilStateRHIRef DepthStencilState;
		FRasterizerStateRHIRef RasterState;
		FVertexDeclarationRHIRef InputLayout;
		EPrimitiveType PrimitiveTopology;
		FBlendStateRHIRef BlendState;
	};

	class FUnrealRHISampler final : public SamplerInternal
	{
	public:
		FSamplerStateRHIRef Sampler;
	};

	class FUnrealFramebuffer : public FramebufferInternal
	{
	public:
		// Array of non-owning pointers (should not be deleted).
		TArray<FRHITexture*> RTVs{};
		// Non-owning pointer (should not be deleted).
		FRHITexture* DSV;
	};

	/// NOOP class for bindful API.
	class FUnrealRHIDescriptorHeap final : public DescriptorHeapInternal
	{
	};
}	 // namespace Zibra::RHI::APIUnrealRHI
