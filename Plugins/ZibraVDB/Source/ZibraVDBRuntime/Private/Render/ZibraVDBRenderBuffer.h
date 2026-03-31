// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "RenderResource.h"
#include "ZibraUnrealRHITypes.h"
#include "ZibraVDBRHI/Public/ZibraUnrealRHI.h"
#include "ZibraVDBRHI/Public/ZibraUnrealRHITypes.h"

// TODO: Convert the buffer type to bitmask
enum class BufferType
{
	Typed,
	ByteAddress
};

inline constexpr char DecompressionPerChannelBlockDataDebugName[] = "ZibraDecompressionPerChannelBlockData";
inline constexpr char DecompressionPerSpatialBlockInfoDebugName[] = "ZibraDecompressionPerSpatialBlockInfo";
inline constexpr char DecompressionPerChannelBlockInfoDebugName[] = "ZibraDecompressionPerChannelBlockInfo";
inline constexpr char RenderBlockBufferDebugName[] = "ZibraRenderBlockBuffer";

template <typename T, size_t Stride, BufferType BufferType, const char* DebugName, EPixelFormat BufferFormat = PF_Unknown,
	bool bSRV = false, bool bUAV = false>
class FZibraVDBRenderBuffer : public FRenderResource
{
public:
	void SetData(const TArray<T>& InVolumeGridData)
	{
		check(InVolumeGridData.Num() <= ElementCount);

		FRHICommandListImmediate& CmdList = FRHICommandListImmediate::Get();

		const auto Size = InVolumeGridData.Num() * sizeof(T);
		void* BufferData = CmdList.LockBuffer(Buffer, 0, Size, EResourceLockMode::RLM_WriteOnly);
		check(BufferData);

		FMemory::Memcpy(BufferData, InVolumeGridData.GetData(), Size);

		CmdList.UnlockBuffer(Buffer);
	}

	ERHIAccess GetCurrentState() const noexcept
	{
		return static_cast<Zibra::RHI::APIUnrealRHI::FUnrealRHIBuffer*>(ZibraRHIBuffer)->CurrentState;
	}

	void SetCurrentState(ERHIAccess InState) noexcept
	{
		static_cast<Zibra::RHI::APIUnrealRHI::FUnrealRHIBuffer*>(ZibraRHIBuffer)->CurrentState = InState;
	}

	void SetElementCount(uint64 InElementCount) noexcept
	{
		ElementCount = InElementCount;
	}

	uint64 GetElementCount() const noexcept
	{
		return ElementCount;
	}

	FShaderResourceViewRHIRef GetBufferSRV() const noexcept
	{
		return BufferSRV;
	}

	//~ Begin FRenderResource Interface
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
	void InitRHI(FRHICommandListBase& RHICmdList) final
#else
	void InitRHI() final
#endif
	{
		check(IsInRenderingThread());

		if constexpr (bSRV && bUAV)
		{
			CreateBufferAndSRVAndUAV();
		}
		else if constexpr (bSRV)
		{
			CreateBufferAndSRV();
		}
		else if constexpr (bUAV)
		{
			CreateBufferAndUAV();
		}
	}

	void ReleaseRHI() final
	{
		check(IsInRenderingThread());

		if (Buffer)
		{
			Buffer.SafeRelease();
		}
		if (BufferUAV)
		{
			BufferUAV.SafeRelease();
		}
		if (BufferSRV)
		{
			BufferSRV.SafeRelease();
		}
		if (ZibraRHIBuffer)
		{
			delete ZibraRHIBuffer;
		}
	}

	//~ End FRenderResource Interface

	FBufferRHIRef Buffer = nullptr;
	FShaderResourceViewRHIRef BufferSRV = nullptr;
	FUnorderedAccessViewRHIRef BufferUAV = nullptr;

	Zibra::RHI::Buffer* ZibraRHIBuffer;
	Zibra::RHI::APIUnrealRHI::FBufferBridge BufferBridge;

	uint64 ElementCount = 0;

private:
	void RegisterZibraRHIBuffer() noexcept
	{
		BufferBridge = {Buffer.GetReference(), Zibra::RHI::APIUnrealRHI::EBufferFormat::Buffer};
		Zibra::RHI::ReturnCode ReturnCode =
			Zibra::RHI::APIUnrealRHI::ZibraUnrealRHI::Get().RegisterBuffer(&BufferBridge, DebugName, &ZibraRHIBuffer);
	}

	void CreateBuffer()
	{
		if (!Buffer)
		{
			EBufferUsageFlags Usage = BUF_None;
			ERHIAccess InitialState = ERHIAccess::Unknown;
			if constexpr (bSRV)
			{
				Usage |= BUF_ShaderResource;
				InitialState = ERHIAccess::SRVMask;
			}
			if constexpr (bUAV)
			{
				Usage |= BUF_UnorderedAccess;
				InitialState = ERHIAccess::UAVMask;
			}

			if constexpr (BufferType == BufferType::ByteAddress)
			{
				Usage |= BUF_ByteAddressBuffer;
			}

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 506
			FRHIBufferCreateDesc CreateInfo(UTF8_TO_TCHAR(DebugName), sizeof(T) * ElementCount, Stride, Usage);
			CreateInfo.SetInitialState(InitialState);

			FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();
			Buffer = RHICmdList.CreateBuffer(CreateInfo);
#else
			FRHIResourceCreateInfo CreateInfo(UTF8_TO_TCHAR(DebugName));

			FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();
			Buffer = RHICmdList.CreateBuffer(sizeof(T) * ElementCount, Usage, Stride, InitialState, CreateInfo);
#endif

			check(Buffer.IsValid());

			RegisterZibraRHIBuffer();
		}
	}

