// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RetroFXSubsystem.h"

#include "Components/MeshComponent.h"
#include "Core/T66PixelationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RetroFXRuntime, Log, All);

namespace
{
	static const TCHAR* CharacterBaseMaterialPath = TEXT("/Game/Materials/M_Character_Unlit.M_Character_Unlit");
	static const TCHAR* EnvironmentBaseMaterialPath = TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit");
	static const TCHAR* FbxBaseMaterialPath = TEXT("/Game/Materials/M_FBX_Unlit.M_FBX_Unlit");
	static const TCHAR* GlbBaseMaterialPath = TEXT("/Game/Materials/M_GLB_Unlit.M_GLB_Unlit");
	static const TCHAR* CharacterRetroGeometryMaterialPath = TEXT("/Game/Materials/Retro/M_Character_Unlit_RetroGeometry.M_Character_Unlit_RetroGeometry");
	static const TCHAR* EnvironmentRetroGeometryMaterialPath = TEXT("/Game/Materials/Retro/M_Environment_Unlit_RetroGeometry.M_Environment_Unlit_RetroGeometry");
	static const TCHAR* FbxRetroGeometryMaterialPath = TEXT("/Game/Materials/Retro/M_FBX_Unlit_RetroGeometry.M_FBX_Unlit_RetroGeometry");
	static const TCHAR* GlbRetroGeometryMaterialPath = TEXT("/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry.M_GLB_Unlit_RetroGeometry");

	static const TCHAR* Ps1CandidatePaths[] = {
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_Sample_Instance.UE5RFX_PPM_PS1_Sample_Instance"),
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_SampleInstance.UE5RFX_PPM_PS1_SampleInstance"),
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_BaseMaterial_Inst.UE5RFX_PPM_PS1_BaseMaterial_Inst"),
		TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_PS1_BaseMaterial.UE5RFX_PPM_PS1_BaseMaterial")
	};

	static const TCHAR* N64BlurPath = TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_N64_Blur.UE5RFX_PPM_N64_Blur");
	static const TCHAR* N64BlurReplaceTonemapperPath = TEXT("/Game/UE5RFX/Materials/PostProcess/UE5RFX_PPM_N64_Blur_ReplaceTonemapper.UE5RFX_PPM_N64_Blur_ReplaceTonemapper");
	static const TCHAR* ChromaticAberrationMaterialPath = TEXT("/Game/Materials/Retro/M_RetroChromaticAberrationPostProcess.M_RetroChromaticAberrationPostProcess");
	static const TCHAR* ResolutionCollectionPath = TEXT("/Game/UE5RFX/Materials/UE5RFX_MaterialParameterCollection.UE5RFX_MaterialParameterCollection");
	static const TCHAR* GeometryCollectionPath = TEXT("/Game/Materials/Retro/MPC_T66_RetroGeometry.MPC_T66_RetroGeometry");
	static constexpr float RetroPostProcessPriority = 5000.0f;

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

	static float GetTargetResolutionHeight(float Value)
	{
		return PercentToInverseRange(Value, 1080.0f, 120.0f);
	}

	enum ET66Ps1VariantBits : uint8
	{
		Ps1Variant_ColorLUT = 1 << 0,
		Ps1Variant_SceneDepthFog = 1 << 1,
		Ps1Variant_BayerDithering = 1 << 2
	};

	static uint8 BuildPs1VariantMask(const FT66RetroFXSettings& Settings)
	{
		uint8 Mask = 0;
		if (PercentToSwitch(Settings.PS1ColorLUTPercent) > 0.5f)
		{
			Mask |= Ps1Variant_ColorLUT;
		}
		if (PercentToSwitch(Settings.PS1SceneDepthFogPercent) > 0.5f)
		{
			Mask |= Ps1Variant_SceneDepthFog;
		}
		if (PercentToSwitch(Settings.PS1BayerDitheringPercent) > 0.5f)
		{
			Mask |= Ps1Variant_BayerDithering;
		}
		return Mask;
	}

	static FString GetPs1VariantMaterialPath(uint8 VariantMask)
	{
		const int32 bUseColorLUT = (VariantMask & Ps1Variant_ColorLUT) ? 1 : 0;
		const int32 bUseSceneDepthFog = (VariantMask & Ps1Variant_SceneDepthFog) ? 1 : 0;
		const int32 bUseBayerDithering = (VariantMask & Ps1Variant_BayerDithering) ? 1 : 0;
		return FString::Printf(
			TEXT("/Game/Materials/Retro/PS1/MI_T66_PS1_C%d_S%d_B%d.MI_T66_PS1_C%d_S%d_B%d"),
			bUseColorLUT,
			bUseSceneDepthFog,
			bUseBayerDithering,
			bUseColorLUT,
			bUseSceneDepthFog,
			bUseBayerDithering);
	}

	static UTexture* GetWhiteFallbackTexture()
	{
		static TObjectPtr<UTexture> Cached = nullptr;
		if (!Cached)
		{
			Cached = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
		}
		if (!Cached)
		{
			Cached = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
		}
		return Cached.Get();
	}

	static void CopyAllScalarParameters(UMaterialInterface* SourceMaterial, UMaterialInstanceDynamic* TargetMaterial)
	{
		if (!SourceMaterial || !TargetMaterial)
		{
			return;
		}

		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> ParameterIds;
		SourceMaterial->GetAllScalarParameterInfo(ParameterInfos, ParameterIds);
		for (const FMaterialParameterInfo& Info : ParameterInfos)
		{
			float Value = 0.0f;
			if (SourceMaterial->GetScalarParameterValue(FHashedMaterialParameterInfo(Info), Value))
			{
				TargetMaterial->SetScalarParameterValue(Info.Name, Value);
			}
		}
	}

	static void CopyAllVectorParameters(UMaterialInterface* SourceMaterial, UMaterialInstanceDynamic* TargetMaterial)
	{
		if (!SourceMaterial || !TargetMaterial)
		{
			return;
		}

		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> ParameterIds;
		SourceMaterial->GetAllVectorParameterInfo(ParameterInfos, ParameterIds);
		for (const FMaterialParameterInfo& Info : ParameterInfos)
		{
			FLinearColor Value = FLinearColor::White;
			if (SourceMaterial->GetVectorParameterValue(FHashedMaterialParameterInfo(Info), Value))
			{
				TargetMaterial->SetVectorParameterValue(Info.Name, Value);
			}
		}
	}

