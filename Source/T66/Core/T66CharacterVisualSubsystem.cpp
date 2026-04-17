// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66CharacterVisuals, Log, All);

static const TCHAR* T66_DefaultCharacterVisualsDTPath = TEXT("/Game/Data/DT_CharacterVisuals.DT_CharacterVisuals");
static const FName T66_AnimSkeletonTag(TEXT("Skeleton"));
static const FName T66_CharactersRootPath(TEXT("/Game/Characters"));
static const TCHAR* T66_CharacterBaseMaterialPath = TEXT("/Game/Materials/M_Character_Unlit.M_Character_Unlit");
static const TCHAR* T66_FbxBaseMaterialPath = TEXT("/Game/Materials/M_FBX_Unlit.M_FBX_Unlit");

struct FT66ResolvedImportedTextureSet
{
	TWeakObjectPtr<UTexture> DiffuseTexture;
	TWeakObjectPtr<UTexture> NormalTexture;
};

static UTexture* GetWhiteFallbackTexture()
{
	static TWeakObjectPtr<UTexture> CachedTexture;
	if (CachedTexture.IsValid())
	{
		return CachedTexture.Get();
	}

	UTexture* LoadedTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
	if (!LoadedTexture)
	{
		LoadedTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
	}

	CachedTexture = LoadedTexture;
	return LoadedTexture;
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

static void T66AppendCharacterVisualAssetPaths(const FT66CharacterVisualRow& Row, TArray<FSoftObjectPath>& OutPaths)
{
	auto AddPath = [&OutPaths](const FSoftObjectPath& Path)
	{
		if (!Path.IsNull())
		{
			OutPaths.AddUnique(Path);
		}
	};

	AddPath(Row.SkeletalMesh.ToSoftObjectPath());
	AddPath(Row.LoopingAnimation.ToSoftObjectPath());
	AddPath(Row.AlertAnimation.ToSoftObjectPath());
	AddPath(Row.RunAnimation.ToSoftObjectPath());
}

static bool T66IsUsableImportedTexture(const UTexture* Texture)
{
	return IsValid(Texture) && Texture != GetWhiteFallbackTexture();
}

static bool T66ShouldRebuildImportedCharacterMaterial(const UMaterialInterface* Material)
{
	const FString BasePath = GetMaterialBasePath(Material);
	return BasePath.Equals(T66_CharacterBaseMaterialPath) || BasePath.Equals(T66_FbxBaseMaterialPath);
}

static void T66GetPreferredTextureNamesForMaterial(const FString& MaterialName, FString& OutDiffuseName, FString& OutNormalName)
{
	OutDiffuseName.Reset();
	OutNormalName.Reset();

	if (MaterialName.StartsWith(TEXT("Material_0_")))
	{
		const FString Suffix = MaterialName.RightChop(11);
		OutDiffuseName = FString::Printf(TEXT("Image_0_%s"), *Suffix);
		OutNormalName = FString::Printf(TEXT("Image_2_%s"), *Suffix);
		return;
	}

	if (MaterialName.StartsWith(TEXT("Material_")))
	{
		OutDiffuseName = TEXT("Image_0");
		OutNormalName = TEXT("Image_2");
	}
}

static int32 T66ScoreTextureAssetForMaterial(const FString& AssetNameLower, const FString& PreferredNameLower, bool bNormalTexture)
{
	int32 Score = 0;

	if (!PreferredNameLower.IsEmpty())
	{
		if (AssetNameLower == PreferredNameLower)
		{
			Score += 1000;
		}
		else if (AssetNameLower.StartsWith(PreferredNameLower))
		{
			Score += 700;
		}
	}

	const bool bLooksNormal = AssetNameLower.Contains(TEXT("normal")) || AssetNameLower.Contains(TEXT("image_2"));
	const bool bLooksDiffuse = AssetNameLower.Contains(TEXT("image_0"))
		|| AssetNameLower.Contains(TEXT("basecolor"))
		|| AssetNameLower.Contains(TEXT("albedo"))
		|| AssetNameLower.Contains(TEXT("diffuse"))
		|| AssetNameLower.Contains(TEXT("texture_pbr"));
	const bool bLooksAuxiliary = AssetNameLower.Contains(TEXT("rough"))
		|| AssetNameLower.Contains(TEXT("metal"))
		|| AssetNameLower.Contains(TEXT("spec"));

	if (bNormalTexture)
	{
		if (bLooksNormal)
		{
			Score += 350;
		}
		if (bLooksDiffuse)
		{
			Score -= 250;
		}
		if (bLooksAuxiliary)
		{
			Score -= 100;
		}
	}
	else
	{
		if (bLooksDiffuse)
		{
			Score += 300;
		}
		if (bLooksNormal || bLooksAuxiliary)
		{
			Score -= 350;
		}
	}

	return Score;
}

static void T66TryReadTextureParameter(
	const UMaterialInterface* SourceMaterial,
	const TCHAR* ParameterName,
	TWeakObjectPtr<UTexture>& OutTexture)
{
	if (OutTexture.IsValid() || !SourceMaterial || !ParameterName)
	{
		return;
	}

	UTexture* Value = nullptr;
	if (SourceMaterial->GetTextureParameterValue(FHashedMaterialParameterInfo(FMaterialParameterInfo(ParameterName)), Value)
		&& T66IsUsableImportedTexture(Value))
	{
		OutTexture = Value;
	}
}

static FT66ResolvedImportedTextureSet T66ExtractImportedCharacterTexturesFromMaterial(const UMaterialInterface* SourceMaterial)
{
	FT66ResolvedImportedTextureSet ExtractedTextures;
	if (!SourceMaterial)
	{
		return ExtractedTextures;
	}

	static const TCHAR* DiffuseParameterNames[] = {
		TEXT("DiffuseColorMap"),
		TEXT("BaseColorTexture"),
		TEXT("BaseColorMap"),
		TEXT("DiffuseTexture"),
		TEXT("Albedo"),
		TEXT("Texture")
	};
	static const TCHAR* NormalParameterNames[] = {
		TEXT("NormalMap"),
		TEXT("NormalTexture"),
		TEXT("Normal")
	};

	for (const TCHAR* ParameterName : DiffuseParameterNames)
	{
		T66TryReadTextureParameter(SourceMaterial, ParameterName, ExtractedTextures.DiffuseTexture);
	}
	for (const TCHAR* ParameterName : NormalParameterNames)
	{
		T66TryReadTextureParameter(SourceMaterial, ParameterName, ExtractedTextures.NormalTexture);
	}

	if (ExtractedTextures.DiffuseTexture.IsValid() && ExtractedTextures.NormalTexture.IsValid())
	{
		return ExtractedTextures;
	}

	TArray<FMaterialParameterInfo> ParameterInfos;
	TArray<FGuid> ParameterIds;
	SourceMaterial->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);

	int32 BestDiffuseScore = MIN_int32;
	int32 BestNormalScore = MIN_int32;
	UTexture* BestDiffuseTexture = nullptr;
	UTexture* BestNormalTexture = nullptr;

	for (const FMaterialParameterInfo& Info : ParameterInfos)
	{
		UTexture* Value = nullptr;
		if (!SourceMaterial->GetTextureParameterValue(FHashedMaterialParameterInfo(Info), Value) || !T66IsUsableImportedTexture(Value))
		{
			continue;
		}

		const FString ParameterNameLower = Info.Name.ToString().ToLower();
		const int32 DiffuseScore = T66ScoreTextureAssetForMaterial(ParameterNameLower, FString(), false);
		if (DiffuseScore > BestDiffuseScore)
		{
			BestDiffuseScore = DiffuseScore;
			BestDiffuseTexture = Value;
		}

		const int32 NormalScore = T66ScoreTextureAssetForMaterial(ParameterNameLower, FString(), true);
		if (NormalScore > BestNormalScore)
		{
			BestNormalScore = NormalScore;
			BestNormalTexture = Value;
		}
	}

	if (!ExtractedTextures.DiffuseTexture.IsValid() && T66IsUsableImportedTexture(BestDiffuseTexture))
	{
		ExtractedTextures.DiffuseTexture = BestDiffuseTexture;
	}
	if (!ExtractedTextures.NormalTexture.IsValid() && T66IsUsableImportedTexture(BestNormalTexture))
	{
		ExtractedTextures.NormalTexture = BestNormalTexture;
	}

	return ExtractedTextures;
}

