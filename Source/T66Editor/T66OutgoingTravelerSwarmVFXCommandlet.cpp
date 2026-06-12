// Copyright Tribulation 66. All Rights Reserved.

#include "T66OutgoingTravelerSwarmVFXCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/T66OutgoingTravelerPoolSubsystem.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionParticleColor.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/PackageName.h"
#include "NiagaraCommon.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraScript.h"
#include "NiagaraMeshRendererMeshProperties.h"
#include "NiagaraMeshRendererProperties.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraSystem.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshOperations.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#if WITH_EDITOR
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_Niagara.h"
#include "NiagaraConstants.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceArrayFloat.h"
#include "NiagaraDataInterfaceArrayInt.h"
#include "NiagaraEditorUtilities.h"
#include "NiagaraEmitterFactoryNew.h"
#include "NiagaraGraph.h"
#include "NiagaraNodeCustomHlsl.h"
#include "NiagaraNodeInput.h"
#include "NiagaraNodeOutput.h"
#include "NiagaraParameterMapHistory.h"
#include "NiagaraSystemFactoryNew.h"
#include "ViewModels/Stack/NiagaraParameterHandle.h"
#include "ViewModels/Stack/NiagaraStackGraphUtilities.h"
#include "UObject/UnrealType.h"
#endif

namespace
{
	constexpr int32 T66OutgoingTravelerProofCount = 5000;
	constexpr int32 T66OutgoingTravelerProductionCount = 20000;
	const TCHAR* T66OutgoingTravelerSystemPackagePath =
		TEXT("/Game/VFXLab/Foundation/OutgoingTravelers/NS_OutgoingTravelerSwarmProof");
	const TCHAR* T66OutgoingTravelerSystemObjectName =
		TEXT("NS_OutgoingTravelerSwarmProof");
	const TCHAR* T66OutgoingTravelerSystemObjectPath =
		TEXT("/Game/VFXLab/Foundation/OutgoingTravelers/NS_OutgoingTravelerSwarmProof.NS_OutgoingTravelerSwarmProof");
	const TCHAR* T66OutgoingTravelerEmitterName =
		TEXT("Emitter_OutgoingTravelerArrayProof");
	const TCHAR* T66OutgoingTravelerArrayParameter =
		TEXT("User.TravelerPositions");
	const TCHAR* T66OutgoingTravelerProductionSystemPackagePath =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/NS_OutgoingTravelerPool");
	const TCHAR* T66OutgoingTravelerProductionSystemObjectName =
		TEXT("NS_OutgoingTravelerPool");
	const TCHAR* T66OutgoingTravelerProductionSystemObjectPath =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/NS_OutgoingTravelerPool.NS_OutgoingTravelerPool");
	const TCHAR* T66OutgoingTravelerProductionEmitterName =
		TEXT("Emitter_OutgoingTravelerPool");
	const TCHAR* T66OutgoingTravelerRotationsParameter =
		TEXT("User.TravelerRotations");
	const TCHAR* T66OutgoingTravelerScalesParameter =
		TEXT("User.TravelerScales");
	const TCHAR* T66OutgoingTravelerColorsParameter =
		TEXT("User.TravelerColors");
	const TCHAR* T66OutgoingTravelerMeshIndicesParameter =
		TEXT("User.TravelerMeshIndices");
	const TCHAR* T66OutgoingTravelerLiveCountParameter =
		TEXT("User.TravelerLiveCount");
	const TCHAR* T66OutgoingTravelerAdditiveMaterialPackagePath =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/M_OutgoingTravelerPool_AdditiveFamily");
	const TCHAR* T66OutgoingTravelerAdditiveMaterialObjectName =
		TEXT("M_OutgoingTravelerPool_AdditiveFamily");
	const TCHAR* T66OutgoingTravelerAdditiveMaterialObjectPath =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/M_OutgoingTravelerPool_AdditiveFamily.M_OutgoingTravelerPool_AdditiveFamily");
	const TCHAR* T66OutgoingTravelerTranslucentMaterialPackagePath =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/M_OutgoingTravelerPool_TranslucentFamily");
	const TCHAR* T66OutgoingTravelerTranslucentMaterialObjectName =
		TEXT("M_OutgoingTravelerPool_TranslucentFamily");
	const TCHAR* T66OutgoingTravelerTranslucentMaterialObjectPath =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/M_OutgoingTravelerPool_TranslucentFamily.M_OutgoingTravelerPool_TranslucentFamily");
	const TCHAR* T66OutgoingTravelerProfileMeshRoot =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/Profiles/Meshes");
	const TCHAR* T66OutgoingTravelerProfileMaterialRoot =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/Profiles/Materials");
	const TCHAR* T66OutgoingTravelerProfileTextureRoot =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/Profiles/Textures");
	const FName T66TravelerTextureParameter(TEXT("TravelerTexture"));
	const FName T66TravelerEmissiveBoostParameter(TEXT("EmissiveBoost"));
	const FName T66TravelerOpacityScaleParameter(TEXT("OpacityScale"));
	constexpr int32 T66MobLootProductionCount = 20000;
	const TCHAR* T66MobLootSystemPackagePath =
		TEXT("/Game/VFX/Foundation/MobLoot/NS_MobLootPool");
	const TCHAR* T66MobLootSystemObjectName =
		TEXT("NS_MobLootPool");
	const TCHAR* T66MobLootSystemObjectPath =
		TEXT("/Game/VFX/Foundation/MobLoot/NS_MobLootPool.NS_MobLootPool");
	const TCHAR* T66MobLootEmitterName =
		TEXT("Emitter_MobLootPool");
	const TCHAR* T66MobLootPositionsParameter =
		TEXT("User.MobLootPositions");
	const TCHAR* T66MobLootScalesParameter =
		TEXT("User.MobLootScales");
	const TCHAR* T66MobLootColorsParameter =
		TEXT("User.MobLootColors");
	const TCHAR* T66MobLootQuantitiesParameter =
		TEXT("User.MobLootQuantities");
	const TCHAR* T66MobLootLiveCountParameter =
		TEXT("User.MobLootLiveCount");

	struct FT66TravelerVisualProfileAuthoringSpec
	{
		const TCHAR* ProfileID = TEXT("");
		const TCHAR* AssetSuffix = TEXT("");
		int32 MeshIndex = 0;
		int32 FamilyTag = 1;
		int32 ElementIndex = 0;
		int32 DeliveryIndex = 0;
		FLinearColor Tint = FLinearColor::White;
		float MeshLength = 120.0f;
		float MeshWidth = 48.0f;
		float MeshRipple = 0.0f;
	};

	struct FT66TravelerMeshSlotDefinition
	{
		FString MeshPath;
		FVector Scale = FVector::OneVector;
		FRotator Rotation = FRotator::ZeroRotator;
		int32 MaterialFamilyTag = 1;
	};

	const FT66TravelerVisualProfileAuthoringSpec T66TravelerVisualProfiles[] =
	{
		{ TEXT("TravelerVisual.Fire.AOE"),         TEXT("Fire_AOE"),         4,  1, 0, 0, FLinearColor(1.00f, 0.18f, 0.03f, 1.0f), 130.0f,  86.0f, 0.32f },
		{ TEXT("TravelerVisual.Fire.Summon"),      TEXT("Fire_Summon"),      5,  1, 0, 1, FLinearColor(1.00f, 0.34f, 0.05f, 1.0f), 176.0f,  34.0f, 0.18f },
		{ TEXT("TravelerVisual.Fire.Bounce"),      TEXT("Fire_Bounce"),      6,  1, 0, 2, FLinearColor(1.00f, 0.24f, 0.02f, 1.0f), 112.0f,  66.0f, 0.40f },
		{ TEXT("TravelerVisual.Fire.DOT"),         TEXT("Fire_DOT"),         7,  1, 0, 3, FLinearColor(1.00f, 0.42f, 0.08f, 1.0f), 104.0f,  72.0f, 0.55f },
		{ TEXT("TravelerVisual.Ice.AOE"),          TEXT("Ice_AOE"),          8,  2, 1, 0, FLinearColor(0.36f, 0.88f, 1.00f, 0.82f), 126.0f,  78.0f, 0.26f },
		{ TEXT("TravelerVisual.Ice.Summon"),       TEXT("Ice_Summon"),       9,  2, 1, 1, FLinearColor(0.58f, 0.96f, 1.00f, 0.84f), 170.0f,  28.0f, 0.12f },
		{ TEXT("TravelerVisual.Ice.Bounce"),       TEXT("Ice_Bounce"),       10, 2, 1, 2, FLinearColor(0.48f, 0.90f, 1.00f, 0.78f), 104.0f,  62.0f, 0.22f },
		{ TEXT("TravelerVisual.Ice.DOT"),          TEXT("Ice_DOT"),          11, 2, 1, 3, FLinearColor(0.50f, 0.86f, 1.00f, 0.74f), 112.0f,  70.0f, 0.34f },
		{ TEXT("TravelerVisual.Electricity.AOE"),  TEXT("Electricity_AOE"),  12, 1, 2, 0, FLinearColor(0.78f, 0.95f, 1.00f, 1.0f), 142.0f,  58.0f, 0.62f },
		{ TEXT("TravelerVisual.Electricity.Summon"), TEXT("Electricity_Summon"), 13, 1, 2, 1, FLinearColor(0.58f, 0.86f, 1.00f, 1.0f), 190.0f,  24.0f, 0.46f },
		{ TEXT("TravelerVisual.Electricity.Bounce"), TEXT("Electricity_Bounce"), 14, 1, 2, 2, FLinearColor(0.68f, 0.92f, 1.00f, 1.0f), 120.0f,  46.0f, 0.72f },
		{ TEXT("TravelerVisual.Electricity.DOT"),  TEXT("Electricity_DOT"),  15, 1, 2, 3, FLinearColor(0.70f, 0.90f, 1.00f, 1.0f), 116.0f,  54.0f, 0.70f },
		{ TEXT("TravelerVisual.Nature.AOE"),       TEXT("Nature_AOE"),       16, 2, 3, 0, FLinearColor(0.42f, 0.95f, 0.34f, 0.78f), 132.0f,  82.0f, 0.30f },
		{ TEXT("TravelerVisual.Nature.Summon"),    TEXT("Nature_Summon"),    17, 2, 3, 1, FLinearColor(0.58f, 1.00f, 0.42f, 0.78f), 164.0f,  32.0f, 0.22f },
		{ TEXT("TravelerVisual.Nature.Bounce"),    TEXT("Nature_Bounce"),    18, 2, 3, 2, FLinearColor(0.32f, 0.86f, 0.26f, 0.76f), 110.0f,  70.0f, 0.48f },
		{ TEXT("TravelerVisual.Nature.DOT"),       TEXT("Nature_DOT"),       19, 2, 3, 3, FLinearColor(0.44f, 0.92f, 0.30f, 0.72f), 112.0f,  76.0f, 0.64f }
	};