	static void CopyAllTextureParameters(UMaterialInterface* SourceMaterial, UMaterialInstanceDynamic* TargetMaterial)
	{
		if (!SourceMaterial || !TargetMaterial)
		{
			return;
		}

		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> ParameterIds;
		SourceMaterial->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
		for (const FMaterialParameterInfo& Info : ParameterInfos)
		{
			UTexture* Value = nullptr;
			if (SourceMaterial->GetTextureParameterValue(FHashedMaterialParameterInfo(Info), Value) && Value)
			{
				TargetMaterial->SetTextureParameterValue(Info.Name, Value);
			}
		}
	}

	static void BackfillRetroSharedParameters(UMaterialInterface* SourceMaterial, UMaterialInstanceDynamic* TargetMaterial)
	{
		if (!TargetMaterial)
		{
			return;
		}

		const UTexture* WhiteTexture = GetWhiteFallbackTexture();

		auto SetScalarWithDefault = [SourceMaterial, TargetMaterial](const TCHAR* ParameterName, float DefaultValue)
		{
			float Value = DefaultValue;
			if (SourceMaterial)
			{
				SourceMaterial->GetScalarParameterValue(FHashedMaterialParameterInfo(FMaterialParameterInfo(ParameterName)), Value);
			}
			TargetMaterial->SetScalarParameterValue(ParameterName, Value);
		};

		auto SetVectorWithDefault = [SourceMaterial, TargetMaterial](const TCHAR* ParameterName, const FLinearColor& DefaultValue)
		{
			FLinearColor Value = DefaultValue;
			if (SourceMaterial)
			{
				SourceMaterial->GetVectorParameterValue(FHashedMaterialParameterInfo(FMaterialParameterInfo(ParameterName)), Value);
			}
			TargetMaterial->SetVectorParameterValue(ParameterName, Value);
		};

		auto SetTextureWithFallback = [SourceMaterial, TargetMaterial, WhiteTexture](const TCHAR* ParameterName)
		{
			UTexture* Value = nullptr;
			if (SourceMaterial)
			{
				SourceMaterial->GetTextureParameterValue(FHashedMaterialParameterInfo(FMaterialParameterInfo(ParameterName)), Value);
			}
			if (!Value)
			{
				Value = const_cast<UTexture*>(WhiteTexture);
			}
			if (Value)
			{
				TargetMaterial->SetTextureParameterValue(ParameterName, Value);
			}
		};

		SetTextureWithFallback(TEXT("DiffuseColorMap"));
		SetTextureWithFallback(TEXT("BaseColorTexture"));
		SetTextureWithFallback(TEXT("NormalMap"));
		SetTextureWithFallback(TEXT("SpecularColorMap"));
		SetTextureWithFallback(TEXT("EmissiveColorMap"));
		SetScalarWithDefault(TEXT("Brightness"), 1.0f);
		SetScalarWithDefault(TEXT("EmissiveColorMapWeight"), 0.0f);
		SetVectorWithDefault(TEXT("Tint"), FLinearColor::White);
		SetVectorWithDefault(TEXT("Color"), FLinearColor::White);
		SetVectorWithDefault(TEXT("BaseColor"), FLinearColor::White);
	}

	static void CopyRetroSourceParameters(UMaterialInterface* SourceMaterial, UMaterialInstanceDynamic* TargetMaterial)
	{
		if (!SourceMaterial || !TargetMaterial)
		{
			return;
		}

		TargetMaterial->CopyMaterialUniformParameters(SourceMaterial);
		CopyAllScalarParameters(SourceMaterial, TargetMaterial);
		CopyAllVectorParameters(SourceMaterial, TargetMaterial);
		CopyAllTextureParameters(SourceMaterial, TargetMaterial);
		BackfillRetroSharedParameters(SourceMaterial, TargetMaterial);
	}

	static bool HasWorldGeometryEnabled(const FT66RetroFXSettings& Settings)
	{
		return Settings.bEnableWorldGeometry
			&& (ClampPercent(Settings.WorldVertexSnapPercent) > KINDA_SMALL_NUMBER
				|| ClampPercent(Settings.WorldVertexNoisePercent) > KINDA_SMALL_NUMBER
				|| ClampPercent(Settings.WorldAffineBlendPercent) > KINDA_SMALL_NUMBER);
	}

	static bool HasCharacterGeometryEnabled(const FT66RetroFXSettings& Settings)
	{
		return Settings.bEnableCharacterGeometry
			&& (ClampPercent(Settings.CharacterVertexSnapPercent) > KINDA_SMALL_NUMBER
				|| ClampPercent(Settings.CharacterVertexNoisePercent) > KINDA_SMALL_NUMBER
				|| ClampPercent(Settings.CharacterAffineBlendPercent) > KINDA_SMALL_NUMBER);
	}

	static bool IsGroupEnabled(ET66RetroGeometryGroup Group, bool bEnableWorldGeometry, bool bEnableCharacterGeometry)
	{
		return Group == ET66RetroGeometryGroup::World ? bEnableWorldGeometry : bEnableCharacterGeometry;
	}

	static FT66RetroFXSettings BuildEffectiveSettings(const FT66RetroFXSettings& Settings)
	{
		if (Settings.bEnableRetroFXMaster)
		{
			return Settings;
		}

		FT66RetroFXSettings DisabledSettings = Settings;
		DisabledSettings.PS1BlendPercent = 0.0f;
		DisabledSettings.PS1DitheringPercent = 0.0f;
		DisabledSettings.PS1BayerDitheringPercent = 0.0f;
		DisabledSettings.PS1ColorLUTPercent = 0.0f;
		DisabledSettings.PS1ColorBoostPercent = 0.0f;
		DisabledSettings.PS1FogPercent = 0.0f;
		DisabledSettings.PS1FogDensityPercent = 0.0f;
		DisabledSettings.PS1FogStartDistancePercent = 0.0f;
		DisabledSettings.PS1FogFallOffDistancePercent = 0.0f;
		DisabledSettings.PS1SceneDepthFogPercent = 0.0f;
		DisabledSettings.bUseRealLowResolution = false;
		DisabledSettings.FakeResolutionSwitchSizePercent = 0.0f;
		DisabledSettings.FakeResolutionSwitchUVPercent = 0.0f;
		DisabledSettings.TargetResolutionHeightPercent = 0.0f;
		DisabledSettings.N64BlurBlendPercent = 0.0f;
		DisabledSettings.N64BlurStepsPercent = 0.0f;
		DisabledSettings.N64LowFakeResolutionPercent = 0.0f;
		DisabledSettings.bUseUE5RFXN64BlurReplaceTonemapper = false;
		DisabledSettings.ChromaticAberrationPercent = 0.0f;
		DisabledSettings.ChromaticDistortionPercent = 0.0f;
		DisabledSettings.bInvertChromaticDistortion = false;
		DisabledSettings.T66PixelationPercent = 0.0f;
		DisabledSettings.bEnableWorldGeometry = false;
		DisabledSettings.WorldVertexSnapPercent = 0.0f;
		DisabledSettings.WorldVertexSnapResolutionPercent = 0.0f;
		DisabledSettings.WorldVertexNoisePercent = 0.0f;
		DisabledSettings.WorldAffineBlendPercent = 0.0f;
		DisabledSettings.WorldAffineDistance1Percent = 0.0f;
		DisabledSettings.WorldAffineDistance2Percent = 0.0f;
		DisabledSettings.WorldAffineDistance3Percent = 0.0f;
		DisabledSettings.bEnableCharacterGeometry = false;
		DisabledSettings.CharacterVertexSnapPercent = 0.0f;
		DisabledSettings.CharacterVertexSnapResolutionPercent = 0.0f;
		DisabledSettings.CharacterVertexNoisePercent = 0.0f;
		DisabledSettings.CharacterAffineBlendPercent = 0.0f;
		DisabledSettings.CharacterAffineDistance1Percent = 0.0f;
		DisabledSettings.CharacterAffineDistance2Percent = 0.0f;
		DisabledSettings.CharacterAffineDistance3Percent = 0.0f;
		return DisabledSettings;
	}

