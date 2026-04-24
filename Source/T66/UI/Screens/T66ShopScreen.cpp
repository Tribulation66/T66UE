// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ShopScreen.h"
#include "Core/T66AchievementsSubsystem.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Layout/Clipping.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 ShopFontDelta = -10;
	constexpr float ShopPermanentCardArtFrameWidth = 178.f;
	constexpr float ShopPermanentCardArtFrameHeight = 154.f;
	constexpr float ShopCardGap = 12.f;
	constexpr int32 ShopStatsPerRow = 3;
	const FLinearColor ShopLockedTint(0.03f, 0.03f, 0.04f, 0.96f);
	const FLinearColor ShopFillTint = FLinearColor::White;
	const FLinearColor ShopPermanentCardFill(0.14f, 0.11f, 0.07f, 1.0f);
	const FLinearColor ShopPermanentCardInsetFill(0.09f, 0.07f, 0.04f, 1.0f);
	const FLinearColor ShopPermanentCardAccent(0.80f, 0.70f, 0.46f, 1.0f);
	const TCHAR* ShopSettingsAssetRoot = TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/");
	const TCHAR* ShopMainMenuAssetRoot = TEXT("SourceAssets/UI/MainMenuReference/");
	TMap<FString, TStrongObjectPtr<UTexture2D>> GShopFileTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GShopGeneratedBrushCache;
	TMap<FString, TSharedPtr<FButtonStyle>> GShopGeneratedButtonStyleCache;

	int32 AdjustShopFontSize(int32 BaseSize)
	{
		return FMath::Max(8, BaseSize + ShopFontDelta);
	}

	FSlateFontInfo ShopBoldFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontBold(AdjustShopFontSize(BaseSize));
	}

	FSlateFontInfo ShopRegularFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontRegular(AdjustShopFontSize(BaseSize));
	}

	FLinearColor T66ShopShellFill()
	{
		return FT66Style::Background();
	}

	FLinearColor T66ShopPanelFill()
	{
		return FT66Style::PanelOuter();
	}

	FLinearColor T66ShopInsetFill()
	{
		return FT66Style::PanelInner();
	}

	FLinearColor T66ShopButtonFill()
	{
		return FLinearColor(0.53f, 0.62f, 0.47f, 1.0f);
	}

	FLinearColor T66ShopButtonDisabledFill()
	{
		return FLinearColor(0.29f, 0.31f, 0.33f, 1.0f);
	}

	FLinearColor T66ShopNeutralButtonFill()
	{
		return FT66Style::ButtonNeutral();
	}

	FLinearColor T66ShopTabActiveText()
	{
		return FLinearColor(0.99f, 0.93f, 0.74f, 1.0f);
	}

	FLinearColor T66ShopTabInactiveText()
	{
		return FLinearColor(0.86f, 0.80f, 0.68f, 1.0f);
	}

	FT66ButtonParams FlattenShopButton(FT66ButtonParams Params)
	{
		Params
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetUseGlow(false);

		if (Params.FontSize > 0)
		{
			Params.SetFontSize(AdjustShopFontSize(static_cast<int32>(Params.FontSize)));
		}

		return Params;
	}

	TSharedRef<SWidget> MakeShopButton(const FT66ButtonParams& Params)
	{
		return FT66Style::MakeButton(FlattenShopButton(Params));
	}

	TSharedRef<SWidget> MakeShopPanel(const TSharedRef<SWidget>& Content, const FLinearColor& FillColor, const FMargin& Padding, ET66PanelType Type = ET66PanelType::Panel)
	{
		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(FillColor)
				.SetPadding(Padding));
	}

	UTexture2D* LoadShopFileTexture(const FString& FilePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GShopFileTextureCache.Find(FilePath))
		{
			return CachedTexture->Get();
		}

	UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
			FilePath,
			TextureFilter::TF_Trilinear,
			false,
			TEXT("ShopTexture"));
		if (!Texture)
		{
			Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
				FilePath,
				TextureFilter::TF_Trilinear,
				TEXT("ShopTexture"));
		}
		if (!Texture)
		{
			return nullptr;
		}

		GShopFileTextureCache.Add(FilePath, TStrongObjectPtr<UTexture2D>(Texture));
		return Texture;
	}

	bool HasBrushResource(const FSlateBrush* Brush)
	{
		return Brush
			&& Brush->DrawAs != ESlateBrushDrawType::NoDrawType
			&& (Brush->GetResourceObject() != nullptr || Brush->GetResourceName() != NAME_None);
	}

	FString MakeShopSettingsAssetPath(const TCHAR* FileName)
	{
		return FString(ShopSettingsAssetRoot) / FileName;
	}

	FString MakeShopMainMenuAssetPath(const TCHAR* FileName)
	{
		return FString(ShopMainMenuAssetRoot) / FileName;
	}

	void EnsureShopRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize);
	FVector2D ResolveShopImageSize(UTexture2D* Texture, const FVector2D& FallbackImageSize);

	const FSlateBrush* ResolveShopGeneratedBrush(const FString& SourceRelativePath, const FVector2D& ImageSize = FVector2D::ZeroVector)
	{
		const FString BrushKey = FString::Printf(TEXT("%s::%.0fx%.0f"), *SourceRelativePath, ImageSize.X, ImageSize.Y);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GShopGeneratedBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = nullptr;
		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			Texture = LoadShopFileTexture(CandidatePath);
			if (Texture)
			{
				break;
			}
		}

		if (!Texture)
		{
			return nullptr;
		}

		const FVector2D ResolvedSize = ResolveShopImageSize(Texture, ImageSize);
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		EnsureShopRuntimeImageBrush(Brush, ResolvedSize);
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GShopGeneratedBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}

	const FButtonStyle* ResolveShopGeneratedButtonStyle(
		const FString& Key,
		const FString& NormalPath,
		const FString& HoverPath,
		const FString& PressedPath,
		const FString& DisabledPath)
	{
		if (const TSharedPtr<FButtonStyle>* CachedStyle = GShopGeneratedButtonStyleCache.Find(Key))
		{
			return CachedStyle->Get();
		}

		const FButtonStyle& NoBorderStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		TSharedPtr<FButtonStyle> Style = MakeShared<FButtonStyle>(NoBorderStyle);
		if (const FSlateBrush* NormalBrush = ResolveShopGeneratedBrush(NormalPath))
		{
			Style->SetNormal(*NormalBrush);
		}
		if (const FSlateBrush* HoverBrush = ResolveShopGeneratedBrush(HoverPath))
		{
			Style->SetHovered(*HoverBrush);
		}
		if (const FSlateBrush* PressedBrush = ResolveShopGeneratedBrush(PressedPath))
		{
			Style->SetPressed(*PressedBrush);
		}
		if (const FSlateBrush* DisabledBrush = ResolveShopGeneratedBrush(DisabledPath))
		{
			Style->SetDisabled(*DisabledBrush);
		}
		Style->SetNormalPadding(FMargin(0.f));
		Style->SetPressedPadding(FMargin(0.f));

		GShopGeneratedButtonStyleCache.Add(Key, Style);
		return Style.Get();
	}

	const FButtonStyle* ResolveShopCompactButtonStyle()
	{
		return ResolveShopGeneratedButtonStyle(
			TEXT("Shop.CompactButton"),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_normal.png")),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_hover.png")),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_pressed.png")),
			MakeShopSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	const FButtonStyle* ResolveShopToggleButtonStyle(const bool bActive)
	{
		return bActive
			? ResolveShopGeneratedButtonStyle(
				TEXT("Shop.ToggleOn"),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_on_normal.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_on_hover.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_on_pressed.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")))
			: ResolveShopGeneratedButtonStyle(
				TEXT("Shop.ToggleOff"),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_off_normal.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_off_hover.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_off_pressed.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	TSharedRef<SWidget> MakeShopGeneratedPanel(
		const FString& SourceRelativePath,
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FLinearColor& Tint = FLinearColor::White,
		const FLinearColor& FallbackFill = T66ShopPanelFill())
	{
		if (const FSlateBrush* Brush = ResolveShopGeneratedBrush(SourceRelativePath))
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(Tint)
				.Padding(Padding)
				[
					Content
				];
		}

		return MakeShopPanel(Content, FallbackFill, Padding);
	}

	TSharedRef<SWidget> MakeShopGeneratedButton(
		const FT66ButtonParams& Params,
		const FButtonStyle* ButtonStyle,
		const FSlateFontInfo& Font,
		const FLinearColor& TextColor,
		const FMargin& ContentPadding)
	{
		TSharedRef<SButton> Button = SNew(SButton)
			.ButtonStyle(ButtonStyle ? ButtonStyle : &FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(ContentPadding)
			.IsEnabled(Params.IsEnabled)
			.OnClicked(FT66Style::DebounceClick(Params.OnClicked))
			[
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center)
			];

		TSharedRef<SBox> Box = SNew(SBox)
			.MinDesiredWidth(Params.MinWidth)
			.Visibility(Params.Visibility)
			[
				Button
			];
		if (Params.Height > 0.f)
		{
			Box->SetHeightOverride(Params.Height);
		}
		return Box;
	}

	class ST66ShopStatueFillWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66ShopStatueFillWidget) {}
			SLATE_ARGUMENT(const FSlateBrush*, StatueBrush)
			SLATE_ARGUMENT(FVector2D, DesiredSize)
			SLATE_ARGUMENT(float, FillFraction)
			SLATE_ARGUMENT(FLinearColor, LockedTint)
			SLATE_ARGUMENT(FLinearColor, FillTint)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			StatueBrush = InArgs._StatueBrush;
			DesiredSize = InArgs._DesiredSize;
			FillFraction = FMath::Clamp(InArgs._FillFraction, 0.f, 1.f);
			LockedTint = InArgs._LockedTint;
			FillTint = InArgs._FillTint;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			if (DesiredSize.X > 0.f && DesiredSize.Y > 0.f)
			{
				return DesiredSize;
			}

			return StatueBrush ? StatueBrush->GetImageSize() : FVector2D(1.f, 1.f);
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			if (!HasBrushResource(StatueBrush))
			{
				return LayerId;
			}

			const bool bEnabled = ShouldBeEnabled(bParentEnabled);
			const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
			const FLinearColor BrushTint = StatueBrush->GetTint(InWidgetStyle);
			const FLinearColor WidgetTint = InWidgetStyle.GetColorAndOpacityTint();
			int32 RetLayerId = LayerId;

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				RetLayerId++,
				AllottedGeometry.ToPaintGeometry(),
				StatueBrush,
				DrawEffects,
				WidgetTint * LockedTint * BrushTint
			);

			if (FillFraction <= KINDA_SMALL_NUMBER)
			{
				return RetLayerId;
			}

			if (FillFraction >= 1.f - KINDA_SMALL_NUMBER)
			{
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					RetLayerId++,
					AllottedGeometry.ToPaintGeometry(),
					StatueBrush,
					DrawEffects,
					WidgetTint * FillTint * BrushTint
				);
				return RetLayerId;
			}

			const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
			const float FillHeight = FMath::Max(0.f, LocalSize.Y * FillFraction);
			if (FillHeight <= KINDA_SMALL_NUMBER)
			{
				return RetLayerId;
			}

			const FVector2D FillTopLeft(0.f, LocalSize.Y - FillHeight);
			const FVector2D FillTopRight(LocalSize.X, LocalSize.Y - FillHeight);
			const FVector2D FillBottomLeft(0.f, LocalSize.Y);
			const FVector2D FillBottomRight(LocalSize.X, LocalSize.Y);
			const FSlateRenderTransform& Transform = AllottedGeometry.GetAccumulatedRenderTransform();
			const FSlateClippingZone FillClip(
				Transform.TransformPoint(FillTopLeft),
				Transform.TransformPoint(FillTopRight),
				Transform.TransformPoint(FillBottomLeft),
				Transform.TransformPoint(FillBottomRight));

			if (FillClip.HasZeroArea())
			{
				return RetLayerId;
			}

			OutDrawElements.PushClip(FillClip);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				RetLayerId++,
				AllottedGeometry.ToPaintGeometry(),
				StatueBrush,
				DrawEffects,
				WidgetTint * FillTint * BrushTint
			);
			OutDrawElements.PopClip();

			return RetLayerId;
		}

	private:
		const FSlateBrush* StatueBrush = nullptr;
		FVector2D DesiredSize = FVector2D::ZeroVector;
		float FillFraction = 0.f;
		FLinearColor LockedTint = FLinearColor::Black;
		FLinearColor FillTint = FLinearColor::White;
	};

	FString GetShopForbiddenChadPartKey(ET66HeroStatType StatType)
	{
		switch (StatType)
		{
			case ET66HeroStatType::Damage:      return TEXT("left_arm");
			case ET66HeroStatType::AttackSpeed: return TEXT("right_arm");
			case ET66HeroStatType::AttackScale: return TEXT("head");
			case ET66HeroStatType::Accuracy:    return TEXT("head");
			case ET66HeroStatType::Armor:       return TEXT("torso");
			case ET66HeroStatType::Evasion:     return TEXT("left_leg");
			case ET66HeroStatType::Luck:        return TEXT("right_leg");
			default:                            return TEXT("unknown");
		}
	}

	void EnsureShopRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	FVector2D ResolveShopImageSize(UTexture2D* Texture, const FVector2D& FallbackImageSize)
	{
		if (FallbackImageSize.X > 0.f && FallbackImageSize.Y > 0.f)
		{
			return FallbackImageSize;
		}

		if (Texture)
		{
			const int32 TextureWidth = Texture->GetSizeX();
			const int32 TextureHeight = Texture->GetSizeY();
			if (TextureWidth > 0 && TextureHeight > 0)
			{
				return FVector2D(static_cast<float>(TextureWidth), static_cast<float>(TextureHeight));
			}
		}

		return FVector2D(1.f, 1.f);
	}

	TSharedPtr<FSlateBrush> MakeShopImageBrush(
		TMap<FString, TSharedPtr<FSlateBrush>>& OwnedBrushes,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		const FString& BrushKey,
		const FString& SourceRelativePath,
		const FString& PackagePath,
		const FString& ObjectPath,
		const FVector2D& ImageSize)
	{
		const TArray<FString> CandidateSourcePaths = T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath);
		const bool bHasSourceFile = CandidateSourcePaths.ContainsByPredicate([](const FString& CandidatePath)
		{
			return FPaths::FileExists(CandidatePath);
		});
		const bool bHasImportedTexture = TexPool && FPackageName::DoesPackageExist(PackagePath);
		TSharedPtr<FSlateBrush> Brush;

		if (bHasImportedTexture)
		{
			if (UTexture2D* AssetTexture = T66RuntimeUITextureAccess::LoadAssetTexture(*ObjectPath, TextureFilter::TF_Trilinear, TEXT("ShopTexture")))
			{
				Brush = MakeShared<FSlateBrush>();
				EnsureShopRuntimeImageBrush(Brush, ResolveShopImageSize(AssetTexture, ImageSize));
				Brush->SetResourceObject(AssetTexture);
				Brush->TintColor = FSlateColor(FLinearColor::White);
			}
			else
			{
				Brush = MakeShared<FSlateBrush>();
				EnsureShopRuntimeImageBrush(Brush, ResolveShopImageSize(nullptr, ImageSize));
				const TSoftObjectPtr<UTexture2D> Soft{ FSoftObjectPath(ObjectPath) };
				T66SlateTexture::BindSharedBrushAsync(TexPool, Soft, Requester, Brush, FName(*BrushKey), false);
			}
		}
		else if (bHasSourceFile)
		{
			UTexture2D* Texture = nullptr;
			for (const FString& CandidatePath : CandidateSourcePaths)
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				Texture = LoadShopFileTexture(CandidatePath);
				if (Texture)
				{
					break;
				}
			}

			if (Texture)
			{
				Brush = MakeShared<FSlateBrush>();
				EnsureShopRuntimeImageBrush(Brush, ResolveShopImageSize(Texture, ImageSize));
				Brush->SetResourceObject(Texture);
				Brush->TintColor = FSlateColor(FLinearColor::White);
			}
			else
			{
				return nullptr;
			}
		}
		else
		{
			return nullptr;
		}

		OwnedBrushes.Add(BrushKey, Brush);
		return Brush;
	}

	TSharedPtr<FSlateBrush> GetShopForbiddenChadPartBrush(
		TMap<FString, TSharedPtr<FSlateBrush>>& OwnedBrushes,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		ET66HeroStatType StatType,
		const FVector2D& ImageSize)
	{
		const FString PartKey = GetShopForbiddenChadPartKey(StatType);
		const FString FileStem = FString::Printf(TEXT("forbidden_chad_%s"), *PartKey);
		const FString BrushKey = FString::Printf(TEXT("forbidden_chad::%s::%.0fx%.0f"), *PartKey, ImageSize.X, ImageSize.Y);
		// Route through the generated source path so runtime dependency remapping also
		// gives us the authoring folder as an editor fallback.
		const FString SourceRelativePath = FString::Printf(TEXT("SourceAssets/UI/PowerUp/Statues/Generated/forbidden_chad/%s.png"), *FileStem);
		const FString PackagePath = FString::Printf(TEXT("/Game/UI/PowerUp/Statues/forbidden_chad/%s"), *FileStem);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *FileStem);

		return MakeShopImageBrush(OwnedBrushes, TexPool, Requester, BrushKey, SourceRelativePath, PackagePath, ObjectPath, ImageSize);
	}

	FString GetShopSecondaryBuffSlug(ET66SecondaryStatType StatType)
	{
		switch (StatType)
		{
		case ET66SecondaryStatType::AoeDamage:      return TEXT("aoe-damage");
		case ET66SecondaryStatType::BounceDamage:   return TEXT("bounce-damage");
		case ET66SecondaryStatType::PierceDamage:   return TEXT("pierce-damage");
		case ET66SecondaryStatType::DotDamage:      return TEXT("dot-damage");
		case ET66SecondaryStatType::CritDamage:     return TEXT("crit-damage");
		case ET66SecondaryStatType::AoeSpeed:       return TEXT("aoe-speed");
		case ET66SecondaryStatType::BounceSpeed:    return TEXT("bounce-speed");
		case ET66SecondaryStatType::PierceSpeed:    return TEXT("pierce-speed");
		case ET66SecondaryStatType::DotSpeed:       return TEXT("dot-speed");
		case ET66SecondaryStatType::CritChance:     return TEXT("crit-chance");
		case ET66SecondaryStatType::AoeScale:       return TEXT("aoe-scale");
		case ET66SecondaryStatType::BounceScale:    return TEXT("bounce-scale");
		case ET66SecondaryStatType::PierceScale:    return TEXT("pierce-scale");
		case ET66SecondaryStatType::DotScale:       return TEXT("dot-scale");
		case ET66SecondaryStatType::AttackRange:    return TEXT("range");
		case ET66SecondaryStatType::Taunt:          return TEXT("taunt");
		case ET66SecondaryStatType::DamageReduction:return TEXT("damage-reduction");
		case ET66SecondaryStatType::ReflectDamage:  return TEXT("damage-reflection");
		case ET66SecondaryStatType::HpRegen:        return TEXT("hp-regen");
		case ET66SecondaryStatType::Crush:          return TEXT("crush");
		case ET66SecondaryStatType::EvasionChance:  return TEXT("dodge");
		case ET66SecondaryStatType::CounterAttack:  return TEXT("counter-attack");
		case ET66SecondaryStatType::LifeSteal:      return TEXT("life-steal");
		case ET66SecondaryStatType::Invisibility:   return TEXT("invisibility");
		case ET66SecondaryStatType::Assassinate:    return TEXT("assassinate");
		case ET66SecondaryStatType::TreasureChest:  return TEXT("treasure-chest");
		case ET66SecondaryStatType::Cheating:       return TEXT("cheating");
		case ET66SecondaryStatType::Stealing:       return TEXT("stealing");
		case ET66SecondaryStatType::LootCrate:      return TEXT("loot-crate");
		case ET66SecondaryStatType::Alchemy:        return TEXT("alchemy");
		case ET66SecondaryStatType::Accuracy:       return TEXT("accuracy");
		default:                                    return FString();
		}
	}

	TSharedPtr<FSlateBrush> GetShopSecondaryBuffBrush(
		TMap<FString, TSharedPtr<FSlateBrush>>& OwnedBrushes,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		ET66SecondaryStatType StatType,
		const FVector2D& ImageSize)
	{
		const FString Slug = GetShopSecondaryBuffSlug(StatType);
		if (Slug.IsEmpty())
		{
			return nullptr;
		}

		const FString FileStem = Slug;
		const FString BrushKey = FString::Printf(TEXT("secondary_buff::%s::%.0fx%.0f"), *Slug, ImageSize.X, ImageSize.Y);
		const FString SourceRelativePath = FString::Printf(TEXT("RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs/%s.png"), *FileStem);
		const FString PackagePath = FString::Printf(TEXT("/Game/UI/PowerUp/SecondaryBuffs/%s"), *FileStem);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *FileStem);

		return MakeShopImageBrush(OwnedBrushes, TexPool, Requester, BrushKey, SourceRelativePath, PackagePath, ObjectPath, ImageSize);
	}

}