	bool T66SaveAsset(UObject* Asset)
	{
		if (!Asset)
		{
			return false;
		}

		UPackage* Package = Asset->GetPackage();
		if (!Package)
		{
			return false;
		}

		Package->MarkPackageDirty();

		const FString PackageName = Package->GetName();
		const FString Filename = FPackageName::LongPackageNameToFilename(
			PackageName,
			FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(Package, Asset, *Filename, SaveArgs);
	}

#if WITH_EDITOR
	template<typename TExpression>
	TExpression* T66AddMaterialExpression(UMaterial& Material, const int32 X, const int32 Y)
	{
		auto* EditorData = Material.GetEditorOnlyData();
		if (!EditorData)
		{
			return nullptr;
		}

		TExpression* Expression = NewObject<TExpression>(&Material);
		if (!Expression)
		{
			return nullptr;
		}

		Expression->MaterialExpressionEditorX = X;
		Expression->MaterialExpressionEditorY = Y;
		EditorData->ExpressionCollection.Expressions.Add(Expression);
		return Expression;
	}

	UMaterial* T66CreateTravelerFamilyMaterial(
		const TCHAR* PackagePath,
		const TCHAR* ObjectName,
		const EBlendMode BlendMode,
		const float DefaultBoost,
		const float DefaultOpacity)
	{
		UPackage* Package = CreatePackage(PackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		if (UMaterial* ExistingMaterial = FindObject<UMaterial>(Package, ObjectName))
		{
			ExistingMaterial->ClearFlags(RF_Public | RF_Standalone);
			ExistingMaterial->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}

		UMaterial* Material = NewObject<UMaterial>(
			Package,
			FName(ObjectName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!Material)
		{
			return nullptr;
		}

		Material->Modify();
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BlendMode;
		Material->SetShadingModel(MSM_Unlit);
		Material->TwoSided = true;
		Material->bUsedWithNiagaraMeshParticles = true;

		UMaterialExpressionParticleColor* ParticleColor =
			T66AddMaterialExpression<UMaterialExpressionParticleColor>(*Material, -900, -140);
		UMaterialExpressionComponentMask* ParticleRgb =
			T66AddMaterialExpression<UMaterialExpressionComponentMask>(*Material, -680, -140);
		UMaterialExpressionTextureSampleParameter2D* TravelerTexture =
			T66AddMaterialExpression<UMaterialExpressionTextureSampleParameter2D>(*Material, -900, -360);
		UMaterialExpressionScalarParameter* EmissiveBoost =
			T66AddMaterialExpression<UMaterialExpressionScalarParameter>(*Material, -900, 100);
		UMaterialExpressionScalarParameter* OpacityScale =
			T66AddMaterialExpression<UMaterialExpressionScalarParameter>(*Material, -900, 300);
		UMaterialExpressionMultiply* TextureColorMultiply =
			T66AddMaterialExpression<UMaterialExpressionMultiply>(*Material, -520, -300);
		UMaterialExpressionMultiply* BoostMultiply =
			T66AddMaterialExpression<UMaterialExpressionMultiply>(*Material, -520, -80);
		UMaterialExpressionMultiply* OpacityMultiply =
			T66AddMaterialExpression<UMaterialExpressionMultiply>(*Material, -260, 220);

		if (!ParticleColor || !ParticleRgb || !TravelerTexture || !EmissiveBoost
			|| !OpacityScale || !TextureColorMultiply || !BoostMultiply || !OpacityMultiply)
		{
			return nullptr;
		}

		TravelerTexture->ParameterName = T66TravelerTextureParameter;
		TravelerTexture->Texture = LoadObject<UTexture2D>(
			nullptr,
			TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
		TravelerTexture->SamplerType = SAMPLERTYPE_Color;

		EmissiveBoost->ParameterName = T66TravelerEmissiveBoostParameter;
		EmissiveBoost->DefaultValue = DefaultBoost;
		OpacityScale->ParameterName = T66TravelerOpacityScaleParameter;
		OpacityScale->DefaultValue = DefaultOpacity;

		ParticleRgb->Input.Connect(0, ParticleColor);
		ParticleRgb->R = 1;
		ParticleRgb->G = 1;
		ParticleRgb->B = 1;

		TextureColorMultiply->A.Connect(0, TravelerTexture);
		TextureColorMultiply->B.Connect(0, ParticleRgb);
		BoostMultiply->A.Connect(0, TextureColorMultiply);
		BoostMultiply->B.Connect(0, EmissiveBoost);

		OpacityMultiply->A.Connect(1, ParticleColor);
		OpacityMultiply->B.Connect(0, OpacityScale);

		auto* EditorData = Material->GetEditorOnlyData();
		if (!EditorData)
		{
			return nullptr;
		}
		EditorData->EmissiveColor.Connect(0, BoostMultiply);
		EditorData->Opacity.Connect(0, OpacityMultiply);

		Material->PreEditChange(nullptr);
		Material->PostEditChange();
		Material->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(Material);
		return Material;
	}

	bool T66CreateTravelerFamilyMaterials(
		UMaterialInterface*& OutAdditiveMaterial,
		UMaterialInterface*& OutTranslucentMaterial)
	{
		UMaterial* AdditiveMaterial = T66CreateTravelerFamilyMaterial(
			T66OutgoingTravelerAdditiveMaterialPackagePath,
			T66OutgoingTravelerAdditiveMaterialObjectName,
			BLEND_Additive,
			8.0f,
			1.0f);
		UMaterial* TranslucentMaterial = T66CreateTravelerFamilyMaterial(
			T66OutgoingTravelerTranslucentMaterialPackagePath,
			T66OutgoingTravelerTranslucentMaterialObjectName,
			BLEND_Translucent,
			2.5f,
			0.72f);
		if (!AdditiveMaterial || !TranslucentMaterial)
		{
			return false;
		}
		if (!T66SaveAsset(AdditiveMaterial) || !T66SaveAsset(TranslucentMaterial))
		{
			return false;
		}

		OutAdditiveMaterial = AdditiveMaterial;
		OutTranslucentMaterial = TranslucentMaterial;
		UE_LOG(LogTemp, Display,
			TEXT("[OutgoingTravelerSwarmVFX] Saved traveler material families additive=%s translucent=%s"),
			*AdditiveMaterial->GetPathName(),
			*TranslucentMaterial->GetPathName());
		return true;
	}

	FString T66ProfileAssetPackagePath(const TCHAR* Root, const TCHAR* Prefix, const TCHAR* AssetSuffix)
	{
		return FString::Printf(TEXT("%s/%s%s"), Root, Prefix, AssetSuffix);
	}

	FString T66ProfileAssetObjectName(const TCHAR* Prefix, const TCHAR* AssetSuffix)
	{
		return FString::Printf(TEXT("%s%s"), Prefix, AssetSuffix);
	}

	FString T66ProfileAssetObjectPath(const TCHAR* Root, const TCHAR* Prefix, const TCHAR* AssetSuffix)
	{
		const FString PackagePath = T66ProfileAssetPackagePath(Root, Prefix, AssetSuffix);
		const FString ObjectName = T66ProfileAssetObjectName(Prefix, AssetSuffix);
		return FString::Printf(TEXT("%s.%s"), *PackagePath, *ObjectName);
	}

	void T66FillTravelerProfileTexturePixels(
		const FT66TravelerVisualProfileAuthoringSpec& Profile,
		TArray<FColor>& OutPixels,
		const int32 Width,
		const int32 Height)
	{
		OutPixels.Reset();
		OutPixels.SetNumZeroed(Width * Height);

		for (int32 Y = 0; Y < Height; ++Y)
		{
			const float V = (static_cast<float>(Y) + 0.5f) / static_cast<float>(Height);
			const float Py = V * 2.0f - 1.0f;
			for (int32 X = 0; X < Width; ++X)
			{
				const float U = (static_cast<float>(X) + 0.5f) / static_cast<float>(Width);
				const float Px = U * 2.0f - 1.0f;
				const float Radius = FMath::Sqrt(Px * Px + Py * Py);
				const float Angle = FMath::Atan2(Py, Px);
				const float ElementPhase = static_cast<float>(Profile.ElementIndex + 1);
				const float DeliveryPhase = static_cast<float>(Profile.DeliveryIndex + 1);
				const float Wave =
					0.5f + 0.5f * FMath::Sin((Px * DeliveryPhase + Py * ElementPhase) * UE_PI * 3.0f + ElementPhase);
				const float Spark =
					0.5f + 0.5f * FMath::Sin(Angle * (4.0f + DeliveryPhase) + Radius * UE_PI * (3.0f + ElementPhase));

				float Mask = 0.0f;
				switch (Profile.DeliveryIndex)
				{
				case 1: // Summon: focused internal line for the helper body.
					Mask = FMath::Clamp(1.0f - FMath::Abs(Py) * 3.2f, 0.0f, 1.0f)
						* FMath::Clamp(1.15f - FMath::Abs(Px) * 0.82f, 0.0f, 1.0f);
					break;
				case 2: // Bounce: angular core.
					Mask = FMath::Clamp(1.0f - (FMath::Abs(Px) * 0.86f + FMath::Abs(Py) * 1.32f), 0.0f, 1.0f);
					break;
				case 3: // DOT: softer patch.
					Mask = FMath::Clamp(1.12f - Radius * (1.12f + Profile.MeshRipple * 0.30f), 0.0f, 1.0f);
					break;
				case 0: // AOE: burst-ring core.
				default:
					Mask = FMath::Clamp(1.0f - Radius * 0.92f, 0.0f, 1.0f);
					Mask *= 0.76f + 0.24f * Spark;
					break;
				}

				const float Intensity = FMath::Clamp(0.12f + Mask * (0.72f + 0.28f * Wave), 0.0f, 1.0f);
				const FLinearColor PixelColor(
					Profile.Tint.R * Intensity,
					Profile.Tint.G * Intensity,
					Profile.Tint.B * Intensity,
					FMath::Clamp(Profile.Tint.A * (0.18f + Mask * 0.82f), 0.0f, 1.0f));
				OutPixels[Y * Width + X] = PixelColor.ToFColor(true);
			}
		}
	}

	UTexture2D* T66CreateTravelerProfileTexture(const FT66TravelerVisualProfileAuthoringSpec& Profile)
	{
		const FString PackagePath = T66ProfileAssetPackagePath(
			T66OutgoingTravelerProfileTextureRoot,
			TEXT("T_TravelerVisual_"),
			Profile.AssetSuffix);
		const FString ObjectName = T66ProfileAssetObjectName(TEXT("T_TravelerVisual_"), Profile.AssetSuffix);
		UPackage* Package = CreatePackage(*PackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		UTexture2D* Texture = FindObject<UTexture2D>(Package, *ObjectName);
		if (!Texture)
		{
			Texture = NewObject<UTexture2D>(
				Package,
				FName(*ObjectName),
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(Texture);
		}
		if (!Texture)
		{
			return nullptr;
		}

		constexpr int32 TextureSize = 64;
		TArray<FColor> Pixels;
		T66FillTravelerProfileTexturePixels(Profile, Pixels, TextureSize, TextureSize);

		Texture->Modify();
		Texture->Source.Init(
			TextureSize,
			TextureSize,
			1,
			1,
			TSF_BGRA8,
			reinterpret_cast<const uint8*>(Pixels.GetData()));
		Texture->SRGB = true;
		Texture->CompressionSettings = TC_Default;
		Texture->MipGenSettings = TMGS_NoMipmaps;
		Texture->Filter = TF_Bilinear;
		Texture->AddressX = TA_Clamp;
		Texture->AddressY = TA_Clamp;
		Texture->PostEditChange();
		Texture->MarkPackageDirty();
		if (!T66SaveAsset(Texture))
		{
			return nullptr;
		}
		return Texture;
	}

	UMaterialInstanceConstant* T66CreateTravelerProfileMaterialInstance(
		const FT66TravelerVisualProfileAuthoringSpec& Profile,
		UMaterialInterface* ParentMaterial,
		UTexture2D* Texture)
	{
		if (!ParentMaterial || !Texture)
		{
			return nullptr;
		}

		const FString PackagePath = T66ProfileAssetPackagePath(
			T66OutgoingTravelerProfileMaterialRoot,
			TEXT("MI_TravelerVisual_"),
			Profile.AssetSuffix);
		const FString ObjectName = T66ProfileAssetObjectName(TEXT("MI_TravelerVisual_"), Profile.AssetSuffix);
		UPackage* Package = CreatePackage(*PackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		UMaterialInstanceConstant* MaterialInstance = FindObject<UMaterialInstanceConstant>(Package, *ObjectName);
		if (!MaterialInstance)
		{
			MaterialInstance = NewObject<UMaterialInstanceConstant>(
				Package,
				FName(*ObjectName),
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(MaterialInstance);
		}
		if (!MaterialInstance)
		{
			return nullptr;
		}

		MaterialInstance->Modify();
		MaterialInstance->SetParentEditorOnly(ParentMaterial, false);
		MaterialInstance->SetTextureParameterValueEditorOnly(
			FMaterialParameterInfo(T66TravelerTextureParameter),
			Texture);
		const bool bAdditiveFamily = Profile.FamilyTag == 1;
		const float EmissiveBoost = bAdditiveFamily
			? 8.0f + static_cast<float>(Profile.ElementIndex) * 0.35f + static_cast<float>(Profile.DeliveryIndex) * 0.15f
			: 2.5f + static_cast<float>(Profile.ElementIndex) * 0.25f + static_cast<float>(Profile.DeliveryIndex) * 0.10f;
		const float OpacityScale = bAdditiveFamily ? 1.0f : 0.72f;
		MaterialInstance->SetScalarParameterValueEditorOnly(
			FMaterialParameterInfo(T66TravelerEmissiveBoostParameter),
			EmissiveBoost);
		MaterialInstance->SetScalarParameterValueEditorOnly(
			FMaterialParameterInfo(T66TravelerOpacityScaleParameter),
			OpacityScale);
		MaterialInstance->PostEditChange();
		MaterialInstance->MarkPackageDirty();
		if (!T66SaveAsset(MaterialInstance))
		{
			return nullptr;
		}
		return MaterialInstance;
	}

	FVertexInstanceID T66CreateTravelerVertexInstance(
		FMeshDescription& MeshDescription,
		TVertexInstanceAttributesRef<FVector2f>& UVs,
		const FVertexID VertexID,
		const FVector2f UV)
	{
		const FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
		UVs[VertexInstanceID] = UV;
		return VertexInstanceID;
	}

	void T66BuildTravelerProfilePolygonPoints(
		const FT66TravelerVisualProfileAuthoringSpec& Profile,
		TArray<FVector2f>& OutPoints)
	{
		OutPoints.Reset();
		const float HalfLength = Profile.MeshLength * 0.5f;
		const float HalfWidth = Profile.MeshWidth * 0.5f;
		const float Ripple = Profile.MeshRipple;

		switch (Profile.DeliveryIndex)
		{
		case 1: // Summon helper dash body.
			OutPoints = {
				FVector2f(-HalfLength, -HalfWidth * 0.45f),
				FVector2f(HalfLength * 0.30f, -HalfWidth * (0.50f + Ripple * 0.12f)),
				FVector2f(HalfLength, 0.0f),
				FVector2f(HalfLength * 0.30f, HalfWidth * (0.50f + Ripple * 0.12f)),
				FVector2f(-HalfLength, HalfWidth * 0.45f),
				FVector2f(-HalfLength * 0.72f, 0.0f)
			};
			break;
		case 2: // Bounce angular shard.
			OutPoints = {
				FVector2f(-HalfLength * 0.92f, -HalfWidth * 0.24f),
				FVector2f(-HalfLength * 0.12f, -HalfWidth),
				FVector2f(HalfLength, -HalfWidth * 0.12f),
				FVector2f(HalfLength * 0.56f, HalfWidth * 0.92f),
				FVector2f(-HalfLength * 0.38f, HalfWidth * 0.46f),
				FVector2f(-HalfLength * 0.08f, HalfWidth * 0.08f)
			};
			break;
		case 3: // DOT patch.
			for (int32 Index = 0; Index < 12; ++Index)
			{
				const float T = static_cast<float>(Index) / 12.0f;
				const float Angle = T * UE_TWO_PI;
				const float R = 0.82f + 0.16f * FMath::Sin(Angle * (2.0f + Profile.ElementIndex) + Ripple * 3.0f)
					+ 0.08f * FMath::Sin(Angle * (5.0f + Profile.DeliveryIndex));
				OutPoints.Add(FVector2f(FMath::Cos(Angle) * HalfLength * R, FMath::Sin(Angle) * HalfWidth * R));
			}
			break;
		case 0: // AOE burst.
		default:
			for (int32 Index = 0; Index < 14; ++Index)
			{
				const float T = static_cast<float>(Index) / 14.0f;
				const float Angle = T * UE_TWO_PI;
				const float Spike = (Index % 2 == 0) ? 1.0f : (0.58f + Ripple * 0.15f);
				OutPoints.Add(FVector2f(FMath::Cos(Angle) * HalfLength * Spike, FMath::Sin(Angle) * HalfWidth * Spike));
			}
			break;
		}
	}

	UStaticMesh* T66CreateTravelerProfileMesh(
		const FT66TravelerVisualProfileAuthoringSpec& Profile,
		UMaterialInterface* ProfileMaterial)
	{
		if (!ProfileMaterial)
		{
			return nullptr;
		}

		const FString PackagePath = T66ProfileAssetPackagePath(
			T66OutgoingTravelerProfileMeshRoot,
			TEXT("SM_TravelerVisual_"),
			Profile.AssetSuffix);
		const FString ObjectName = T66ProfileAssetObjectName(TEXT("SM_TravelerVisual_"), Profile.AssetSuffix);
		UPackage* Package = CreatePackage(*PackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		UStaticMesh* Mesh = FindObject<UStaticMesh>(Package, *ObjectName);
		if (!Mesh)
		{
			Mesh = NewObject<UStaticMesh>(
				Package,
				FName(*ObjectName),
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(Mesh);
		}
		if (!Mesh)
		{
			return nullptr;
		}

		Mesh->Modify();
		Mesh->bAllowCPUAccess = true;
		Mesh->GetStaticMaterials().Reset();
		Mesh->GetStaticMaterials().Add(FStaticMaterial(ProfileMaterial, TEXT("TravelerVisualProfile")));
		Mesh->SetLightMapResolution(4);
		Mesh->SetLightingGuid();

		FMeshDescription MeshDescription;
		FStaticMeshAttributes Attributes(MeshDescription);
		Attributes.Register();

		TVertexAttributesRef<FVector3f> Positions = Attributes.GetVertexPositions();
		TVertexInstanceAttributesRef<FVector2f> UVs = Attributes.GetVertexInstanceUVs();
		UVs.SetNumChannels(1);
		TPolygonGroupAttributesRef<FName> MaterialSlotNames = Attributes.GetPolygonGroupMaterialSlotNames();
		const FPolygonGroupID PolygonGroupID = MeshDescription.CreatePolygonGroup();
		MaterialSlotNames[PolygonGroupID] = TEXT("TravelerVisualProfile");

		TArray<FVector2f> Points;
		T66BuildTravelerProfilePolygonPoints(Profile, Points);
		if (Points.Num() < 3)
		{
			return nullptr;
		}

		auto ToUV = [&](const FVector2f& Point)
		{
			const float U = FMath::Clamp((Point.X / FMath::Max(1.0f, Profile.MeshLength)) + 0.5f, 0.0f, 1.0f);
			const float V = FMath::Clamp((Point.Y / FMath::Max(1.0f, Profile.MeshWidth)) + 0.5f, 0.0f, 1.0f);
			return FVector2f(U, V);
		};

		auto PointToPosition = [](const FVector2f& Point, const int32 PlaneIndex)
		{
			switch (PlaneIndex)
			{
			case 1:
				return FVector3f(Point.X, Point.Y, 0.0f);
			case 2:
				return FVector3f(0.0f, Point.X, Point.Y);
			case 0:
			default:
				return FVector3f(Point.X, 0.0f, Point.Y);
			}
		};

		auto AddProfileFan = [&](const int32 PlaneIndex)
		{
			const FVertexID CenterVertex = MeshDescription.CreateVertex();
			Positions[CenterVertex] = FVector3f(0.0f, 0.0f, 0.0f);

			TArray<FVertexID> PerimeterVertices;
			PerimeterVertices.Reserve(Points.Num());
			for (const FVector2f& Point : Points)
			{
				const FVertexID VertexID = MeshDescription.CreateVertex();
				Positions[VertexID] = PointToPosition(Point, PlaneIndex);
				PerimeterVertices.Add(VertexID);
			}

			for (int32 Index = 0; Index < PerimeterVertices.Num(); ++Index)
			{
				const int32 NextIndex = (Index + 1) % PerimeterVertices.Num();
				TArray<FVertexInstanceID> Triangle;
				Triangle.Reserve(3);
				Triangle.Add(T66CreateTravelerVertexInstance(MeshDescription, UVs, CenterVertex, FVector2f(0.5f, 0.5f)));
				Triangle.Add(T66CreateTravelerVertexInstance(MeshDescription, UVs, PerimeterVertices[Index], ToUV(Points[Index])));
				Triangle.Add(T66CreateTravelerVertexInstance(MeshDescription, UVs, PerimeterVertices[NextIndex], ToUV(Points[NextIndex])));
				MeshDescription.CreatePolygon(PolygonGroupID, Triangle);
			}
		};

		// Three cheap profile planes keep the authored silhouette visible from gameplay and proof cameras.
		AddProfileFan(0);
		AddProfileFan(1);
		AddProfileFan(2);

		FStaticMeshOperations::ComputeTriangleTangentsAndNormals(MeshDescription);
		FStaticMeshOperations::ComputeTangentsAndNormals(
			MeshDescription,
			EComputeNTBsFlags::Normals | EComputeNTBsFlags::Tangents);

		UStaticMesh::FBuildMeshDescriptionsParams BuildParams;
		BuildParams.bBuildSimpleCollision = false;
		BuildParams.bAllowCpuAccess = true;
		BuildParams.bFastBuild = true;
		BuildParams.bCommitMeshDescription = true;
		if (!Mesh->BuildFromMeshDescriptions({ &MeshDescription }, BuildParams))
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to build traveler profile mesh %s"), *ObjectName);
			return nullptr;
		}
		Mesh->SetMaterial(0, ProfileMaterial);

		Mesh->MarkPackageDirty();
		if (!T66SaveAsset(Mesh))
		{
			return nullptr;
		}
		return Mesh;
	}

	bool T66BuildTravelerProfileMeshSlots(
		UMaterialInterface* AdditiveMaterial,
		UMaterialInterface* TranslucentMaterial,
		TArray<FT66TravelerMeshSlotDefinition>& OutMeshSlots)
	{
		OutMeshSlots.Reset();
		OutMeshSlots.Reserve(
			UT66OutgoingTravelerPoolSubsystem::TravelerVisualProfileSlotBase
			+ UT66OutgoingTravelerPoolSubsystem::TravelerVisualProfileSlotCount);

		const FT66TravelerVisualProfileAuthoringSpec FallbackProfiles[] =
		{
			{ TEXT("TemporaryVisual.Sphere"),   TEXT("Temporary_Sphere"),   0, 1, 0, 0, FLinearColor(0.08f, 0.52f, 1.00f, 1.0f), 106.0f, 72.0f, 0.20f },
			{ TEXT("TemporaryVisual.Cone"),     TEXT("Temporary_Cone"),     1, 1, 0, 1, FLinearColor(0.08f, 0.52f, 1.00f, 1.0f), 150.0f, 34.0f, 0.18f },
			{ TEXT("TemporaryVisual.Cylinder"), TEXT("Temporary_Cylinder"), 2, 1, 0, 3, FLinearColor(0.08f, 0.52f, 1.00f, 1.0f), 100.0f, 68.0f, 0.24f },
			{ TEXT("TemporaryVisual.Cube"),     TEXT("Temporary_Cube"),     3, 1, 0, 2, FLinearColor(0.08f, 0.52f, 1.00f, 1.0f), 104.0f, 64.0f, 0.28f }
		};

		auto AddAuthoredSlot =
			[&](const FT66TravelerVisualProfileAuthoringSpec& Profile)
		{
			if (Profile.MeshIndex != OutMeshSlots.Num())
			{
				UE_LOG(LogTemp, Error,
					TEXT("[OutgoingTravelerSwarmVFX] Traveler slot order mismatch profile=%s mesh_index=%d next_slot=%d"),
					Profile.ProfileID,
					Profile.MeshIndex,
					OutMeshSlots.Num());
				return false;
			}

			UTexture2D* Texture = T66CreateTravelerProfileTexture(Profile);
			if (!Texture)
			{
				UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to create texture for %s"), Profile.ProfileID);
				return false;
			}

			UMaterialInterface* ParentMaterial = Profile.FamilyTag == 2 ? TranslucentMaterial : AdditiveMaterial;
			UMaterialInstanceConstant* MaterialInstance =
				T66CreateTravelerProfileMaterialInstance(Profile, ParentMaterial, Texture);
			if (!MaterialInstance)
			{
				UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to create material instance for %s"), Profile.ProfileID);
				return false;
			}

			UStaticMesh* Mesh = T66CreateTravelerProfileMesh(Profile, MaterialInstance);
			if (!Mesh)
			{
				UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to create mesh for %s"), Profile.ProfileID);
				return false;
			}

			const FString MeshObjectPath = T66ProfileAssetObjectPath(
				T66OutgoingTravelerProfileMeshRoot,
				TEXT("SM_TravelerVisual_"),
				Profile.AssetSuffix);
			OutMeshSlots.Add({ MeshObjectPath, FVector::OneVector, FRotator::ZeroRotator, Profile.FamilyTag });
			UE_LOG(LogTemp, Display,
				TEXT("[OutgoingTravelerSwarmVFX] Authored traveler slot profile=%s mesh_index=%d family=%d mesh=%s material=%s texture=%s"),
				Profile.ProfileID,
				Profile.MeshIndex,
				Profile.FamilyTag,
				*Mesh->GetPathName(),
				*MaterialInstance->GetPathName(),
				*Texture->GetPathName());
			return true;
		};

		for (const FT66TravelerVisualProfileAuthoringSpec& Profile : FallbackProfiles)
		{
			if (!AddAuthoredSlot(Profile))
			{
				return false;
			}
		}

		for (const FT66TravelerVisualProfileAuthoringSpec& Profile : T66TravelerVisualProfiles)
		{
			if (!AddAuthoredSlot(Profile))
			{
				return false;
			}
		}

		if (OutMeshSlots.Num() != UT66OutgoingTravelerPoolSubsystem::TravelerVisualProfileSlotBase
			+ UT66OutgoingTravelerPoolSubsystem::TravelerVisualProfileSlotCount)
		{
			UE_LOG(LogTemp, Error,
				TEXT("[OutgoingTravelerSwarmVFX] Traveler mesh slot count mismatch slots=%d expected=%d"),
				OutMeshSlots.Num(),
				UT66OutgoingTravelerPoolSubsystem::TravelerVisualProfileSlotBase
					+ UT66OutgoingTravelerPoolSubsystem::TravelerVisualProfileSlotCount);
			return false;
		}
		return true;
	}

	UNiagaraNodeFunctionCall* T66AddNiagaraModuleFromAssetPath(
		const TCHAR* AssetPath,
		UNiagaraNodeOutput& TargetOutputNode)
	{
		UNiagaraScript* ModuleScript = Cast<UNiagaraScript>(FSoftObjectPath(AssetPath).TryLoad());
		if (!ModuleScript)
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Missing Niagara module %s"), AssetPath);
			return nullptr;
		}

		return FNiagaraStackGraphUtilities::AddScriptModuleToStack(ModuleScript, TargetOutputNode);
	}

	template<typename ValueType>
	void T66SetRapidIterationParameter(
		const FString& UniqueEmitterName,
		UNiagaraScript& TargetScript,
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		const FNiagaraTypeDefinition& InputType,
		const ValueType Value)
	{
		static_assert(!TIsUECoreVariant<ValueType, double>::Value, "Double core variant. Must be float type.");
		const FNiagaraParameterHandle InputHandle = FNiagaraParameterHandle::CreateModuleParameterHandle(InputName);
		const FNiagaraParameterHandle AliasedInputHandle =
			FNiagaraParameterHandle::CreateAliasedModuleParameterHandle(InputHandle, &TargetFunctionCallNode);
		FNiagaraVariable InputVariable(InputType, AliasedInputHandle.GetParameterHandleString());
		const TCHAR* EmitterNameForRapidIteration =
			(TargetScript.GetUsage() == ENiagaraScriptUsage::SystemSpawnScript ||
				TargetScript.GetUsage() == ENiagaraScriptUsage::SystemUpdateScript)
			? nullptr
			: *UniqueEmitterName;
		FNiagaraVariable RapidIterationParameter = FNiagaraUtilities::ConvertVariableToRapidIterationConstantName(
			InputVariable,
			EmitterNameForRapidIteration,
			TargetScript.GetUsage());
		RapidIterationParameter.SetValue(Value);

		constexpr bool bAddParameterIfMissing = true;
		TargetScript.RapidIterationParameters.SetParameterData(
			RapidIterationParameter.GetData(),
			RapidIterationParameter,
			bAddParameterIfMissing);
	}

	bool T66LogAndResolveStackInput(
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		FNiagaraVariable& OutInputVariable)
	{
		TArray<FNiagaraVariable> Inputs;
		FNiagaraStackGraphUtilities::GetStackFunctionInputs(
			TargetFunctionCallNode,
			Inputs,
			FCompileConstantResolver(),
			FNiagaraStackGraphUtilities::ENiagaraGetStackFunctionInputPinsOptions::ModuleInputsOnly,
			false);

		const FString RequestedInputName = InputName.ToString();
		for (const FNiagaraVariable& Input : Inputs)
		{
			const FString CandidateName = Input.GetName().ToString();
			UE_LOG(LogTemp, Display,
				TEXT("[OutgoingTravelerSwarmVFX] Stack input candidate function=%s input=%s type=%s"),
				*TargetFunctionCallNode.GetFunctionName(),
				*CandidateName,
				*Input.GetType().GetNameText().ToString());

			if (CandidateName.Equals(RequestedInputName, ESearchCase::IgnoreCase)
				|| CandidateName.EndsWith(TEXT(".") + RequestedInputName, ESearchCase::IgnoreCase)
				|| CandidateName.EndsWith(RequestedInputName, ESearchCase::IgnoreCase))
			{
				OutInputVariable = Input;
				return true;
			}
		}

		UE_LOG(LogTemp, Warning,
			TEXT("[OutgoingTravelerSwarmVFX] Could not resolve stack input '%s' on function=%s. Candidate count=%d"),
			*RequestedInputName,
			*TargetFunctionCallNode.GetFunctionName(),
			Inputs.Num());
		return false;
	}

	template<typename ValueType>
	bool T66SetRapidIterationParameterResolved(
		const FString& UniqueEmitterName,
		UNiagaraScript& TargetScript,
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		const FNiagaraTypeDefinition& ExpectedInputType,
		const ValueType Value)
	{
		FNiagaraVariable ResolvedInput;
		const FName EffectiveInputName = T66LogAndResolveStackInput(
			TargetFunctionCallNode,
			InputName,
			ResolvedInput)
			? FNiagaraParameterHandle(ResolvedInput.GetName()).GetName()
			: InputName;

		T66SetRapidIterationParameter<ValueType>(
			UniqueEmitterName,
			TargetScript,
			TargetFunctionCallNode,
			EffectiveInputName,
			ExpectedInputType,
			Value);
		UE_LOG(LogTemp, Display,
			TEXT("[OutgoingTravelerSwarmVFX] Set rapid iteration input function=%s requested=%s effective=%s value=%s"),
			*TargetFunctionCallNode.GetFunctionName(),
			*InputName.ToString(),
			*EffectiveInputName.ToString(),
			*LexToString(Value));
		return true;
	}

	bool T66SetRapidIterationEnumParameter(
		const FString& UniqueEmitterName,
		UNiagaraScript& TargetScript,
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		const TCHAR* EnumObjectPath,
		const int32 Value)
	{
		UEnum* Enum = LoadObject<UEnum>(nullptr, EnumObjectPath);
		if (!Enum)
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Missing Niagara enum %s"), EnumObjectPath);
			return false;
		}

		T66SetRapidIterationParameter<int32>(
			UniqueEmitterName,
			TargetScript,
			TargetFunctionCallNode,
			InputName,
			FNiagaraTypeDefinition(Enum),
			Value);
		return true;
	}

	UEdGraphPin* T66FindNiagaraPin(
		UNiagaraNode& Node,
		const EEdGraphPinDirection Direction,
		const FNiagaraTypeDefinition& Type,
		const FName OptionalName = NAME_None)
	{
		const UEdGraphSchema_Niagara* Schema = GetDefault<UEdGraphSchema_Niagara>();
		for (UEdGraphPin* Pin : Node.Pins)
		{
			if (!Pin || Pin->Direction != Direction)
			{
				continue;
			}
			if (!OptionalName.IsNone() && Pin->PinName != OptionalName)
			{
				continue;
			}
			if (Schema->PinToTypeDefinition(Pin) == Type)
			{
				return Pin;
			}
		}
		return nullptr;
	}

	UEdGraphPin* T66FindNiagaraModuleInputPin(
		UNiagaraNode& Node,
		const FNiagaraTypeDefinition& Type,
		const FName InputName)
	{
		const UEdGraphSchema_Niagara* Schema = GetDefault<UEdGraphSchema_Niagara>();
		const FString InputNameString = InputName.ToString();
		for (UEdGraphPin* Pin : Node.Pins)
		{
			if (!Pin || Pin->Direction != EGPD_Input)
			{
				continue;
			}
			if (Schema->PinToTypeDefinition(Pin) != Type)
			{
				continue;
			}

			const FString PinNameString = Pin->PinName.ToString();
			if (Pin->PinName == InputName || PinNameString.EndsWith(TEXT(".") + InputNameString))
			{
				return Pin;
			}
		}
		return nullptr;
	}

	bool T66ConnectPins(UEdGraphPin* OutputPin, UEdGraphPin* InputPin)
	{
		if (!OutputPin || !InputPin)
		{
			return false;
		}

		const UEdGraphSchema* Schema = OutputPin->GetSchema();
		if (!Schema || Schema != InputPin->GetSchema())
		{
			return false;
		}

		return Schema->TryCreateConnection(OutputPin, InputPin);
	}

	UEdGraphPin* T66CreateNiagaraTypedPin(
		UNiagaraNode& Node,
		const EEdGraphPinDirection Direction,
		const FNiagaraTypeDefinition& Type,
		const FName PinName)
	{
		const UEdGraphSchema_Niagara* NiagaraSchema = GetDefault<UEdGraphSchema_Niagara>();
		if (!NiagaraSchema)
		{
			return nullptr;
		}

		return Node.CreatePin(Direction, NiagaraSchema->TypeDefinitionToPinType(Type), PinName);
	}

	bool T66SetReflectedString(UObject& Object, const FName PropertyName, const FString& Value);
	void T66RebuildCustomHlslSignatureFromPins(UNiagaraNodeCustomHlsl& CustomNode);

	bool T66ConnectParameterToModuleInput(
		UNiagaraGraph& Graph,
		UNiagaraNodeFunctionCall& ModuleNode,
		const TCHAR* ParameterName,
		const FNiagaraTypeDefinition& Type,
		const FName InputName)
	{
		(void)Graph;
		const FNiagaraParameterHandle InputHandle = FNiagaraParameterHandle::CreateModuleParameterHandle(InputName);
		const FNiagaraParameterHandle AliasedInputHandle =
			FNiagaraParameterHandle::CreateAliasedModuleParameterHandle(InputHandle, &ModuleNode);
		UEdGraphPin& ModuleInputPin = FNiagaraStackGraphUtilities::GetOrCreateStackFunctionInputOverridePin(
			ModuleNode,
			AliasedInputHandle,
			Type,
			FGuid(),
			FGuid());

		const FNiagaraVariableBase LinkedParameter(Type, FName(ParameterName));
		TSet<FNiagaraVariableBase> KnownParameters;
		KnownParameters.Add(LinkedParameter);
		FNiagaraStackGraphUtilities::SetLinkedParameterValueForFunctionInput(
			ModuleInputPin,
			LinkedParameter,
			KnownParameters,
			ENiagaraDefaultMode::Custom);
		return true;
	}

	bool T66SetModuleInputCustomExpression(
		UNiagaraNodeFunctionCall& ModuleNode,
		const FName InputName,
		const FNiagaraTypeDefinition& Type,
		const FString& Expression)
	{
		const FNiagaraParameterHandle InputHandle = FNiagaraParameterHandle::CreateModuleParameterHandle(InputName);
		const FNiagaraParameterHandle AliasedInputHandle =
			FNiagaraParameterHandle::CreateAliasedModuleParameterHandle(InputHandle, &ModuleNode);
		UEdGraphPin& ModuleInputPin = FNiagaraStackGraphUtilities::GetOrCreateStackFunctionInputOverridePin(
			ModuleNode,
			AliasedInputHandle,
			Type,
			FGuid(),
			FGuid());

		if (ModuleInputPin.LinkedTo.Num() > 0)
		{
			ModuleInputPin.BreakAllPinLinks();
		}

		UNiagaraNode* OverrideNode = Cast<UNiagaraNode>(ModuleInputPin.GetOwningNode());
		UEdGraph* Graph = OverrideNode ? OverrideNode->GetGraph() : nullptr;
		const UEdGraphSchema_Niagara* Schema = OverrideNode
			? Cast<UEdGraphSchema_Niagara>(OverrideNode->GetSchema())
			: nullptr;
		if (!OverrideNode || !Graph || !Schema)
		{
			return false;
		}

		UEdGraphPin* OverrideParameterMapInput = T66FindNiagaraPin(
			*OverrideNode,
			EGPD_Input,
			FNiagaraTypeDefinition::GetParameterMapDef());
		if (!OverrideParameterMapInput || OverrideParameterMapInput->LinkedTo.Num() == 0)
		{
			return false;
		}

		UEdGraphPin* PreviousStackOutput = OverrideParameterMapInput->LinkedTo[0];
		if (!PreviousStackOutput)
		{
			return false;
		}

		Graph->Modify();
		FGraphNodeCreator<UNiagaraNodeCustomHlsl> DynamicInputCreator(*Graph);
		UNiagaraNodeCustomHlsl* DynamicInputNode = DynamicInputCreator.CreateNode();
		if (!DynamicInputNode)
		{
			return false;
		}
		DynamicInputNode->Modify();
		DynamicInputNode->ScriptUsage = ENiagaraScriptUsage::DynamicInput;
		DynamicInputNode->Signature.Name = TEXT("Custom Hlsl");
		if (!T66SetReflectedString(*DynamicInputNode, TEXT("FunctionDisplayName"), TEXT("Custom Hlsl")))
		{
			return false;
		}
		T66CreateNiagaraTypedPin(
			*DynamicInputNode,
			EGPD_Input,
			FNiagaraTypeDefinition::GetParameterMapDef(),
			TEXT("Map"));
		T66CreateNiagaraTypedPin(
			*DynamicInputNode,
			EGPD_Output,
			Type,
			TEXT("CustomHLSLOutput"));
		if (!T66SetReflectedString(*DynamicInputNode, TEXT("CustomHlsl"), Expression))
		{
			return false;
		}
		DynamicInputCreator.Finalize();
		T66RebuildCustomHlslSignatureFromPins(*DynamicInputNode);

		UEdGraphPin* DynamicParameterMapInput = T66FindNiagaraPin(
			*DynamicInputNode,
			EGPD_Input,
			FNiagaraTypeDefinition::GetParameterMapDef());
		UEdGraphPin* DynamicValueOutput = T66FindNiagaraPin(
			*DynamicInputNode,
			EGPD_Output,
			Type);
		if (!DynamicParameterMapInput || !DynamicValueOutput)
		{
			return false;
		}

		ModuleInputPin.Modify();
		return
			T66ConnectPins(PreviousStackOutput, DynamicParameterMapInput) &&
			T66ConnectPins(DynamicValueOutput, &ModuleInputPin);
	}

	bool T66SetReflectedString(UObject& Object, const FName PropertyName, const FString& Value)
	{
		FStrProperty* StringProperty = FindFProperty<FStrProperty>(Object.GetClass(), PropertyName);
		if (!StringProperty)
		{
			return false;
		}

		StringProperty->SetPropertyValue_InContainer(&Object, Value);
		return true;
	}

	bool T66SetReflectedObject(UObject& Object, const FName PropertyName, UObject* Value)
	{
		FObjectPropertyBase* ObjectProperty = FindFProperty<FObjectPropertyBase>(Object.GetClass(), PropertyName);
		if (!ObjectProperty)
		{
			return false;
		}

		ObjectProperty->SetObjectPropertyValue_InContainer(&Object, Value);
		return true;
	}

	void T66RebuildCustomHlslSignatureFromPins(UNiagaraNodeCustomHlsl& CustomNode)
	{
		FNiagaraFunctionSignature Signature = CustomNode.Signature;
		Signature.Inputs.Empty();
		Signature.Outputs.Empty();

		FPinCollectorArray InputPins;
		FPinCollectorArray OutputPins;
		CustomNode.GetInputPins(InputPins);
		CustomNode.GetOutputPins(OutputPins);

		for (UEdGraphPin* Pin : InputPins)
		{
			if (Pin)
			{
				Signature.Inputs.Add(UEdGraphSchema_Niagara::PinToNiagaraVariable(Pin, true));
			}
		}

		for (UEdGraphPin* Pin : OutputPins)
		{
			if (Pin)
			{
				Signature.Outputs.Add(UEdGraphSchema_Niagara::PinToNiagaraVariable(Pin, false));
			}
		}

		CustomNode.Signature = Signature;
	}

	bool T66InsertCustomHlslModuleAtStackEnd(
		UNiagaraNodeOutput& TargetOutputNode,
		UNiagaraNodeCustomHlsl& CustomNode)
	{
		UEdGraphPin* TargetInputPin = T66FindNiagaraPin(
			TargetOutputNode,
			EGPD_Input,
			FNiagaraTypeDefinition::GetParameterMapDef());
		UEdGraphPin* CustomInputPin = T66FindNiagaraPin(
			CustomNode,
			EGPD_Input,
			FNiagaraTypeDefinition::GetParameterMapDef(),
			TEXT("InputMap"));
		UEdGraphPin* CustomOutputPin = T66FindNiagaraPin(
			CustomNode,
			EGPD_Output,
			FNiagaraTypeDefinition::GetParameterMapDef(),
			TEXT("OutputMap"));
		if (!TargetInputPin || !CustomInputPin || !CustomOutputPin || TargetInputPin->LinkedTo.Num() == 0)
		{
			return false;
		}

		UEdGraphPin* PreviousOutputPin = TargetInputPin->LinkedTo[0];
		if (!PreviousOutputPin)
		{
			return false;
		}

		TargetInputPin->BreakAllPinLinks(true);
		return
			T66ConnectPins(PreviousOutputPin, CustomInputPin) &&
			T66ConnectPins(CustomOutputPin, TargetInputPin);
	}

	bool T66AddArrayDataInterfaceInput(
		UNiagaraGraph& Graph,
		UNiagaraNodeCustomHlsl& CustomNode,
		const FName PinName,
		const TCHAR* ParameterName,
		const FNiagaraTypeDefinition& ArrayType,
		UClass* DataInterfaceClass)
	{
		UEdGraphPin* CustomArrayPin = T66CreateNiagaraTypedPin(
			CustomNode,
			EGPD_Input,
			ArrayType,
			PinName);
		if (!CustomArrayPin)
		{
			return false;
		}

		FGraphNodeCreator<UNiagaraNodeInput> InputNodeCreator(Graph);
		UNiagaraNodeInput* ArrayInputNode = InputNodeCreator.CreateNode();
		ArrayInputNode->Input = FNiagaraVariable(ArrayType, ParameterName);
		ArrayInputNode->Usage = ENiagaraInputNodeUsage::Parameter;
		ArrayInputNode->ExposureOptions.bRequired = true;
		if (!T66SetReflectedObject(
				*ArrayInputNode,
				TEXT("DataInterface"),
				DataInterfaceClass ? NewObject<UObject>(ArrayInputNode, DataInterfaceClass, NAME_None, RF_Transactional) : nullptr))
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to assign traveler array data interface %s"),
				ParameterName);
			return false;
		}
		InputNodeCreator.Finalize();

		UEdGraphPin* ArrayOutputPin = T66FindNiagaraPin(*ArrayInputNode, EGPD_Output, ArrayType);
		return T66ConnectPins(ArrayOutputPin, CustomArrayPin);
	}

	bool T66AddParameterInput(
		UNiagaraGraph& Graph,
		UNiagaraNodeCustomHlsl& CustomNode,
		const FName PinName,
		const TCHAR* ParameterName,
		const FNiagaraTypeDefinition& Type)
	{
		UEdGraphPin* CustomPin = T66CreateNiagaraTypedPin(
			CustomNode,
			EGPD_Input,
			Type,
			PinName);
		if (!CustomPin)
		{
			return false;
		}

		FGraphNodeCreator<UNiagaraNodeInput> InputNodeCreator(Graph);
		UNiagaraNodeInput* InputNode = InputNodeCreator.CreateNode();
		InputNode->Input = FNiagaraVariable(Type, ParameterName);
		InputNode->Usage = ENiagaraInputNodeUsage::Parameter;
		InputNode->ExposureOptions.bRequired = true;
		InputNodeCreator.Finalize();

		UEdGraphPin* OutputPin = T66FindNiagaraPin(*InputNode, EGPD_Output, Type);
		return T66ConnectPins(OutputPin, CustomPin);
	}

	bool T66AddTravelerArrayInput(
		UNiagaraGraph& Graph,
		UNiagaraNodeCustomHlsl& CustomNode)
	{
		return T66AddArrayDataInterfaceInput(
			Graph,
			CustomNode,
			TEXT("TravelerPositions"),
			T66OutgoingTravelerArrayParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat3::StaticClass()),
			UNiagaraDataInterfaceArrayFloat3::StaticClass());
	}

	bool T66AddProductionTravelerArrayInputs(
		UNiagaraGraph& Graph,
		UNiagaraNodeCustomHlsl& CustomNode)
	{
		return
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("TravelerPositions"),
				T66OutgoingTravelerArrayParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat3::StaticClass()),
				UNiagaraDataInterfaceArrayFloat3::StaticClass()) &&
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("TravelerRotations"),
				T66OutgoingTravelerRotationsParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayQuat::StaticClass()),
				UNiagaraDataInterfaceArrayQuat::StaticClass()) &&
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("TravelerScales"),
				T66OutgoingTravelerScalesParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat3::StaticClass()),
				UNiagaraDataInterfaceArrayFloat3::StaticClass()) &&
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("TravelerColors"),
				T66OutgoingTravelerColorsParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayColor::StaticClass()),
				UNiagaraDataInterfaceArrayColor::StaticClass()) &&
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("TravelerMeshIndices"),
				T66OutgoingTravelerMeshIndicesParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayInt32::StaticClass()),
				UNiagaraDataInterfaceArrayInt32::StaticClass()) &&
			T66AddParameterInput(
				Graph,
				CustomNode,
				TEXT("TravelerLiveCount"),
				T66OutgoingTravelerLiveCountParameter,
				FNiagaraTypeDefinition::GetIntDef());
	}

	UNiagaraNodeCustomHlsl* T66CreateTravelerArrayModule(
		UNiagaraGraph& Graph,
		const TCHAR* NodeName,
		const FString& Hlsl,
		const bool bProductionPool = false)
	{
		FGraphNodeCreator<UNiagaraNodeCustomHlsl> NodeCreator(Graph);
		UNiagaraNodeCustomHlsl* CustomNode = NodeCreator.CreateNode();
		if (!CustomNode)
		{
			return nullptr;
		}

		CustomNode->ScriptUsage = ENiagaraScriptUsage::Module;
		CustomNode->Signature.Name = FName(NodeName);
		if (!T66SetReflectedString(*CustomNode, TEXT("FunctionDisplayName"), NodeName))
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to name custom HLSL node %s"), NodeName);
			return nullptr;
		}
		T66CreateNiagaraTypedPin(*CustomNode, EGPD_Input, FNiagaraTypeDefinition::GetParameterMapDef(), TEXT("InputMap"));
		T66CreateNiagaraTypedPin(*CustomNode, EGPD_Output, FNiagaraTypeDefinition::GetParameterMapDef(), TEXT("OutputMap"));
		if (!T66SetReflectedString(*CustomNode, TEXT("CustomHlsl"), Hlsl))
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to assign custom HLSL for %s"), NodeName);
			return nullptr;
		}
		NodeCreator.Finalize();

		const bool bAddedInputs = bProductionPool
			? T66AddProductionTravelerArrayInputs(Graph, *CustomNode)
			: T66AddTravelerArrayInput(Graph, *CustomNode);
		if (!bAddedInputs)
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to add traveler array input for %s"), NodeName);
			return nullptr;
		}

		T66RebuildCustomHlslSignatureFromPins(*CustomNode);
		return CustomNode;
	}

	FString T66SpawnHlsl()
	{
		return TEXT(
			"int TravelerCount = 0;\n"
			"TravelerPositions.Length(TravelerCount);\n"
			"int TravelerIndex = ExecIndex();\n"
			"if (TravelerCount > 0)\n"
			"{\n"
			"    TravelerIndex = clamp(TravelerIndex, 0, TravelerCount - 1);\n"
			"}\n"
			"float3 TravelerPosition = float3(0.0, 0.0, 0.0);\n"
			"if (TravelerCount > 0)\n"
			"{\n"
			"    TravelerPositions.Get(TravelerIndex, TravelerPosition);\n"
			"}\n"
			"Particles.T66TravelerIndex = TravelerIndex;\n"
			"Particles.Position = TravelerPosition;\n"
			"Particles.Scale = float3(1.0, 1.0, 1.0);\n"
			"Particles.Color = float4(1.0, 0.30, 0.04, 1.0);\n"
			"Particles.Lifetime = 600.0;\n");
	}

	FString T66UpdateHlsl()
	{
		return TEXT(
			"int TravelerCount = 0;\n"
			"TravelerPositions.Length(TravelerCount);\n"
			"int TravelerIndex = Particles.T66TravelerIndex;\n"
			"if (TravelerCount > 0)\n"
			"{\n"
			"    TravelerIndex = clamp(TravelerIndex, 0, TravelerCount - 1);\n"
			"}\n"
			"float3 TravelerPosition = Particles.Position;\n"
			"if (TravelerCount > 0)\n"
			"{\n"
			"    TravelerPositions.Get(TravelerIndex, TravelerPosition);\n"
			"}\n"
			"Particles.Position = TravelerPosition;\n"
			"Particles.Scale = float3(1.0, 1.0, 1.0);\n"
			"Particles.Color = float4(1.0, 0.30, 0.04, 1.0);\n");
	}

	FString T66ProductionTravelerHlsl(const bool bSpawnScript, const bool bDebugConstantVisible)
	{
		if (bDebugConstantVisible)
		{
			FString DebugHlsl = TEXT(
				"int TravelerIndex = T66_TRAVELER_INDEX_EXPR;\n"
				"int DebugColumn = TravelerIndex % 4;\n"
				"int DebugRow = TravelerIndex / 4;\n"
				"int bAlive = TravelerIndex < 16 ? 1 : 0;\n"
				"float DebugX = 20000.0 + ((float)DebugRow - 1.5) * 180.0;\n"
				"float DebugY = ((float)DebugColumn - 1.5) * 180.0;\n"
				"float DebugZ = 9000.0;\n"
				"int DebugMeshIndex = TravelerIndex + 4;\n"
				"float4 DebugColor = float4(1.0, 0.10 + 0.05 * (float)DebugColumn, 0.05 + 0.20 * (float)DebugRow, 1.0);\n"
				"Particles.T66TravelerIndex = TravelerIndex;\n"
				"Particles.Position = bAlive != 0 ? float3(DebugX, DebugY, DebugZ) : float3(0.0, 0.0, -100000.0);\n"
				"Particles.Scale = bAlive != 0 ? float3(250.0, 250.0, 250.0) : float3(0.0, 0.0, 0.0);\n"
				"Particles.Color = bAlive != 0 ? DebugColor : float4(1.0, 1.0, 1.0, 0.0);\n"
				"Particles.MeshIndex = 0;\n"
				"Particles.VisibilityTag = bAlive != 0 ? (DebugMeshIndex + 1) : 0;\n"
				"Particles.Lifetime = 600.0;\n");
			DebugHlsl.ReplaceInline(
				TEXT("T66_TRAVELER_INDEX_EXPR"),
				bSpawnScript ? TEXT("ExecIndex()") : TEXT("Particles.T66TravelerIndex"));
			return DebugHlsl;
		}

		FString Hlsl = TEXT(
			"int TravelerCount = max(0, TravelerLiveCount);\n"
			"int TravelerIndex = T66_TRAVELER_INDEX_EXPR;\n"
			"int bAlive = TravelerIndex < TravelerCount ? 1 : 0;\n"
			"float3 TravelerPosition = float3(0.0, 0.0, -100000.0);\n"
			"float4 TravelerRotation = float4(0.0, 0.0, 0.0, 1.0);\n"
			"float3 TravelerScale = float3(0.0, 0.0, 0.0);\n"
			"float4 TravelerColor = float4(1.0, 1.0, 1.0, 0.0);\n"
			"int TravelerMeshIndex = 0;\n"
			"if (bAlive != 0)\n"
			"{\n"
			"    TravelerPositions.Get(TravelerIndex, TravelerPosition);\n"
			"    int RotationCount = 0;\n"
			"    TravelerRotations.Length(RotationCount);\n"
			"    if (TravelerIndex < RotationCount)\n"
			"    {\n"
			"        TravelerRotations.Get(TravelerIndex, TravelerRotation);\n"
			"    }\n"
			"    int ScaleCount = 0;\n"
			"    TravelerScales.Length(ScaleCount);\n"
			"    if (TravelerIndex < ScaleCount)\n"
			"    {\n"
			"        TravelerScales.Get(TravelerIndex, TravelerScale);\n"
			"    }\n"
			"    int ColorCount = 0;\n"
			"    TravelerColors.Length(ColorCount);\n"
			"    if (TravelerIndex < ColorCount)\n"
			"    {\n"
			"        TravelerColors.Get(TravelerIndex, TravelerColor);\n"
			"    }\n"
			"    int MeshIndexCount = 0;\n"
			"    TravelerMeshIndices.Length(MeshIndexCount);\n"
			"    if (TravelerIndex < MeshIndexCount)\n"
			"    {\n"
			"        TravelerMeshIndices.Get(TravelerIndex, TravelerMeshIndex);\n"
			"    }\n"
			"}\n"
			"int TravelerFamilyTag = 1;\n"
			"if ((TravelerMeshIndex >= 8 && TravelerMeshIndex <= 11) || (TravelerMeshIndex >= 16 && TravelerMeshIndex <= 19))\n"
			"{\n"
			"    TravelerFamilyTag = 2;\n"
			"}\n"
			"Particles.T66TravelerIndex = TravelerIndex;\n"
			"Particles.Position = TravelerPosition;\n"
			"Particles.MeshOrientation = TravelerRotation;\n"
			"Particles.Scale = TravelerScale;\n"
			"Particles.Color = TravelerColor;\n"
			"Particles.MeshIndex = 0;\n"
			"Particles.VisibilityTag = bAlive != 0 ? (TravelerMeshIndex + 1) : 0;\n"
			"Particles.Lifetime = 600.0;\n");
		Hlsl.ReplaceInline(
			TEXT("T66_TRAVELER_INDEX_EXPR"),
			bSpawnScript ? TEXT("ExecIndex()") : TEXT("Particles.T66TravelerIndex"));
		return Hlsl;
	}

	bool T66AddArrayDrivenEmitter(UNiagaraSystem& System)
	{
		UNiagaraEmitter* Emitter = NewObject<UNiagaraEmitter>(
			GetTransientPackage(),
			T66OutgoingTravelerEmitterName,
			RF_Transactional);
		if (!Emitter)
		{
			return false;
		}

		Emitter->SetUniqueEmitterName(T66OutgoingTravelerEmitterName);
		UNiagaraEmitterFactoryNew::InitializeEmitter(Emitter, false);
		Emitter->SetUniqueEmitterName(T66OutgoingTravelerEmitterName);

		FVersionedNiagaraEmitterData* EmitterData = Emitter->GetLatestEmitterData();
		if (!EmitterData || !EmitterData->GraphSource)
		{
			return false;
		}

		EmitterData->SimTarget = ENiagaraSimTarget::GPUComputeSim;
		EmitterData->bLocalSpace = true;
		EmitterData->bDeterminism = true;
		EmitterData->bRequiresPersistentIDs = false;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		EmitterData->FixedBounds = FBox(FVector(-1400.0f, -900.0f, -80.0f), FVector(1400.0f, 900.0f, 80.0f));
		EmitterData->AllocationMode = EParticleAllocationMode::FixedCount;
		EmitterData->PreAllocationCount = T66OutgoingTravelerProofCount;

		UNiagaraNodeOutput* EmitterUpdateOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->EmitterUpdateScriptProps.Script);
		UNiagaraNodeOutput* ParticleSpawnOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->SpawnScriptProps.Script);
		UNiagaraNodeOutput* ParticleUpdateOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->UpdateScriptProps.Script);
		if (!EmitterUpdateOutputNode || !ParticleSpawnOutputNode || !ParticleUpdateOutputNode)
		{
			return false;
		}

		EmitterData->InterpolatedSpawnMode = ENiagaraInterpolatedSpawnMode::NoInterpolation;
		if (EmitterData->SpawnScriptProps.Script)
		{
			EmitterData->SpawnScriptProps.Script->SetUsage(ENiagaraScriptUsage::ParticleSpawnScript);
			ParticleSpawnOutputNode->SetUsage(ENiagaraScriptUsage::ParticleSpawnScript);
			ParticleSpawnOutputNode->SetUsageId(EmitterData->SpawnScriptProps.Script->GetUsageId());
		}

		T66AddNiagaraModuleFromAssetPath(TEXT("/Niagara/Modules/Emitter/EmitterState.EmitterState"), *EmitterUpdateOutputNode);
		UNiagaraNodeFunctionCall* SpawnBurstNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Emitter/SpawnBurst_Instantaneous.SpawnBurst_Instantaneous"),
			*EmitterUpdateOutputNode);
		if (!SpawnBurstNode || !EmitterData->EmitterUpdateScriptProps.Script)
		{
			return false;
		}

		if (!T66ConnectParameterToModuleInput(
			*EmitterUpdateOutputNode->GetNiagaraGraph(),
			*SpawnBurstNode,
			TEXT("Emitter.Age"),
			FNiagaraTypeDefinition::GetFloatDef(),
			TEXT("Age")))
		{
			return false;
		}

		if (!T66SetRapidIterationParameterResolved<int32>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Count"),
			FNiagaraTypeDefinition::GetIntDef(),
			T66OutgoingTravelerProofCount))
		{
			return false;
		}
		T66SetRapidIterationParameter<float>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Probability"),
			FNiagaraTypeDefinition::GetFloatDef(),
			1.0f);
		T66SetRapidIterationParameter<float>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Time"),
			FNiagaraTypeDefinition::GetFloatDef(),
			0.0f);

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Initialization/InitializeParticle.InitializeParticle"),
			*ParticleSpawnOutputNode);
		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Location/SystemLocation.SystemLocation"),
			*ParticleSpawnOutputNode);

		const TArray<FNiagaraVariable> SpawnVars =
		{
			SYS_PARAM_PARTICLES_LIFETIME,
			SYS_PARAM_PARTICLES_SCALE,
			SYS_PARAM_PARTICLES_COLOR,
			FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Particles.T66TravelerIndex"))
		};
		const TArray<FString> SpawnDefaults =
		{
			TEXT("600.0"),
			TEXT("X=1.0 Y=1.0 Z=1.0"),
			TEXT("(R=1.0,G=0.30,B=0.04,A=1.0)"),
			TEXT("0")
		};
		FNiagaraStackGraphUtilities::AddParameterModuleToStack(
			SpawnVars,
			*ParticleSpawnOutputNode,
			INDEX_NONE,
			SpawnDefaults);

		UNiagaraNodeCustomHlsl* SpawnArrayNode = T66CreateTravelerArrayModule(
			*ParticleSpawnOutputNode->GetNiagaraGraph(),
			TEXT("T66ArrayTravelerSpawn"),
			T66SpawnHlsl());
		if (!SpawnArrayNode || !T66InsertCustomHlslModuleAtStackEnd(*ParticleSpawnOutputNode, *SpawnArrayNode))
		{
			return false;
		}

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Update/Lifetime/UpdateAge.UpdateAge"),
			*ParticleUpdateOutputNode);
		UNiagaraNodeCustomHlsl* UpdateArrayNode = T66CreateTravelerArrayModule(
			*ParticleUpdateOutputNode->GetNiagaraGraph(),
			TEXT("T66ArrayTravelerUpdate"),
			T66UpdateHlsl());
		if (!UpdateArrayNode || !T66InsertCustomHlslModuleAtStackEnd(*ParticleUpdateOutputNode, *UpdateArrayNode))
		{
			return false;
		}

		UStaticMesh* ProofMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		UMaterialInterface* ProofMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
		if (!ProofMesh)
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Missing proof cube mesh."));
			return false;
		}

		UNiagaraMeshRendererProperties* MeshRenderer =
			NewObject<UNiagaraMeshRendererProperties>(Emitter, NAME_None, RF_Transactional);
		if (!MeshRenderer)
		{
			return false;
		}

		MeshRenderer->Modify();
		FNiagaraMeshRendererMeshProperties MeshProperties;
		MeshProperties.Mesh = ProofMesh;
		MeshProperties.Scale = FVector(0.06f, 0.06f, 0.06f);
		MeshProperties.Rotation = FRotator::ZeroRotator;
		MeshProperties.PivotOffset = FVector::ZeroVector;
		MeshRenderer->Meshes.Reset();
		MeshRenderer->Meshes.Add(MeshProperties);
		MeshRenderer->SourceMode = ENiagaraRendererSourceDataMode::Particles;
		MeshRenderer->FacingMode = ENiagaraMeshFacingMode::Default;
		MeshRenderer->bOverrideMaterials = ProofMaterial != nullptr;
		MeshRenderer->OverrideMaterials.Reset();
		if (ProofMaterial)
		{
			FNiagaraMeshMaterialOverride MaterialOverride;
			MaterialOverride.ExplicitMat = ProofMaterial;
			MeshRenderer->OverrideMaterials.Add(MaterialOverride);
		}
		MeshRenderer->bCastShadows = false;
		MeshRenderer->bEnableCameraDistanceCulling = false;
		MeshRenderer->bEnableFrustumCulling = false;
		MeshRenderer->SortMode = ENiagaraSortMode::None;
		MeshRenderer->SetIsEnabled(true);

		Emitter->AddRenderer(MeshRenderer, EmitterData->Version.VersionGuid);
		FNiagaraEditorUtilities::AddEmitterToSystem(System, *Emitter, FGuid(), true);
		return true;
	}

	bool T66AddProductionPoolEmitter(
		UNiagaraSystem& System,
		const bool bDebugConstantVisible,
		const bool bDebugProofRenderer)
	{
		UNiagaraEmitter* Emitter = NewObject<UNiagaraEmitter>(
			GetTransientPackage(),
			T66OutgoingTravelerProductionEmitterName,
			RF_Transactional);
		if (!Emitter)
		{
			return false;
		}

		Emitter->SetUniqueEmitterName(T66OutgoingTravelerProductionEmitterName);
		UNiagaraEmitterFactoryNew::InitializeEmitter(Emitter, false);
		Emitter->SetUniqueEmitterName(T66OutgoingTravelerProductionEmitterName);

		FVersionedNiagaraEmitterData* EmitterData = Emitter->GetLatestEmitterData();
		if (!EmitterData || !EmitterData->GraphSource)
		{
			return false;
		}

		EmitterData->InterpolatedSpawnMode = ENiagaraInterpolatedSpawnMode::NoInterpolation;
		if (EmitterData->SpawnScriptProps.Script)
		{
			EmitterData->SpawnScriptProps.Script->SetUsage(ENiagaraScriptUsage::ParticleSpawnScript);
		}
		EmitterData->SimTarget = ENiagaraSimTarget::GPUComputeSim;
		EmitterData->bLocalSpace = true;
		EmitterData->bDeterminism = true;
		EmitterData->bRequiresPersistentIDs = false;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		EmitterData->FixedBounds = FBox(FVector(-50000.0f, -50000.0f, -10000.0f), FVector(50000.0f, 50000.0f, 10000.0f));
		EmitterData->AllocationMode = EParticleAllocationMode::FixedCount;
		EmitterData->PreAllocationCount = T66OutgoingTravelerProductionCount;

		UNiagaraNodeOutput* EmitterUpdateOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->EmitterUpdateScriptProps.Script);
		UNiagaraNodeOutput* ParticleSpawnOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->SpawnScriptProps.Script);
		UNiagaraNodeOutput* ParticleUpdateOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->UpdateScriptProps.Script);
		if (!EmitterUpdateOutputNode || !ParticleSpawnOutputNode || !ParticleUpdateOutputNode)
		{
			return false;
		}
		UNiagaraNodeFunctionCall* EmitterStateNode =
			T66AddNiagaraModuleFromAssetPath(TEXT("/Niagara/Modules/Emitter/EmitterState.EmitterState"), *EmitterUpdateOutputNode);
		if (!EmitterStateNode || !EmitterData->EmitterUpdateScriptProps.Script)
		{
			return false;
		}
		T66SetRapidIterationParameter<float>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*EmitterStateNode,
			TEXT("Loop Duration"),
			FNiagaraTypeDefinition::GetFloatDef(),
			86400.0f);
		if (!T66SetRapidIterationEnumParameter(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*EmitterStateNode,
			TEXT("Life Cycle Mode"),
			TEXT("/Niagara/Enums/ENiagaraEmitterLifeCycleMode.ENiagaraEmitterLifeCycleMode"),
			1) ||
			!T66SetRapidIterationEnumParameter(
				Emitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*EmitterStateNode,
				TEXT("Loop Behavior"),
				TEXT("/Niagara/Enums/ENiagara_EmitterStateOptions.ENiagara_EmitterStateOptions"),
				1))
		{
			return false;
		}
		UNiagaraNodeFunctionCall* SpawnBurstNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Emitter/SpawnBurst_Instantaneous.SpawnBurst_Instantaneous"),
			*EmitterUpdateOutputNode);
		if (!SpawnBurstNode || !EmitterData->EmitterUpdateScriptProps.Script)
		{
			return false;
		}

		if (!T66ConnectParameterToModuleInput(
			*EmitterUpdateOutputNode->GetNiagaraGraph(),
			*SpawnBurstNode,
			TEXT("Emitter.Age"),
			FNiagaraTypeDefinition::GetFloatDef(),
			TEXT("Age")))
		{
			return false;
		}

		if (!T66SetRapidIterationParameterResolved<int32>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Count"),
			FNiagaraTypeDefinition::GetIntDef(),
			T66OutgoingTravelerProductionCount))
		{
			return false;
		}
		T66SetRapidIterationParameter<float>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Probability"),
			FNiagaraTypeDefinition::GetFloatDef(),
			1.0f);
		T66SetRapidIterationParameter<float>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Time"),
			FNiagaraTypeDefinition::GetFloatDef(),
			0.0f);

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Initialization/InitializeParticle.InitializeParticle"),
			*ParticleSpawnOutputNode);
		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Location/SystemLocation.SystemLocation"),
			*ParticleSpawnOutputNode);

		const TArray<FNiagaraVariable> SpawnVars =
		{
			SYS_PARAM_PARTICLES_LIFETIME,
			SYS_PARAM_PARTICLES_SCALE,
			SYS_PARAM_PARTICLES_COLOR,
			SYS_PARAM_PARTICLES_MESH_ORIENTATION,
			SYS_PARAM_PARTICLES_SPRITE_SIZE,
			SYS_PARAM_PARTICLES_MESH_INDEX,
			SYS_PARAM_PARTICLES_VISIBILITY_TAG,
			FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Particles.T66TravelerIndex"))
		};
		const TArray<FString> SpawnDefaults = bDebugConstantVisible
			? TArray<FString>
			{
				TEXT("600.0"),
				TEXT("X=20.0 Y=20.0 Z=20.0"),
				TEXT("(R=1.0,G=0.12,B=0.04,A=1.0)"),
				TEXT("X=0.0 Y=0.0 Z=0.0 W=1.0"),
				TEXT("X=150.0 Y=150.0"),
				TEXT("0"),
				TEXT("1"),
				TEXT("0")
			}
			: TArray<FString>
			{
				TEXT("600.0"),
				TEXT("X=0.0 Y=0.0 Z=0.0"),
				TEXT("(R=1.0,G=1.0,B=1.0,A=0.0)"),
				TEXT("X=0.0 Y=0.0 Z=0.0 W=1.0"),
				TEXT("X=0.0 Y=0.0"),
				TEXT("0"),
				TEXT("0"),
				TEXT("0")
			};
		FNiagaraStackGraphUtilities::AddParameterModuleToStack(
			SpawnVars,
			*ParticleSpawnOutputNode,
			INDEX_NONE,
			SpawnDefaults);

		UNiagaraNodeCustomHlsl* SpawnArrayNode = T66CreateTravelerArrayModule(
			*ParticleSpawnOutputNode->GetNiagaraGraph(),
			TEXT("T66OutgoingTravelerPoolSpawn"),
			T66ProductionTravelerHlsl(true, bDebugConstantVisible),
			!bDebugConstantVisible);
		if (!SpawnArrayNode || !T66InsertCustomHlslModuleAtStackEnd(*ParticleSpawnOutputNode, *SpawnArrayNode))
		{
			return false;
		}

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Update/Lifetime/UpdateAge.UpdateAge"),
			*ParticleUpdateOutputNode);
		UNiagaraNodeCustomHlsl* UpdateArrayNode = T66CreateTravelerArrayModule(
			*ParticleUpdateOutputNode->GetNiagaraGraph(),
			TEXT("T66OutgoingTravelerPoolUpdate"),
			T66ProductionTravelerHlsl(false, bDebugConstantVisible),
			!bDebugConstantVisible);
		if (!UpdateArrayNode || !T66InsertCustomHlslModuleAtStackEnd(*ParticleUpdateOutputNode, *UpdateArrayNode))
		{
			return false;
		}

		if (bDebugProofRenderer)
		{
			UStaticMesh* ProofMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			UMaterialInterface* ProofMaterial = LoadObject<UMaterialInterface>(
				nullptr,
				TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
			if (!ProofMesh)
			{
				UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Missing debug proof cube mesh."));
				return false;
			}

			UNiagaraMeshRendererProperties* MeshRenderer =
				NewObject<UNiagaraMeshRendererProperties>(Emitter, NAME_None, RF_Transactional);
			if (!MeshRenderer)
			{
				return false;
			}

			MeshRenderer->Modify();
			FNiagaraMeshRendererMeshProperties MeshProperties;
			MeshProperties.Mesh = ProofMesh;
			MeshProperties.Scale = FVector::OneVector;
			MeshProperties.Rotation = FRotator::ZeroRotator;
			MeshProperties.PivotOffset = FVector::ZeroVector;
			MeshRenderer->Meshes.Reset();
			MeshRenderer->Meshes.Add(MeshProperties);
			MeshRenderer->SourceMode = ENiagaraRendererSourceDataMode::Particles;
			MeshRenderer->FacingMode = ENiagaraMeshFacingMode::Default;
			MeshRenderer->PositionBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_POSITION);
			MeshRenderer->ColorBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_COLOR);
			MeshRenderer->MeshOrientationBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_MESH_ORIENTATION);
			MeshRenderer->ScaleBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_SCALE);
			MeshRenderer->bOverrideMaterials = ProofMaterial != nullptr;
			MeshRenderer->OverrideMaterials.Reset();
			if (ProofMaterial)
			{
				FNiagaraMeshMaterialOverride MaterialOverride;
				MaterialOverride.ExplicitMat = ProofMaterial;
				MeshRenderer->OverrideMaterials.Add(MaterialOverride);
			}
			MeshRenderer->bCastShadows = false;
			MeshRenderer->bEnableCameraDistanceCulling = false;
			MeshRenderer->bEnableFrustumCulling = false;
			MeshRenderer->SortMode = ENiagaraSortMode::ViewDepth;
			MeshRenderer->SetIsEnabled(true);

			Emitter->AddRenderer(MeshRenderer, EmitterData->Version.VersionGuid);

			UMaterialInterface* SpriteMaterial = LoadObject<UMaterialInterface>(
				nullptr,
				TEXT("/Niagara/DefaultAssets/DefaultSpriteMaterial.DefaultSpriteMaterial"));
			UNiagaraSpriteRendererProperties* SpriteRenderer =
				NewObject<UNiagaraSpriteRendererProperties>(Emitter, NAME_None, RF_Transactional);
			if (!SpriteRenderer)
			{
				return false;
			}

			SpriteRenderer->Modify();
			SpriteRenderer->Material = SpriteMaterial;
			SpriteRenderer->SourceMode = ENiagaraRendererSourceDataMode::Particles;
			SpriteRenderer->Alignment = ENiagaraSpriteAlignment::Unaligned;
			SpriteRenderer->FacingMode = ENiagaraSpriteFacingMode::FaceCamera;
			SpriteRenderer->PositionBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_POSITION);
			SpriteRenderer->ColorBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_COLOR);
			SpriteRenderer->SpriteSizeBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_SPRITE_SIZE);
			SpriteRenderer->bCastShadows = false;
			SpriteRenderer->bEnableCameraDistanceCulling = false;
			SpriteRenderer->SortMode = ENiagaraSortMode::None;
			SpriteRenderer->SetIsEnabled(true);

			Emitter->AddRenderer(SpriteRenderer, EmitterData->Version.VersionGuid);
			FNiagaraEditorUtilities::AddEmitterToSystem(System, *Emitter, FGuid(), true);
			UE_LOG(LogTemp, Display, TEXT("[OutgoingTravelerSwarmVFX] Added debug proof cube and sprite renderers."));
			return true;
		}

		UMaterialInterface* AdditiveMaterial = nullptr;
		UMaterialInterface* TranslucentMaterial = nullptr;
		if (!T66CreateTravelerFamilyMaterials(AdditiveMaterial, TranslucentMaterial))
		{
			return false;
		}

		TArray<FT66TravelerMeshSlotDefinition> MeshSlots;
		if (!T66BuildTravelerProfileMeshSlots(AdditiveMaterial, TranslucentMaterial, MeshSlots))
		{
			return false;
		}

		for (int32 SlotIndex = 0; SlotIndex < MeshSlots.Num(); ++SlotIndex)
		{
			const FT66TravelerMeshSlotDefinition& MeshSlot = MeshSlots[SlotIndex];
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshSlot.MeshPath);
			if (!Mesh)
			{
				UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Missing production mesh %s"), *MeshSlot.MeshPath);
				return false;
			}
			UMaterialInterface* SlotMaterial = Mesh->GetMaterial(0);
			if (!SlotMaterial)
			{
				UE_LOG(LogTemp, Error,
					TEXT("[OutgoingTravelerSwarmVFX] Missing production material for mesh %s"),
					*MeshSlot.MeshPath);
				return false;
			}

			const FName RendererName(*FString::Printf(TEXT("TravelerVisualSlotRenderer_%02d"), SlotIndex));
			UNiagaraMeshRendererProperties* MeshRenderer =
				NewObject<UNiagaraMeshRendererProperties>(Emitter, RendererName, RF_Transactional);
			if (!MeshRenderer)
			{
				return false;
			}

			MeshRenderer->Modify();
			FNiagaraMeshRendererMeshProperties MeshProperties;
			MeshProperties.Mesh = Mesh;
			MeshProperties.Scale = MeshSlot.Scale;
			MeshProperties.Rotation = MeshSlot.Rotation;
			MeshProperties.PivotOffset = FVector::ZeroVector;
			MeshRenderer->Meshes.Reset();
			MeshRenderer->Meshes.Add(MeshProperties);
			MeshRenderer->SourceMode = ENiagaraRendererSourceDataMode::Particles;
			MeshRenderer->FacingMode = ENiagaraMeshFacingMode::Default;
			MeshRenderer->PositionBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_POSITION);
			MeshRenderer->ColorBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_COLOR);
			MeshRenderer->MeshOrientationBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_MESH_ORIENTATION);
			MeshRenderer->ScaleBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_SCALE);
			MeshRenderer->RendererVisibilityTagBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_VISIBILITY_TAG);
			MeshRenderer->RendererVisibility = SlotIndex + 1;
			MeshRenderer->bOverrideMaterials = true;
			MeshRenderer->OverrideMaterials.Reset();
			FNiagaraMeshMaterialOverride& MaterialOverride = MeshRenderer->OverrideMaterials.AddDefaulted_GetRef();
			MaterialOverride.ExplicitMat = SlotMaterial;
			MeshRenderer->bCastShadows = false;
			MeshRenderer->bEnableCameraDistanceCulling = false;
			MeshRenderer->bEnableFrustumCulling = false;
			MeshRenderer->SortMode = ENiagaraSortMode::ViewDepth;
			MeshRenderer->bSortOnlyWhenTranslucent = true;
			MeshRenderer->SetIsEnabled(true);

			Emitter->AddRenderer(MeshRenderer, EmitterData->Version.VersionGuid);
		}
		UE_LOG(LogTemp, Display,
			TEXT("[OutgoingTravelerSwarmVFX] Added traveler visibility-tag renderers slot_count=%d material_source=slot_mesh_materials"),
			MeshSlots.Num());
		FNiagaraEditorUtilities::AddEmitterToSystem(System, *Emitter, FGuid(), true);
		return true;
	}

	bool T66AddMobLootArrayInputs(
		UNiagaraGraph& Graph,
		UNiagaraNodeCustomHlsl& CustomNode)
	{
		return
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("MobLootPositions"),
				T66MobLootPositionsParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat3::StaticClass()),
				UNiagaraDataInterfaceArrayFloat3::StaticClass()) &&
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("MobLootScales"),
				T66MobLootScalesParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat::StaticClass()),
				UNiagaraDataInterfaceArrayFloat::StaticClass()) &&
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("MobLootColors"),
				T66MobLootColorsParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayColor::StaticClass()),
				UNiagaraDataInterfaceArrayColor::StaticClass()) &&
			T66AddArrayDataInterfaceInput(
				Graph,
				CustomNode,
				TEXT("MobLootQuantities"),
				T66MobLootQuantitiesParameter,
				FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayInt32::StaticClass()),
				UNiagaraDataInterfaceArrayInt32::StaticClass());
	}

	UNiagaraNodeCustomHlsl* T66CreateMobLootArrayModule(
		UNiagaraGraph& Graph,
		const TCHAR* NodeName,
		const FString& Hlsl)
	{
		FGraphNodeCreator<UNiagaraNodeCustomHlsl> NodeCreator(Graph);
		UNiagaraNodeCustomHlsl* CustomNode = NodeCreator.CreateNode();
		if (!CustomNode)
		{
			return nullptr;
		}

		CustomNode->ScriptUsage = ENiagaraScriptUsage::Module;
		CustomNode->Signature.Name = FName(NodeName);
		if (!T66SetReflectedString(*CustomNode, TEXT("FunctionDisplayName"), NodeName))
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to name Mob Loot custom HLSL node %s"), NodeName);
			return nullptr;
		}
		T66CreateNiagaraTypedPin(*CustomNode, EGPD_Input, FNiagaraTypeDefinition::GetParameterMapDef(), TEXT("InputMap"));
		T66CreateNiagaraTypedPin(*CustomNode, EGPD_Output, FNiagaraTypeDefinition::GetParameterMapDef(), TEXT("OutputMap"));
		if (!T66SetReflectedString(*CustomNode, TEXT("CustomHlsl"), Hlsl))
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to assign Mob Loot custom HLSL for %s"), NodeName);
			return nullptr;
		}
		NodeCreator.Finalize();

		if (!T66AddMobLootArrayInputs(Graph, *CustomNode))
		{
			UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to add Mob Loot array inputs for %s"), NodeName);
			return nullptr;
		}

		T66RebuildCustomHlslSignatureFromPins(*CustomNode);
		return CustomNode;
	}

	FString T66MobLootPoolHlsl(const bool bSpawnScript)
	{
		FString Hlsl = TEXT(
			"int MobLootCount = 0;\n"
			"MobLootPositions.Length(MobLootCount);\n"
			"int MobLootIndex = T66_MOB_LOOT_INDEX_EXPR;\n"
			"int bAlive = MobLootIndex < MobLootCount ? 1 : 0;\n"
			"float3 LootPosition = float3(0.0, 0.0, -100000.0);\n"
			"float LootScale = 0.0;\n"
			"float4 LootColor = float4(1.0, 0.74, 0.18, 0.0);\n"
			"int LootQuantity = 0;\n"
			"if (bAlive != 0)\n"
			"{\n"
			"    MobLootPositions.Get(MobLootIndex, LootPosition);\n"
			"    int ScaleCount = 0;\n"
			"    MobLootScales.Length(ScaleCount);\n"
			"    if (MobLootIndex < ScaleCount)\n"
			"    {\n"
			"        MobLootScales.Get(MobLootIndex, LootScale);\n"
			"    }\n"
			"    int ColorCount = 0;\n"
			"    MobLootColors.Length(ColorCount);\n"
			"    if (MobLootIndex < ColorCount)\n"
			"    {\n"
			"        MobLootColors.Get(MobLootIndex, LootColor);\n"
			"    }\n"
			"    int QuantityCount = 0;\n"
			"    MobLootQuantities.Length(QuantityCount);\n"
			"    if (MobLootIndex < QuantityCount)\n"
			"    {\n"
			"        MobLootQuantities.Get(MobLootIndex, LootQuantity);\n"
			"    }\n"
			"}\n"
			"float QuantityScale = min((float)LootQuantity, 10.0) * 2.0;\n"
			"float SpriteSize = bAlive != 0 ? max(12.0, 38.0 * max(LootScale, 0.01) + QuantityScale) : 0.0;\n"
			"Particles.T66MobLootIndex = MobLootIndex;\n"
			"Particles.Position = LootPosition;\n"
			"Particles.Color = LootColor;\n"
			"Particles.SpriteSize = float2(SpriteSize, SpriteSize);\n"
			"Particles.VisibilityTag = bAlive;\n"
			"Particles.Lifetime = 600.0;\n");
		Hlsl.ReplaceInline(
			TEXT("T66_MOB_LOOT_INDEX_EXPR"),
			bSpawnScript ? TEXT("ExecIndex()") : TEXT("Particles.T66MobLootIndex"));
		return Hlsl;
	}

	bool T66AddMobLootPoolEmitter(UNiagaraSystem& System)
	{
		UNiagaraEmitter* Emitter = NewObject<UNiagaraEmitter>(
			GetTransientPackage(),
			T66MobLootEmitterName,
			RF_Transactional);
		if (!Emitter)
		{
			return false;
		}

		Emitter->SetUniqueEmitterName(T66MobLootEmitterName);
		UNiagaraEmitterFactoryNew::InitializeEmitter(Emitter, false);
		Emitter->SetUniqueEmitterName(T66MobLootEmitterName);

		FVersionedNiagaraEmitterData* EmitterData = Emitter->GetLatestEmitterData();
		if (!EmitterData || !EmitterData->GraphSource)
		{
			return false;
		}

		EmitterData->InterpolatedSpawnMode = ENiagaraInterpolatedSpawnMode::NoInterpolation;
		if (EmitterData->SpawnScriptProps.Script)
		{
			EmitterData->SpawnScriptProps.Script->SetUsage(ENiagaraScriptUsage::ParticleSpawnScript);
		}
		EmitterData->SimTarget = ENiagaraSimTarget::GPUComputeSim;
		EmitterData->bLocalSpace = false;
		EmitterData->bDeterminism = true;
		EmitterData->bRequiresPersistentIDs = false;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		EmitterData->FixedBounds = FBox(FVector(-50000.0f, -50000.0f, -10000.0f), FVector(50000.0f, 50000.0f, 10000.0f));
		EmitterData->AllocationMode = EParticleAllocationMode::FixedCount;
		EmitterData->PreAllocationCount = T66MobLootProductionCount;

		UNiagaraNodeOutput* EmitterUpdateOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->EmitterUpdateScriptProps.Script);
		UNiagaraNodeOutput* ParticleSpawnOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->SpawnScriptProps.Script);
		UNiagaraNodeOutput* ParticleUpdateOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->UpdateScriptProps.Script);
		if (!EmitterUpdateOutputNode || !ParticleSpawnOutputNode || !ParticleUpdateOutputNode)
		{
			return false;
		}
		if (EmitterData->EmitterUpdateScriptProps.Script)
		{
			EmitterData->EmitterUpdateScriptProps.Script->SetUsage(ENiagaraScriptUsage::EmitterUpdateScript);
			EmitterUpdateOutputNode->SetUsage(ENiagaraScriptUsage::EmitterUpdateScript);
			EmitterUpdateOutputNode->SetUsageId(EmitterData->EmitterUpdateScriptProps.Script->GetUsageId());
		}
		if (EmitterData->SpawnScriptProps.Script)
		{
			EmitterData->SpawnScriptProps.Script->SetUsage(ENiagaraScriptUsage::ParticleSpawnScript);
			ParticleSpawnOutputNode->SetUsage(ENiagaraScriptUsage::ParticleSpawnScript);
			ParticleSpawnOutputNode->SetUsageId(EmitterData->SpawnScriptProps.Script->GetUsageId());
		}
		if (EmitterData->UpdateScriptProps.Script)
		{
			EmitterData->UpdateScriptProps.Script->SetUsage(ENiagaraScriptUsage::ParticleUpdateScript);
			ParticleUpdateOutputNode->SetUsage(ENiagaraScriptUsage::ParticleUpdateScript);
			ParticleUpdateOutputNode->SetUsageId(EmitterData->UpdateScriptProps.Script->GetUsageId());
		}

		UNiagaraNodeFunctionCall* EmitterStateNode =
			T66AddNiagaraModuleFromAssetPath(TEXT("/Niagara/Modules/Emitter/EmitterState.EmitterState"), *EmitterUpdateOutputNode);
		if (!EmitterStateNode || !EmitterData->EmitterUpdateScriptProps.Script)
		{
			return false;
		}
		T66SetRapidIterationParameter<float>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*EmitterStateNode,
			TEXT("Loop Duration"),
			FNiagaraTypeDefinition::GetFloatDef(),
			86400.0f);
		if (!T66SetRapidIterationEnumParameter(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*EmitterStateNode,
			TEXT("Life Cycle Mode"),
			TEXT("/Niagara/Enums/ENiagaraEmitterLifeCycleMode.ENiagaraEmitterLifeCycleMode"),
			1) ||
			!T66SetRapidIterationEnumParameter(
				Emitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*EmitterStateNode,
				TEXT("Loop Behavior"),
				TEXT("/Niagara/Enums/ENiagara_EmitterStateOptions.ENiagara_EmitterStateOptions"),
				1))
		{
			return false;
		}
		UNiagaraNodeFunctionCall* SpawnBurstNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Emitter/SpawnBurst_Instantaneous.SpawnBurst_Instantaneous"),
			*EmitterUpdateOutputNode);
		if (!SpawnBurstNode || !EmitterData->EmitterUpdateScriptProps.Script)
		{
			return false;
		}

		if (!T66SetRapidIterationParameterResolved<int32>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Count"),
			FNiagaraTypeDefinition::GetIntDef(),
			T66MobLootProductionCount))
		{
			return false;
		}
		T66SetRapidIterationParameter<float>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Probability"),
			FNiagaraTypeDefinition::GetFloatDef(),
			1.0f);
		T66SetRapidIterationParameter<float>(
			Emitter->GetUniqueEmitterName(),
			*EmitterData->EmitterUpdateScriptProps.Script,
			*SpawnBurstNode,
			TEXT("Spawn Time"),
			FNiagaraTypeDefinition::GetFloatDef(),
			0.0f);

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Initialization/InitializeParticle.InitializeParticle"),
			*ParticleSpawnOutputNode);
		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Location/SystemLocation.SystemLocation"),
			*ParticleSpawnOutputNode);

		const TArray<FNiagaraVariable> SpawnVars =
		{
			SYS_PARAM_PARTICLES_LIFETIME,
			SYS_PARAM_PARTICLES_COLOR,
			SYS_PARAM_PARTICLES_SPRITE_SIZE,
			SYS_PARAM_PARTICLES_VISIBILITY_TAG,
			FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Particles.T66MobLootIndex"))
		};
		const TArray<FString> SpawnDefaults =
		{
			TEXT("600.0"),
			TEXT("(R=1.0,G=0.74,B=0.18,A=0.0)"),
			TEXT("X=0.0 Y=0.0"),
			TEXT("0"),
			TEXT("0")
		};
		FNiagaraStackGraphUtilities::AddParameterModuleToStack(
			SpawnVars,
			*ParticleSpawnOutputNode,
			INDEX_NONE,
			SpawnDefaults);

		UNiagaraNodeCustomHlsl* SpawnArrayNode = T66CreateMobLootArrayModule(
			*ParticleSpawnOutputNode->GetNiagaraGraph(),
			TEXT("T66MobLootPoolSpawn"),
			T66MobLootPoolHlsl(true));
		if (!SpawnArrayNode || !T66InsertCustomHlslModuleAtStackEnd(*ParticleSpawnOutputNode, *SpawnArrayNode))
		{
			return false;
		}

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Update/Lifetime/UpdateAge.UpdateAge"),
			*ParticleUpdateOutputNode);
		UNiagaraNodeCustomHlsl* UpdateArrayNode = T66CreateMobLootArrayModule(
			*ParticleUpdateOutputNode->GetNiagaraGraph(),
			TEXT("T66MobLootPoolUpdate"),
			T66MobLootPoolHlsl(false));
		if (!UpdateArrayNode || !T66InsertCustomHlslModuleAtStackEnd(*ParticleUpdateOutputNode, *UpdateArrayNode))
		{
			return false;
		}

		UMaterialInterface* SpriteMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Niagara/DefaultAssets/DefaultSpriteMaterial.DefaultSpriteMaterial"));

		UNiagaraSpriteRendererProperties* SpriteRenderer =
			NewObject<UNiagaraSpriteRendererProperties>(Emitter, NAME_None, RF_Transactional);
		if (!SpriteRenderer)
		{
			return false;
		}

		SpriteRenderer->Modify();
		SpriteRenderer->Material = SpriteMaterial;
		SpriteRenderer->SourceMode = ENiagaraRendererSourceDataMode::Particles;
		SpriteRenderer->Alignment = ENiagaraSpriteAlignment::Unaligned;
		SpriteRenderer->FacingMode = ENiagaraSpriteFacingMode::FaceCamera;
		SpriteRenderer->SortMode = ENiagaraSortMode::None;
		SpriteRenderer->bCastShadows = false;
		SpriteRenderer->bEnableCameraDistanceCulling = false;
		SpriteRenderer->PositionBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_POSITION);
		SpriteRenderer->ColorBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_COLOR);
		SpriteRenderer->SpriteSizeBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_SPRITE_SIZE);
		SpriteRenderer->RendererVisibilityTagBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_VISIBILITY_TAG);
		SpriteRenderer->RendererVisibility = 1;
		SpriteRenderer->SetIsEnabled(true);

		Emitter->AddRenderer(SpriteRenderer, EmitterData->Version.VersionGuid);
		FNiagaraEditorUtilities::AddEmitterToSystem(System, *Emitter, FGuid(), true);
		return true;
	}

	UNiagaraSystem* T66CreateTravelerSwarmSystem()
	{
		UPackage* Package = CreatePackage(T66OutgoingTravelerSystemPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		if (UNiagaraSystem* ExistingSystem = FindObject<UNiagaraSystem>(Package, T66OutgoingTravelerSystemObjectName))
		{
			ExistingSystem->ClearFlags(RF_Public | RF_Standalone);
			ExistingSystem->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}

		UNiagaraSystem* System = NewObject<UNiagaraSystem>(
			Package,
			FName(T66OutgoingTravelerSystemObjectName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!System)
		{
			return nullptr;
		}

		UNiagaraSystemFactoryNew::InitializeSystem(System, true);
		System->Modify();
		System->SetFixedBounds(FBox(FVector(-1400.0f, -900.0f, -80.0f), FVector(1400.0f, 900.0f, 80.0f)));

		const FNiagaraTypeDefinition ArrayType(UNiagaraDataInterfaceArrayFloat3::StaticClass());
		const FNiagaraVariable ArrayParameter(ArrayType, T66OutgoingTravelerArrayParameter);
		System->GetExposedParameters().AddParameter(ArrayParameter, true, true);
		System->GetExposedParameters().SetDataInterface(
			NewObject<UNiagaraDataInterfaceArrayFloat3>(System, NAME_None, RF_Transactional),
			ArrayParameter);

		if (!T66AddArrayDrivenEmitter(*System))
		{
			return nullptr;
		}

		FAssetRegistryModule::AssetCreated(System);
		return System;
	}

	void T66AddSystemArrayParameter(
		UNiagaraSystem& System,
		const TCHAR* ParameterName,
		const FNiagaraTypeDefinition& ArrayType,
		UClass* DataInterfaceClass)
	{
		const FNiagaraVariable ArrayParameter(ArrayType, ParameterName);
		System.GetExposedParameters().AddParameter(ArrayParameter, true, true);
		System.GetExposedParameters().SetDataInterface(
			DataInterfaceClass ? Cast<UNiagaraDataInterface>(NewObject<UObject>(&System, DataInterfaceClass, NAME_None, RF_Transactional)) : nullptr,
			ArrayParameter);
	}

	UNiagaraSystem* T66CreateTravelerProductionPoolSystem(
		const bool bDebugConstantVisible,
		const bool bDebugProofRenderer)
	{
		UPackage* Package = CreatePackage(T66OutgoingTravelerProductionSystemPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		if (UNiagaraSystem* ExistingSystem = FindObject<UNiagaraSystem>(Package, T66OutgoingTravelerProductionSystemObjectName))
		{
			ExistingSystem->ClearFlags(RF_Public | RF_Standalone);
			ExistingSystem->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}

		UNiagaraSystem* System = NewObject<UNiagaraSystem>(
			Package,
			FName(T66OutgoingTravelerProductionSystemObjectName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!System)
		{
			return nullptr;
		}

		UNiagaraSystemFactoryNew::InitializeSystem(System, true);
		System->Modify();
		System->SetFixedBounds(FBox(FVector(-50000.0f, -50000.0f, -10000.0f), FVector(50000.0f, 50000.0f, 10000.0f)));

		T66AddSystemArrayParameter(
			*System,
			T66OutgoingTravelerArrayParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat3::StaticClass()),
			UNiagaraDataInterfaceArrayFloat3::StaticClass());
		T66AddSystemArrayParameter(
			*System,
			T66OutgoingTravelerRotationsParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayQuat::StaticClass()),
			UNiagaraDataInterfaceArrayQuat::StaticClass());
		T66AddSystemArrayParameter(
			*System,
			T66OutgoingTravelerScalesParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat3::StaticClass()),
			UNiagaraDataInterfaceArrayFloat3::StaticClass());
		T66AddSystemArrayParameter(
			*System,
			T66OutgoingTravelerColorsParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayColor::StaticClass()),
			UNiagaraDataInterfaceArrayColor::StaticClass());
		T66AddSystemArrayParameter(
			*System,
			T66OutgoingTravelerMeshIndicesParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayInt32::StaticClass()),
			UNiagaraDataInterfaceArrayInt32::StaticClass());
		System->GetExposedParameters().AddParameter(
			FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), T66OutgoingTravelerLiveCountParameter),
			true,
			true);

		if (!T66AddProductionPoolEmitter(*System, bDebugConstantVisible, bDebugProofRenderer))
		{
			return nullptr;
		}

		FAssetRegistryModule::AssetCreated(System);
		return System;
	}

	UNiagaraSystem* T66CreateMobLootPoolSystem()
	{
		UPackage* Package = CreatePackage(T66MobLootSystemPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		if (UNiagaraSystem* ExistingSystem = FindObject<UNiagaraSystem>(Package, T66MobLootSystemObjectName))
		{
			ExistingSystem->ClearFlags(RF_Public | RF_Standalone);
			ExistingSystem->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}

		UNiagaraSystem* System = NewObject<UNiagaraSystem>(
			Package,
			FName(T66MobLootSystemObjectName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!System)
		{
			return nullptr;
		}

		UNiagaraSystemFactoryNew::InitializeSystem(System, true);
		System->Modify();
		System->SetFixedBounds(FBox(FVector(-50000.0f, -50000.0f, -10000.0f), FVector(50000.0f, 50000.0f, 10000.0f)));

		T66AddSystemArrayParameter(
			*System,
			T66MobLootPositionsParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat3::StaticClass()),
			UNiagaraDataInterfaceArrayFloat3::StaticClass());
		T66AddSystemArrayParameter(
			*System,
			T66MobLootScalesParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayFloat::StaticClass()),
			UNiagaraDataInterfaceArrayFloat::StaticClass());
		T66AddSystemArrayParameter(
			*System,
			T66MobLootColorsParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayColor::StaticClass()),
			UNiagaraDataInterfaceArrayColor::StaticClass());
		T66AddSystemArrayParameter(
			*System,
			T66MobLootQuantitiesParameter,
			FNiagaraTypeDefinition(UNiagaraDataInterfaceArrayInt32::StaticClass()),
			UNiagaraDataInterfaceArrayInt32::StaticClass());
		System->GetExposedParameters().AddParameter(
			FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), T66MobLootLiveCountParameter),
			true,
			true);

		if (!T66AddMobLootPoolEmitter(*System))
		{
			return nullptr;
		}

		FAssetRegistryModule::AssetCreated(System);
		return System;
	}