	static FString GetMaterialBasePath(const UMaterialInterface* Material)
	{
		if (!Material)
		{
			return FString();
		}

		UMaterialInterface* MutableMaterial = const_cast<UMaterialInterface*>(Material);
		if (const UMaterialInterface* BaseMaterial = MutableMaterial ? MutableMaterial->GetBaseMaterial() : nullptr)
		{
			return BaseMaterial->GetPathName();
		}

		return Material->GetPathName();
	}
}

void UT66RetroFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UT66RetroFXSubsystem::Deinitialize()
{
	UpdateGeometrySpawnBinding(nullptr, false);
	RestoreManagedMaterials(true, true);
	RestoreResolutionRuntimeDefaults();
	ManagedGeometrySlots.Reset();
	ManagedGeometryWorld = nullptr;
	bWorldGeometryActive = false;
	bCharacterGeometryActive = false;
	bResolutionRuntimeActive = false;

	ActiveVolume = nullptr;
	Ps1PostProcessDMI = nullptr;
	N64BlurDMI = nullptr;
	N64BlurReplaceTonemapperDMI = nullptr;
	ChromaticAberrationDMI = nullptr;
	ResolutionCollection = nullptr;
	GeometryCollection = nullptr;
	ActivePs1MaterialPath.Reset();
	CharacterRetroGeometryMaterial = nullptr;
	EnvironmentRetroGeometryMaterial = nullptr;
	FbxRetroGeometryMaterial = nullptr;
	GlbRetroGeometryMaterial = nullptr;

	Super::Deinitialize();
}

void UT66RetroFXSubsystem::ApplyCurrentSettings(UWorld* World)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PlayerSettings = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			UE_LOG(LogT66RetroFXRuntime, Log, TEXT("ApplyCurrentSettings: applying saved Retro FX settings to world=%s"), *GetNameSafe(World ? World : ResolveWorld(nullptr)));
			ApplySettings(PlayerSettings->GetRetroFXSettings(), World);
		}
		else
		{
			UE_LOG(LogT66RetroFXRuntime, Warning, TEXT("ApplyCurrentSettings: Player settings subsystem was null"));
		}
	}
	else
	{
		UE_LOG(LogT66RetroFXRuntime, Warning, TEXT("ApplyCurrentSettings: GameInstance was null"));
	}
}

