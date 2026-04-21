// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66TDVisualSubsystem.h"

#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"

namespace
{
	FString T66TDSanitizeVisualID(const FString& RawValue)
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
}

FString UT66TDVisualSubsystem::SanitizeVisualID(const FString& RawValue)
{
	return T66TDSanitizeVisualID(RawValue);
}

UTexture2D* UT66TDVisualSubsystem::LoadCachedLooseTexture(const FString& CacheKey, const FString& RelativePath)
{
	if (CacheKey.IsEmpty() || RelativePath.IsEmpty())
	{
		return nullptr;
	}

	if (const TObjectPtr<UTexture2D>* ExistingTexture = CachedTextures.Find(CacheKey))
	{
		return ExistingTexture->Get();
	}

	for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
	{
		if (!IFileManager::Get().FileExists(*CandidatePath))
		{
			continue;
		}

		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(CandidatePath, TextureFilter::TF_Nearest, false, TEXT("TDVisualLooseTexture")))
		{
			CachedTextures.Add(CacheKey, Texture);
			return Texture;
		}
	}

	return nullptr;
}

UTexture2D* UT66TDVisualSubsystem::LoadHeroTexture(const FName HeroID, const FString& HeroDisplayName)
{
	const FString VisualID = SanitizeVisualID(HeroDisplayName);
	if (VisualID.IsEmpty())
	{
		return nullptr;
	}

	return LoadCachedLooseTexture(
		FString::Printf(TEXT("Hero:%s:%s"), *HeroID.ToString(), *VisualID),
		FString::Printf(TEXT("SourceAssets/TD/Heroes/Singles/TD_%s.png"), *VisualID));
}

UTexture2D* UT66TDVisualSubsystem::LoadEnemyTexture(const FString& EnemyVisualID)
{
	const FString VisualID = SanitizeVisualID(EnemyVisualID);
	if (VisualID.IsEmpty())
	{
		return nullptr;
	}

	return LoadCachedLooseTexture(
		FString::Printf(TEXT("Enemy:%s"), *VisualID),
		FString::Printf(TEXT("SourceAssets/TD/Enemies/Singles/TD_%s.png"), *VisualID));
}

UTexture2D* UT66TDVisualSubsystem::LoadBossTexture(const FString& EnemyVisualID)
{
	const FString VisualID = SanitizeVisualID(EnemyVisualID);
	if (VisualID.IsEmpty())
	{
		return nullptr;
	}

	return LoadCachedLooseTexture(
		FString::Printf(TEXT("Boss:%s"), *VisualID),
		FString::Printf(TEXT("SourceAssets/TD/Bosses/Singles/TD_%s_Boss.png"), *VisualID));
}