static FT66ResolvedImportedTextureSet T66ResolveImportedCharacterTextures(const UMaterialInterface* SourceMaterial)
{
	static TMap<FString, FT66ResolvedImportedTextureSet> CachedTexturesByMaterialPath;

	FT66ResolvedImportedTextureSet EmptyResult;
	if (!SourceMaterial)
	{
		return EmptyResult;
	}

	const FString MaterialPath = SourceMaterial->GetPathName();
	if (const FT66ResolvedImportedTextureSet* Cached = CachedTexturesByMaterialPath.Find(MaterialPath))
	{
		return *Cached;
	}

	FT66ResolvedImportedTextureSet ResolvedTextures = T66ExtractImportedCharacterTexturesFromMaterial(SourceMaterial);
	const FString PackageName = FPackageName::ObjectPathToPackageName(MaterialPath);
	const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);
	if (PackagePath.IsEmpty())
	{
		CachedTexturesByMaterialPath.Add(MaterialPath, ResolvedTextures);
		return ResolvedTextures;
	}

	FString PreferredDiffuseName;
	FString PreferredNormalName;
	T66GetPreferredTextureNamesForMaterial(SourceMaterial->GetName(), PreferredDiffuseName, PreferredNormalName);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = false;
	Filter.PackagePaths.Add(FName(*PackagePath));
	Filter.ClassPaths.Add(UTexture::StaticClass()->GetClassPathName());

	TArray<FAssetData> TextureAssets;
	AssetRegistry.GetAssets(Filter, TextureAssets);

	int32 BestDiffuseScore = MIN_int32;
	int32 BestNormalScore = MIN_int32;
	FAssetData BestDiffuseAsset;
	FAssetData BestNormalAsset;
	const FString PreferredDiffuseLower = PreferredDiffuseName.ToLower();
	const FString PreferredNormalLower = PreferredNormalName.ToLower();

	for (const FAssetData& Asset : TextureAssets)
	{
		const FString AssetNameLower = Asset.AssetName.ToString().ToLower();

		const int32 DiffuseScore = T66ScoreTextureAssetForMaterial(AssetNameLower, PreferredDiffuseLower, false);
		if (DiffuseScore > BestDiffuseScore)
		{
			BestDiffuseScore = DiffuseScore;
			BestDiffuseAsset = Asset;
		}

		const int32 NormalScore = T66ScoreTextureAssetForMaterial(AssetNameLower, PreferredNormalLower, true);
		if (NormalScore > BestNormalScore)
		{
			BestNormalScore = NormalScore;
			BestNormalAsset = Asset;
		}
	}

	if (!ResolvedTextures.DiffuseTexture.IsValid() && BestDiffuseScore > 0)
	{
		ResolvedTextures.DiffuseTexture = Cast<UTexture>(BestDiffuseAsset.GetAsset());
	}
	if (!ResolvedTextures.NormalTexture.IsValid() && BestNormalScore > 0)
	{
		ResolvedTextures.NormalTexture = Cast<UTexture>(BestNormalAsset.GetAsset());
	}

	CachedTexturesByMaterialPath.Add(MaterialPath, ResolvedTextures);
	return ResolvedTextures;
}