void UT66RetroFXSubsystem::ApplySettings(const FT66RetroFXSettings& Settings, UWorld* World)
{
	UWorld* TargetWorld = ResolveWorld(World);
	if (!TargetWorld)
	{
		UE_LOG(LogT66RetroFXRuntime, Warning, TEXT("ApplySettings: resolved world was null"));
		return;
	}

	UE_LOG(LogT66RetroFXRuntime, Log,
		TEXT("ApplySettings: world=%s MasterEnabled=%s PS1Blend=%.2f PS1Dither=%.2f PS1Bayer=%.2f PS1ColorLUT=%.2f PS1ColorBoost=%.2f ChromaticStrength=%.2f DistortionStrength=%.2f InvertDistortion=%s PS1FogEnable=%.2f PS1SceneDepthFog=%.2f PS1FogDensity=%.2f PS1FogStart=%.2f PS1FogFalloff=%.2f RealLowRes=%s FakeSize=%.2f FakeUV=%.2f TargetRes=%.2f"),
		*GetNameSafe(TargetWorld),
		Settings.bEnableRetroFXMaster ? TEXT("true") : TEXT("false"),
		Settings.PS1BlendPercent,
		Settings.PS1DitheringPercent,
		Settings.PS1BayerDitheringPercent,
		Settings.PS1ColorLUTPercent,
		Settings.PS1ColorBoostPercent,
		Settings.ChromaticAberrationPercent,
		Settings.ChromaticDistortionPercent,
		Settings.bInvertChromaticDistortion ? TEXT("true") : TEXT("false"),
		Settings.PS1FogPercent,
		Settings.PS1SceneDepthFogPercent,
		Settings.PS1FogDensityPercent,
		Settings.PS1FogStartDistancePercent,
		Settings.PS1FogFallOffDistancePercent,
		Settings.bUseRealLowResolution ? TEXT("true") : TEXT("false"),
		Settings.FakeResolutionSwitchSizePercent,
		Settings.FakeResolutionSwitchUVPercent,
		Settings.TargetResolutionHeightPercent);

	const FT66RetroFXSettings EffectiveSettings = BuildEffectiveSettings(Settings);

	EnsureBlendablesInWorld(TargetWorld);
	EnsurePs1PostProcessDMI(EffectiveSettings);
	EnsureBlendableEntry(ChromaticAberrationDMI);
	ApplyBlendableWeights(EffectiveSettings);
	ApplyPs1Parameters(EffectiveSettings);
	ApplyChromaticAberrationParameters(EffectiveSettings);
	ApplyN64Parameters(EffectiveSettings);
	ApplyResolutionCollection(EffectiveSettings, TargetWorld);
	ApplyResolutionRuntime(EffectiveSettings, TargetWorld);
	ApplyGeometryCollection(EffectiveSettings, TargetWorld);
	ApplyGeometryMaterials(EffectiveSettings, TargetWorld);

	if (UGameInstance* GI = TargetWorld->GetGameInstance())
	{
		if (UT66PixelationSubsystem* Pixelation = GI->GetSubsystem<UT66PixelationSubsystem>())
		{
			const int32 PixelationLevel = FMath::RoundToInt(PercentToRange(EffectiveSettings.T66PixelationPercent, 0.0f, 10.0f));
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

	if (!IsValid(ActiveVolume) || ActiveVolume->GetWorld() != World)
	{
		ActiveVolume = nullptr;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;
		APostProcessVolume* UseVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!UseVolume)
		{
			return;
		}
		UseVolume->bUnbound = true;
		UseVolume->BlendWeight = 1.0f;
		UseVolume->Priority = RetroPostProcessPriority;
		UseVolume->Settings.WeightedBlendables.Array.Reset();
#if WITH_EDITOR
		UseVolume->SetActorLabel(TEXT("DEV_RetroFX_PostProcessVolume"));
#endif
		ActiveVolume = UseVolume;
	}

	GetOrCreateDMI(LoadN64BlurMaterial(false), N64BlurDMI);
	GetOrCreateDMI(LoadN64BlurMaterial(true), N64BlurReplaceTonemapperDMI);
	GetOrCreateDMI(LoadChromaticAberrationMaterial(), ChromaticAberrationDMI);

	EnsureBlendableEntry(N64BlurDMI);
	EnsureBlendableEntry(N64BlurReplaceTonemapperDMI);
}

void UT66RetroFXSubsystem::EnsurePs1PostProcessDMI(const FT66RetroFXSettings& Settings)
{
	UMaterialInterface* DesiredMaterial = LoadPs1PostProcessMaterialVariant(Settings);
	const FString DesiredPath = DesiredMaterial ? DesiredMaterial->GetPathName() : FString();
	if (Ps1PostProcessDMI && ActivePs1MaterialPath != DesiredPath)
	{
		RemoveBlendableEntry(Ps1PostProcessDMI);
		Ps1PostProcessDMI = nullptr;
	}

	ActivePs1MaterialPath = DesiredPath;
	GetOrCreateDMI(DesiredMaterial, Ps1PostProcessDMI);
	EnsureBlendableEntry(Ps1PostProcessDMI);

	UE_LOG(
		LogT66RetroFXRuntime,
		Log,
		TEXT("EnsurePs1PostProcessDMI: material=%s colorLUT=%s sceneDepthFog=%s bayer=%s"),
		DesiredMaterial ? *DesiredMaterial->GetPathName() : TEXT("<null>"),
		PercentToSwitch(Settings.PS1ColorLUTPercent) > 0.5f ? TEXT("true") : TEXT("false"),
		PercentToSwitch(Settings.PS1SceneDepthFogPercent) > 0.5f ? TEXT("true") : TEXT("false"),
		PercentToSwitch(Settings.PS1BayerDitheringPercent) > 0.5f ? TEXT("true") : TEXT("false"));
}

void UT66RetroFXSubsystem::ApplyBlendableWeights(const FT66RetroFXSettings& Settings)
{
	const float Ps1Weight = PercentToUnit(Settings.PS1BlendPercent);
	const float ChromaticWeight = (ClampPercent(Settings.ChromaticAberrationPercent) > KINDA_SMALL_NUMBER
		|| ClampPercent(Settings.ChromaticDistortionPercent) > KINDA_SMALL_NUMBER)
		? 1.0f
		: 0.0f;
	SetBlendableWeight(Ps1PostProcessDMI, Ps1Weight);
	SetBlendableWeight(ChromaticAberrationDMI, ChromaticWeight);

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

	UE_LOG(LogT66RetroFXRuntime, Log, TEXT("ApplyBlendableWeights: PS1Weight=%.3f N64Weight=%.3f ChromaticWeight=%.3f ReplaceTonemapper=%s"),
		Ps1Weight,
		N64Weight,
		ChromaticWeight,
		Settings.bUseUE5RFXN64BlurReplaceTonemapper ? TEXT("true") : TEXT("false"));
}

void UT66RetroFXSubsystem::ApplyPs1Parameters(const FT66RetroFXSettings& Settings)
{
	if (!Ps1PostProcessDMI)
	{
		UE_LOG(LogT66RetroFXRuntime, Warning, TEXT("ApplyPs1Parameters: PS1 post-process DMI was null"));
		return;
	}

	const float DitheringStrength = PercentToRange(Settings.PS1DitheringPercent, 0.0f, 3.0f);
	const float FogSwitch = PercentToSwitch(Settings.PS1FogPercent);
	const float FogDensity = PercentToRange(Settings.PS1FogDensityPercent, 0.0f, 1.0f) * FogSwitch;
	const float FogStartDistance = PercentToInverseRange(Settings.PS1FogStartDistancePercent, 8000.0f, 0.0f);
	const float FogFallOffDistance = PercentToInverseRange(Settings.PS1FogFallOffDistancePercent, 10000.0f, 250.0f);
	const float ColorBoost = PercentToRange(Settings.PS1ColorBoostPercent, 1.0f, 4.0f);

	SetScalarParameter(Ps1PostProcessDMI, TEXT("Dithering Strength"), DitheringStrength);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("LUT Color Boost"), ColorBoost);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("FogDesnsity"), FogDensity);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("FogStartDistance"), FogStartDistance);
	SetScalarParameter(Ps1PostProcessDMI, TEXT("FogFallOffDistance"), FogFallOffDistance);

	UE_LOG(LogT66RetroFXRuntime, Log,
		TEXT("ApplyPs1Parameters: DitheringStrength=%.3f Bayer=%s ColorLUT=%s SceneDepthFog=%s ColorBoost=%.3f FogEnabled=%s FogDensity=%.3f FogStartDistance=%.3f FogFalloffDistance=%.3f"),
		DitheringStrength,
		PercentToSwitch(Settings.PS1BayerDitheringPercent) > 0.5f ? TEXT("true") : TEXT("false"),
		PercentToSwitch(Settings.PS1ColorLUTPercent) > 0.5f ? TEXT("true") : TEXT("false"),
		PercentToSwitch(Settings.PS1SceneDepthFogPercent) > 0.5f ? TEXT("true") : TEXT("false"),
		ColorBoost,
		FogSwitch > 0.5f ? TEXT("true") : TEXT("false"),
		FogDensity,
		FogStartDistance,
		FogFallOffDistance);
}

