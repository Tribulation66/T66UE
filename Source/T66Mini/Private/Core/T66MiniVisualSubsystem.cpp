// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniVisualSubsystem.h"

#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"

namespace
{
	FString T66MiniSanitizeVisualId(const FString& RawValue)
	{
		FString Sanitized;
		Sanitized.Reserve(RawValue.Len());
		for (const TCHAR Character : RawValue)
		{
			if (FChar::IsAlnum(Character) || Character == TEXT('_'))
			{
				Sanitized.AppendChar(Character);
			}
		}

		return Sanitized;
	}

	FString T66MiniExtractAssetName(const FString& ObjectPath)
	{
		if (ObjectPath.IsEmpty())
		{
			return FString();
		}

		FString AssetName = ObjectPath;
		int32 SlashIndex = INDEX_NONE;
		if (AssetName.FindLastChar(TEXT('/'), SlashIndex))
		{
			AssetName.RightChopInline(SlashIndex + 1, EAllowShrinking::No);
		}

		int32 DotIndex = INDEX_NONE;
		if (AssetName.FindChar(TEXT('.'), DotIndex))
		{
			AssetName.LeftInline(DotIndex, EAllowShrinking::No);
		}

		return AssetName;
	}
}

UTexture2D* UT66MiniVisualSubsystem::LoadLooseTexture(const FString& RelativePath)
{
	return LoadCachedTexture(RelativePath, RelativePath);
}

UTexture2D* UT66MiniVisualSubsystem::LoadBackgroundTexture()
{
	return LoadImportedOrLooseTexture(
		TEXT("Background"),
		TEXT("/Game/Mini/Background.Background"),
		TEXT("SourceAssets/Mini/Background.png"));
}

UTexture2D* UT66MiniVisualSubsystem::LoadHeroTexture(const FString& HeroDisplayName)
{
	const FString VisualId = T66MiniSanitizeVisualId(HeroDisplayName);
	return LoadImportedOrLooseTexture(
		FString::Printf(TEXT("Hero:%s"), *VisualId),
		FString::Printf(TEXT("/Game/Mini/Sprites/Heroes/%s.%s"), *VisualId, *VisualId),
		FString::Printf(TEXT("SourceAssets/Mini/Heroes/Singles/%s.png"), *VisualId));
}

UTexture2D* UT66MiniVisualSubsystem::LoadHeroAnimationTexture(const FString& HeroDisplayName, const FString& FrameKey)
{
	const FString VisualId = T66MiniSanitizeVisualId(HeroDisplayName);
	return LoadCachedTextureFromCandidates(
		FString::Printf(TEXT("HeroAnimation:%s:%s"), *VisualId, *FrameKey),
		{
			FString::Printf(TEXT("SourceAssets/Mini/Heroes/AnimationSets/%s/%s_%s.png"), *VisualId, *VisualId, *FrameKey),
			FString::Printf(TEXT("SourceAssets/Mini/Heroes/Singles/%s.png"), *VisualId)
		});
}

UTexture2D* UT66MiniVisualSubsystem::LoadHeroProjectileTexture(const FString& HeroDisplayName)
{
	const FString VisualId = T66MiniSanitizeVisualId(HeroDisplayName);
	TArray<FString> CandidatePaths = {
		FString::Printf(TEXT("SourceAssets/Mini/Heroes/AnimationSets/%s/%s_SwordProjectile.png"), *VisualId, *VisualId),
		FString::Printf(TEXT("SourceAssets/Mini/Heroes/AnimationSets/%s/%s_PrimaryProjectile.png"), *VisualId, *VisualId)
	};

	if (VisualId.Equals(TEXT("Arthur"), ESearchCase::IgnoreCase))
	{
		CandidatePaths.Add(TEXT("SourceAssets/UI/HUDGenerated/arthur_ultimate_colossal_sword.png"));
		CandidatePaths.Add(TEXT("SourceAssets/Import/VFX/ProjectileMeshPack/previews/Arthur_Sword.png"));
	}

	return LoadCachedTextureFromCandidates(FString::Printf(TEXT("HeroProjectile:%s"), *VisualId), CandidatePaths);
}