static void T66ApplySafeCharacterMaterialOverrides(USkeletalMeshComponent* TargetMesh, FName VisualID)
{
	if (!TargetMesh)
	{
		return;
	}

	static TSet<FString> LoggedMaterialRebuilds;
	static TSet<FString> LoggedMaterialRebuildSkips;
	UTexture* WhiteTexture = GetWhiteFallbackTexture();

	for (int32 MaterialIndex = 0; MaterialIndex < TargetMesh->GetNumMaterials(); ++MaterialIndex)
	{
		UMaterialInterface* SourceMaterial = TargetMesh->GetMaterial(MaterialIndex);
		if (!T66ShouldRebuildImportedCharacterMaterial(SourceMaterial))
		{
			continue;
		}

		UMaterialInterface* BaseMaterial = SourceMaterial ? SourceMaterial->GetBaseMaterial() : nullptr;
		if (!BaseMaterial)
		{
			continue;
		}

		const FT66ResolvedImportedTextureSet TextureSet = T66ResolveImportedCharacterTextures(SourceMaterial);
		UTexture* DiffuseTexture = TextureSet.DiffuseTexture.Get();
		UTexture* NormalTexture = TextureSet.NormalTexture.Get();
		if (!DiffuseTexture)
		{
			const FString SourceMaterialPath = SourceMaterial->GetPathName();
			if (!LoggedMaterialRebuildSkips.Contains(SourceMaterialPath))
			{
				LoggedMaterialRebuildSkips.Add(SourceMaterialPath);
				UE_LOG(
					LogT66CharacterVisuals,
					Verbose,
					TEXT("[MATERIAL] Skipped imported material rebuild for VisualID=%s Slot=%d Source=%s because no usable diffuse texture was found. Keeping original material."),
					*VisualID.ToString(),
					MaterialIndex,
					*SourceMaterialPath);
			}
			continue;
		}
		if (!NormalTexture)
		{
			NormalTexture = WhiteTexture;
		}

		UMaterialInstanceDynamic* SafeMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, TargetMesh);
		if (!SafeMaterial)
		{
			continue;
		}

		SafeMaterial->SetTextureParameterValue(TEXT("DiffuseColorMap"), DiffuseTexture);
		SafeMaterial->SetTextureParameterValue(TEXT("BaseColorTexture"), DiffuseTexture);
		SafeMaterial->SetTextureParameterValue(TEXT("NormalMap"), NormalTexture);
		SafeMaterial->SetTextureParameterValue(TEXT("SpecularColorMap"), WhiteTexture);
		SafeMaterial->SetTextureParameterValue(TEXT("EmissiveColorMap"), WhiteTexture);
		SafeMaterial->SetTextureParameterValue(TEXT("ShininessMap"), WhiteTexture);
		SafeMaterial->SetScalarParameterValue(TEXT("DiffuseColorMapWeight"), 1.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("NormalMapWeight"), TextureSet.NormalTexture.IsValid() ? 1.0f : 0.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("SpecularColorMapWeight"), 0.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("EmissiveColorMapWeight"), 0.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("ShininessMapWeight"), 0.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("AmbientColorMapWeight"), 0.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("OpacityMapWeight"), 0.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("OpacityMaskMapWeight"), 0.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("Brightness"), 1.0f);
		SafeMaterial->SetScalarParameterValue(TEXT("Shininess"), 0.5f);
		SafeMaterial->SetVectorParameterValue(TEXT("AmbientColor"), FLinearColor::White);
		SafeMaterial->SetVectorParameterValue(TEXT("SpecularColor"), FLinearColor::White);
		SafeMaterial->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
		SafeMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
		SafeMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
		SafeMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		TargetMesh->SetMaterial(MaterialIndex, SafeMaterial);

		const FString SourceMaterialPath = SourceMaterial->GetPathName();
		if (!LoggedMaterialRebuilds.Contains(SourceMaterialPath))
		{
			LoggedMaterialRebuilds.Add(SourceMaterialPath);
			UE_LOG(
				LogT66CharacterVisuals,
				Verbose,
				TEXT("[MATERIAL] Rebuilt imported material for VisualID=%s Slot=%d Source=%s Diffuse=%s Normal=%s"),
				*VisualID.ToString(),
				MaterialIndex,
				*SourceMaterialPath,
				DiffuseTexture ? *DiffuseTexture->GetPathName() : TEXT("(fallback white)"),
				TextureSet.NormalTexture.IsValid() ? *TextureSet.NormalTexture->GetPathName() : TEXT("(fallback white)"));
		}
	}
}

/** If the given path points to a non-animation (e.g. SkeletalMesh), try the same path with _Anim suffix (e.g. AM_X.AM_X -> AM_X_Anim.AM_X_Anim). */
template <typename TObjectType>
static TObjectType* TryLoadSoftObjectIfPackageExists(const TSoftObjectPtr<TObjectType>& SoftPath)
{
	if (TObjectType* LoadedObject = SoftPath.Get())
	{
		return LoadedObject;
	}

	const FSoftObjectPath ObjectPath = SoftPath.ToSoftObjectPath();
	const FString PackageName = ObjectPath.GetLongPackageName();
	if (PackageName.IsEmpty() || !FPackageName::DoesPackageExist(PackageName))
	{
		return nullptr;
	}

	return SoftPath.LoadSynchronous();
}