void UT66RetroFXSubsystem::ApplyChromaticAberrationParameters(const FT66RetroFXSettings& Settings)
{
	if (!ChromaticAberrationDMI)
	{
		UE_LOG(LogT66RetroFXRuntime, Warning, TEXT("ApplyChromaticAberrationParameters: chromatic aberration DMI was null"));
		return;
	}

	const float ChromaticStrength = PercentToRange(Settings.ChromaticAberrationPercent, 0.0f, 0.05f);
	float DistortionAmount = PercentToRange(Settings.ChromaticDistortionPercent, 0.0f, 0.20f);
	DistortionAmount *= Settings.bInvertChromaticDistortion ? 1.0f : -1.0f;

	SetScalarParameter(ChromaticAberrationDMI, TEXT("ChromaticStrength"), ChromaticStrength);
	SetScalarParameter(ChromaticAberrationDMI, TEXT("DistortionAmount"), DistortionAmount);

	UE_LOG(LogT66RetroFXRuntime, Log,
		TEXT("ApplyChromaticAberrationParameters: ChromaticStrength=%.4f DistortionAmount=%.4f Invert=%s"),
		ChromaticStrength,
		DistortionAmount,
		Settings.bInvertChromaticDistortion ? TEXT("true") : TEXT("false"));
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
		UE_LOG(LogT66RetroFXRuntime, Warning, TEXT("ApplyResolutionCollection: world was null"));
		return;
	}

	UMaterialParameterCollection* Collection = LoadResolutionCollection();
	if (!Collection)
	{
		UE_LOG(LogT66RetroFXRuntime, Warning, TEXT("ApplyResolutionCollection: resolution MPC was null"));
		return;
	}

	UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(Collection);
	if (!Instance)
	{
		UE_LOG(LogT66RetroFXRuntime, Warning, TEXT("ApplyResolutionCollection: resolution MPC instance was null"));
		return;
	}

	const float ResolutionSwitchSize = PercentToSwitch(Settings.FakeResolutionSwitchSizePercent);
	const float ResolutionSwitchUV = PercentToSwitch(Settings.FakeResolutionSwitchUVPercent);
	const float TargetResolutionHeight = GetTargetResolutionHeight(Settings.TargetResolutionHeightPercent);

	Instance->SetScalarParameterValue(TEXT("ResolutionSwitchSize"), ResolutionSwitchSize);
	Instance->SetScalarParameterValue(TEXT("ResolutionSwitchUV"), ResolutionSwitchUV);
	Instance->SetScalarParameterValue(TEXT("TargetResolutionHeight"), TargetResolutionHeight);

	UE_LOG(LogT66RetroFXRuntime, Log, TEXT("ApplyResolutionCollection: ResolutionSwitchSize=%.3f ResolutionSwitchUV=%.3f TargetResolutionHeight=%.3f"),
		ResolutionSwitchSize,
		ResolutionSwitchUV,
		TargetResolutionHeight);
}

