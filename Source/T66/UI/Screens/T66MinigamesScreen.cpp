// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MinigamesScreen.h"

#include "Core/T66LocalizationSubsystem.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	TMap<FString, TStrongObjectPtr<UTexture2D>> GMinigamesGeneratedTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GMinigamesGeneratedBrushCache;
	TMap<FString, TSharedPtr<FButtonStyle>> GMinigamesGeneratedButtonStyleCache;

	bool T66IsMinigamesPausedGameplayWidget(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		return PC && PC->IsPaused();
	}

	FLinearColor T66MinigamesInsetFill()
	{
		return FLinearColor(0.046f, 0.018f, 0.020f, 0.98f);
	}

	FLinearColor T66MinigamesRowFill()
	{
		return FLinearColor(0.018f, 0.020f, 0.030f, 0.98f);
	}

	FLinearColor T66MinigamesAccentRed()
	{
		return FLinearColor(0.92f, 0.05f, 0.12f, 1.0f);
	}

	FLinearColor T66MinigamesAccentCrimson()
	{
		return FLinearColor(0.92f, 0.05f, 0.12f, 1.0f);
	}

	FLinearColor T66MinigamesBrightText()
	{
		return FLinearColor(1.0f, 0.88f, 0.84f, 1.0f);
	}

	FString MakeMinigamesUltrakillElementPath(const TCHAR* FileName)
	{
		return FString(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements")) / FString(FileName ? FileName : TEXT(""));
	}

	FString MakeMinigamesUltrakillSquareElementPath(const TCHAR* FileName)
	{
		return FString(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant")) / FString(FileName ? FileName : TEXT(""));
	}

	FString MakeMinigamesSettingsAssetPath(const TCHAR* FileName)
	{
		const FString Name(FileName);
		const auto BasicButtonPath = [](const TCHAR* State) -> FString
		{
			const FString StateName(State ? State : TEXT("normal"));
			if (StateName.Equals(TEXT("disabled"), ESearchCase::IgnoreCase))
			{
				return MakeMinigamesUltrakillSquareElementPath(TEXT("cta_new_game_button_disabled_red_square_variant.png"));
			}
			const FString CTAState = StateName.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
				? FString(TEXT("normal"))
				: StateName.ToLower();
			return MakeMinigamesUltrakillSquareElementPath(*FString::Printf(TEXT("cta_new_game_button_%s_red_square_variant.png"), *CTAState));
		};
		const auto SelectButtonPath = [](const TCHAR* State) -> FString
		{
			const FString StateName(State ? State : TEXT("normal"));
			const FString PillState = StateName.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
				? FString(TEXT("normal"))
				: StateName.ToLower();
			return MakeMinigamesUltrakillSquareElementPath(*FString::Printf(TEXT("cta_new_game_button_%s_red_square_variant.png"), *PillState));
		};

		if (Name.StartsWith(TEXT("settings_toggle_on_")))
		{
			return SelectButtonPath(TEXT("selected"));
		}
		if (Name.StartsWith(TEXT("settings_compact_neutral_")) || Name.StartsWith(TEXT("settings_toggle_off_")))
		{
			if (Name.Contains(TEXT("_hover"))) return SelectButtonPath(TEXT("hover"));
			if (Name.Contains(TEXT("_pressed"))) return SelectButtonPath(TEXT("pressed"));
			return SelectButtonPath(TEXT("normal"));
		}
		if (Name.StartsWith(TEXT("settings_toggle_inactive_")))
		{
			return BasicButtonPath(TEXT("disabled"));
		}
		if (Name == TEXT("settings_content_shell_frame.png"))
		{
			return MakeMinigamesUltrakillSquareElementPath(TEXT("main_panel_normal_square_variant.png"));
		}
		if (Name == TEXT("settings_row_shell_full.png") || Name == TEXT("settings_row_shell_split.png"))
		{
			return MakeMinigamesUltrakillSquareElementPath(TEXT("player_row_panel_normal_square_variant.png"));
		}
		if (Name == TEXT("settings_dropdown_field.png"))
		{
			return MakeMinigamesUltrakillSquareElementPath(TEXT("dropdown_field_normal_square_variant.png"));
		}

		return FString(TEXT("SourceAssets/UI/Reference/Shared")) / Name;
	}

	FMargin GetMinigamesGeneratedBrushMargin(const FString& SourceRelativePath)
	{
		if (SourceRelativePath.Contains(TEXT("inner_panel_normal.png")))
		{
			return FMargin(0.067f, 0.043f, 0.067f, 0.043f);
		}
		if (SourceRelativePath.Contains(TEXT("main_panel_normal.png")) || SourceRelativePath.Contains(TEXT("main_panel_normal_square_variant.png")))
		{
			return FMargin(0.070f, 0.120f, 0.070f, 0.120f);
		}
		if (SourceRelativePath.Contains(TEXT("player_row_panel_")))
		{
			return FMargin(0.070f, 0.280f, 0.070f, 0.280f);
		}
		if (SourceRelativePath.Contains(TEXT("dropdown_field_normal.png")) || SourceRelativePath.Contains(TEXT("dropdown_field_normal_square_variant.png")))
		{
			return FMargin(0.06f, 0.34f, 0.06f, 0.34f);
		}
		if (T66ScreenSlateHelpers::IsReferenceChromePillButtonAssetPath(SourceRelativePath))
		{
			return FMargin(0.093f, 0.213f, 0.093f, 0.213f);
		}
		if (T66ScreenSlateHelpers::IsReferenceChromeCTAButtonAssetPath(SourceRelativePath))
		{
			return FMargin(0.083f, 0.231f, 0.083f, 0.231f);
		}
		return FMargin(0.f);
	}

	bool IsZeroMinigamesMargin(const FMargin& Margin)
	{
		return FMath::IsNearlyZero(Margin.Left)
			&& FMath::IsNearlyZero(Margin.Top)
			&& FMath::IsNearlyZero(Margin.Right)
			&& FMath::IsNearlyZero(Margin.Bottom);
	}

	bool IsMinigamesSlicedButtonPath(const FString& SourceRelativePath)
	{
		return T66ScreenSlateHelpers::IsReferenceChromeButtonAssetPath(SourceRelativePath);
	}

	UTexture2D* LoadMinigamesGeneratedTexture(const FString& SourceRelativePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GMinigamesGeneratedTextureCache.Find(SourceRelativePath))
		{
			return CachedTexture->Get();
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			const TextureFilter Filter = IsMinigamesSlicedButtonPath(SourceRelativePath)
				? TextureFilter::TF_Nearest
				: TextureFilter::TF_Trilinear;

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				Filter,
				true,
				TEXT("MinigamesGeneratedUI"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					Filter,
					TEXT("MinigamesGeneratedUI"));
			}

			if (Texture)
			{
				GMinigamesGeneratedTextureCache.Add(SourceRelativePath, TStrongObjectPtr<UTexture2D>(Texture));
				return Texture;
			}
		}

		return nullptr;
	}

	const FSlateBrush* ResolveMinigamesGeneratedBrush(const FString& SourceRelativePath, const FVector2D& ImageSize = FVector2D::ZeroVector)
	{
		const FString BrushKey = FString::Printf(TEXT("%s::%.0fx%.0f"), *SourceRelativePath, ImageSize.X, ImageSize.Y);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GMinigamesGeneratedBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = LoadMinigamesGeneratedTexture(SourceRelativePath);
		if (!Texture)
		{
			if (!T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(SourceRelativePath))
			{
				return nullptr;
			}

			TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
			const FMargin BrushMargin = GetMinigamesGeneratedBrushMargin(SourceRelativePath);
			const bool bSlicedButton = IsMinigamesSlicedButtonPath(SourceRelativePath);
			const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f ? ImageSize : FVector2D(1.f, 1.f);
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Brush,
				SourceRelativePath,
				ResolvedSize,
				bSlicedButton ? FMargin(0.f) : BrushMargin,
				bSlicedButton || IsZeroMinigamesMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box);
			GMinigamesGeneratedBrushCache.Add(BrushKey, Brush);
			return Brush.Get();
		}

		const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f
			? ImageSize
			: FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()));

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		const FMargin BrushMargin = GetMinigamesGeneratedBrushMargin(SourceRelativePath);
		const bool bSlicedButton = IsMinigamesSlicedButtonPath(SourceRelativePath);
		Brush->DrawAs = bSlicedButton || IsZeroMinigamesMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolvedSize;
		Brush->Margin = bSlicedButton ? FMargin(0.f) : BrushMargin;
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GMinigamesGeneratedBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}

	const FButtonStyle* ResolveMinigamesGeneratedButtonStyle(
		const FString& Key,
		const FString& NormalPath,
		const FString& HoverPath,
		const FString& PressedPath,
		const FString& DisabledPath)
	{
		if (const TSharedPtr<FButtonStyle>* CachedStyle = GMinigamesGeneratedButtonStyleCache.Find(Key))
		{
			return CachedStyle->Get();
		}

		const FButtonStyle& NoBorderStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		TSharedPtr<FButtonStyle> Style = MakeShared<FButtonStyle>(NoBorderStyle);
		if (const FSlateBrush* NormalBrush = ResolveMinigamesGeneratedBrush(NormalPath))
		{
			Style->SetNormal(*NormalBrush);
		}
		if (const FSlateBrush* HoverBrush = ResolveMinigamesGeneratedBrush(HoverPath))
		{
			Style->SetHovered(*HoverBrush);
		}
		if (const FSlateBrush* PressedBrush = ResolveMinigamesGeneratedBrush(PressedPath))
		{
			Style->SetPressed(*PressedBrush);
		}
		if (const FSlateBrush* DisabledBrush = ResolveMinigamesGeneratedBrush(DisabledPath))
		{
			Style->SetDisabled(*DisabledBrush);
		}
		Style->SetNormalPadding(FMargin(0.f));
		Style->SetPressedPadding(FMargin(0.f));

		GMinigamesGeneratedButtonStyleCache.Add(Key, Style);
		return Style.Get();
	}

	const FButtonStyle* ResolveMinigamesCompactButtonStyle()
	{
		return ResolveMinigamesGeneratedButtonStyle(
			TEXT("Minigames.CompactButton"),
			MakeMinigamesSettingsAssetPath(TEXT("settings_compact_neutral_normal.png")),
			MakeMinigamesSettingsAssetPath(TEXT("settings_compact_neutral_hover.png")),
			MakeMinigamesSettingsAssetPath(TEXT("settings_compact_neutral_pressed.png")),
			MakeMinigamesSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	TSharedRef<SWidget> MakeMinigamesGeneratedPanel(
		const FString& SourceRelativePath,
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FLinearColor& Tint = FLinearColor::White,
		const FLinearColor& FallbackFill = T66MinigamesInsetFill())
	{
		if (const FSlateBrush* Brush = ResolveMinigamesGeneratedBrush(SourceRelativePath))
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(Tint)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeMinigamesGeneratedButton(
		const FT66ButtonParams& Params,
		const FButtonStyle* ButtonStyle,
		const FSlateFontInfo& Font,
		const FLinearColor& TextColor,
		const FMargin& ContentPadding)
	{
		if (!ButtonStyle)
		{
			return FT66Style::MakeBareButton(
				FT66BareButtonParams(
					Params.OnClicked,
					SNew(STextBlock)
					.Text(Params.Label)
					.Font(Font)
					.ColorAndOpacity(TextColor)
					.Justification(ETextJustify::Center))
				.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.SetPadding(ContentPadding)
				.SetEnabled(Params.IsEnabled)
				.SetMinWidth(Params.MinWidth)
				.SetHeight(Params.Height)
				.SetVisibility(Params.Visibility));
		}

		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			Params.OnClicked,
			SNew(STextBlock)
			.Text(Params.Label)
			.Font(Font)
			.ColorAndOpacity(TextColor)
			.Justification(ETextJustify::Center),
			&ButtonStyle->Normal,
			&ButtonStyle->Hovered,
			&ButtonStyle->Pressed,
			&ButtonStyle->Disabled,
			Params.MinWidth,
			Params.Height,
			ContentPadding,
			Params.IsEnabled,
			Params.Visibility);
	}
}

UT66MinigamesScreen::UT66MinigamesScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Minigames;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66MinigamesScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66MinigamesScreen::BuildSlateUI()
{
	const FText MinigamesText = NSLOCTEXT("T66.MiniGames", "Title", "MINIGAMES");
	const FText MinigamesDescriptionText = NSLOCTEXT("T66.MiniGames", "Description", "Relax or play minigames with friends to earn ChadCoupons you can redeem in the main game.");
	const FText ActiveSliceTitle = NSLOCTEXT("T66.MiniGames", "SliceActiveTitle", "CHADPOCALYPSE MINI");
	const FText ActiveSliceBody = NSLOCTEXT("T66.MiniGames", "SliceActiveBody", "A 2D survivor minigame with its own saves, heroes, idols, enemies, and progression.");
	const FText ActiveSliceTag = NSLOCTEXT("T66.MiniGames", "SliceActiveTag", "PLAY");
	const FText TDTitle = NSLOCTEXT("T66.MiniGames", "SliceTDTitle", "CHADPOCALYPSE TD");
	const FText TDBody = NSLOCTEXT("T66.MiniGames", "SliceTDBody", "A tower defense minigame with hero placement, enemy waves, upgrades, and rotating maps.");
	const FText TDTag = NSLOCTEXT("T66.MiniGames", "SliceTDTag", "PLAY");
	const FText IdleTitle = NSLOCTEXT("T66.MiniGames", "SliceIdleTitle", "IDLE CHADPOCALYPSE");
	const FText IdleBody = NSLOCTEXT("T66.MiniGames", "SliceIdleBody", "An offline-progress idle minigame with heroes, upgrades, stage pushing, and comeback rewards.");
	const FText IdleTag = NSLOCTEXT("T66.MiniGames", "SliceIdleTag", "PROTOTYPE");
	const FText DeckTitle = NSLOCTEXT("T66.MiniGames", "SliceDeckTitle", "CHADPOCALYPSE DECKBUILDER");
	const FText DeckBody = NSLOCTEXT("T66.MiniGames", "SliceDeckBody", "A dungeon-descent deckbuilder with card combat, route choices, relics, and reward drafts.");
	const FText DeckTag = NSLOCTEXT("T66.MiniGames", "SliceDeckTag", "PROTOTYPE");
	const FText VersusTitle = NSLOCTEXT("T66.MiniGames", "SliceVersusTitle", "VERSUS");
	const FText VersusBody = NSLOCTEXT("T66.MiniGames", "SliceVersusBody", "A 1v1 arcade gauntlet where friends compete across cabinet games like Whack-a-Mole.");
	const FText VersusTag = NSLOCTEXT("T66.MiniGames", "SliceVersusTag", "SETUP");

	const auto MakeSlicePanel = [&](const FText& Title, const FText& Body, const FText& Tag, const FLinearColor& Accent, const bool bClickable, FOnClicked ClickDelegate = FOnClicked()) -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> SliceContent =
			SNew(SBox)
			.HeightOverride(96.f)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				MakeMinigamesGeneratedPanel(
					MakeMinigamesSettingsAssetPath(TEXT("settings_row_shell_split.png")),
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 14.f, 0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(Title)
							.Font(FT66Style::Tokens::FontBold(22))
							.ColorAndOpacity(T66MinigamesBrightText())
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(Body)
							.Font(FT66Style::Tokens::FontRegular(12))
							.ColorAndOpacity(FLinearColor(0.84f, 0.62f, 0.58f, 1.0f))
							.AutoWrapText(true)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(238.f)
						.HeightOverride(36.f)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							MakeMinigamesGeneratedPanel(
								MakeMinigamesSettingsAssetPath(bClickable ? TEXT("settings_toggle_on_normal.png") : TEXT("settings_toggle_inactive_normal.png")),
								SNew(STextBlock)
								.Text(Tag)
								.Font(FT66Style::Tokens::FontBold(bClickable ? 18 : 12))
								.ColorAndOpacity(bClickable ? FLinearColor(1.0f, 0.96f, 0.88f, 1.f) : FLinearColor(0.84f, 0.62f, 0.58f, 1.0f))
								.Justification(ETextJustify::Center)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								.Clipping(EWidgetClipping::ClipToBounds),
								FMargin(14.f, 6.f),
								FLinearColor::White,
								bClickable ? Accent : FLinearColor(0.18f, 0.20f, 0.24f, 1.f))
						]
					],
					FMargin(24.f, 7.f),
					bClickable ? FLinearColor::White : FLinearColor(0.72f, 0.76f, 0.82f, 1.f),
					T66MinigamesRowFill())
			];

		if (!bClickable)
		{
			return SliceContent;
		}

		return FT66Style::MakeBareButton(
			FT66BareButtonParams(ClickDelegate, SliceContent)
			.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder")));
	};

	const TSharedRef<SWidget> Content = SNew(SBox)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.BorderBackgroundColor(FLinearColor::Transparent)
			.Padding(FMargin(-14.f, 0.f, -14.f, 4.f))
			[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.f, 0.f, 0.f, 4.f)
						[
							SNew(STextBlock)
							.Text(MinigamesText)
							.Font(T66ScreenSlateHelpers::MakeFrontendChromeTitleFont())
							.ColorAndOpacity(T66MinigamesBrightText())
							.Justification(ETextJustify::Center)
							.ShadowOffset(FVector2D(0.f, 3.f))
							.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 8.f)
						[
							MakeMinigamesGeneratedPanel(
								T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(TEXT("normal")),
								SNew(STextBlock)
								.Text(MinigamesDescriptionText)
								.Font(FT66Style::Tokens::FontRegular(17))
								.ColorAndOpacity(T66MinigamesBrightText())
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
								.Clipping(EWidgetClipping::ClipToBounds),
								FMargin(18.f, 7.f),
								FLinearColor::White,
								T66MinigamesInsetFill())
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.Padding(0.f)
						[
							SNew(SScrollBox)
							.ScrollBarVisibility(EVisibility::Visible)
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 8.f)
							[
								MakeSlicePanel(ActiveSliceTitle, ActiveSliceBody, ActiveSliceTag, T66MinigamesAccentRed(), true, FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenMiniChadpocalypseClicked))
							]
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 8.f)
							[
								MakeSlicePanel(TDTitle, TDBody, TDTag, FLinearColor(0.88f, 0.34f, 0.22f, 1.0f), true, FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenChadpocalypseTDClicked))
							]
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 8.f)
							[
								MakeSlicePanel(IdleTitle, IdleBody, IdleTag, T66MinigamesAccentCrimson(), true, FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenIdleChadpocalypseClicked))
							]
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 8.f)
							[
								MakeSlicePanel(DeckTitle, DeckBody, DeckTag, FLinearColor(0.26f, 0.64f, 0.78f, 1.0f), true, FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenChadpocalypseDeckbuilderClicked))
							]
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 0.f)
							[
								MakeSlicePanel(VersusTitle, VersusBody, VersusTag, FLinearColor(0.30f, 0.76f, 0.94f, 1.0f), true, FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenVersusClicked))
							]
						]
					]
				]
		];

	return T66ScreenSlateHelpers::MakeTopBarScreenRoot(
		UIManager,
		Content,
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black),
		FLinearColor::Transparent);
}

void UT66MinigamesScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66MinigamesScreen::HandleLanguageChanged);
	}
}

void UT66MinigamesScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66MinigamesScreen::HandleLanguageChanged);
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66MinigamesScreen::OnBackClicked()
{
	if (T66IsMinigamesPausedGameplayWidget(this) && UIManager)
	{
		ShowModal(ET66ScreenType::PauseMenu);
		return;
	}

	NavigateBack();
}

FReply UT66MinigamesScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenMiniChadpocalypseClicked()
{
	NavigateTo(ET66ScreenType::MiniMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenChadpocalypseTDClicked()
{
	NavigateTo(ET66ScreenType::TDMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenIdleChadpocalypseClicked()
{
	NavigateTo(ET66ScreenType::IdleMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenChadpocalypseDeckbuilderClicked()
{
	NavigateTo(ET66ScreenType::DeckMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenVersusClicked()
{
	NavigateTo(ET66ScreenType::VersusMainMenu);
	return FReply::Handled();
}

void UT66MinigamesScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}