	void CreateSRV()
	{
		if (Buffer && !BufferSRV)
		{
			FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();
			if constexpr (BufferType == BufferType::Typed)
			{
				BufferSRV = RHICmdList.CreateShaderResourceView(Buffer, FRHIViewDesc::CreateBufferSRV()
																			.SetFormat(BufferFormat)
																			.SetType(FRHIViewDesc::EBufferType::Typed)
																			.SetNumElements(Buffer->GetSize() / sizeof(T)));
			}
			else
			{
				check((Buffer->GetUsage() & EBufferUsageFlags::ByteAddressBuffer) == EBufferUsageFlags::ByteAddressBuffer);
				BufferSRV = RHICmdList.CreateShaderResourceView(Buffer, FRHIViewDesc::CreateBufferSRV()
																			.SetType(FRHIViewDesc::EBufferType::Raw)
																			.SetNumElements(Buffer->GetSize() / sizeof(uint32_t)));
			}
			check(BufferSRV.IsValid());
		}
	}

	void CreateUAV()
	{
		if (Buffer && !BufferUAV)
		{
			FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();
			if constexpr (BufferType == BufferType::Typed)
			{
				BufferUAV = RHICmdList.CreateUnorderedAccessView(Buffer, FRHIViewDesc::CreateBufferUAV()
																			 .SetFormat(BufferFormat)
																			 .SetType(FRHIViewDesc::EBufferType::Typed)
																			 .SetNumElements(Buffer->GetSize() / sizeof(T)));
			}
			else
			{
				check((Buffer->GetUsage() & EBufferUsageFlags::ByteAddressBuffer) == EBufferUsageFlags::ByteAddressBuffer);
				BufferUAV = RHICmdList.CreateUnorderedAccessView(Buffer, FRHIViewDesc::CreateBufferUAV()
																			 .SetType(FRHIViewDesc::EBufferType::Raw)
																			 .SetNumElements(Buffer->GetSize() / sizeof(uint32_t)));
			}

			check(BufferUAV.IsValid());
		}
	}

	void CreateBufferAndSRV()
	{
		CreateBuffer();
		CreateSRV();
	}

	void CreateBufferAndUAV()
	{
		CreateBuffer();
		CreateUAV();
	}

	void CreateBufferAndSRVAndUAV()
	{
		CreateBuffer();
		CreateSRV();
		CreateUAV();
	}
};

template <typename T, const char* DebugName, bool bSRV = false, bool bUAV = false>
class FZibraVDBTypedRenderBuffer final : public FZibraVDBRenderBuffer<T, 0, BufferType::Typed, DebugName, PF_R16F, bSRV, bUAV>
{
};

template <const char* DebugName, bool bSRV = false, bool bUAV = false>
class FZibraVDBByteAddressRenderBuffer final
	: public FZibraVDBRenderBuffer<uint32_t, 4, BufferType::ByteAddress, DebugName, PF_Unknown, bSRV, bUAV>
{
};

class FZibraVDBNativeResource final : public FRenderResource
{
public:
	struct FRenderParameters
	{
		FIntVector BlockGridSize;
		FIntVector NumGroups;
		int SpacialBlockCount;
		int ChannelBlockCount;

		int DensityChannelMask;
		int TemperatureChannelMask;
		int FlamesChannelMask;

		int PreviousDensityChannelMask;
		int PreviousTemperatureChannelMask;
		int PreviousFlamesChannelMask;

		float DensityChannelScale;
		float TemperatureChannelScale;
		float FlamesChannelScale;

		FIntVector TextureSize;
		FIntVector MaxBlockGridSize;

		int MaxRenderBlockCountCBRT;
		int MaxRenderBlockCountCBRTMultiply;
		int MaxRenderBlockCountCBRTShift1;
		int MaxRenderBlockCountCBRTShift2;

		float SequenceChannelScales[8];

		bool bHasPerFrameEqualTransformations;
	};

	struct FFrameParameters
	{
		uint32_t SpatialBlockCount;
		uint32_t ChannelBlockCount;
		FIntVector AABBSize;
		FIntVector FrameSize;
		float ChannelScales[8];
		TArray<FString> ChannelNames;
		FMatrix44f IndexToWorldTransformationInVoxels;
		FMatrix44f IndexToWorldTransform;
		FMatrix44f IndexToWorldRotation;
		FVector3f IndexToWorldScale;
		FVector3f IndexToUnrealUnitsScale;
		FVector3f IndexToWorldTranslationInVoxels;
	};

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
	void InitResource(FRHICommandListBase& RHICmdList) final;
#else
	void InitResource() final;
#endif
	void ReleaseResource() final;

	int CurrentFrameIndex;
	FRenderParameters RenderParameters;
	TArray<FFrameParameters> FrameHeaders;
	FZibraVDBTypedRenderBuffer<FFloat16, DecompressionPerChannelBlockDataDebugName, true, true> DecompressionPerChannelBlockData;
	FZibraVDBByteAddressRenderBuffer<DecompressionPerSpatialBlockInfoDebugName, true, true> DecompressionPerSpatialBlockInfo;
	FZibraVDBByteAddressRenderBuffer<DecompressionPerChannelBlockInfoDebugName, true, true> DecompressionPerChannelBlockInfo;
	FZibraVDBByteAddressRenderBuffer<RenderBlockBufferDebugName, true, true> RenderBlockBuffer;
};