void UT66RetroFXSubsystem::ApplyResolutionRuntime(const FT66RetroFXSettings& Settings, UWorld* World)
{
	CaptureResolutionRuntimeDefaults();

	if (!Settings.bUseRealLowResolution)
	{
		if (bResolutionRuntimeActive)
		{
			UE_LOG(LogT66RetroFXRuntime, Log, TEXT("ApplyResolutionRuntime: restoring original runtime resolution defaults"));
			RestoreResolutionRuntimeDefaults();
			bResolutionRuntimeActive = false;
		}
		else
		{
			UE_LOG(LogT66RetroFXRuntime, Log, TEXT("ApplyResolutionRuntime: real low resolution disabled; runtime override inactive"));
		}
		return;
	}

	const float ViewportHeight = FMath::Max(GetViewportHeight(World), 1.0f);
	const float TargetHeight = GetTargetResolutionHeight(Settings.TargetResolutionHeightPercent);
	const float ScreenPercentage = FMath::Clamp((TargetHeight / ViewportHeight) * 100.0f, 5.0f, 100.0f);

	ExecuteConsoleCommand(World, FString::Printf(TEXT("r.ScreenPercentage %.2f"), ScreenPercentage));
	bResolutionRuntimeActive = true;
	UE_LOG(LogT66RetroFXRuntime, Log, TEXT("ApplyResolutionRuntime: enabled real low resolution ViewportHeight=%.2f TargetHeight=%.2f ScreenPercentage=%.2f"),
		ViewportHeight,
		TargetHeight,
		ScreenPercentage);
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

	static constexpr float SafeDisabledSnapStrength = 0.001f;
	static constexpr float DefaultSnapTargetResolution = 600.0f;
	static constexpr float DefaultAffineDistance1 = 400.0f;
	static constexpr float DefaultAffineDistance2 = 1200.0f;
	static constexpr float DefaultAffineDistance3 = 1800.0f;

	const bool bEnableWorldGeometry = HasWorldGeometryEnabled(Settings);
	const bool bEnableCharacterGeometry = HasCharacterGeometryEnabled(Settings);

	const float WorldNoiseAmount = bEnableWorldGeometry
		? PercentToRange(Settings.WorldVertexNoisePercent, 0.0f, 8.0f)
		: 0.0f;
	const float CharacterNoiseAmount = bEnableCharacterGeometry
		? PercentToRange(Settings.CharacterVertexNoisePercent, 0.0f, 8.0f)
		: 0.0f;
	const float WorldVertexSnapStrength = bEnableWorldGeometry
		? FMath::Max(PercentToRange(Settings.WorldVertexSnapPercent, 0.0f, 4.0f), SafeDisabledSnapStrength)
		: SafeDisabledSnapStrength;
	const float CharacterVertexSnapStrength = bEnableCharacterGeometry
		? FMath::Max(PercentToRange(Settings.CharacterVertexSnapPercent, 0.0f, 4.0f), SafeDisabledSnapStrength)
		: SafeDisabledSnapStrength;
	const float WorldAffineBlend = bEnableWorldGeometry
		? PercentToUnit(Settings.WorldAffineBlendPercent)
		: 0.0f;
	const float CharacterAffineBlend = bEnableCharacterGeometry
		? PercentToUnit(Settings.CharacterAffineBlendPercent)
		: 0.0f;
	const float WorldSnapTargetResolution = bEnableWorldGeometry
		? PercentToInverseRange(Settings.WorldVertexSnapResolutionPercent, 1080.0f, 120.0f)
		: DefaultSnapTargetResolution;
	const float CharacterSnapTargetResolution = bEnableCharacterGeometry
		? PercentToInverseRange(Settings.CharacterVertexSnapResolutionPercent, 1080.0f, 120.0f)
		: DefaultSnapTargetResolution;
	const float WorldAffineDistance1 = bEnableWorldGeometry
		? PercentToRange(Settings.WorldAffineDistance1Percent, 100.0f, 700.0f)
		: DefaultAffineDistance1;
	const float WorldAffineDistance2 = bEnableWorldGeometry
		? PercentToRange(Settings.WorldAffineDistance2Percent, 300.0f, 2100.0f)
		: DefaultAffineDistance2;
	const float WorldAffineDistance3 = bEnableWorldGeometry
		? PercentToRange(Settings.WorldAffineDistance3Percent, 600.0f, 3000.0f)
		: DefaultAffineDistance3;
	const float CharacterAffineDistance1 = bEnableCharacterGeometry
		? PercentToRange(Settings.CharacterAffineDistance1Percent, 100.0f, 700.0f)
		: DefaultAffineDistance1;
	const float CharacterAffineDistance2 = bEnableCharacterGeometry
		? PercentToRange(Settings.CharacterAffineDistance2Percent, 300.0f, 2100.0f)
		: DefaultAffineDistance2;
	const float CharacterAffineDistance3 = bEnableCharacterGeometry
		? PercentToRange(Settings.CharacterAffineDistance3Percent, 600.0f, 3000.0f)
		: DefaultAffineDistance3;

	Instance->SetScalarParameterValue(TEXT("WorldGeometryEnable"), bEnableWorldGeometry ? 1.0f : 0.0f);
	Instance->SetScalarParameterValue(TEXT("WorldVertexSnapStrength"), WorldVertexSnapStrength);
	Instance->SetScalarParameterValue(TEXT("WorldVertexSnapTargetResolution"), WorldSnapTargetResolution);
	Instance->SetVectorParameterValue(TEXT("WorldVertexNoiseOffset"), FLinearColor(WorldNoiseAmount, WorldNoiseAmount, WorldNoiseAmount, 0.0f));
	Instance->SetScalarParameterValue(TEXT("WorldAffineBlend"), WorldAffineBlend);
	Instance->SetScalarParameterValue(TEXT("WorldAffineDistance1"), WorldAffineDistance1);
	Instance->SetScalarParameterValue(TEXT("WorldAffineDistance2"), WorldAffineDistance2);
	Instance->SetScalarParameterValue(TEXT("WorldAffineDistance3"), WorldAffineDistance3);

	Instance->SetScalarParameterValue(TEXT("CharacterGeometryEnable"), bEnableCharacterGeometry ? 1.0f : 0.0f);
	Instance->SetScalarParameterValue(TEXT("CharacterVertexSnapStrength"), CharacterVertexSnapStrength);
	Instance->SetScalarParameterValue(TEXT("CharacterVertexSnapTargetResolution"), CharacterSnapTargetResolution);
	Instance->SetVectorParameterValue(TEXT("CharacterVertexNoiseOffset"), FLinearColor(CharacterNoiseAmount, CharacterNoiseAmount, CharacterNoiseAmount, 0.0f));
	Instance->SetScalarParameterValue(TEXT("CharacterAffineBlend"), CharacterAffineBlend);
	Instance->SetScalarParameterValue(TEXT("CharacterAffineDistance1"), CharacterAffineDistance1);
	Instance->SetScalarParameterValue(TEXT("CharacterAffineDistance2"), CharacterAffineDistance2);
	Instance->SetScalarParameterValue(TEXT("CharacterAffineDistance3"), CharacterAffineDistance3);

	UE_LOG(LogT66RetroFXRuntime, Log,
		TEXT("ApplyGeometryCollection: WorldEnabled=%s Snap=%.3f SnapRes=%.3f Noise=%.3f Affine=%.3f CharacterEnabled=%s Snap=%.3f SnapRes=%.3f Noise=%.3f Affine=%.3f"),
		bEnableWorldGeometry ? TEXT("true") : TEXT("false"),
		WorldVertexSnapStrength,
		WorldSnapTargetResolution,
		WorldNoiseAmount,
		WorldAffineBlend,
		bEnableCharacterGeometry ? TEXT("true") : TEXT("false"),
		CharacterVertexSnapStrength,
		CharacterSnapTargetResolution,
		CharacterNoiseAmount,
		CharacterAffineBlend);
}

void UT66RetroFXSubsystem::ApplyGeometryMaterials(const FT66RetroFXSettings& Settings, UWorld* World)
{
	const bool bEnableWorldGeometry = HasWorldGeometryEnabled(Settings);
	const bool bEnableCharacterGeometry = HasCharacterGeometryEnabled(Settings);

	bWorldGeometryActive = bEnableWorldGeometry;
	bCharacterGeometryActive = bEnableCharacterGeometry;
	UpdateGeometrySpawnBinding(World, bEnableWorldGeometry || bEnableCharacterGeometry);

	if (!bEnableWorldGeometry && !bEnableCharacterGeometry)
	{
		RestoreManagedMaterials(true, true);
		CleanupManagedSlots();
		return;
	}

	RefreshWorldGeometryMaterials(World, bEnableWorldGeometry, bEnableCharacterGeometry);
	RestoreManagedMaterials(!bEnableWorldGeometry, !bEnableCharacterGeometry);
	CleanupManagedSlots();

	UE_LOG(LogT66RetroFXRuntime, Log,
		TEXT("ApplyGeometryMaterials: runtime swapping enabled World=%s Character=%s ManagedSlots=%d"),
		bEnableWorldGeometry ? TEXT("true") : TEXT("false"),
		bEnableCharacterGeometry ? TEXT("true") : TEXT("false"),
		ManagedGeometrySlots.Num());
}

void UT66RetroFXSubsystem::RefreshWorldGeometryMaterials(UWorld* World, bool bEnableWorldGeometry, bool bEnableCharacterGeometry)
{
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		RefreshActorGeometryMaterials(*It, bEnableWorldGeometry, bEnableCharacterGeometry);
	}
}

void UT66RetroFXSubsystem::RefreshActorGeometryMaterials(AActor* Actor, bool bEnableWorldGeometry, bool bEnableCharacterGeometry)
{
	if (!Actor)
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;
	Actor->GetComponents<UMeshComponent>(MeshComponents);
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		RefreshMeshComponentGeometryMaterials(MeshComponent, bEnableWorldGeometry, bEnableCharacterGeometry);
	}
}

