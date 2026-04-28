// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PowerUpScreen.h"
#include "Core/T66AchievementsSubsystem.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Layout/Clipping.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
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
	constexpr int32 ShopFontDelta = -2;
	constexpr float ShopCardGap = 12.f;
	constexpr int32 ShopDiplomaUpgradeCount = UT66BuffSubsystem::MaxFillStepsPerStat;
	constexpr int32 ShopDiplomaStatIncrease = 1;
	const FLinearColor ShopPermanentCardFill(0.14f, 0.11f, 0.07f, 1.0f);
	const FLinearColor ShopPermanentCardAccent(0.80f, 0.70f, 0.46f, 1.0f);
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

	FLinearColor T66PowerUpShellFill()
	{
		return FT66Style::Background();
	}

	FLinearColor T66PowerUpPanelFill()
	{
		return FT66Style::PanelOuter();
	}

	FLinearColor T66PowerUpInsetFill()
	{
		return FT66Style::PanelInner();
	}

	FLinearColor T66PowerUpButtonFill()
	{
		return FLinearColor(0.53f, 0.62f, 0.47f, 1.0f);
	}

	FLinearColor T66PowerUpButtonDisabledFill()
	{
		return FLinearColor(0.29f, 0.31f, 0.33f, 1.0f);
	}

	FLinearColor T66PowerUpNeutralButtonFill()
	{
		return FT66Style::ButtonNeutral();
	}

	FLinearColor T66PowerUpTabActiveText()
	{
		return FLinearColor(0.99f, 0.93f, 0.74f, 1.0f);
	}

	FLinearColor T66PowerUpTabInactiveText()
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
		const FString Name(FileName);
		const auto TopBarPath = [](const TCHAR* State) -> FString
		{
			return FString::Printf(TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_%s.png"), State);
		};

		if (Name.StartsWith(TEXT("settings_toggle_on_")))
		{
			return TopBarPath(TEXT("pressed"));
		}
		if (Name.StartsWith(TEXT("settings_compact_neutral_")) || Name.StartsWith(TEXT("settings_toggle_off_")))
		{
			if (Name.Contains(TEXT("_hover"))) return TopBarPath(TEXT("hover"));
			if (Name.Contains(TEXT("_pressed"))) return TopBarPath(TEXT("pressed"));
			return TopBarPath(TEXT("normal"));
		}
		if (Name.StartsWith(TEXT("settings_toggle_inactive_")))
		{
			return TopBarPath(TEXT("disabled"));
		}
		if (Name == TEXT("settings_content_shell_frame.png"))
		{
			return TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/panel_large_normal.png");
		}
		if (Name == TEXT("settings_row_shell_full.png") || Name == TEXT("settings_row_shell_split.png"))
		{
			return TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/modal_frame_normal.png");
		}
		if (Name == TEXT("settings_dropdown_field.png"))
		{
			return TEXT("SourceAssets/UI/MasterLibrary/Slices/Controls/dropdown_field_normal.png");
		}

		return FString(TEXT("SourceAssets/UI/MasterLibrary/Slices/")) / Name;
	}

	FMargin GetShopGeneratedBrushMargin(const FString& SourceRelativePath)
	{
		if (SourceRelativePath.Contains(TEXT("panel_large_normal.png")))
		{
			return FMargin(0.067f, 0.043f, 0.067f, 0.043f);
		}
		if (SourceRelativePath.Contains(TEXT("modal_frame_normal.png")))
		{
			return FMargin(0.052f, 0.094f, 0.052f, 0.094f);
		}
		if (SourceRelativePath.Contains(TEXT("dropdown_field_normal.png")))
		{
			return FMargin(0.06f, 0.34f, 0.06f, 0.34f);
		}
		if (SourceRelativePath.Contains(TEXT("topbar_nav_")))
		{
			return FMargin(0.093f, 0.213f, 0.093f, 0.213f);
		}

		return FMargin(0.f);
	}

	bool IsZeroShopMargin(const FMargin& Margin)
	{
		return FMath::IsNearlyZero(Margin.Left)
			&& FMath::IsNearlyZero(Margin.Top)
			&& FMath::IsNearlyZero(Margin.Right)
			&& FMath::IsNearlyZero(Margin.Bottom);
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
		const FMargin BrushMargin = GetShopGeneratedBrushMargin(SourceRelativePath);
		Brush->DrawAs = IsZeroShopMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolvedSize;
		Brush->Margin = BrushMargin;
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
			TEXT("PowerUp.CompactButton"),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_normal.png")),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_hover.png")),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_pressed.png")),
			MakeShopSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	const FButtonStyle* ResolveShopToggleButtonStyle(const bool bActive)
	{
		return bActive
			? ResolveShopGeneratedButtonStyle(
				TEXT("PowerUp.ToggleOn"),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_on_normal.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_on_hover.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_on_pressed.png")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")))
			: ResolveShopGeneratedButtonStyle(
				TEXT("PowerUp.ToggleOff"),
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
		const FLinearColor& FallbackFill = T66PowerUpPanelFill())
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
		const TSharedRef<SWidget> ButtonContent = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center));

		return FT66Style::MakeBareButton(
			FT66BareButtonParams(Params.OnClicked, ButtonContent)
			.SetButtonStyle(ButtonStyle ? ButtonStyle : &FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.SetPadding(ContentPadding)
			.SetHAlign(HAlign_Center)
			.SetVAlign(VAlign_Center)
			.SetEnabled(Params.IsEnabled)
			.SetMinWidth(Params.MinWidth)
			.SetHeight(Params.Height)
			.SetVisibility(Params.Visibility));
	}

	class ST66PowerUpStatueFillWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66PowerUpStatueFillWidget) {}
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