static UAnimationAsset* LoadAnimationFallbackWithAnimSuffix(const TSoftObjectPtr<UAnimationAsset>& SoftPath)
{
	FString PathStr = SoftPath.ToString();
	if (PathStr.IsEmpty() || PathStr.Contains(TEXT("_Anim."))) return nullptr;
	int32 DotIdx;
	if (!PathStr.FindLastChar(TEXT('.'), DotIdx)) return nullptr;
	FString Base = PathStr.Left(DotIdx);
	FString ObjName = PathStr.Mid(DotIdx + 1);
	FString NewPath = Base + TEXT("_Anim.") + ObjName + TEXT("_Anim");
	return TryLoadSoftObjectIfPackageExists(TSoftObjectPtr<UAnimationAsset>(FSoftObjectPath(NewPath)));
}

/** If path is Package_Anim.Object_Anim and package doesn't exist, try Package.Object_Anim (FBX import creates package without _Anim). */
static UAnimationAsset* LoadAnimationFallbackStripPackageAnimSuffix(const TSoftObjectPtr<UAnimationAsset>& SoftPath)
{
	FString PathStr = SoftPath.ToString();
	int32 DotIdx;
	if (PathStr.IsEmpty() || !PathStr.FindLastChar(TEXT('.'), DotIdx)) return nullptr;
	FString Base = PathStr.Left(DotIdx);
	FString ObjName = PathStr.Mid(DotIdx + 1);
	const bool bBaseHasAnimSuffix = Base.EndsWith(TEXT("_Anim"));
	const bool bObjectHasAnimSuffix = ObjName.EndsWith(TEXT("_Anim"));
	if (!bBaseHasAnimSuffix && !bObjectHasAnimSuffix)
	{
		return nullptr;
	}

	const FString BaseStrip = bBaseHasAnimSuffix ? Base.LeftChop(5) : Base;
	const FString ObjNameStrip = bObjectHasAnimSuffix ? ObjName.LeftChop(5) : ObjName;
	const TArray<FString> CandidatePaths = {
		BaseStrip + TEXT(".") + ObjNameStrip,
		BaseStrip + TEXT(".") + ObjName,
		Base + TEXT(".") + ObjNameStrip
	};

	for (const FString& CandidatePath : CandidatePaths)
	{
		if (UAnimationAsset* Animation = TryLoadSoftObjectIfPackageExists(TSoftObjectPtr<UAnimationAsset>(FSoftObjectPath(CandidatePath))))
		{
			return Animation;
		}
	}

	return nullptr;
}

FName UT66CharacterVisualSubsystem::GetFallbackVisualID(FName VisualID)
{
	if (VisualID.IsNone())
	{
		return NAME_None;
	}

	const FString VisualName = VisualID.ToString();
	const int32 LastUnderscoreIndex = VisualName.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	if (LastUnderscoreIndex == INDEX_NONE)
	{
		return NAME_None;
	}

	const FString FallbackName = VisualName.Left(LastUnderscoreIndex);
	if (FallbackName.IsEmpty() || FallbackName == VisualName)
	{
		return NAME_None;
	}

	return FName(*FallbackName);
}

UAnimationAsset* UT66CharacterVisualSubsystem::FindFallbackLoopingAnim(USkeleton* Skeleton) const
{
	if (!Skeleton)
	{
		return nullptr;
	}

	const FName SkelKey(*Skeleton->GetPathName());
	if (const TObjectPtr<UAnimationAsset>* Cached = SkeletonAnimCache.Find(SkelKey))
	{
		return Cached ? Cached->Get() : nullptr;
	}

	UAnimationAsset* Chosen = nullptr;
	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& Registry = ARM.Get();

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(T66_CharactersRootPath);
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());

	TArray<FAssetData> Assets;
	Registry.GetAssets(Filter, Assets);

	const FString SkeletonPath = Skeleton->GetPathName();

	// Prefer an Idle if present, otherwise Walk/Run, otherwise first match.
	int32 BestScore = -1;
	FAssetData BestAsset;

	for (const FAssetData& A : Assets)
	{
		FString Tag;
		if (!A.GetTagValue(T66_AnimSkeletonTag, Tag))
		{
			continue;
		}
		// Tag formats can vary; be permissive.
		if (!Tag.Contains(SkeletonPath))
		{
			continue;
		}

		const FString Name = A.AssetName.ToString().ToLower();
		int32 Score = 0;
		if (Name.Contains("idle")) Score += 100;
		if (Name.Contains("walk")) Score += 50;
		if (Name.Contains("run")) Score += 40;
		if (Name.Contains("loop")) Score += 10;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestAsset = A;
		}
	}

	if (BestScore >= 0)
	{
		UObject* Obj = BestAsset.GetAsset(); // loads
		Chosen = Cast<UAnimationAsset>(Obj);
	}

	SkeletonAnimCache.Add(SkelKey, Chosen);
	return Chosen;
}

UDataTable* UT66CharacterVisualSubsystem::GetVisualsDataTable() const
{
	if (CachedVisualsDataTable)
	{
		return CachedVisualsDataTable;
	}

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(const_cast<UGameInstance*>(GI)))
		{
			if (UDataTable* LoadedDataTable = T66GI->GetCharacterVisualsDataTable())
			{
				CachedVisualsDataTable = LoadedDataTable;
				return CachedVisualsDataTable;
			}

			// Optional: wire this in BP_T66GameInstance later; keep a safe hardcoded fallback for now.
			if (!T66GI->CharacterVisualsDataTable.IsNull())
			{
				CachedVisualsDataTable = T66GI->CharacterVisualsDataTable.Get();
				if (CachedVisualsDataTable)
				{
					return CachedVisualsDataTable;
				}
			}
		}
	}

	// Fallback: load by canonical path if present.
	CachedVisualsDataTable = LoadObject<UDataTable>(nullptr, T66_DefaultCharacterVisualsDTPath);
	return CachedVisualsDataTable;
}

