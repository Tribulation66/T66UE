// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBCommon.h"

#include "Algo/Count.h"
#include "Render/ZibraVDBRenderBuffer.h"

#include <fstream>

DEFINE_LOG_CATEGORY(LogZibraVDBRuntime);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 501
FArchive& operator<<(FArchive& Ar, FIntVector4& IntVector4)
{
	Ar << IntVector4.X;
	Ar << IntVector4.Y;
	Ar << IntVector4.Z;
	Ar << IntVector4.W;

	return Ar;
}
#endif

FZibraTexture3DRHIRef ZibraVDB::Utils::CreateTexture3D(const TCHAR* DebugName, uint32 SizeX, uint32 SizeY, uint32 SizeZ,
	uint8 Format, uint32 NumMips, ETextureCreateFlags Flags, ERHIAccess InResourceState)
{
	return RHICreateTexture(FRHITextureCreateDesc(DebugName, ETextureDimension::Texture3D)
								.SetExtent((int32) SizeX, (int32) SizeY)
								.SetDepth((uint16) SizeZ)
								.SetFormat((EPixelFormat) Format)
								.SetNumMips((uint8) NumMips)
								.SetFlags(Flags)
								.SetInitialState(InResourceState)
								.SetClearValue(FClearValueBinding::Black));
}

FZibraTexture3DRHIRef ZibraVDB::Utils::CreateTexture2D(const TCHAR* DebugName, uint32 SizeX, uint32 SizeY, uint32 SizeZ,
	uint8 Format, uint32 NumMips, ETextureCreateFlags Flags, ERHIAccess InResourceState)
{
	return RHICreateTexture(FRHITextureCreateDesc(DebugName, ETextureDimension::Texture2D)
								.SetExtent((int32) SizeX, (int32) SizeY)
								.SetFormat((EPixelFormat) Format)
								.SetNumMips((uint8) NumMips)
								.SetFlags(Flags)
								.SetInitialState(InResourceState)
								.SetClearValue(FClearValueBinding::Black));
}

uint32 ZibraVDB::Utils::CosToUint16(const float cosinus) noexcept
{
	check(cosinus >= -1.0f);
	check(cosinus <= 1.0f);
	return ((cosinus + 1.0f) / 2.0f) * (UINT16_MAX - 1);
}
float ZibraVDB::Utils::Uint16ToCos(const uint32 value) noexcept
{
	check(value <= UINT16_MAX - 1);
	return (float(value) / (UINT16_MAX - 1)) * 2.0f - 1.0f;
}

TStaticArray<uint32, 3> ZibraVDB::Utils::CalculateDivisionInvariantIntegers(const uint32 N) noexcept
{
	uint32 Temp = N;
	uint32 NFloorLog2 = 0;
	while (Temp >>= 1)
		++NFloorLog2;

	const uint32 Shift1 = (N & (N - 1)) != 0;
	const uint32 Shift2 = NFloorLog2;
	const uint32 Multiply = static_cast<uint32>((1ull << (32 + Shift1 + Shift2)) / N - (1ull << 32ull)) + 1u;

	TStaticArray<uint32, 3> Result{};
	Result[0] = Multiply;
	Result[1] = Shift1;
	Result[2] = Shift2;
	return Result;
}