UT66PowerUpScreen::UT66PowerUpScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PowerUp;
	bIsModal = false;
}

void UT66PowerUpScreen::OnScreenActivated_Implementation()
{
	FString RequestedPowerUpTab;
	bool bHasRequestedPowerUpTab = false;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66PowerUpTab="), RequestedPowerUpTab))
	{
		bHasRequestedPowerUpTab = true;
		bShowingSingleUse =
			RequestedPowerUpTab.Equals(TEXT("SingleUse"), ESearchCase::IgnoreCase)
			|| RequestedPowerUpTab.Equals(TEXT("Single"), ESearchCase::IgnoreCase)
			|| RequestedPowerUpTab.Equals(TEXT("Temporary"), ESearchCase::IgnoreCase);
	}

	if (HasBuiltSlateUI() && !bNeedsWarmActivationRefresh)
	{
		if (bHasRequestedPowerUpTab)
		{
			ForceRebuildSlate();
		}
		else
		{
			SetShowingSingleUse(bShowingSingleUse);
		}
		return;
	}

	Super::OnScreenActivated_Implementation();
}

UT66LocalizationSubsystem* UT66PowerUpScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66BuffSubsystem* UT66PowerUpScreen::GetBuffSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66BuffSubsystem>();
	}
	return nullptr;
}

void UT66PowerUpScreen::SetShowingSingleUse(bool bInShowingSingleUse)
{
	bShowingSingleUse = bInShowingSingleUse;
	if (PageSwitcher.IsValid())
	{
		PageSwitcher->SetActiveWidgetIndex(bShowingSingleUse ? 1 : 0);
	}
}