const FT66CharacterVisualRow* UT66CharacterVisualSubsystem::FindVisualRow(FName VisualID, FName* OutResolvedVisualID) const
{
	UDataTable* DT = GetVisualsDataTable();
	if (!DT || VisualID.IsNone())
	{
		return nullptr;
	}

	FName ResolvedVisualID = VisualID;
	const FT66CharacterVisualRow* Row = DT->FindRow<FT66CharacterVisualRow>(ResolvedVisualID, TEXT("FindVisualRow"));
	if (!Row)
	{
		const FName FallbackVisualID = GetFallbackVisualID(VisualID);
		if (!FallbackVisualID.IsNone())
		{
			ResolvedVisualID = FallbackVisualID;
			Row = DT->FindRow<FT66CharacterVisualRow>(ResolvedVisualID, TEXT("FindVisualRowFallback"));
		}
	}

	if (OutResolvedVisualID)
	{
		*OutResolvedVisualID = ResolvedVisualID;
	}

	return Row;
}

FT66ResolvedCharacterVisual UT66CharacterVisualSubsystem::ResolveVisual(FName VisualID)
{
	if (VisualID.IsNone())
	{
		return FT66ResolvedCharacterVisual();
	}

	if (const FT66ResolvedCharacterVisual* Existing = ResolvedCache.Find(VisualID))
	{
		return *Existing;
	}

	// [GOLD] Track total sync-load time for this visual ID.
	const double ResolveStart = FPlatformTime::Seconds();

	FT66ResolvedCharacterVisual Res;
	Res.bHasRow = false;

	UDataTable* DT = GetVisualsDataTable();
	if (!DT)
	{
		ResolvedCache.Add(VisualID, Res);
		return Res;
	}

	FName ResolvedVisualID = VisualID;
	FT66CharacterVisualRow* Row = DT->FindRow<FT66CharacterVisualRow>(ResolvedVisualID, TEXT("ResolveVisual"));
	if (!Row)
	{
		const FName FallbackVisualID = GetFallbackVisualID(VisualID);
		if (!FallbackVisualID.IsNone())
		{
			if (FT66CharacterVisualRow* FallbackRow = DT->FindRow<FT66CharacterVisualRow>(FallbackVisualID, TEXT("ResolveVisualFallback")))
			{
				UE_LOG(
					LogT66CharacterVisuals,
					Verbose,
					TEXT("[MESH] ResolveVisual: missing row for '%s'; falling back to '%s'"),
					*VisualID.ToString(),
					*FallbackVisualID.ToString());
				ResolvedVisualID = FallbackVisualID;
				Row = FallbackRow;
			}
		}
	}
	if (!Row)
	{
#if !UE_BUILD_SHIPPING
		static int32 LoggedMisses = 0;
		if (LoggedMisses < 8)
		{
			++LoggedMisses;
			TArray<FName> AllRows = DT->GetRowNames();
			FString RowList;
			for (int32 i = 0; i < FMath::Min(AllRows.Num(), 10); ++i)
			{
				if (i > 0) RowList += TEXT(", ");
				RowList += AllRows[i].ToString();
			}
			if (AllRows.Num() > 10) RowList += FString::Printf(TEXT("... +%d more"), AllRows.Num() - 10);
			UE_LOG(LogT66CharacterVisuals, Error, TEXT("[MESH] ResolveVisual: NO ROW for VisualID='%s' in DT_CharacterVisuals (%d rows). First rows: [%s]. Reimport via ImportData.py!"),
				*VisualID.ToString(), AllRows.Num(), *RowList);
		}
#endif
	}
	if (Row)
	{
		Res.Row = *Row;
		Res.bHasRow = true;

		if (!Res.Row.SkeletalMesh.IsNull())
		{
			Res.Mesh = TryLoadSoftObjectIfPackageExists(Res.Row.SkeletalMesh);
			UE_LOG(LogT66CharacterVisuals, Log, TEXT("[MESH] ResolveVisual VisualID=%s ResolvedRow=%s SkeletalMesh path=%s Loaded=%s"),
				*VisualID.ToString(), *ResolvedVisualID.ToString(), *Res.Row.SkeletalMesh.ToString(), Res.Mesh ? TEXT("YES") : TEXT("NO"));
		}
		else
		{
			UE_LOG(LogT66CharacterVisuals, Warning, TEXT("[MESH] ResolveVisual VisualID=%s ResolvedRow=%s SkeletalMesh path is NULL in DataTable row!"), *VisualID.ToString(), *ResolvedVisualID.ToString());
		}
		if (!Res.Row.LoopingAnimation.IsNull())
		{
			Res.LoopingAnim = TryLoadSoftObjectIfPackageExists(Res.Row.LoopingAnimation);
			if (!Res.LoopingAnim)
				Res.LoopingAnim = LoadAnimationFallbackWithAnimSuffix(Res.Row.LoopingAnimation);
			if (!Res.LoopingAnim)
				Res.LoopingAnim = LoadAnimationFallbackStripPackageAnimSuffix(Res.Row.LoopingAnimation);
		}
		if (!Res.Row.AlertAnimation.IsNull())
		{
			Res.AlertAnim = TryLoadSoftObjectIfPackageExists(Res.Row.AlertAnimation);
			if (!Res.AlertAnim)
				Res.AlertAnim = LoadAnimationFallbackWithAnimSuffix(Res.Row.AlertAnimation);
			if (!Res.AlertAnim)
				Res.AlertAnim = LoadAnimationFallbackStripPackageAnimSuffix(Res.Row.AlertAnimation);
			UE_LOG(LogT66CharacterVisuals, Log, TEXT("[ANIM] ResolveVisual VisualID=%s ResolvedRow=%s AlertAnimation path=%s AlertAnim=%s"),
				*VisualID.ToString(), *ResolvedVisualID.ToString(), *Res.Row.AlertAnimation.ToString(), Res.AlertAnim ? *Res.AlertAnim->GetName() : TEXT("(null)"));
		}
		else
		{
			UE_LOG(LogT66CharacterVisuals, Log, TEXT("[ANIM] ResolveVisual VisualID=%s ResolvedRow=%s AlertAnimation is null (no alert anim row)"), *VisualID.ToString(), *ResolvedVisualID.ToString());
		}
		if (!Res.Row.RunAnimation.IsNull())
		{
			Res.RunAnim = TryLoadSoftObjectIfPackageExists(Res.Row.RunAnimation);
			if (!Res.RunAnim)
				Res.RunAnim = LoadAnimationFallbackWithAnimSuffix(Res.Row.RunAnimation);
			if (!Res.RunAnim)
				Res.RunAnim = LoadAnimationFallbackStripPackageAnimSuffix(Res.Row.RunAnimation);
		}
		// If no explicit animation is set, try to find any AnimSequence for this Skeleton (cached).
		if (!Res.LoopingAnim && Res.Mesh && Res.Mesh->GetSkeleton())
		{
			Res.LoopingAnim = FindFallbackLoopingAnim(Res.Mesh->GetSkeleton());
		}
	}

	// [GOLD] Log total resolve time (sync loads are expensive; this helps identify stage-entry hitches).
	const double ResolveMs = (FPlatformTime::Seconds() - ResolveStart) * 1000.0;
	if (ResolveMs > 1.0)
	{
		if (ResolveMs >= 250.0)
		{
			UE_LOG(LogT66CharacterVisuals, Log, TEXT("[GOLD] ResolveVisual: %s took %.1fms (sync asset loads — consider preloading)"), *VisualID.ToString(), ResolveMs);
		}
		else
		{
			UE_LOG(LogT66CharacterVisuals, Verbose, TEXT("[GOLD] ResolveVisual: %s took %.1fms (sync asset loads — consider preloading)"), *VisualID.ToString(), ResolveMs);
		}
	}
	else
	{
		UE_LOG(LogT66CharacterVisuals, Log, TEXT("[GOLD] ResolveVisual: %s resolved in %.2fms (cached or no assets)"), *VisualID.ToString(), ResolveMs);
	}

	ResolvedCache.Add(VisualID, Res);
	return Res;
}

