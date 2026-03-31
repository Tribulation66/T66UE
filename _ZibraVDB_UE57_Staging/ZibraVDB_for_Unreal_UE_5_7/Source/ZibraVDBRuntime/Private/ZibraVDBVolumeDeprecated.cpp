// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBVolumeDeprecated.h"

UZibraVDBVolume::UZibraVDBVolume() noexcept = default;
UZibraVDBVolume::UZibraVDBVolume(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UZibraVDBVolume::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	Ar << FileName << FrameCount << MinBBox << MaxBBox << MaxBlockCount << CompressionRate2 << ZeroOffset << QuantizationParameter
	   << UsedChannels << TemperatureRange << BinaryData;
}
