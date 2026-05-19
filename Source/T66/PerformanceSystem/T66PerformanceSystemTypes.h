// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66PerformanceSystemTypes.generated.h"

UENUM(BlueprintType)
enum class ET66PerformanceSeverity : uint8
{
	Info UMETA(DisplayName = "Info"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error"),
	Critical UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ET66PerformanceConfidence : uint8
{
	Exact UMETA(DisplayName = "Exact"),
	Sampled UMETA(DisplayName = "Sampled"),
	Inferred UMETA(DisplayName = "Inferred"),
	Unavailable UMETA(DisplayName = "Unavailable")
};

UENUM(BlueprintType)
enum class ET66PerformanceProtonStatus : uint8
{
	Detected UMETA(DisplayName = "Detected"),
	Likely UMETA(DisplayName = "Likely"),
	NotDetected UMETA(DisplayName = "Not Detected"),
	Unavailable UMETA(DisplayName = "Unavailable")
};

USTRUCT(BlueprintType)
struct T66_API FT66PerformanceAttribution
{
	GENERATED_BODY()

	FT66PerformanceAttribution() = default;

	FT66PerformanceAttribution(
		const FString& InName,
		const FString& InValue,
		const ET66PerformanceConfidence InConfidence,
		const FString& InSource,
		const double InSampleWindowSeconds)
		: Name(InName)
		, Value(InValue)
		, Confidence(InConfidence)
		, Source(InSource)
		, SampleWindowSeconds(InSampleWindowSeconds)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	FString Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	ET66PerformanceConfidence Confidence = ET66PerformanceConfidence::Unavailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	FString Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	double SampleWindowSeconds = 0.0;
};
