// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "UObject/UObjectBase.h"
#include "ZibraLicensingManager.h"

#include "ZibraVDBDiagnosticsInfo.generated.h"

USTRUCT()
struct FZibraVDBDiagnosticsInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString Version;

	UPROPERTY()
	FString UnrealVersion;

	UPROPERTY()
	FString OS;

	UPROPERTY()
	FString OSVersion;

	UPROPERTY()
	uint32 FreeDiskSpaceMB = 0;

	UPROPERTY()
	uint64 PhysicalMemoryMB = 0;

	UPROPERTY()
	FString GPU;

	UPROPERTY()
	FString GPUDriverVersion;

	UPROPERTY()
	FString VRAMMB;

	UPROPERTY()
	FString CurrentRHI;

	UPROPERTY()
	FString DefaultRHI;

	UPROPERTY()
	TArray<FString> D3D12TargetShaderModels;

	UPROPERTY()
	bool IsLicenseKeySaved = 0;

	UPROPERTY()
	FString LicenseStatus[size_t(FZibraLicensingManager::Product::Count)];

	static FZibraVDBDiagnosticsInfo CollectInfo();
	static FString CollectInfoString();

private:
	static bool bIsCached;
	static FZibraVDBDiagnosticsInfo Cache;
};