void UT66CharacterVisualSubsystem::PreloadCharacterVisual(FName VisualID)
{
	if (VisualID.IsNone() || ResolvedCache.Contains(VisualID) || PendingPreloadHandles.Contains(VisualID))
	{
		return;
	}

	FName ResolvedVisualID = VisualID;
	const FT66CharacterVisualRow* Row = FindVisualRow(VisualID, &ResolvedVisualID);
	if (!Row)
	{
		return;
	}

	TArray<FSoftObjectPath> PendingPaths;
	PendingPaths.Reserve(4);
	T66AppendCharacterVisualAssetPaths(*Row, PendingPaths);

	bool bAllAssetsLoaded = true;
	for (int32 Index = PendingPaths.Num() - 1; Index >= 0; --Index)
	{
		const FSoftObjectPath& AssetPath = PendingPaths[Index];
		if (AssetPath.IsNull())
		{
			PendingPaths.RemoveAtSwap(Index, 1, EAllowShrinking::No);
			continue;
		}

		if (AssetPath.ResolveObject())
		{
			PendingPaths.RemoveAtSwap(Index, 1, EAllowShrinking::No);
		}
		else
		{
			bAllAssetsLoaded = false;
		}
	}

	if (bAllAssetsLoaded || PendingPaths.Num() <= 0)
	{
		ResolveVisual(VisualID);
		return;
	}

	TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		PendingPaths,
		FStreamableDelegate::CreateUObject(this, &UT66CharacterVisualSubsystem::HandleCharacterVisualPreloadCompleted, VisualID));
	if (!Handle.IsValid())
	{
		ResolveVisual(VisualID);
		return;
	}

	PendingPreloadHandles.Add(VisualID, Handle);
}

void UT66CharacterVisualSubsystem::HandleCharacterVisualPreloadCompleted(FName VisualID)
{
	PendingPreloadHandles.Remove(VisualID);
	ResolveVisual(VisualID);
}

FName UT66CharacterVisualSubsystem::GetHeroVisualID(FName HeroID, ET66BodyType BodyType, FName SkinID)
{
	if (HeroID.IsNone()) return NAME_None;
	const TCHAR TypeChar = (BodyType == ET66BodyType::TypeA) ? TEXT('A') : TEXT('B');
	FString Key = FString::Printf(TEXT("%s_Type%c"), *HeroID.ToString(), TypeChar);
	if (SkinID != NAME_None && SkinID != FName(TEXT("Default")))
	{
		Key += TEXT("_");
		Key += SkinID.ToString();
	}
	return FName(*Key);
}

FName UT66CharacterVisualSubsystem::GetCompanionVisualID(FName CompanionID, FName SkinID)
{
	if (CompanionID.IsNone()) return NAME_None;
	if (SkinID.IsNone() || SkinID == FName(TEXT("Default")))
	{
		return CompanionID;
	}
	return FName(*(CompanionID.ToString() + TEXT("_") + SkinID.ToString()));
}

void UT66CharacterVisualSubsystem::GetMovementAnimsForVisual(FName VisualID, UAnimationAsset*& OutWalk, UAnimationAsset*& OutRun, UAnimationAsset*& OutAlert)
{
	OutWalk = nullptr;
	OutRun = nullptr;
	OutAlert = nullptr;
	const FT66ResolvedCharacterVisual Res = ResolveVisual(VisualID);
	if (!Res.bHasRow) return;
	OutWalk = Res.LoopingAnim;
	OutRun = Res.RunAnim;
	OutAlert = Res.AlertAnim;
}