void UT66RetroFXSubsystem::RefreshMeshComponentGeometryMaterials(UMeshComponent* MeshComponent, bool bEnableWorldGeometry, bool bEnableCharacterGeometry)
{
	if (!MeshComponent)
	{
		return;
	}

	for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
	{
		UMaterialInterface* CurrentMaterial = MeshComponent->GetMaterial(MaterialIndex);
		const int32 ManagedIndex = FindManagedSlotIndex(MeshComponent, MaterialIndex);
		if (ManagedIndex != INDEX_NONE)
		{
			FT66RetroManagedMaterialSlot& Slot = ManagedGeometrySlots[ManagedIndex];
			if (CurrentMaterial == Slot.RetroMaterial)
			{
				if (!IsGroupEnabled(Slot.Group, bEnableWorldGeometry, bEnableCharacterGeometry))
				{
					RestoreManagedSlot(ManagedIndex);
				}
				continue;
			}

			Slot.OriginalMaterial = CurrentMaterial;
			Slot.RetroMaterial = nullptr;
		}

		ET66RetroGeometryGroup Group = ET66RetroGeometryGroup::World;
		UMaterialInterface* RetroBaseMaterial = ResolveRetroGeometryMaterial(CurrentMaterial, Group);
		if (!RetroBaseMaterial || !IsGroupEnabled(Group, bEnableWorldGeometry, bEnableCharacterGeometry))
		{
			continue;
		}

		UMaterialInstanceDynamic* RetroMID = UMaterialInstanceDynamic::Create(RetroBaseMaterial, this);
		if (!RetroMID)
		{
			continue;
		}

		CopyRetroSourceParameters(CurrentMaterial, RetroMID);
		MeshComponent->SetMaterial(MaterialIndex, RetroMID);

		if (ManagedIndex != INDEX_NONE)
		{
			FT66RetroManagedMaterialSlot& Slot = ManagedGeometrySlots[ManagedIndex];
			Slot.Group = Group;
			Slot.RetroMaterial = RetroMID;
			Slot.OriginalMaterial = CurrentMaterial;
		}
		else
		{
			FT66RetroManagedMaterialSlot& Slot = ManagedGeometrySlots.AddDefaulted_GetRef();
			Slot.MeshComponent = MeshComponent;
			Slot.MaterialIndex = MaterialIndex;
			Slot.Group = Group;
			Slot.OriginalMaterial = CurrentMaterial;
			Slot.RetroMaterial = RetroMID;
		}
	}
}

void UT66RetroFXSubsystem::RestoreManagedMaterials(bool bRestoreWorldGeometry, bool bRestoreCharacterGeometry)
{
	if (!bRestoreWorldGeometry && !bRestoreCharacterGeometry)
	{
		return;
	}

	for (int32 Index = ManagedGeometrySlots.Num() - 1; Index >= 0; --Index)
	{
		const ET66RetroGeometryGroup Group = ManagedGeometrySlots[Index].Group;
		if ((Group == ET66RetroGeometryGroup::World && bRestoreWorldGeometry)
			|| (Group == ET66RetroGeometryGroup::Character && bRestoreCharacterGeometry))
		{
			RestoreManagedSlot(Index);
		}
	}
}