UTexture2D* UT66MiniVisualSubsystem::LoadCompanionTexture(const FString& CompanionVisualID)
{
	const FString VisualId = T66MiniSanitizeVisualId(CompanionVisualID);
	return LoadImportedOrLooseTexture(
		FString::Printf(TEXT("Companion:%s"), *VisualId),
		FString::Printf(TEXT("/Game/Mini/Sprites/Companions/%s.%s"), *VisualId, *VisualId),
		FString::Printf(TEXT("SourceAssets/Mini/Companions/Singles/%s.png"), *VisualId));
}

UTexture2D* UT66MiniVisualSubsystem::LoadCompanionAnimationTexture(const FString& CompanionVisualID, const FString& FrameKey)
{
	const FString VisualId = T66MiniSanitizeVisualId(CompanionVisualID);
	return LoadCachedTextureFromCandidates(
		FString::Printf(TEXT("CompanionAnimation:%s:%s"), *VisualId, *FrameKey),
		{
			FString::Printf(TEXT("SourceAssets/Mini/Companions/AnimationSets/%s/%s_%s.png"), *VisualId, *VisualId, *FrameKey),
			FString::Printf(TEXT("SourceAssets/Mini/Companions/Singles/%s.png"), *VisualId)
		});
}

UTexture2D* UT66MiniVisualSubsystem::LoadIdolEffectTexture(const FName IdolID)
{
	const FString VisualId = T66MiniSanitizeVisualId(IdolID.ToString());
	return LoadCachedTextureFromCandidates(
		FString::Printf(TEXT("IdolEffect:%s"), *VisualId),
		{
			FString::Printf(TEXT("SourceAssets/Mini/Idols/Effects/Singles/%s.png"), *VisualId),
			FString::Printf(TEXT("SourceAssets/IdolSprites/%s_white.png"), *VisualId),
			FString::Printf(TEXT("SourceAssets/IdolSprites/%s_yellow.png"), *VisualId),
			FString::Printf(TEXT("SourceAssets/IdolSprites/%s_black.png"), *VisualId)
		});
}

UTexture2D* UT66MiniVisualSubsystem::LoadEnemyTexture(const FString& EnemyVisualID)
{
	const FString VisualId = T66MiniSanitizeVisualId(EnemyVisualID);
	return LoadImportedOrLooseTexture(
		FString::Printf(TEXT("Enemy:%s"), *VisualId),
		FString::Printf(TEXT("/Game/Mini/Sprites/Enemies/%s.%s"), *VisualId, *VisualId),
		FString::Printf(TEXT("SourceAssets/Mini/Enemies/Singles/%s.png"), *VisualId));
}

UTexture2D* UT66MiniVisualSubsystem::LoadBossTexture(const FString& EnemyVisualID)
{
	const FString VisualId = T66MiniSanitizeVisualId(EnemyVisualID);
	return LoadImportedOrLooseTexture(
		FString::Printf(TEXT("Boss:%s"), *VisualId),
		FString::Printf(TEXT("/Game/Mini/Sprites/Bosses/%s_Boss.%s_Boss"), *VisualId, *VisualId),
		FString::Printf(TEXT("SourceAssets/Mini/Bosses/Singles/%s_Boss.png"), *VisualId));
}

UTexture2D* UT66MiniVisualSubsystem::LoadInteractableTexture(const FString& InteractableVisualID)
{
	const FString VisualId = T66MiniSanitizeVisualId(InteractableVisualID);
	return LoadImportedOrLooseTexture(
		FString::Printf(TEXT("Interactable:%s"), *VisualId),
		FString::Printf(TEXT("/Game/Mini/Sprites/Interactables/%s.%s"), *VisualId, *VisualId),
		FString::Printf(TEXT("SourceAssets/Mini/Interactables/Singles/%s.png"), *VisualId));
}