UT66ShopScreen::UT66ShopScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PowerUp;
	bIsModal = false;
}

void UT66ShopScreen::OnScreenActivated_Implementation()
{
	if (HasBuiltSlateUI() && !bNeedsWarmActivationRefresh)
	{
		SetShowingSingleUse(bShowingSingleUse);
		return;
	}

	Super::OnScreenActivated_Implementation();
}

UT66LocalizationSubsystem* UT66ShopScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66BuffSubsystem* UT66ShopScreen::GetBuffSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66BuffSubsystem>();
	}
	return nullptr;
}

void UT66ShopScreen::SetShowingSingleUse(bool bInShowingSingleUse)
{
	bShowingSingleUse = bInShowingSingleUse;
	if (PageSwitcher.IsValid())
	{
		PageSwitcher->SetActiveWidgetIndex(bShowingSingleUse ? 1 : 0);
	}
}

FReply UT66ShopScreen::HandleBackClicked()
{
	NavigateBack();
	return FReply::Handled();
}

FReply UT66ShopScreen::HandleShowPermanentClicked()
{
	SetShowingSingleUse(false);
	return FReply::Handled();
}

FReply UT66ShopScreen::HandleShowSingleUseClicked()
{
	SetShowingSingleUse(true);
	return FReply::Handled();
}

