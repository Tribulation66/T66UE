// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "T66PerformanceSystemSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "T66 Performance System"))
class T66_API UT66PerformanceSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "General")
	bool bEnablePerformanceSystem = true;

	UPROPERTY(Config, EditAnywhere, Category = "Snapshots", meta = (ClampMin = "1.0", ClampMax = "120.0"))
	double SnapshotCadenceSeconds = 5.0;

	UPROPERTY(Config, EditAnywhere, Category = "Snapshots", meta = (ClampMin = "5.0", ClampMax = "300.0"))
	double FrameWindowSeconds = 60.0;

	UPROPERTY(Config, EditAnywhere, Category = "Snapshots", meta = (ClampMin = "10", ClampMax = "5000"))
	int32 MaxCapturedLogLines = 200;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Frame", meta = (ClampMin = "1.0", ClampMax = "500.0"))
	double HitchThresholdMs = 50.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Frame", meta = (ClampMin = "1.0", ClampMax = "120.0"))
	double SustainedLowFpsThreshold = 50.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Frame", meta = (ClampMin = "1.0", ClampMax = "60.0"))
	double SustainedLowFpsWindowSeconds = 10.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Frame", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	double StutterStdDevThresholdMs = 8.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Frame", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	double FrameDetectorCooldownSeconds = 5.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Memory", meta = (ClampMin = "30.0", ClampMax = "1800.0"))
	double MemorySlopeWindowSeconds = 300.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Memory", meta = (ClampMin = "1.0", ClampMax = "4096.0"))
	double MemoryGrowthWarningMbPerMinute = 128.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|GC", meta = (ClampMin = "1.0", ClampMax = "500.0"))
	double GCPauseWarningMs = 12.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Hang", meta = (ClampMin = "1.0", ClampMax = "60.0"))
	double BasicHangFrameDeltaSeconds = 8.0;

	UPROPERTY(Config, EditAnywhere, Category = "Detectors|Project Operations", meta = (ClampMin = "1.0", ClampMax = "5000.0"))
	double ProjectOperationWarningMs = 25.0;

	UPROPERTY(Config, EditAnywhere, Category = "Self Cost", meta = (ClampMin = "10.0", ClampMax = "5000.0"))
	double DetectorBudgetUs = 200.0;

	UPROPERTY(Config, EditAnywhere, Category = "Self Cost", meta = (ClampMin = "50.0", ClampMax = "10000.0"))
	double FrameworkFrameBudgetUs = 500.0;

	UPROPERTY(Config, EditAnywhere, Category = "Privacy")
	bool bIncludeHardwareFingerprintInDevelopment = true;

	UPROPERTY(Config, EditAnywhere, Category = "Privacy")
	bool bIncludeHardwareFingerprintInShipping = false;

	UPROPERTY(Config, EditAnywhere, Category = "Retention", meta = (ClampMin = "16", ClampMax = "4096"))
	int32 DevelopmentDirectoryBudgetMb = 256;

	UPROPERTY(Config, EditAnywhere, Category = "Retention", meta = (ClampMin = "1", ClampMax = "512"))
	int32 DevelopmentSessionBudgetMb = 25;

	UPROPERTY(Config, EditAnywhere, Category = "Retention", meta = (ClampMin = "16", ClampMax = "4096"))
	int32 ShippingDirectoryBudgetMb = 64;

	UPROPERTY(Config, EditAnywhere, Category = "Retention", meta = (ClampMin = "1", ClampMax = "512"))
	int32 ShippingSessionBudgetMb = 10;
};

