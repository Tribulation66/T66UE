// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RetroFXSubsystem.h"

#include "Core/T66PixelationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

namespace
{
	static const TCHAR* Ps1CandidatePaths[] = {
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_Sample_Instance.UE5RFX_PPM_PS1_Sample_Instance"),
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_SampleInstance.UE5RFX_PPM_PS1_SampleInstance"),
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_BaseMaterial_Inst.UE5RFX_PPM_PS1_BaseMaterial_Inst"),
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_BaseMaterial.UE5RFX_PPM_PS1_BaseMaterial")
	};

	static const TCHAR* N64BlurPath = TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_N64_Blur.UE5RFX_PPM_N64_Blur");
	static const TCHAR* N64BlurReplaceTonemapperPath = TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_N64_Blur_ReplaceTonemapper.UE5RFX_PPM_N64_Blur_ReplaceTonemapper");
	static const TCHAR* CrtCandidatePaths[] = {
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_CRT_Sample_Instance.UE5RFX_PPM_CRT_Sample_Instance"),
		TEXT("/Game/UE5RFX/Materials/PostProcess/BaseMaterials/UE5RFX_PPM_CRT.UE5RFX_PPM_CRT")
	};
	static const TCHAR* ResolutionCollectionPath = TEXT("/Game/UE5RFX/Materials/UE5RFX_MaterialParameterCollection.UE5RFX_MaterialParameterCollection");
	static const TCHAR* GeometryCollectionPath = TEXT("/Game/Materials/Retro/MPC_T66_RetroGeometry.MPC_T66_RetroGeometry");

	static UMaterialInterface* LoadFirstAvailableMaterial(const TCHAR* const* CandidatePaths, int32 CandidateCount)
	{
		for (int32 Index = 0; Index < CandidateCount; ++Index)
		{
			if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, CandidatePaths[Index]))
			{
				return Material;
			}
		}
		return nullptr;
	}

	static float ClampPercent(float Value)
	{
		return FMath::Clamp(Value, 0.0f, 100.0f);
	}

	static float PercentToUnit(float Value)
	{
		return ClampPercent(Value) / 100.0f;
	}

	static float PercentToSwitch(float Value)
	{
		return ClampPercent(Value) > KINDA_SMALL_NUMBER ? 1.0f : 0.0f;
	}

	static float PercentToRange(float Value, float MinValue, float MaxValue)
	{
		return FMath::Lerp(MinValue, MaxValue, PercentToUnit(Value));
	}

	static float PercentToInverseRange(float Value, float WeakValue, float StrongValue)
	{
		return FMath::Lerp(WeakValue, StrongValue, PercentToUnit(Value));
	}
}

void UT66RetroFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UT66RetroFXSubsystem::Deinitialize()
{
	ActiveVolume = nullptr;
	Ps1PostProcessDMI = nullptr;
	N64BlurDMI = nullptr;
	N64BlurReplaceTonemapperDMI = nullptr;
	CRTDMI = nullptr;
	ResolutionCollection = nullptr;
	GeometryCollection = nullptr;

	Super::Deinitialize();
}

void UT66RetroFXSubsystem::ApplyCurrentSettings(UWorld* World)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PlayerSettings = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			ApplySettings(PlayerSettings->GetRetroFXSettings(), World);
		}
	}
}

void UT66RetroFXSubsystem::ApplySettings(const FT66RetroFXSettings& Settings, UWorld* World)
{
	UWorld* TargetWorld = ResolveWorld(World);
	if (!TargetWorld)
	{
		return;
	}

	EnsureBlendablesInWorld(TargetWorld);
	ApplyBlendableWeights(Settings);
	ApplyPs1Parameters(Settings);
	ApplyN64Parameters(Settings);
	ApplyResolutionCollection(Settings, TargetWorld);
	ApplyGeometryCollection(Settings, TargetWorld);

	if (UGameInstance* GI = TargetWorld->GetGameInstance())
	{
		if (UT66PixelationSubsystem* Pixelation = GI->GetSubsystem<UT66PixelationSubsystem>())
		{
			const int32 PixelationLevel = FMath::RoundToInt(PercentToRange(Settings.T66PixelationPercent, 0.0f, 10.0f));
			Pixelation->SetPixelationLevel(PixelationLevel);
		}
	}
}

