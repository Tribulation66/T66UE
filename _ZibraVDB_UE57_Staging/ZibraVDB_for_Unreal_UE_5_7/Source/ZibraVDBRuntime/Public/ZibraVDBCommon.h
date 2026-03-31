// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "RHIAccess.h"
#include "RHIFwd.h"
#include "RHIResources.h"
#include "Runtime/Launch/Resources/Version.h"
#include "ZibraVDBFile.h"

DECLARE_LOG_CATEGORY_EXTERN(LogZibraVDBRuntime, Log, All);

constexpr auto ZIBRAVDB_BLOCK_SIZE = 8;
constexpr auto ZIBRAVDB_RENDER_BLOCK_PADDING = 1;
constexpr auto ZIBRAVDB_RENDER_BLOCK_SIZE = ZIBRAVDB_BLOCK_SIZE + ZIBRAVDB_RENDER_BLOCK_PADDING;
constexpr auto ZIBRAVDB_MAX_DISPATCH_BLOCKS_NUMBER = 65535;
constexpr auto ZIBRAVDB_MAX_BLOCKS_FOR_RAYMARCHING_INTERPOLATION = 500000;

template <typename T, size_t size>
constexpr size_t ZIB_ARR_SIZE(T (&)[size])
{
	return size;
}

class FZibraVDBNativeResource;

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 501
using FZibraTexture3DRHIRef = FTextureRHIRef;
using FZibraTexture2DRHIRef = FTextureRHIRef;
#else
using FZibraTexture3DRHIRef = FTexture3DRHIRef;
using FZibraTexture2DRHIRef = FTexture2DRHIRef;
#endif

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 501
FArchive& operator<<(FArchive& Ar, FIntVector4& IntVector4);
#endif

namespace ZibraVDB::Utils
{
	FZibraTexture3DRHIRef CreateTexture3D(const TCHAR* DebugName, uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint8 Format,
		uint32 NumMips, ETextureCreateFlags Flags, ERHIAccess InResourceState);
	FZibraTexture3DRHIRef CreateTexture2D(const TCHAR* DebugName, uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint8 Format,
		uint32 NumMips, ETextureCreateFlags Flags, ERHIAccess InResourceState);

	uint32 CosToUint16(const float cosinus) noexcept;
	float Uint16ToCos(const uint32 value) noexcept;
	constexpr float MetersToCantimeters = 100.f;
	constexpr float MetersToUnrealUnits = MetersToCantimeters;

	// Source: https://gmplib.org/~tege/divcnst-pldi94.pdf
	TStaticArray<uint32, 3> CalculateDivisionInvariantIntegers(const uint32 N) noexcept;
}	 // namespace ZibraVDB::Utils

UENUM(BlueprintType)
enum class ResolutionDownscaleFactors : uint8
{
	Original = 0 UMETA(DisplayName = "1"),
	Half = 1 UMETA(DisplayName = "1/2"),
	Quarter = 2 UMETA(DisplayName = "1/4"),
	Eighth = 3 UMETA(DisplayName = "1/8")
};

inline int GetDownscaleFactorExponent(ResolutionDownscaleFactors Factor)
{
	return FMath::Pow(2.f, static_cast<int>(Factor));
}

inline float ConvertDownscaleFactorToScale(ResolutionDownscaleFactors Factor)
{
	return 1.0f / GetDownscaleFactorExponent(Factor);
}