FReply UT66PowerUpScreen::HandleBackClicked()
{
	NavigateBack();
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleShowPermanentClicked()
{
	SetShowingSingleUse(false);
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleShowSingleUseClicked()
{
	SetShowingSingleUse(true);
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleUnlockClicked(ET66HeroStatType StatType)
{
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	if (Buffs && Buffs->UnlockNextFillStep(StatType))
	{
		RefreshScreen();
	}
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandlePurchaseSingleUseClicked(ET66SecondaryStatType StatType)
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

void UT66PowerUpScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	bNeedsWarmActivationRefresh = false;
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66PowerUpScreen::BuildSlateUI()
{
	bNeedsWarmActivationRefresh = false;

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	OwnedBrushes.Reset();

	const int32 SingleUsePercent = FMath::RoundToInt((UT66BuffSubsystem::SingleUseSecondaryBuffMultiplier - 1.f) * 100.f);
	const FText PermanentTabText = NSLOCTEXT("T66.PowerUp", "PermanentTab", "DIPLOMAS");
	const FText SingleUseTabText = NSLOCTEXT("T66.PowerUp", "SingleUseTab", "DRUGS");
	const FText PermanentHintText = FText::Format(
		NSLOCTEXT("T66.PowerUp", "PermanentHint", "Graduate diplomas for permanent +{0} primary-stat upgrades across every run."),
		FText::AsNumber(ShopDiplomaStatIncrease));
	const FText SingleUseHintText = FText::Format(
		NSLOCTEXT("T66.PowerUp", "SingleUseHint", "Buy drug cards for +{0}% secondary-stat boosts. Owned drugs can be equipped from Hero Selection, up to 4 total per run."),
		FText::AsNumber(SingleUsePercent));

	const int32 Balance = Achievements ? Achievements->GetChadCouponBalance() : (Buffs ? Buffs->GetChadCouponBalance() : 0);

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
		return NSLOCTEXT("T66.PowerUp", "StatUnknown", "?");
	};
	auto GetSecondaryLabel = [Loc](ET66SecondaryStatType StatType) -> FText
	{
		return Loc ? Loc->GetText_SecondaryStatName(StatType) : FText::FromString(TEXT("?"));
	};

	auto GetSingleUseDrugName = [](ET66SecondaryStatType StatType) -> FText
	{
		switch (StatType)
		{
			case ET66SecondaryStatType::AoeDamage:       return NSLOCTEXT("T66.PowerUp", "Drug_AoeDamage", "OXYMETHOLONE");
			case ET66SecondaryStatType::BounceDamage:    return NSLOCTEXT("T66.PowerUp", "Drug_BounceDamage", "METHANDROSTENOLONE");
			case ET66SecondaryStatType::PierceDamage:    return NSLOCTEXT("T66.PowerUp", "Drug_PierceDamage", "FLUOXYMESTERONE");
			case ET66SecondaryStatType::DotDamage:       return NSLOCTEXT("T66.PowerUp", "Drug_DotDamage", "NANDROLONE DECANOATE");
			case ET66SecondaryStatType::AoeSpeed:        return NSLOCTEXT("T66.PowerUp", "Drug_AoeSpeed", "CAFFEINE CITRATE");
			case ET66SecondaryStatType::BounceSpeed:     return NSLOCTEXT("T66.PowerUp", "Drug_BounceSpeed", "MODAFINIL");
			case ET66SecondaryStatType::PierceSpeed:     return NSLOCTEXT("T66.PowerUp", "Drug_PierceSpeed", "EPHEDRINE HCL");
			case ET66SecondaryStatType::DotSpeed:        return NSLOCTEXT("T66.PowerUp", "Drug_DotSpeed", "SALBUTAMOL SULFATE");
			case ET66SecondaryStatType::AoeScale:        return NSLOCTEXT("T66.PowerUp", "Drug_AoeScale", "TESTOSTERONE ENANTHATE");
			case ET66SecondaryStatType::BounceScale:     return NSLOCTEXT("T66.PowerUp", "Drug_BounceScale", "BOLDENONE UNDECYLENATE");
			case ET66SecondaryStatType::PierceScale:     return NSLOCTEXT("T66.PowerUp", "Drug_PierceScale", "DROSTANOLONE PROPIONATE");
			case ET66SecondaryStatType::DotScale:        return NSLOCTEXT("T66.PowerUp", "Drug_DotScale", "METHENOLONE ENANTHATE");
			case ET66SecondaryStatType::CritDamage:      return NSLOCTEXT("T66.PowerUp", "Drug_CritDamage", "TRENBOLONE ACETATE");
			case ET66SecondaryStatType::CritChance:      return NSLOCTEXT("T66.PowerUp", "Drug_CritChance", "STANOZOLOL");
			case ET66SecondaryStatType::AttackRange:     return NSLOCTEXT("T66.PowerUp", "Drug_AttackRange", "CLENBUTEROL HCL");
			case ET66SecondaryStatType::Accuracy:        return NSLOCTEXT("T66.PowerUp", "Drug_Accuracy", "ATOMOXETINE HCL");
			case ET66SecondaryStatType::Taunt:           return NSLOCTEXT("T66.PowerUp", "Drug_Taunt", "HYDROCORTISONE");
			case ET66SecondaryStatType::DamageReduction: return NSLOCTEXT("T66.PowerUp", "Drug_DamageReduction", "PREDNISONE");
			case ET66SecondaryStatType::ReflectDamage:   return NSLOCTEXT("T66.PowerUp", "Drug_ReflectDamage", "DEXAMETHASONE");
			case ET66SecondaryStatType::Crush:           return NSLOCTEXT("T66.PowerUp", "Drug_Crush", "BETAMETHASONE");
			case ET66SecondaryStatType::EvasionChance:   return NSLOCTEXT("T66.PowerUp", "Drug_EvasionChance", "SCOPOLAMINE HBR");
			case ET66SecondaryStatType::CounterAttack:   return NSLOCTEXT("T66.PowerUp", "Drug_CounterAttack", "LIDOCAINE HCL");
			case ET66SecondaryStatType::Invisibility:    return NSLOCTEXT("T66.PowerUp", "Drug_Invisibility", "DIPHENHYDRAMINE HCL");
			case ET66SecondaryStatType::Assassinate:     return NSLOCTEXT("T66.PowerUp", "Drug_Assassinate", "ATROPINE SULFATE");
			case ET66SecondaryStatType::TreasureChest:   return NSLOCTEXT("T66.PowerUp", "Drug_TreasureChest", "NICOTINAMIDE RIBOSIDE");
			case ET66SecondaryStatType::Cheating:        return NSLOCTEXT("T66.PowerUp", "Drug_Cheating", "SILDENAFIL CITRATE");
			case ET66SecondaryStatType::Stealing:        return NSLOCTEXT("T66.PowerUp", "Drug_Stealing", "LOPERAMIDE HCL");
			case ET66SecondaryStatType::LootCrate:       return NSLOCTEXT("T66.PowerUp", "Drug_LootCrate", "METFORMIN HCL");
			default:                                     return NSLOCTEXT("T66.PowerUp", "Drug_Unknown", "COMPOUND");
		}
	};

	auto GetSingleUseEffectText = [&](ET66SecondaryStatType StatType) -> FText
	{
		return FText::Format(
			NSLOCTEXT("T66.PowerUp", "SingleUseEffectFormat", "+{0}% {1}"),
			FText::AsNumber(SingleUsePercent),
			GetSecondaryLabel(StatType));
	};

	auto GetPermanentEffectText = [&](ET66HeroStatType StatType) -> FText
	{
		return FText::Format(
			NSLOCTEXT("T66.PowerUp", "PermanentEffectFormat", "+{0} {1}"),
			FText::AsNumber(ShopDiplomaStatIncrease),
			GetStatLabel(StatType));
	};

	auto GetDrugRowTitle = [&](ET66HeroStatType StatType) -> FText
	{
		return FText::Format(
			NSLOCTEXT("T66.PowerUp", "DrugRowTitle", "{0} Drugs"),
			GetStatLabel(StatType));
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

	auto MakeUpperText = [](const FText& Text) -> FText
	{
		return FText::FromString(Text.ToString().ToUpper());
	};

	auto GetDiplomaAssetSlug = [](ET66HeroStatType StatType) -> FString
	{
		switch (StatType)
		{
			case ET66HeroStatType::Damage:      return TEXT("damage");
			case ET66HeroStatType::AttackSpeed: return TEXT("attack_speed");
			case ET66HeroStatType::AttackScale: return TEXT("attack_scale");
			case ET66HeroStatType::Accuracy:    return TEXT("accuracy");
			case ET66HeroStatType::Armor:       return TEXT("armor");
			case ET66HeroStatType::Evasion:     return TEXT("evasion");
			case ET66HeroStatType::Luck:        return TEXT("luck");
			default: break;
		}

		return TEXT("damage");
	};

	auto GetDiplomaRankAssetName = [](int32 VisibleUnlockedSteps) -> FString
	{
		switch (VisibleUnlockedSteps)
		{
			case 1: return TEXT("bachelors");
			case 2: return TEXT("masters");
			case 3: return TEXT("phd");
			case 4: return TEXT("magistrate");
			default: break;
		}

		return TEXT("");
	};

	auto GetDiplomaImagePath = [&](ET66HeroStatType StatType, int32 VisibleUnlockedSteps) -> FString
	{
		if (VisibleUnlockedSteps <= 0)
		{
			return TEXT("SourceAssets/UI/PowerUp/Diplomas/Generated/dropout_cardboard_box_imagegen_20260426.png");
		}

		return FString::Printf(
			TEXT("SourceAssets/UI/PowerUp/Diplomas/Generated/%s_%s.png"),
			*GetDiplomaAssetSlug(StatType),
			*GetDiplomaRankAssetName(VisibleUnlockedSteps));
	};

	auto GetDiplomaRankTitle = [&](ET66HeroStatType StatType, int32 VisibleUnlockedSteps) -> FText
	{
		const FText StatLabel = GetStatLabel(StatType);
		switch (VisibleUnlockedSteps)
		{
			case 1:
				return FText::Format(NSLOCTEXT("T66.PowerUp", "BachelorInStat", "BACHELOR'S IN {0}"), MakeUpperText(StatLabel));
			case 2:
				return FText::Format(NSLOCTEXT("T66.PowerUp", "MasterInStat", "MASTER'S IN {0}"), MakeUpperText(StatLabel));
			case 3:
				return FText::Format(NSLOCTEXT("T66.PowerUp", "PHDInStat", "PHD IN {0}"), MakeUpperText(StatLabel));
			case 4:
				return FText::Format(NSLOCTEXT("T66.PowerUp", "MagistrateInStat", "MAGISTRATE IN {0}"), MakeUpperText(StatLabel));
			default:
				break;
		}

		const FText DropoutProgram = StatType == ET66HeroStatType::Damage
			? NSLOCTEXT("T66.PowerUp", "DiplomaDropoutProgramStrength", "STRENGTH")
			: MakeUpperText(StatLabel);
		return FText::Format(
			NSLOCTEXT("T66.PowerUp", "DiplomaRankDropoutUniversity", "{0} UNIVERSITY DROPOUT"),
			DropoutProgram);
	};

	auto MakePermanentStatPanel = [&](ET66HeroStatType StatType) -> TSharedRef<SWidget>
	{
		const int32 Cost = Buffs ? Buffs->GetCostForNextFillStepUnlock(StatType) : 0;
		const int32 UnlockedSteps = Buffs ? Buffs->GetUnlockedFillStepCount(StatType) : 0;
		const int32 VisibleUnlockedSteps = FMath::Clamp(UnlockedSteps, 0, ShopDiplomaUpgradeCount);
		const bool bDiplomaMaxed = VisibleUnlockedSteps >= ShopDiplomaUpgradeCount;
		const FText ButtonText = bDiplomaMaxed
			? NSLOCTEXT("T66.PowerUp", "Max", "MAX")
			: NSLOCTEXT("T66.PowerUp", "Graduate", "GRADUATE");
		const FText CostText = FText::AsNumber(Cost);
		const FSlateBrush* DiplomaBrush = ResolveShopGeneratedBrush(GetDiplomaImagePath(StatType, VisibleUnlockedSteps), FVector2D(244.f, 244.f));
		const FSlateBrush* CouponBrush = ResolveShopGeneratedBrush(TEXT("SourceAssets/UI/MasterLibrary/Slices/IconsGenerated/icon_07_coupon_ticket_imagegen_20260425_v2.png"), FVector2D(30.f, 24.f));
		const FText DiplomaTitle = GetDiplomaRankTitle(StatType, VisibleUnlockedSteps);
		const TSharedRef<SWidget> DiplomaImageWidget = DiplomaBrush
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(244.f)
				.HeightOverride(238.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SNew(SImage)
						.Image(DiplomaBrush)
					]
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(244.f)
				.HeightOverride(238.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.PowerUp", "MissingDiplomaArt", "DIPLOMA"))
					.Font(ShopBoldFont(20))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]);

		return MakeShopGeneratedPanel(
			MakeShopSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(DiplomaTitle)
				.Font(ShopBoldFont(28))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.WrapTextAt(250.f)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 12.f)
			[
				DiplomaImageWidget
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SBox)
				.MinDesiredHeight(20.f)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(GetPermanentEffectText(StatType))
				.Font(ShopRegularFont(16))
				.ColorAndOpacity(ShopPermanentCardAccent)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.WrapTextAt(250.f)
			]
			+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(0.f, 10.f, 0.f, 0.f)
			[
				MakeShopGeneratedButton(
					FT66ButtonParams(ButtonText, FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleUnlockClicked, StatType), ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(54.f)
					.SetColor(TAttribute<FSlateColor>::CreateLambda([bDiplomaMaxed, Balance, Cost]() -> FSlateColor
					{
						if (bDiplomaMaxed)
						{
							return FSlateColor(T66PowerUpButtonFill());
						}

						return FSlateColor(Balance >= Cost ? T66PowerUpButtonFill() : T66PowerUpButtonDisabledFill());
					}))
					.SetEnabled(TAttribute<bool>::CreateLambda([bDiplomaMaxed, Balance, Cost]() { return !bDiplomaMaxed && Balance >= Cost; }))
					.SetContent(
						SNew(SBox)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(ButtonText)
								.Font(ShopBoldFont(23))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(CostText)
								.Font(ShopBoldFont(21))
								.ColorAndOpacity(ShopPermanentCardAccent)
								.Visibility(bDiplomaMaxed ? EVisibility::Collapsed : EVisibility::Visible)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(30.f)
								.HeightOverride(24.f)
								.Visibility(bDiplomaMaxed || !CouponBrush ? EVisibility::Collapsed : EVisibility::Visible)
								[
									SNew(SImage)
									.Image(CouponBrush)
								]
							]
						])
					,
					ResolveShopCompactButtonStyle(),
					ShopBoldFont(23),
					FT66Style::Tokens::Text,
					FMargin(18.f, 10.f, 18.f, 8.f)
				)
			],
			FMargin(20.f, 18.f, 20.f, 18.f),
			FLinearColor::White,
			ShopPermanentCardFill);
	};

	auto MakeSingleUseSecondaryCard = [&](ET66SecondaryStatType StatType) -> TSharedRef<SWidget>
	{
		const int32 Cost = Buffs ? Buffs->GetSingleUseBuffCost() : UT66BuffSubsystem::SingleUseBuffCostCC;
		const FText CostText = FText::AsNumber(Cost);
		const TSharedPtr<FSlateBrush> IconBrush = GetShopSecondaryBuffBrush(OwnedBrushes, TexPool, this, StatType, FVector2D(124.f, 124.f));
		const FSlateBrush* CouponBrush = ResolveShopGeneratedBrush(TEXT("SourceAssets/UI/MasterLibrary/Slices/IconsGenerated/icon_07_coupon_ticket_imagegen_20260425_v2.png"), FVector2D(26.f, 21.f));
		const TSharedRef<SWidget> IconWidget = IconBrush.IsValid()
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(124.f)
				.HeightOverride(110.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SNew(SImage)
						.Image(IconBrush.Get())
					]
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(124.f)
				.HeightOverride(110.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.PowerUp", "MissingSecondaryArt", "ART"))
					.Font(ShopBoldFont(12))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]);

		return MakeShopGeneratedPanel(
			MakeShopSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(GetSingleUseDrugName(StatType))
				.Font(ShopBoldFont(17))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.WrapTextAt(210.f)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 8.f)
			[
				IconWidget
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(GetSingleUseEffectText(StatType))
				.Font(ShopRegularFont(14))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.WrapTextAt(210.f)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SSpacer)
			]
			+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom)
			[
				MakeShopGeneratedButton(
					FT66ButtonParams(NSLOCTEXT("T66.PowerUp", "BuySingleUse", "BUY"), FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandlePurchaseSingleUseClicked, StatType), ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(44.f)
					.SetColor(TAttribute<FSlateColor>::CreateLambda([Balance, Cost]() -> FSlateColor
					{
						return FSlateColor(Balance >= Cost ? T66PowerUpButtonFill() : T66PowerUpButtonDisabledFill());
					}))
					.SetEnabled(TAttribute<bool>::CreateLambda([Balance, Cost]() { return Balance >= Cost; }))
					.SetContent(
						SNew(SBox)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.PowerUp", "BuySingleUseText", "BUY"))
								.Font(ShopBoldFont(15))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(CostText)
								.Font(ShopBoldFont(15))
								.ColorAndOpacity(ShopPermanentCardAccent)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(7.f, 0.f, 0.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(26.f)
								.HeightOverride(21.f)
								.Visibility(CouponBrush ? EVisibility::Visible : EVisibility::Collapsed)
								[
									SNew(SImage)
									.Image(CouponBrush)
								]
							]
						])
					,
					ResolveShopCompactButtonStyle(),
					ShopBoldFont(15),
					FT66Style::Tokens::Text,
					FMargin(10.f, 6.f, 10.f, 5.f)
				)
			],
			FMargin(14.f, 12.f, 14.f, 14.f),
			FLinearColor::White,
			T66PowerUpPanelFill());
	};

	TSharedRef<SHorizontalBox> DiplomaColumns = SNew(SHorizontalBox);
	for (int32 StatIndex = 0; StatIndex < PermanentCardOrder.Num(); ++StatIndex)
	{
		DiplomaColumns->AddSlot()
			.AutoWidth()
			.Padding(StatIndex > 0 ? FMargin(ShopCardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
			[
				SNew(SBox)
				.WidthOverride(300.f)
				[
					MakePermanentStatPanel(PermanentCardOrder[StatIndex])
				]
			];
	}

	TSharedRef<SWidget> PermanentPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			MakeShopGeneratedPanel(
				MakeShopSettingsAssetPath(TEXT("settings_row_shell_full.png")),
				SNew(STextBlock)
				.Text(PermanentHintText)
				.Font(ShopRegularFont(13))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center),
				FMargin(18.f, 12.f),
				FLinearColor::White,
				T66PowerUpInsetFill())
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Horizontal)
			.ScrollBarVisibility(EVisibility::Visible)
			+ SScrollBox::Slot()
			[
				DiplomaColumns
			]
		];

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
			T66PowerUpInsetFill())
	];

	for (int32 RowIndex = 0; RowIndex < SingleUseRows.Num(); ++RowIndex)
	{
		const FSingleUseRowDef& RowDef = SingleUseRows[RowIndex];
		const int32 CardCount = RowDef.SecondaryStats.Num();
		TSharedRef<SHorizontalBox> CardsRow = SNew(SHorizontalBox);
		CardsRow->AddSlot()
			.FillWidth(1.f)
			.Padding(0.f, 0.f, ShopCardGap, 0.f)
			[
				MakeShopGeneratedPanel(
					MakeShopSettingsAssetPath(TEXT("settings_row_shell_full.png")),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(GetDrugRowTitle(RowDef.PrimaryStat))
						.Font(ShopBoldFont(28))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
						.WrapTextAt(320.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::Format(
							NSLOCTEXT("T66.PowerUp", "SingleUseRowCaption", "{0} drug cards"),
							FText::AsNumber(CardCount)))
						.Font(ShopRegularFont(12))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.Justification(ETextJustify::Center)
					],
					FMargin(16.f, 12.f),
					FLinearColor::White,
					T66PowerUpInsetFill())
			];
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
					CardsRow,
					FMargin(14.f, 12.f),
					FLinearColor::White,
					T66PowerUpPanelFill())
			];
	}

	TSharedRef<SWidget> SingleUsePage =
		SNew(SScrollBox)
		.ScrollBarVisibility(EVisibility::Visible)
		+ SScrollBox::Slot()
		[
			SingleUseRowsBox
		];

	const float TopInset = T66ScreenSlateHelpers::GetFrontendChromeTopInset(UIManager);
	const TSharedRef<SWidget> Root =
		SNew(SBox)
		.Padding(FMargin(14.f, TopInset, 14.f, 0.f))
		[
			MakeShopGeneratedPanel(
				MakeShopSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
		SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(T66ScreenSlateHelpers::GetFrontendChromeMetrics().HeaderPadding)
				[
					SNew(SOverlay)
					+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.PowerUp", "ScreenTitle", "POWER UP"))
						.Font(T66ScreenSlateHelpers::MakeFrontendChromeTitleFont())
						.ColorAndOpacity(FLinearColor(0.83f, 0.68f, 0.34f, 1.0f))
						.ShadowOffset(FVector2D(0.f, 2.f))
						.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.75f))
					]
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
						[
							MakeShopGeneratedButton(
								FT66ButtonParams(PermanentTabText, FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleShowPermanentClicked), ET66ButtonType::Primary)
								.SetMinWidth(T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabMinWidth)
								.SetHeight(T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabHeight)
								.SetColor(!bShowingSingleUse ? T66PowerUpButtonFill() : T66PowerUpNeutralButtonFill()),
								ResolveShopToggleButtonStyle(!bShowingSingleUse),
								T66ScreenSlateHelpers::MakeFrontendChromeTabFont(),
								!bShowingSingleUse ? T66PowerUpTabActiveText() : T66PowerUpTabInactiveText(),
								T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabPadding)
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							MakeShopGeneratedButton(
								FT66ButtonParams(SingleUseTabText, FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleShowSingleUseClicked), ET66ButtonType::Neutral)
								.SetMinWidth(T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabMinWidth)
								.SetHeight(T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabHeight)
								.SetColor(bShowingSingleUse ? T66PowerUpButtonFill() : T66PowerUpNeutralButtonFill()),
								ResolveShopToggleButtonStyle(bShowingSingleUse),
								T66ScreenSlateHelpers::MakeFrontendChromeTabFont(),
								bShowingSingleUse ? T66PowerUpTabActiveText() : T66PowerUpTabInactiveText(),
								T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabPadding)
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
		T66PowerUpShellFill())
		];

	if (const FSlateBrush* SceneBackgroundBrush = ResolveShopGeneratedBrush(TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/main_menu_scene_plate_imagegen_20260425_v1.png")))
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(SceneBackgroundBrush)
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.48f))
			]
			+ SOverlay::Slot()
			[
				Root
			];
	}

	return Root;
}