#endif
}

UT66OutgoingTravelerSwarmVFXCommandlet::UT66OutgoingTravelerSwarmVFXCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UT66OutgoingTravelerSwarmVFXCommandlet::Main(const FString& Params)
{
	const bool bMobLootPool = FParse::Param(*Params, TEXT("MobLootPool"));
	const bool bProductionPool = FParse::Param(*Params, TEXT("ProductionPool"));
	const bool bDebugConstantVisible = FParse::Param(*Params, TEXT("DebugConstantVisible"));
	const bool bDebugProofRenderer = FParse::Param(*Params, TEXT("DebugProofRenderer"));
	const TCHAR* TargetDescription = bMobLootPool
		? TEXT("production Mob Loot pool")
		: (bProductionPool ? TEXT("production outgoing traveler pool") : TEXT("isolated Phase 0 proof"));
	const TCHAR* TargetSystemPath = bMobLootPool
		? T66MobLootSystemObjectPath
		: (bProductionPool ? T66OutgoingTravelerProductionSystemObjectPath : T66OutgoingTravelerSystemObjectPath);
	const int32 TargetCount = bMobLootPool
		? T66MobLootProductionCount
		: (bProductionPool ? T66OutgoingTravelerProductionCount : T66OutgoingTravelerProofCount);

	UE_LOG(LogTemp, Display, TEXT("[OutgoingTravelerSwarmVFX] Building %s array-fed Niagara system. Params=%s"),
		TargetDescription,
		*Params);
	UE_LOG(LogTemp, Display, TEXT("[OutgoingTravelerSwarmVFX] TargetSystem=%s FixedCount=%d MaxProjectiles untouched by this commandlet."),
		TargetSystemPath,
		TargetCount);

#if WITH_EDITOR
	UNiagaraSystem* System = bMobLootPool
		? T66CreateMobLootPoolSystem()
		: (bProductionPool ? T66CreateTravelerProductionPoolSystem(bDebugConstantVisible, bDebugProofRenderer) : T66CreateTravelerSwarmSystem());
#else
	UNiagaraSystem* System = LoadObject<UNiagaraSystem>(
		nullptr,
		TargetSystemPath);
#endif
	if (!System)
	{
		UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to create or load %s"),
			TargetSystemPath);
		return 1;
	}

#if WITH_EDITOR
	System->PostEditChange();
	System->RequestCompile(true);
#endif

	if (!T66SaveAsset(System))
	{
		UE_LOG(LogTemp, Error, TEXT("[OutgoingTravelerSwarmVFX] Failed to save %s"), *System->GetPathName());
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("[OutgoingTravelerSwarmVFX] Saved %s"), *System->GetPathName());
	return 0;
}
