// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66TemporaryBuffUIUtils.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "Engine/Texture2D.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Styling/SlateBrush.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	TMap<FString, TStrongObjectPtr<UTexture2D>> GTemporaryBuffFileTextureCache;

	void InitializeRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
		Brush->TintColor = FSlateColor(FLinearColor::White);
	}

	FVector2D ResolveImageSize(UTexture2D* Texture, const FVector2D& DesiredImageSize)
	{
		if (DesiredImageSize.X > 0.f && DesiredImageSize.Y > 0.f)
		{
			return DesiredImageSize;
		}

		if (Texture)
		{
			const int32 Width = Texture->GetSizeX();
			const int32 Height = Texture->GetSizeY();
			if (Width > 0 && Height > 0)
			{
				return FVector2D(static_cast<float>(Width), static_cast<float>(Height));
			}
		}

		return FVector2D(1.f, 1.f);
	}

	UTexture2D* LoadFileTexture(const FString& FilePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* Cached = GTemporaryBuffFileTextureCache.Find(FilePath))
		{
			return Cached->Get();
		}

		UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
			FilePath,
			TextureFilter::TF_Trilinear,
			false,
			TEXT("TempBuffTexture"));
		if (!Texture)
		{
			Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
				FilePath,
				TextureFilter::TF_Trilinear,
				TEXT("TempBuffTexture"));
		}
		if (!Texture)
		{
			return nullptr;
		}

		GTemporaryBuffFileTextureCache.Add(FilePath, TStrongObjectPtr<UTexture2D>(Texture));
		return Texture;
	}
}

FString T66TemporaryBuffUI::GetSecondaryBuffIconSlug(ET66SecondaryStatType StatType)
{
	switch (StatType)
	{
	case ET66SecondaryStatType::AoeDamage:       return TEXT("aoe-damage");
	case ET66SecondaryStatType::BounceDamage:    return TEXT("bounce-damage");
	case ET66SecondaryStatType::PierceDamage:    return TEXT("pierce-damage");
	case ET66SecondaryStatType::DotDamage:       return TEXT("dot-damage");
	case ET66SecondaryStatType::CritDamage:      return TEXT("crit-damage");
	case ET66SecondaryStatType::AoeSpeed:        return TEXT("aoe-speed");
	case ET66SecondaryStatType::BounceSpeed:     return TEXT("bounce-speed");
	case ET66SecondaryStatType::PierceSpeed:     return TEXT("pierce-speed");
	case ET66SecondaryStatType::DotSpeed:        return TEXT("dot-speed");
	case ET66SecondaryStatType::CritChance:      return TEXT("crit-chance");
	case ET66SecondaryStatType::AoeScale:        return TEXT("aoe-scale");
	case ET66SecondaryStatType::BounceScale:     return TEXT("bounce-scale");
	case ET66SecondaryStatType::PierceScale:     return TEXT("pierce-scale");
	case ET66SecondaryStatType::DotScale:        return TEXT("dot-scale");
	case ET66SecondaryStatType::AttackRange:     return TEXT("range");
	case ET66SecondaryStatType::Taunt:           return TEXT("taunt");
	case ET66SecondaryStatType::DamageReduction: return TEXT("damage-reduction");
	case ET66SecondaryStatType::ReflectDamage:   return TEXT("damage-reflection");
	case ET66SecondaryStatType::HpRegen:         return TEXT("hp-regen");
	case ET66SecondaryStatType::Crush:           return TEXT("crush");
	case ET66SecondaryStatType::EvasionChance:   return TEXT("dodge");
	case ET66SecondaryStatType::CounterAttack:   return TEXT("counter-attack");
	case ET66SecondaryStatType::LifeSteal:       return TEXT("life-steal");
	case ET66SecondaryStatType::Invisibility:    return TEXT("invisibility");
	case ET66SecondaryStatType::Assassinate:     return TEXT("assassinate");
	case ET66SecondaryStatType::TreasureChest:   return TEXT("treasure-chest");
	case ET66SecondaryStatType::Cheating:        return TEXT("cheating");
	case ET66SecondaryStatType::Stealing:        return TEXT("stealing");
	case ET66SecondaryStatType::LootCrate:       return TEXT("loot-crate");
	case ET66SecondaryStatType::Alchemy:         return TEXT("alchemy");
	case ET66SecondaryStatType::Accuracy:        return TEXT("accuracy");
	default:                                     return FString();
	}
}

TSharedPtr<FSlateBrush> T66TemporaryBuffUI::CreateSecondaryBuffBrush(
	UT66UITexturePoolSubsystem* TexPool,
	UObject* Requester,
	ET66SecondaryStatType StatType,
	const FVector2D& ImageSize)
{
	const FString Slug = GetSecondaryBuffIconSlug(StatType);
	if (Slug.IsEmpty())
	{
		return nullptr;
	}

	const FString FileStem = Slug;
	const FString SourceRelativePath = FString::Printf(TEXT("RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs/%s.png"), *FileStem);
	const FString PackagePath = FString::Printf(TEXT("/Game/UI/PowerUp/SecondaryBuffs/%s"), *FileStem);
	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *FileStem);

	const TArray<FString> CandidateSourcePaths = T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath);
	const bool bHasSourceFile = CandidateSourcePaths.ContainsByPredicate([](const FString& CandidatePath)
	{
		return FPaths::FileExists(CandidatePath);
	});
	const bool bHasImportedTexture = TexPool && FPackageName::DoesPackageExist(PackagePath);

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	if (bHasImportedTexture)
	{
		if (UTexture2D* AssetTexture = T66RuntimeUITextureAccess::LoadAssetTexture(*ObjectPath, TextureFilter::TF_Trilinear, TEXT("TempBuffTexture")))
		{
			InitializeRuntimeImageBrush(Brush, ResolveImageSize(AssetTexture, ImageSize));
			Brush->SetResourceObject(AssetTexture);
		}
		else
		{
			InitializeRuntimeImageBrush(Brush, ResolveImageSize(nullptr, ImageSize));
			const TSoftObjectPtr<UTexture2D> SoftTexture{ FSoftObjectPath(ObjectPath) };
			T66SlateTexture::BindSharedBrushAsync(TexPool, SoftTexture, Requester, Brush, FName(*FString::Printf(TEXT("TempBuff_%s"), *Slug)), false);
		}

		return Brush;
	}

	if (bHasSourceFile)
	{
		UTexture2D* Texture = nullptr;
		for (const FString& CandidatePath : CandidateSourcePaths)
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			Texture = LoadFileTexture(CandidatePath);
			if (Texture)
			{
				break;
			}
		}

		if (!Texture)
		{
			return nullptr;
		}

		InitializeRuntimeImageBrush(Brush, ResolveImageSize(Texture, ImageSize));
		Brush->SetResourceObject(Texture);
		return Brush;
	}

	return nullptr;
}