FReply UT66ShopScreen::HandleUnlockClicked(ET66HeroStatType StatType)
{
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	if (Buffs && Buffs->UnlockNextFillStep(StatType))
	{
		RefreshScreen();
	}
	return FReply::Handled();
}

FReply UT66ShopScreen::HandlePurchaseSingleUseClicked(ET66SecondaryStatType StatType)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->PurchaseSingleUseBuff(StatType))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

void UT66ShopScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	bNeedsWarmActivationRefresh = false;
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66ShopScreen::BuildSlateUI()
{
	bNeedsWarmActivationRefresh = false;

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	OwnedBrushes.Reset();

	const FText PermanentTabText = NSLOCTEXT("T66.Shop", "PermanentTab", "PERMANENT");
	const FText SingleUseTabText = NSLOCTEXT("T66.Shop", "SingleUseTab", "SINGLE USE");
	const FText ForbiddenChadSetText = NSLOCTEXT("T66.Shop", "ForbiddenChadSet", "FORBIDDEN CHAD");
	const FText SingleUseHintText = NSLOCTEXT("T66.Shop", "SingleUseHint", "Buy stackable buffs for any secondary stat. Owned buffs can be selected from Hero Selection, up to 4 total per run.");
	const FText SingleUseBonusText = NSLOCTEXT("T66.Shop", "SingleUseBonus", "+10% when selected");

	const int32 Balance = Achievements ? Achievements->GetChadCouponBalance() : (Buffs ? Buffs->GetChadCouponBalance() : 0);
	const FText BalanceBadgeText = FText::Format(NSLOCTEXT("T66.Shop", "BalanceBadge", "{0} CC"), FText::AsNumber(Balance));

	auto GetStatLabel = [Loc](ET66HeroStatType Stat) -> FText
	{
		if (Loc)
		{
			switch (Stat)
			{
				case ET66HeroStatType::Damage:      return Loc->GetText_Stat_Damage();
				case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
				case ET66HeroStatType::AttackScale: return Loc->GetText_Stat_AttackScale();
				case ET66HeroStatType::Accuracy:    return Loc->GetText_Stat_Accuracy();
				case ET66HeroStatType::Armor:       return Loc->GetText_Stat_Armor();
				case ET66HeroStatType::Evasion:     return Loc->GetText_Stat_Evasion();
				case ET66HeroStatType::Luck:        return Loc->GetText_Stat_Luck();
				default: break;
			}
		}
		return NSLOCTEXT("T66.Shop", "StatUnknown", "?");
	};
	auto GetSecondaryLabel = [Loc](ET66SecondaryStatType StatType) -> FText
	{
		return Loc ? Loc->GetText_SecondaryStatName(StatType) : FText::FromString(TEXT("?"));
	};

	struct FSingleUseRowDef
	{
		ET66HeroStatType PrimaryStat = ET66HeroStatType::Damage;
		TArray<ET66SecondaryStatType> SecondaryStats;
	};

	const TArray<FSingleUseRowDef> SingleUseRows = {
		{ ET66HeroStatType::Damage,      { ET66SecondaryStatType::AoeDamage, ET66SecondaryStatType::BounceDamage, ET66SecondaryStatType::PierceDamage, ET66SecondaryStatType::DotDamage } },
		{ ET66HeroStatType::AttackSpeed, { ET66SecondaryStatType::AoeSpeed, ET66SecondaryStatType::BounceSpeed, ET66SecondaryStatType::PierceSpeed, ET66SecondaryStatType::DotSpeed } },
		{ ET66HeroStatType::AttackScale, { ET66SecondaryStatType::AoeScale, ET66SecondaryStatType::BounceScale, ET66SecondaryStatType::PierceScale, ET66SecondaryStatType::DotScale } },
		{ ET66HeroStatType::Accuracy,    { ET66SecondaryStatType::CritDamage, ET66SecondaryStatType::CritChance, ET66SecondaryStatType::AttackRange, ET66SecondaryStatType::Accuracy } },
		{ ET66HeroStatType::Armor,       { ET66SecondaryStatType::Taunt, ET66SecondaryStatType::DamageReduction, ET66SecondaryStatType::ReflectDamage, ET66SecondaryStatType::Crush } },
		{ ET66HeroStatType::Evasion,     { ET66SecondaryStatType::EvasionChance, ET66SecondaryStatType::CounterAttack, ET66SecondaryStatType::Invisibility, ET66SecondaryStatType::Assassinate } },
		{ ET66HeroStatType::Luck,        { ET66SecondaryStatType::TreasureChest, ET66SecondaryStatType::Cheating, ET66SecondaryStatType::Stealing, ET66SecondaryStatType::LootCrate } },
	};
	const TArray<ET66HeroStatType> PermanentCardOrder = {
		ET66HeroStatType::Damage,
		ET66HeroStatType::AttackSpeed,
		ET66HeroStatType::AttackScale,
		ET66HeroStatType::Accuracy,
		ET66HeroStatType::Armor,
		ET66HeroStatType::Evasion,
		ET66HeroStatType::Luck
	};

	auto GetPermanentPartLabel = [](ET66HeroStatType StatType) -> FText
	{
		switch (StatType)
		{
			case ET66HeroStatType::Damage:      return NSLOCTEXT("T66.Shop", "ForbiddenChadLeftArm", "LEFT ARM");
			case ET66HeroStatType::AttackSpeed: return NSLOCTEXT("T66.Shop", "ForbiddenChadRightArm", "RIGHT ARM");
			case ET66HeroStatType::AttackScale: return NSLOCTEXT("T66.Shop", "ForbiddenChadHead", "HEAD");
			case ET66HeroStatType::Accuracy:    return NSLOCTEXT("T66.Shop", "ForbiddenChadEyes", "EYES");
			case ET66HeroStatType::Armor:       return NSLOCTEXT("T66.Shop", "ForbiddenChadTorso", "TORSO");
			case ET66HeroStatType::Evasion:     return NSLOCTEXT("T66.Shop", "ForbiddenChadLeftLeg", "LEFT LEG");
			case ET66HeroStatType::Luck:        return NSLOCTEXT("T66.Shop", "ForbiddenChadRightLeg", "RIGHT LEG");
			default:                            return NSLOCTEXT("T66.Shop", "ForbiddenChadUnknown", "PART");
		}
	};

	auto MakePermanentStatPanel = [&](ET66HeroStatType StatType) -> TSharedRef<SWidget>
	{
		const int32 Cost = Buffs ? Buffs->GetCostForNextFillStepUnlock(StatType) : 0;
		const bool bMaxed = Buffs && Buffs->IsStatMaxed(StatType);
		const int32 UnlockedSteps = Buffs ? Buffs->GetUnlockedFillStepCount(StatType) : 0;
		const float FillFraction = static_cast<float>(UnlockedSteps) / static_cast<float>(UT66BuffSubsystem::MaxFillStepsPerStat);
		const FText ButtonText = bMaxed
			? NSLOCTEXT("T66.Shop", "Max", "MAX")
			: FText::Format(NSLOCTEXT("T66.Shop", "PermanentCost", "{0} CC"), FText::AsNumber(Cost));
		const FText ProgressText = FText::Format(
			NSLOCTEXT("T66.Shop", "PermanentProgress", "{0}/{1}"),
			FText::AsNumber(UnlockedSteps),
			FText::AsNumber(UT66BuffSubsystem::MaxFillStepsPerStat));
		const TSharedPtr<FSlateBrush> PartBrush = GetShopForbiddenChadPartBrush(OwnedBrushes, TexPool, this, StatType, FVector2D::ZeroVector);
		const TSharedRef<SWidget> PartWidget = PartBrush.IsValid()
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(ShopPermanentCardArtFrameWidth)
				.HeightOverride(ShopPermanentCardArtFrameHeight)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::Both)
					[
						SNew(ST66ShopStatueFillWidget)
						.StatueBrush(PartBrush.Get())
						.DesiredSize(PartBrush->GetImageSize())
						.FillFraction(FillFraction)
						.LockedTint(ShopLockedTint)
						.FillTint(ShopFillTint)
					]
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(ShopPermanentCardArtFrameWidth)
				.HeightOverride(ShopPermanentCardArtFrameHeight)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Shop", "MissingForbiddenChadArt", "ART"))
					.Font(ShopBoldFont(16))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]);

		return MakeShopGeneratedPanel(
			MakeShopSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(ForbiddenChadSetText)
				.Font(ShopBoldFont(12))
				.ColorAndOpacity(ShopPermanentCardAccent)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(GetPermanentPartLabel(StatType))
				.Font(ShopBoldFont(20))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(GetStatLabel(StatType))
				.Font(ShopRegularFont(14))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				MakeShopGeneratedPanel(
					MakeShopSettingsAssetPath(TEXT("settings_row_shell_full.png")),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						PartWidget
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(ProgressText)
						.Font(ShopRegularFont(13))
						.ColorAndOpacity(ShopPermanentCardAccent)
						.Justification(ETextJustify::Center)
					],
					FMargin(14.f, 10.f),
					FLinearColor(0.94f, 0.90f, 0.78f, 1.0f),
					ShopPermanentCardInsetFill)
			]
			+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(0.f, 10.f, 0.f, 0.f)
			[
				MakeShopGeneratedButton(
					FT66ButtonParams(ButtonText, FOnClicked::CreateUObject(this, &UT66ShopScreen::HandleUnlockClicked, StatType), ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(44.f)
					.SetColor(TAttribute<FSlateColor>::CreateLambda([bMaxed, Balance, Cost]() -> FSlateColor
					{
						if (bMaxed)
						{
							return FSlateColor(T66ShopButtonFill());
						}

						return FSlateColor(Balance >= Cost ? T66ShopButtonFill() : T66ShopButtonDisabledFill());
					}))
					.SetEnabled(TAttribute<bool>::CreateLambda([bMaxed, Balance, Cost]() { return !bMaxed && Balance >= Cost; }))
					,
					ResolveShopCompactButtonStyle(),
					ShopBoldFont(16),
					FT66Style::Tokens::Text,
					FMargin(14.f, 7.f, 14.f, 6.f)
				)
			],
			FMargin(16.f, 14.f, 16.f, 16.f),
			FLinearColor::White,
			ShopPermanentCardFill);
	};

	auto MakeSingleUseSecondaryCard = [&](ET66SecondaryStatType StatType) -> TSharedRef<SWidget>
	{
		const int32 Cost = Buffs ? Buffs->GetSingleUseBuffCost() : UT66BuffSubsystem::SingleUseBuffCostCC;
		const int32 OwnedCount = Buffs ? Buffs->GetOwnedSingleUseBuffCount(StatType) : 0;
		const FText OwnedText = FText::Format(NSLOCTEXT("T66.Shop", "SingleUseOwnedCount", "Owned: {0}"), FText::AsNumber(OwnedCount));
		const FText ButtonText = FText::Format(NSLOCTEXT("T66.Shop", "SingleUseCost", "BUY {0} CC"), FText::AsNumber(Cost));
		const TSharedPtr<FSlateBrush> IconBrush = GetShopSecondaryBuffBrush(OwnedBrushes, TexPool, this, StatType, FVector2D(64.f, 64.f));
		const TSharedRef<SWidget> IconWidget = IconBrush.IsValid()
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(64.f)
				.HeightOverride(64.f)
				[
					SNew(SImage)
					.Image(IconBrush.Get())
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(64.f)
				.HeightOverride(64.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Shop", "MissingSecondaryArt", "ART"))
					.Font(ShopBoldFont(12))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]);

		return MakeShopGeneratedPanel(
			MakeShopSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				IconWidget
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(GetSecondaryLabel(StatType))
				.Font(ShopBoldFont(11))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.WrapTextAt(120.f)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(SingleUseBonusText)
				.Font(ShopRegularFont(10))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(OwnedText)
				.Font(ShopRegularFont(10))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SSpacer)
			]
			+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom)
			[
				MakeShopGeneratedButton(
					FT66ButtonParams(ButtonText, FOnClicked::CreateUObject(this, &UT66ShopScreen::HandlePurchaseSingleUseClicked, StatType), ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(34.f)
					.SetColor(TAttribute<FSlateColor>::CreateLambda([Balance, Cost]() -> FSlateColor
					{
						return FSlateColor(Balance >= Cost ? T66ShopButtonFill() : T66ShopButtonDisabledFill());
					}))
					.SetEnabled(TAttribute<bool>::CreateLambda([Balance, Cost]() { return Balance >= Cost; }))
					,
					ResolveShopCompactButtonStyle(),
					ShopBoldFont(12),
					FT66Style::Tokens::Text,
					FMargin(10.f, 5.f, 10.f, 4.f)
				)
			],
			FMargin(12.f, 10.f, 12.f, 12.f),
			FLinearColor::White,
			T66ShopPanelFill());
	};

	auto MakeStatRow = [&](const TArray<ET66HeroStatType>& DisplayOrder, int32 StartIndex, const TFunction<TSharedRef<SWidget>(ET66HeroStatType)>& CardBuilder) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> StatRow = SNew(SHorizontalBox);
		for (int32 LocalIndex = 0; LocalIndex < ShopStatsPerRow; ++LocalIndex)
		{
			const int32 StatIndex = StartIndex + LocalIndex;
			StatRow->AddSlot()
				.FillWidth(1.f)
				.Padding(LocalIndex < ShopStatsPerRow - 1 ? FMargin(0.f, 0.f, ShopCardGap, 0.f) : FMargin(0.f))
				[
					StatIndex < DisplayOrder.Num()
						? CardBuilder(DisplayOrder[StatIndex])
						: StaticCastSharedRef<SWidget>(SNew(SSpacer))
				];
		}

		return StatRow;
	};

	TSharedRef<SVerticalBox> PermanentRowsBox = SNew(SVerticalBox);
	for (int32 StartIndex = 0; StartIndex < PermanentCardOrder.Num(); StartIndex += ShopStatsPerRow)
	{
		PermanentRowsBox->AddSlot()
			.FillHeight(1.f)
			.Padding(0.f, StartIndex > 0 ? ShopCardGap : 0.f, 0.f, 0.f)
			[
				MakeStatRow(PermanentCardOrder, StartIndex, TFunction<TSharedRef<SWidget>(ET66HeroStatType)>([&](ET66HeroStatType StatType)
				{
					return MakePermanentStatPanel(StatType);
				}))
			];
	}

	TSharedRef<SWidget> PermanentPage = StaticCastSharedRef<SWidget>(PermanentRowsBox);

	TSharedRef<SVerticalBox> SingleUseRowsBox = SNew(SVerticalBox);
	SingleUseRowsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
	[
		MakeShopGeneratedPanel(
			MakeShopSettingsAssetPath(TEXT("settings_row_shell_full.png")),
			SNew(STextBlock)
			.Text(SingleUseHintText)
			.Font(ShopRegularFont(13))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			.Justification(ETextJustify::Center),
			FMargin(18.f, 12.f),
			FLinearColor::White,
			T66ShopInsetFill())
	];

	for (int32 RowIndex = 0; RowIndex < SingleUseRows.Num(); ++RowIndex)
	{
		const FSingleUseRowDef& RowDef = SingleUseRows[RowIndex];
		const int32 CardCount = RowDef.SecondaryStats.Num();
		TSharedRef<SHorizontalBox> CardsRow = SNew(SHorizontalBox);
		for (int32 CardIndex = 0; CardIndex < CardCount; ++CardIndex)
		{
			CardsRow->AddSlot()
				.FillWidth(1.f)
				.Padding(CardIndex < CardCount - 1 ? FMargin(0.f, 0.f, ShopCardGap, 0.f) : FMargin(0.f))
				[
					MakeSingleUseSecondaryCard(RowDef.SecondaryStats[CardIndex])
				];
		}

		SingleUseRowsBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, RowIndex > 0 ? ShopCardGap : 0.f, 0.f, 0.f)
			[
				MakeShopGeneratedPanel(
					MakeShopSettingsAssetPath(TEXT("settings_row_shell_split.png")),
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, ShopCardGap, 0.f)
					[
						MakeShopGeneratedPanel(
							MakeShopSettingsAssetPath(TEXT("settings_row_shell_full.png")),
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(GetStatLabel(RowDef.PrimaryStat))
								.Font(ShopBoldFont(18))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::Format(
									NSLOCTEXT("T66.Shop", "SingleUseRowCaption", "{0} secondary buffs"),
									FText::AsNumber(CardCount)))
								.Font(ShopRegularFont(10))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								.Justification(ETextJustify::Center)
							],
							FMargin(16.f, 12.f),
							FLinearColor::White,
							T66ShopInsetFill())
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						CardsRow
					],
					FMargin(14.f, 12.f),
					FLinearColor::White,
					T66ShopPanelFill())
			];
	}

	TSharedRef<SWidget> SingleUsePage =
		SNew(SScrollBox)
		.ScrollBarVisibility(EVisibility::Visible)
		+ SScrollBox::Slot()
		[
			SingleUseRowsBox
		];

	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 22.f;
	const float TopInset = UIManager
		? FMath::Max(0.f, (UIManager->GetFrontendTopBarContentHeight() - TopBarOverlapPx) / ResponsiveScale)
		: 0.f;
	const TSharedRef<SWidget> Root =
		SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		[
			MakeShopGeneratedPanel(
				MakeShopSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
		SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f).HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
						[
							MakeShopGeneratedButton(
								FT66ButtonParams(PermanentTabText, FOnClicked::CreateUObject(this, &UT66ShopScreen::HandleShowPermanentClicked), ET66ButtonType::Primary)
								.SetMinWidth(180.f)
								.SetHeight(52.f)
								.SetColor(!bShowingSingleUse ? T66ShopButtonFill() : T66ShopNeutralButtonFill()),
								ResolveShopToggleButtonStyle(!bShowingSingleUse),
								ShopBoldFont(18),
								!bShowingSingleUse ? T66ShopTabActiveText() : T66ShopTabInactiveText(),
								FMargin(18.f, 10.f, 18.f, 8.f))
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							MakeShopGeneratedButton(
								FT66ButtonParams(SingleUseTabText, FOnClicked::CreateUObject(this, &UT66ShopScreen::HandleShowSingleUseClicked), ET66ButtonType::Neutral)
								.SetMinWidth(180.f)
								.SetHeight(52.f)
								.SetColor(bShowingSingleUse ? T66ShopButtonFill() : T66ShopNeutralButtonFill()),
								ResolveShopToggleButtonStyle(bShowingSingleUse),
								ShopBoldFont(18),
								bShowingSingleUse ? T66ShopTabActiveText() : T66ShopTabInactiveText(),
								FMargin(18.f, 10.f, 18.f, 8.f))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(18.f, 0.f, 0.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(154.f)
							.HeightOverride(52.f)
							[
								MakeShopGeneratedPanel(
									MakeShopMainMenuAssetPath(TEXT("TopBar/currency_slot_blank.png")),
									SNew(STextBlock)
									.Text(BalanceBadgeText)
									.Font(ShopBoldFont(16))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center),
									FMargin(18.f, 10.f, 18.f, 8.f),
									FLinearColor::White,
									T66ShopInsetFill())
							]
						]
					]
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SBox)
					[
						SAssignNew(PageSwitcher, SWidgetSwitcher)
						.WidgetIndex(bShowingSingleUse ? 1 : 0)
						+ SWidgetSwitcher::Slot()[ PermanentPage ]
						+ SWidgetSwitcher::Slot()[ SingleUsePage ]
					]
				]
			]
		,
		FMargin(18.f),
		FLinearColor::White,
		T66ShopShellFill())
		];

	return Root;
}
