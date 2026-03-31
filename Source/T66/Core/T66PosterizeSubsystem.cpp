// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PosterizeSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Posterize, Log, All);

static const TCHAR* PosterizeMaterialPath = TEXT("/Game/UI/M_PosterizePostProcess.M_PosterizePostProcess");
static const FName ParamNameSteps(TEXT("Steps"));
static const FName ParamNameIntensity(TEXT("Intensity"));

UMaterialInterface* UT66PosterizeSubsystem::GetOrCreatePosterizeMaterial()
{
	UMaterialInterface* Loaded = LoadObject<UMaterialInterface>(nullptr, PosterizeMaterialPath);
	if (!Loaded)
	{
		UE_LOG(LogT66Posterize, Warning, TEXT("[Posterize] Failed to load material at %s - posterization disabled."), PosterizeMaterialPath);
	}
	return Loaded;
}

void UT66PosterizeSubsystem::SetEnabled(bool bEnable)
{
	bCurrentlyEnabled = bEnable;
	UWorld* World = GetWorld();
	if (!World) return;

	EnsureBlendableInWorld(World);
	ApplyParametersToBlendable();
}

void UT66PosterizeSubsystem::SetSteps(float InSteps)
{
	Steps = FMath::Clamp(InSteps, 2.f, 64.f);
	ApplyParametersToBlendable();
}

void UT66PosterizeSubsystem::SetIntensity(float InIntensity)
{
	Intensity = FMath::Clamp(InIntensity, 0.f, 1.f);
	ApplyParametersToBlendable();
}

void UT66PosterizeSubsystem::EnsureBlendableInWorld(UWorld* World)
{
	if (!World) return;

	if (PosterizeVolume && PosterizeVolume->GetWorld() == World)
	{
		return;
	}
	if (PosterizeVolume && PosterizeVolume->GetWorld() != World)
	{
		PosterizeVolume = nullptr;
		PosterizeDMI = nullptr;
	}

	APostProcessVolume* UseVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		if (It->bUnbound) { UseVolume = *It; break; }
	}
	if (!UseVolume)
	{
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			UseVolume = *It; break;
		}
	}
	if (!UseVolume)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UseVolume = World->SpawnActor<APostProcessVolume>(
			APostProcessVolume::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (UseVolume) UseVolume->bUnbound = true;
	}
	if (!UseVolume) return;

	PosterizeVolume = UseVolume;

	UMaterialInterface* BaseMat = GetOrCreatePosterizeMaterial();
	if (!BaseMat) return;

	if (!PosterizeDMI)
	{
		PosterizeDMI = UMaterialInstanceDynamic::Create(BaseMat, this);
	}
	if (!PosterizeDMI) return;

	FPostProcessSettings& PPS = UseVolume->Settings;
	for (int32 i = PPS.WeightedBlendables.Array.Num() - 1; i >= 0; --i)
	{
		if (PPS.WeightedBlendables.Array[i].Object == PosterizeDMI)
		{
			PPS.WeightedBlendables.Array.RemoveAt(i);
			break;
		}
	}
	PPS.WeightedBlendables.Array.Insert(FWeightedBlendable(0.0f, PosterizeDMI), 0);
}

void UT66PosterizeSubsystem::ApplyParametersToBlendable()
{
	if (!PosterizeDMI || !PosterizeVolume) return;

	const float Weight = bCurrentlyEnabled ? 1.0f : 0.0f;
	PosterizeDMI->SetScalarParameterValue(ParamNameSteps, Steps);
	PosterizeDMI->SetScalarParameterValue(ParamNameIntensity, Intensity);

	FPostProcessSettings& PPS = PosterizeVolume->Settings;
	for (FWeightedBlendable& Blend : PPS.WeightedBlendables.Array)
	{
		if (Blend.Object == PosterizeDMI)
		{
			Blend.Weight = Weight;
			break;
		}
	}
}

// Console commands for tuning
namespace
{
	static UT66PosterizeSubsystem* GetPosterizeSub()
	{
		UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
		if (!World) World = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWorld() : nullptr;
		if (!World || !World->GetGameInstance()) return nullptr;
		return World->GetGameInstance()->GetSubsystem<UT66PosterizeSubsystem>();
	}
}

static FAutoConsoleCommand PosterizeOnCmd(
	TEXT("PosterizeOn"),
	TEXT("Enable full-screen posterization."),
	FConsoleCommandDelegate::CreateLambda([]() { if (auto* S = GetPosterizeSub()) S->SetEnabled(true); })
);

static FAutoConsoleCommand PosterizeOffCmd(
	TEXT("PosterizeOff"),
	TEXT("Disable full-screen posterization."),
	FConsoleCommandDelegate::CreateLambda([]() { if (auto* S = GetPosterizeSub()) S->SetEnabled(false); })
);

static FAutoConsoleCommand PosterizeStepsCmd(
	TEXT("PosterizeSteps"),
	TEXT("Set posterization steps (2-64). Usage: PosterizeSteps 10"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		if (Args.Num() > 0)
		{
			if (auto* S = GetPosterizeSub()) S->SetSteps(FCString::Atof(*Args[0]));
		}
	})
);

static FAutoConsoleCommand PosterizeIntensityCmd(
	TEXT("PosterizeIntensity"),
	TEXT("Set posterization blend intensity (0-1). Usage: PosterizeIntensity 0.85"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		if (Args.Num() > 0)
		{
			if (auto* S = GetPosterizeSub()) S->SetIntensity(FCString::Atof(*Args[0]));
		}
	})
);