void UT66RetroFXSubsystem::EnsureBlendablesInWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (ActiveVolume && ActiveVolume->GetWorld() == World)
	{
		return;
	}

	if (ActiveVolume && ActiveVolume->GetWorld() != World)
	{
		ActiveVolume = nullptr;
	}

	APostProcessVolume* UseVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		if (It->bUnbound)
		{
			UseVolume = *It;
			break;
		}
	}
	if (!UseVolume)
	{
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			UseVolume = *It;
			break;
		}
	}
	if (!UseVolume)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UseVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (UseVolume)
		{
			UseVolume->bUnbound = true;
		}
	}
	if (!UseVolume)
	{
		return;
	}

	ActiveVolume = UseVolume;

	GetOrCreateDMI(LoadPs1PostProcessMaterial(), Ps1PostProcessDMI);
	GetOrCreateDMI(LoadN64BlurMaterial(false), N64BlurDMI);
	GetOrCreateDMI(LoadN64BlurMaterial(true), N64BlurReplaceTonemapperDMI);
	GetOrCreateDMI(LoadCRTMaterial(), CRTDMI);

	EnsureBlendableEntry(Ps1PostProcessDMI);
	EnsureBlendableEntry(N64BlurDMI);
	EnsureBlendableEntry(N64BlurReplaceTonemapperDMI);
	EnsureBlendableEntry(CRTDMI);
}

void UT66RetroFXSubsystem::ApplyBlendableWeights(const FT66RetroFXSettings& Settings)
{
	SetBlendableWeight(Ps1PostProcessDMI, PercentToUnit(Settings.PS1BlendPercent));
	SetBlendableWeight(CRTDMI, PercentToUnit(Settings.CRTBlendPercent));

	const float N64Weight = PercentToUnit(Settings.N64BlurBlendPercent);
	if (Settings.bUseUE5RFXN64BlurReplaceTonemapper)
	{
		SetBlendableWeight(N64BlurDMI, 0.0f);
		SetBlendableWeight(N64BlurReplaceTonemapperDMI, N64Weight);
	}
	else
	{
		SetBlendableWeight(N64BlurDMI, N64Weight);
		SetBlendableWeight(N64BlurReplaceTonemapperDMI, 0.0f);
	}
}

void UT66RetroFXSubsystem::ApplyPs1Parameters(const FT66RetroFXSettings& Settings)
{
	if (!Ps1PostProcessDMI)
	{
		return;
	}

	const float DitheringUnit = PercentToUnit(Settings.PS1DitheringPercent);
	const float FogSwitch = PercentToSwitch(Settings.PS1FogPercent);
	const float FogDensity = PercentToRange(Settings.PS1FogDensityPercent, 0.0f, 0.2f) * FogSwitch;
	const float FogStartDistance = PercentToInverseRange(Settings.PS1FogStartDistancePercent, 8000.0f, 0.0f);
	const float FogFallOffDistance = PercentToInverseRange(Settings.PS1FogFallOffDistancePercent, 10000.0f, 250.0f);
	const float ColorBoost = PercentToRange(Settings.PS1ColorBoostPercent, 1.0f, 2.5f);

	SetScalarParameter(Ps1PostProcessDMI, TEXT("UseDithering"), PercentToSwitch(Settings.PS1DitheringPercent));
	SetScalarParameter(Ps1PostProcessDMI, TEXT("Dithering"), DitheringUnit);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("UseBayerDithering"), PercentToSwitch(Settings.PS1BayerDitheringPercent));
	SetScalarParameter(Ps1PostProcessDMI, TEXT("UseColorLUT"), PercentToSwitch(Settings.PS1ColorLUTPercent));
	SetScalarParameter(Ps1PostProcessDMI, TEXT("ColorBoost"), ColorBoost);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("UseFog"), FogSwitch);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("FogDesnsity"), FogDensity);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("FogDensity"), FogDensity);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("FogStartDistance"), FogStartDistance);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("FogFallOffDistance"), FogFallOffDistance);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("UseSceneDepthForFog"), PercentToSwitch(Settings.PS1SceneDepthFogPercent));
}