void UT66RetroFXSubsystem::RestoreManagedSlot(int32 SlotIndex)
{
	if (!ManagedGeometrySlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	const FT66RetroManagedMaterialSlot Slot = ManagedGeometrySlots[SlotIndex];
	if (UMeshComponent* MeshComponent = Slot.MeshComponent.Get())
	{
		if (Slot.MaterialIndex >= 0
			&& Slot.MaterialIndex < MeshComponent->GetNumMaterials()
			&& MeshComponent->GetMaterial(Slot.MaterialIndex) == Slot.RetroMaterial
			&& Slot.OriginalMaterial)
		{
			MeshComponent->SetMaterial(Slot.MaterialIndex, Slot.OriginalMaterial);
		}
	}

	ManagedGeometrySlots.RemoveAtSwap(SlotIndex);
}

void UT66RetroFXSubsystem::CleanupManagedSlots()
{
	for (int32 Index = ManagedGeometrySlots.Num() - 1; Index >= 0; --Index)
	{
		const FT66RetroManagedMaterialSlot& Slot = ManagedGeometrySlots[Index];
		if (!Slot.MeshComponent.IsValid() || !Slot.OriginalMaterial || !Slot.RetroMaterial)
		{
			ManagedGeometrySlots.RemoveAtSwap(Index);
		}
	}
}

int32 UT66RetroFXSubsystem::FindManagedSlotIndex(const UMeshComponent* MeshComponent, int32 MaterialIndex) const
{
	for (int32 Index = 0; Index < ManagedGeometrySlots.Num(); ++Index)
	{
		const FT66RetroManagedMaterialSlot& Slot = ManagedGeometrySlots[Index];
		if (Slot.MeshComponent.Get() == MeshComponent && Slot.MaterialIndex == MaterialIndex)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

void UT66RetroFXSubsystem::UpdateGeometrySpawnBinding(UWorld* World, bool bShouldListen)
{
	if (ManagedGeometryWorld && ManagedGeometryWorld != World)
	{
		if (GeometrySpawnHandle.IsValid())
		{
			ManagedGeometryWorld->RemoveOnActorSpawnedHandler(GeometrySpawnHandle);
			GeometrySpawnHandle.Reset();
		}

		RestoreManagedMaterials(true, true);
		ManagedGeometrySlots.Reset();
		ManagedGeometryWorld = nullptr;
	}

	if (!bShouldListen)
	{
		if (ManagedGeometryWorld && GeometrySpawnHandle.IsValid())
		{
			ManagedGeometryWorld->RemoveOnActorSpawnedHandler(GeometrySpawnHandle);
			GeometrySpawnHandle.Reset();
		}
		ManagedGeometryWorld = nullptr;
		return;
	}

	if (!World)
	{
		return;
	}

	if (ManagedGeometryWorld != World)
	{
		ManagedGeometryWorld = World;
	}

	if (!GeometrySpawnHandle.IsValid())
	{
		GeometrySpawnHandle = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &UT66RetroFXSubsystem::HandleActorSpawned));
	}
}

void UT66RetroFXSubsystem::HandleActorSpawned(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	RefreshActorGeometryMaterials(Actor, bWorldGeometryActive, bCharacterGeometryActive);
}

UMaterialInterface* UT66RetroFXSubsystem::LoadPs1PostProcessMaterial()
{
	return LoadFirstAvailableMaterial(Ps1CandidatePaths, UE_ARRAY_COUNT(Ps1CandidatePaths));
}

UMaterialInterface* UT66RetroFXSubsystem::LoadPs1PostProcessMaterialVariant(const FT66RetroFXSettings& Settings)
{
	const uint8 VariantMask = BuildPs1VariantMask(Settings);
	const FString VariantPath = GetPs1VariantMaterialPath(VariantMask);
	if (UMaterialInterface* VariantMaterial = LoadObject<UMaterialInterface>(nullptr, *VariantPath))
	{
		return VariantMaterial;
	}

	return LoadPs1PostProcessMaterial();
}

UMaterialInterface* UT66RetroFXSubsystem::LoadN64BlurMaterial(bool bReplaceTonemapper)
{
	return LoadObject<UMaterialInterface>(nullptr, bReplaceTonemapper ? N64BlurReplaceTonemapperPath : N64BlurPath);
}

UMaterialInterface* UT66RetroFXSubsystem::LoadCharacterRetroGeometryMaterial()
{
	if (!CharacterRetroGeometryMaterial)
	{
		CharacterRetroGeometryMaterial = LoadObject<UMaterialInterface>(nullptr, CharacterRetroGeometryMaterialPath);
	}
	return CharacterRetroGeometryMaterial;
}

UMaterialInterface* UT66RetroFXSubsystem::LoadEnvironmentRetroGeometryMaterial()
{
	if (!EnvironmentRetroGeometryMaterial)
	{
		EnvironmentRetroGeometryMaterial = LoadObject<UMaterialInterface>(nullptr, EnvironmentRetroGeometryMaterialPath);
	}
	return EnvironmentRetroGeometryMaterial;
}

UMaterialInterface* UT66RetroFXSubsystem::LoadFbxRetroGeometryMaterial()
{
	if (!FbxRetroGeometryMaterial)
	{
		FbxRetroGeometryMaterial = LoadObject<UMaterialInterface>(nullptr, FbxRetroGeometryMaterialPath);
	}
	return FbxRetroGeometryMaterial;
}

UMaterialInterface* UT66RetroFXSubsystem::LoadGlbRetroGeometryMaterial()
{
	if (!GlbRetroGeometryMaterial)
	{
		GlbRetroGeometryMaterial = LoadObject<UMaterialInterface>(nullptr, GlbRetroGeometryMaterialPath);
	}
	return GlbRetroGeometryMaterial;
}

UMaterialInterface* UT66RetroFXSubsystem::LoadChromaticAberrationMaterial()
{
	return LoadObject<UMaterialInterface>(nullptr, ChromaticAberrationMaterialPath);
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

UMaterialInterface* UT66RetroFXSubsystem::ResolveRetroGeometryMaterial(UMaterialInterface* SourceMaterial, ET66RetroGeometryGroup& OutGroup)
{
	OutGroup = ET66RetroGeometryGroup::World;
	if (!SourceMaterial)
	{
		return nullptr;
	}

	const FString BasePath = GetMaterialBasePath(SourceMaterial);
	if (BasePath.Equals(CharacterBaseMaterialPath))
	{
		OutGroup = ET66RetroGeometryGroup::Character;
		return LoadCharacterRetroGeometryMaterial();
	}
	if (BasePath.Equals(FbxBaseMaterialPath))
	{
		OutGroup = ET66RetroGeometryGroup::Character;
		return LoadFbxRetroGeometryMaterial();
	}
	if (BasePath.Equals(EnvironmentBaseMaterialPath))
	{
		OutGroup = ET66RetroGeometryGroup::World;
		return LoadEnvironmentRetroGeometryMaterial();
	}
	if (BasePath.Equals(GlbBaseMaterialPath))
	{
		OutGroup = ET66RetroGeometryGroup::World;
		return LoadGlbRetroGeometryMaterial();
	}

	return nullptr;
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

void UT66RetroFXSubsystem::RemoveBlendableEntry(UObject* BlendableObject)
{
	if (!ActiveVolume || !BlendableObject)
	{
		return;
	}

	FPostProcessSettings& PPS = ActiveVolume->Settings;
	for (int32 Index = PPS.WeightedBlendables.Array.Num() - 1; Index >= 0; --Index)
	{
		if (PPS.WeightedBlendables.Array[Index].Object == BlendableObject)
		{
			PPS.WeightedBlendables.Array.RemoveAt(Index);
		}
	}
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

void UT66RetroFXSubsystem::CaptureResolutionRuntimeDefaults()
{
	if (bResolutionRuntimeDefaultsCaptured)
	{
		return;
	}

	if (IConsoleVariable* ScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
	{
		OriginalScreenPercentage = ScreenPercentage->GetFloat();
	}
	if (IConsoleVariable* SecondaryScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SecondaryScreenPercentage.GameViewport")))
	{
		OriginalSecondaryScreenPercentage = SecondaryScreenPercentage->GetFloat();
	}
	if (IConsoleVariable* UpscaleQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Upscale.Quality")))
	{
		OriginalUpscaleQuality = UpscaleQuality->GetInt();
	}

	bResolutionRuntimeDefaultsCaptured = true;
}

void UT66RetroFXSubsystem::RestoreResolutionRuntimeDefaults()
{
	if (!bResolutionRuntimeDefaultsCaptured)
	{
		return;
	}

	UWorld* World = ResolveWorld(nullptr);
	ExecuteConsoleCommand(World, FString::Printf(TEXT("r.Upscale.Quality %d"), OriginalUpscaleQuality));
	ExecuteConsoleCommand(World, FString::Printf(TEXT("r.ScreenPercentage %.2f"), OriginalScreenPercentage));
	ExecuteConsoleCommand(World, FString::Printf(TEXT("r.SecondaryScreenPercentage.GameViewport %.2f"), OriginalSecondaryScreenPercentage));
	bResolutionRuntimeActive = false;
}

float UT66RetroFXSubsystem::GetViewportHeight(UWorld* World) const
{
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize(0.0f, 0.0f);
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		if (ViewportSize.Y > KINDA_SMALL_NUMBER)
		{
			return ViewportSize.Y;
		}

		if (GEngine->GameViewport->Viewport)
		{
			return static_cast<float>(GEngine->GameViewport->Viewport->GetSizeXY().Y);
		}
	}

	if (World)
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			FVector2D ViewportSize(0.0f, 0.0f);
			ViewportClient->GetViewportSize(ViewportSize);
			if (ViewportSize.Y > KINDA_SMALL_NUMBER)
			{
				return ViewportSize.Y;
			}
		}
	}

	return 1080.0f;
}

void UT66RetroFXSubsystem::ExecuteConsoleCommand(UWorld* World, const FString& Command) const
{
	if (Command.IsEmpty())
	{
		return;
	}

	if (World)
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->ConsoleCommand(Command, true);
			return;
		}
	}

	if (GEngine)
	{
		GEngine->Exec(World, *Command);
	}
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