UTexture2D* UT66MiniVisualSubsystem::LoadItemTexture(const FName ItemID, const FString& IconPath)
{
	const FString SanitizedItemID = T66MiniSanitizeVisualId(ItemID.ToString());
	const FString AssetNameFromPath = T66MiniExtractAssetName(IconPath);
	const FString SanitizedAssetName = T66MiniSanitizeVisualId(AssetNameFromPath);
	const FString PreferredAssetName = !SanitizedAssetName.IsEmpty()
		? SanitizedAssetName
		: (!SanitizedItemID.IsEmpty() ? FString::Printf(TEXT("%s_black"), *SanitizedItemID) : FString());
	const FString CacheKey = FString::Printf(TEXT("Item:%s:%s"), *SanitizedItemID, *PreferredAssetName);
	if (TObjectPtr<UTexture2D>* ExistingTexture = CachedTextures.Find(CacheKey))
	{
		return ExistingTexture->Get();
	}

	const TArray<FString> CandidateAssetPaths = {
		PreferredAssetName.IsEmpty() ? FString() : FString::Printf(TEXT("/Game/Mini/Sprites/Items/%s.%s"), *PreferredAssetName, *PreferredAssetName),
		SanitizedItemID.IsEmpty() ? FString() : FString::Printf(TEXT("/Game/Mini/Sprites/Items/%s.%s"), *SanitizedItemID, *SanitizedItemID)
	};

	for (const FString& AssetPath : CandidateAssetPaths)
	{
		if (AssetPath.IsEmpty())
		{
			continue;
		}

		if (UTexture2D* ImportedTexture = LoadImportedTexture(AssetPath))
		{
			CachedTextures.Add(CacheKey, ImportedTexture);
			return ImportedTexture;
		}
	}

	TArray<FString> CandidateRelativePaths;
	if (!PreferredAssetName.IsEmpty())
	{
		CandidateRelativePaths.Add(FString::Printf(TEXT("SourceAssets/Mini/Items/Singles/%s.png"), *PreferredAssetName));
		CandidateRelativePaths.Add(FString::Printf(TEXT("SourceAssets/ItemSprites/%s.png"), *PreferredAssetName));
	}
	if (!SanitizedItemID.IsEmpty())
	{
		CandidateRelativePaths.Add(FString::Printf(TEXT("SourceAssets/Mini/Items/Singles/%s_black.png"), *SanitizedItemID));
		CandidateRelativePaths.Add(FString::Printf(TEXT("SourceAssets/ItemSprites/%s_black.png"), *SanitizedItemID));
	}

	return LoadCachedTextureFromCandidates(CacheKey, CandidateRelativePaths);
}

FString UT66MiniVisualSubsystem::BuildAbsoluteProjectPath(const FString& RelativePath)
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / RelativePath);
}

UTexture2D* UT66MiniVisualSubsystem::LoadImportedTexture(const FString& AssetPath)
{
	if (AssetPath.IsEmpty())
	{
		return nullptr;
	}

	return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *AssetPath));
}

UTexture2D* UT66MiniVisualSubsystem::LoadImportedOrLooseTexture(const FString& CacheKey, const FString& AssetPath, const FString& RelativePath)
{
	if (TObjectPtr<UTexture2D>* ExistingTexture = CachedTextures.Find(CacheKey))
	{
		return ExistingTexture->Get();
	}

	if (UTexture2D* ImportedTexture = LoadImportedTexture(AssetPath))
	{
		CachedTextures.Add(CacheKey, ImportedTexture);
		return ImportedTexture;
	}

	return LoadCachedTexture(CacheKey, RelativePath);
}

UTexture2D* UT66MiniVisualSubsystem::LoadCachedTexture(const FString& CacheKey, const FString& RelativePath)
{
	if (TObjectPtr<UTexture2D>* ExistingTexture = CachedTextures.Find(CacheKey))
	{
		return ExistingTexture->Get();
	}

	const FString AbsolutePath = BuildAbsoluteProjectPath(RelativePath);
	UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(AbsolutePath);
	if (Texture)
	{
		Texture->SRGB = true;
		Texture->Filter = TextureFilter::TF_Trilinear;
		Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Texture->NeverStream = true;
		CachedTextures.Add(CacheKey, Texture);
	}

	return Texture;
}

UTexture2D* UT66MiniVisualSubsystem::LoadCachedTextureFromCandidates(const FString& CacheKey, const TArray<FString>& RelativePaths)
{
	if (TObjectPtr<UTexture2D>* ExistingTexture = CachedTextures.Find(CacheKey))
	{
		return ExistingTexture->Get();
	}

	for (const FString& RelativePath : RelativePaths)
	{
		if (!FPaths::FileExists(BuildAbsoluteProjectPath(RelativePath)))
		{
			continue;
		}

		if (UTexture2D* Texture = LoadCachedTexture(CacheKey, RelativePath))
		{
			return Texture;
		}
	}

	return RelativePaths.Num() > 0 ? LoadCachedTexture(CacheKey, RelativePaths[0]) : nullptr;
}