void UT66RetroFXSubsystem::ApplyN64Parameters(const FT66RetroFXSettings& Settings)
{
	const float BlurSteps = PercentToRange(Settings.N64BlurStepsPercent, 0.0f, 12.0f);
	const float UseLowFakeResolution = PercentToSwitch(Settings.N64LowFakeResolutionPercent);

	SetScalarParameter(N64BlurDMI, TEXT("BlurSteps"), BlurSteps);
	SetScalarParameter(N64BlurDMI, TEXT("UseLowFakeResolution"), UseLowFakeResolution);
	SetScalarParameter(N64BlurReplaceTonemapperDMI, TEXT("BlurSteps"), BlurSteps);
	SetScalarParameter(N64BlurReplaceTonemapperDMI, TEXT("UseLowFakeResolution"), UseLowFakeResolution);
}

void UT66RetroFXSubsystem::ApplyResolutionCollection(const FT66RetroFXSettings& Settings, UWorld* World)
{
	if (!World)
	{
		return;
	}

	UMaterialParameterCollection* Collection = LoadResolutionCollection();
	if (!Collection)
	{
		return;
	}

	UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(Collection);
	if (!Instance)
	{
		return;
	}

	Instance->SetScalarParameterValue(TEXT("ResolutionSwitchSize"), PercentToSwitch(Settings.FakeResolutionSwitchSizePercent));
	Instance->SetScalarParameterValue(TEXT("ResolutionSwitchUV"), PercentToSwitch(Settings.FakeResolutionSwitchUVPercent));
	Instance->SetScalarParameterValue(TEXT("TargetResolutionHeight"), PercentToInverseRange(Settings.TargetResolutionHeightPercent, 1080.0f, 120.0f));
}

void UT66RetroFXSubsystem::ApplyGeometryCollection(const FT66RetroFXSettings& Settings, UWorld* World)
{
	if (!World)
	{
		return;
	}

	UMaterialParameterCollection* Collection = LoadGeometryCollection();
	if (!Collection)
	{
		return;
	}

	UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(Collection);
	if (!Instance)
	{
		return;
	}

	const float WorldNoiseAmount = PercentToRange(Settings.WorldVertexNoisePercent, 0.0f, 8.0f);
	const float CharacterNoiseAmount = PercentToRange(Settings.CharacterVertexNoisePercent, 0.0f, 8.0f);

	Instance->SetScalarParameterValue(TEXT("WorldVertexSnapStrength"), PercentToRange(Settings.WorldVertexSnapPercent, 0.0f, 4.0f));
	Instance->SetScalarParameterValue(TEXT("WorldVertexSnapTargetResolution"), PercentToInverseRange(Settings.WorldVertexSnapResolutionPercent, 1080.0f, 120.0f));
	Instance->SetVectorParameterValue(TEXT("WorldVertexNoiseOffset"), FLinearColor(WorldNoiseAmount, WorldNoiseAmount, WorldNoiseAmount, 0.0f));
	Instance->SetScalarParameterValue(TEXT("WorldAffineBlend"), PercentToUnit(Settings.WorldAffineBlendPercent));
	Instance->SetScalarParameterValue(TEXT("WorldAffineDistance1"), PercentToRange(Settings.WorldAffineDistance1Percent, 100.0f, 700.0f));
	Instance->SetScalarParameterValue(TEXT("WorldAffineDistance2"), PercentToRange(Settings.WorldAffineDistance2Percent, 300.0f, 2100.0f));
	Instance->SetScalarParameterValue(TEXT("WorldAffineDistance3"), PercentToRange(Settings.WorldAffineDistance3Percent, 600.0f, 3000.0f));

	Instance->SetScalarParameterValue(TEXT("CharacterVertexSnapStrength"), PercentToRange(Settings.CharacterVertexSnapPercent, 0.0f, 4.0f));
	Instance->SetScalarParameterValue(TEXT("CharacterVertexSnapTargetResolution"), PercentToInverseRange(Settings.CharacterVertexSnapResolutionPercent, 1080.0f, 120.0f));
	Instance->SetVectorParameterValue(TEXT("CharacterVertexNoiseOffset"), FLinearColor(CharacterNoiseAmount, CharacterNoiseAmount, CharacterNoiseAmount, 0.0f));
	Instance->SetScalarParameterValue(TEXT("CharacterAffineBlend"), PercentToUnit(Settings.CharacterAffineBlendPercent));
	Instance->SetScalarParameterValue(TEXT("CharacterAffineDistance1"), PercentToRange(Settings.CharacterAffineDistance1Percent, 100.0f, 700.0f));
	Instance->SetScalarParameterValue(TEXT("CharacterAffineDistance2"), PercentToRange(Settings.CharacterAffineDistance2Percent, 300.0f, 2100.0f));
	Instance->SetScalarParameterValue(TEXT("CharacterAffineDistance3"), PercentToRange(Settings.CharacterAffineDistance3Percent, 600.0f, 3000.0f));
}