bool UT66CharacterVisualSubsystem::ApplyCharacterVisual(
	FName VisualID,
	USkeletalMeshComponent* TargetMesh,
	USceneComponent* PlaceholderToHide,
	bool bEnableSingleNodeAnimation,
	bool bUseAlertAnimation,
	bool bIsPreviewContext)
{
	if (VisualID.IsNone() || !TargetMesh)
	{
		return false;
	}

	const FT66ResolvedCharacterVisual Res = ResolveVisual(VisualID);
	if (!Res.bHasRow || !Res.Mesh)
	{
		UE_LOG(LogT66CharacterVisuals, Warning, TEXT("[MESH] ApplyCharacterVisual FAILED for VisualID=%s: bHasRow=%d, Mesh=%s"),
			*VisualID.ToString(), Res.bHasRow ? 1 : 0, Res.Mesh ? *Res.Mesh->GetName() : TEXT("(null)"));
		return false;
	}

	// Clear any previous dynamic material overrides before setting the new mesh.
	// Without this, MIDs from a previous hero persist in OverrideMaterials and bleed
	// onto the next hero (e.g. Hero 1 textures appearing on Hero 2).
	TargetMesh->EmptyOverrideMaterials();

	TargetMesh->SetSkeletalMesh(Res.Mesh);
	TargetMesh->SetRelativeRotation(Res.Row.MeshRelativeRotation);

	const FVector Scale = Res.Row.MeshRelativeScale.IsNearlyZero() ? FVector(1.f, 1.f, 1.f) : Res.Row.MeshRelativeScale;
	TargetMesh->SetRelativeScale3D(Scale);

	FVector RelLoc = Res.Row.MeshRelativeLocation;

	// ACharacter owners use capsule-based alignment (below); skip bAutoGroundToActorOrigin
	// for them so the two adjustments don't interfere at extreme scales.
	const bool bIsCharacterOwner = Cast<ACharacter>(TargetMesh->GetOwner()) != nullptr;
	if (Res.Row.bAutoGroundToActorOrigin && !bIsCharacterOwner)
	{
		const FBoxSphereBounds B = Res.Mesh->GetBounds();
		const float BottomZ = (B.Origin.Z - B.BoxExtent.Z) * Scale.Z;
		RelLoc.Z -= BottomZ;
	}

	// Default for ACharacter: place mesh origin (feet pivot) at capsule bottom (ground contact).
	// Standard game-ready character meshes have their pivot at the feet (Z=0 in mesh local space).
	// Previous bounds-based approach was incorrect: it used the bounding box bottom, which can
	// extend below the feet due to root bone offsets or extra geometry.
	if (ACharacter* OwnerChar = Cast<ACharacter>(TargetMesh->GetOwner()))
	{
		if (UCapsuleComponent* Cap = OwnerChar->GetCapsuleComponent())
		{
			const float CapsuleHalfHeight = Cap->GetScaledCapsuleHalfHeight();
			RelLoc.Z = Res.Row.MeshRelativeLocation.Z - CapsuleHalfHeight;

			const FBoxSphereBounds B = Res.Mesh->GetBounds();
			UE_LOG(LogT66CharacterVisuals, Verbose, TEXT("[MESH ALIGN] %s: Pivot-at-feet approach. RelZ=%.2f CapsuleHH=%.2f | Bounds Origin.Z=%.2f Extent.Z=%.2f Scale=%.2f MeshHeight=%.2f"),
				*VisualID.ToString(), RelLoc.Z, CapsuleHalfHeight,
				B.Origin.Z, B.BoxExtent.Z, Scale.Z, B.BoxExtent.Z * 2.f * Scale.Z);
		}
	}
	TargetMesh->SetRelativeLocation(RelLoc);
	T66ApplySafeCharacterMaterialOverrides(TargetMesh, VisualID);

	TargetMesh->SetHiddenInGame(false, true);
	TargetMesh->SetVisibility(true, true);

	// In preview context: force highest mip streaming on all material textures so
	// the character isn't blank/black, and log material info for diagnostics.
	if (bIsPreviewContext)
	{
		TargetMesh->bForceMipStreaming = true;
		TargetMesh->StreamingDistanceMultiplier = 50.f;
#if !UE_BUILD_SHIPPING
		const int32 NumMats = TargetMesh->GetNumMaterials();
		for (int32 MatIdx = 0; MatIdx < NumMats; ++MatIdx)
		{
			UMaterialInterface* Mat = TargetMesh->GetMaterial(MatIdx);
			UE_LOG(LogT66CharacterVisuals, Log, TEXT("[MESH] Preview material slot %d/%d: %s (class=%s) for VisualID=%s"),
				MatIdx, NumMats,
				Mat ? *Mat->GetName() : TEXT("(null)"),
				Mat ? *Mat->GetClass()->GetName() : TEXT("N/A"),
				*VisualID.ToString());
		}
#endif
	}

	if (PlaceholderToHide)
	{
		PlaceholderToHide->SetVisibility(false, true);
		PlaceholderToHide->SetHiddenInGame(true, true);
	}

	if (bEnableSingleNodeAnimation)
	{
		UAnimationAsset* AnimToPlay = (bUseAlertAnimation && Res.AlertAnim) ? Res.AlertAnim : Res.LoopingAnim;
		// Log animation type and duration for debugging
		float AnimDuration = 0.f;
		FString AnimClass = TEXT("(null)");
		if (AnimToPlay)
		{
			AnimClass = AnimToPlay->GetClass()->GetName();
			AnimDuration = AnimToPlay->GetPlayLength();
		}
		UE_LOG(LogT66CharacterVisuals, Verbose, TEXT("[ANIM] ApplyCharacterVisual VisualID=%s bUseAlertAnimation=%d bIsPreviewContext=%d Res.AlertAnim=%s Res.LoopingAnim=%s AnimToPlay=%s Class=%s Duration=%.3f"),
			*VisualID.ToString(), bUseAlertAnimation ? 1 : 0, bIsPreviewContext ? 1 : 0,
			Res.AlertAnim ? *Res.AlertAnim->GetName() : TEXT("(null)"),
			Res.LoopingAnim ? *Res.LoopingAnim->GetName() : TEXT("(null)"),
			AnimToPlay ? *AnimToPlay->GetName() : TEXT("(null)"),
			*AnimClass, AnimDuration);
		if (AnimToPlay)
		{
			// Preview (hero selection): ensure no AnimBlueprint overrides, mesh always ticks so animation is visible in scene capture.
			if (bIsPreviewContext)
			{
				TargetMesh->SetAnimInstanceClass(nullptr);
				TargetMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
				TargetMesh->SetComponentTickEnabled(true);
				TargetMesh->bEnableUpdateRateOptimizations = false; // Preview is only visible to SceneCapture; avoid skipping anim updates.
			}
			// Reinitialize animation state after mesh change (otherwise PlayAnimation fails after SetSkeletalMesh).
			TargetMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			TargetMesh->InitAnim(true);
			// In preview we use alert anim and want it to loop; otherwise use row's loop setting.
			const bool bLoop = bUseAlertAnimation ? true : Res.Row.bLoopAnimation;
			TargetMesh->PlayAnimation(AnimToPlay, bLoop);
			TargetMesh->SetPosition(0.f);
			UE_LOG(LogT66CharacterVisuals, Verbose, TEXT("[ANIM] PlayAnimation called: AnimMode=%d IsPlaying=%d Position=%.3f bLoop=%d"),
				(int32)TargetMesh->GetAnimationMode(), TargetMesh->IsPlaying() ? 1 : 0, TargetMesh->GetPosition(), bLoop ? 1 : 0);
		}
	}

	// ============================================================
	// INVIS fail-safe:
	// If an imported mesh winds up far away from the owning actor (bad pivot/bounds),
	// the enemy/boss can become effectively invisible even though gameplay/collision exists.
	// Detect that case and fall back to placeholder visuals, and log actionable diagnostics.
	// ============================================================
	{
		const AActor* Owner = Cast<AActor>(TargetMesh->GetOwner());
		const FVector OwnerLoc = Owner ? Owner->GetActorLocation() : TargetMesh->GetComponentLocation();
		const FBoxSphereBounds WorldB = TargetMesh->CalcBounds(TargetMesh->GetComponentTransform());

		const float DistToOwner = (WorldB.Origin - OwnerLoc).Size();
		const bool bBoundsTiny = (WorldB.SphereRadius < 1.f);
		const bool bBoundsFar = (DistToOwner > 5000.f); // 50m+ away from capsule = effectively invisible in normal gameplay.
		const bool bBoundsFinite =
			FMath::IsFinite(WorldB.Origin.X) && FMath::IsFinite(WorldB.Origin.Y) && FMath::IsFinite(WorldB.Origin.Z) &&
			FMath::IsFinite(WorldB.BoxExtent.X) && FMath::IsFinite(WorldB.BoxExtent.Y) && FMath::IsFinite(WorldB.BoxExtent.Z) &&
			FMath::IsFinite(WorldB.SphereRadius);

		if (bBoundsTiny || bBoundsFar || !bBoundsFinite)
		{
#if !UE_BUILD_SHIPPING
			static int32 LoggedInvisFixes = 0;
			if (LoggedInvisFixes < 32)
			{
				++LoggedInvisFixes;
				const FString MeshName = Res.Mesh ? Res.Mesh->GetName() : FString(TEXT("None"));
				const FString MeshPath = Res.Mesh ? Res.Mesh->GetPathName() : FString(TEXT("None"));
				const FString OwnerName = Owner ? Owner->GetName() : FString(TEXT("None"));

				UE_LOG(LogT66CharacterVisuals, Error,
					TEXT("[INVIS] Rejecting visual apply (fallback to placeholder). VisualID=%s Owner=%s Mesh=%s MeshPath=%s DistToOwner=%.1f BoundsCenter=(%.1f,%.1f,%.1f) BoundsExtent=(%.1f,%.1f,%.1f) SphereRadius=%.1f RelLoc=(%.1f,%.1f,%.1f) RelRot=(P=%.1f,Y=%.1f,R=%.1f) RelScale=(%.3f,%.3f,%.3f) AutoCenterXY=%d"),
					*VisualID.ToString(),
					*OwnerName,
					*MeshName,
					*MeshPath,
					DistToOwner,
					WorldB.Origin.X, WorldB.Origin.Y, WorldB.Origin.Z,
					WorldB.BoxExtent.X, WorldB.BoxExtent.Y, WorldB.BoxExtent.Z,
					WorldB.SphereRadius,
					RelLoc.X, RelLoc.Y, RelLoc.Z,
					Res.Row.MeshRelativeRotation.Pitch, Res.Row.MeshRelativeRotation.Yaw, Res.Row.MeshRelativeRotation.Roll,
					Scale.X, Scale.Y, Scale.Z,
					1
				);
			}
#endif

			// Revert: hide skeletal mesh, show placeholder.
			TargetMesh->SetHiddenInGame(true, true);
			TargetMesh->SetVisibility(false, true);
			if (PlaceholderToHide)
			{
				PlaceholderToHide->SetHiddenInGame(false, true);
				PlaceholderToHide->SetVisibility(true, true);
			}
			return false;
		}
	}

	return true;
}