UMaterialInterface* UT66RetroFXSubsystem::LoadPs1PostProcessMaterial()
{
	return LoadFirstAvailableMaterial(Ps1CandidatePaths, UE_ARRAY_COUNT(Ps1CandidatePaths));
}

UMaterialInterface* UT66RetroFXSubsystem::LoadN64BlurMaterial(bool bReplaceTonemapper)
{
	return LoadObject<UMaterialInterface>(nullptr, bReplaceTonemapper ? N64BlurReplaceTonemapperPath : N64BlurPath);
}

UMaterialInterface* UT66RetroFXSubsystem::LoadCRTMaterial()
{
	return LoadFirstAvailableMaterial(CrtCandidatePaths, UE_ARRAY_COUNT(CrtCandidatePaths));
}

UMaterialParameterCollection* UT66RetroFXSubsystem::LoadResolutionCollection()
{
	if (!ResolutionCollection)
	{
		ResolutionCollection = LoadObject<UMaterialParameterCollection>(nullptr, ResolutionCollectionPath);
	}
	return ResolutionCollection;
}

UMaterialParameterCollection* UT66RetroFXSubsystem::LoadGeometryCollection()
{
	if (!GeometryCollection)
	{
		GeometryCollection = LoadObject<UMaterialParameterCollection>(nullptr, GeometryCollectionPath);
	}
	return GeometryCollection;
}

UMaterialInstanceDynamic* UT66RetroFXSubsystem::GetOrCreateDMI(UMaterialInterface* BaseMaterial, TObjectPtr<UMaterialInstanceDynamic>& CachedDMI)
{
	if (CachedDMI || !BaseMaterial)
	{
		return CachedDMI;
	}

	CachedDMI = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	return CachedDMI;
}

void UT66RetroFXSubsystem::EnsureBlendableEntry(UMaterialInstanceDynamic* DMI)
{
	if (!ActiveVolume || !DMI)
	{
		return;
	}

	FPostProcessSettings& PPS = ActiveVolume->Settings;
	for (const FWeightedBlendable& Blend : PPS.WeightedBlendables.Array)
	{
		if (Blend.Object == DMI)
		{
			return;
		}
	}

	PPS.WeightedBlendables.Array.Add(FWeightedBlendable(0.0f, DMI));
}

void UT66RetroFXSubsystem::SetBlendableWeight(UMaterialInstanceDynamic* DMI, float Weight)
{
	if (!ActiveVolume || !DMI)
	{
		return;
	}

	FPostProcessSettings& PPS = ActiveVolume->Settings;
	for (FWeightedBlendable& Blend : PPS.WeightedBlendables.Array)
	{
		if (Blend.Object == DMI)
		{
			Blend.Weight = Weight;
			return;
		}
	}
}

void UT66RetroFXSubsystem::SetScalarParameter(UMaterialInstanceDynamic* DMI, FName ParamName, float Value)
{
	if (!DMI)
	{
		return;
	}

	DMI->SetScalarParameterValue(ParamName, Value);
}

UWorld* UT66RetroFXSubsystem::ResolveWorld(UWorld* PreferredWorld) const
{
	if (PreferredWorld)
	{
		return PreferredWorld;
	}

	if (UWorld* CurrentPlayWorld = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		return CurrentPlayWorld;
	}

	if (GEngine && GEngine->GameViewport)
	{
		return GEngine->GameViewport->GetWorld();
	}

	return nullptr;
}
