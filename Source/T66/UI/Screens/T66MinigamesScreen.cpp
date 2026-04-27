// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MinigamesScreen.h"

#include "Core/T66LocalizationSubsystem.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "UObject/StrongObjectPtr.h"
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

	bool T66IsPausedGameplayWidget(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		return PC && PC->IsPaused();
	}

	FLinearColor T66FrontendShellFill()
	{
		return FLinearColor(0.004f, 0.005f, 0.010f, 0.985f);
	}

	FLinearColor T66MinigamesInsetFill()
	{
		return FLinearColor(0.024f, 0.025f, 0.030f, 1.0f);
	}

	FString MakeSettingsAssetPath(const TCHAR* FileName)
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

	FMargin GetMinigamesGeneratedBrushMargin(const FString& SourceRelativePath)
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

	bool IsZeroMinigamesMargin(const FMargin& Margin)
	{
		return FMath::IsNearlyZero(Margin.Left)
			&& FMath::IsNearlyZero(Margin.Top)
			&& FMath::IsNearlyZero(Margin.Right)
			&& FMath::IsNearlyZero(Margin.Bottom);
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

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				TextureFilter::TF_Trilinear,
				true,
				TEXT("MinigamesGeneratedUI"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Trilinear,
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
			return nullptr;
		}

		const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f
			? ImageSize
			: FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()));

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		const FMargin BrushMargin = GetMinigamesGeneratedBrushMargin(SourceRelativePath);
		Brush->DrawAs = IsZeroMinigamesMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolvedSize;
		Brush->Margin = BrushMargin;
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
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_normal.png")),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_hover.png")),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_pressed.png")),
			MakeSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
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
		return FT66Style::MakeBareButton(
			FT66BareButtonParams(
				Params.OnClicked,
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center))
			.SetButtonStyle(ButtonStyle ? ButtonStyle : &FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.SetPadding(ContentPadding)
			.SetEnabled(Params.IsEnabled)
			.SetMinWidth(Params.MinWidth)
			.SetHeight(Params.Height)
			.SetVisibility(Params.Visibility));
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
	const FText ComingSoonTitle = NSLOCTEXT("T66.MiniGames", "SliceComingSoonTitle", "COMING SOON");
	const FText ComingSoonBody = NSLOCTEXT("T66.MiniGames", "SliceComingSoonBody", "Reserved slot for future mini-game releases.");
	const FText ComingSoonTag = NSLOCTEXT("T66.MiniGames", "SliceComingSoonTag", "IN DEVELOPMENT");
	const float TopInset = T66ScreenSlateHelpers::GetFrontendChromeTopInset(UIManager);

	const TAttribute<FMargin> SafeContentInsets = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafeFrameInsets();
	});

	const auto MakeSlicePanel = [&](const FText& Title, const FText& Body, const FText& Tag, const FLinearColor& Accent, const bool bClickable, FOnClicked ClickDelegate = FOnClicked()) -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> SliceContent =
			SNew(SBox)
			.HeightOverride(98.f)
			[
				MakeMinigamesGeneratedPanel(
					MakeSettingsAssetPath(TEXT("settings_row_shell_split.png")),
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(Title)
							.Font(FT66Style::Tokens::FontBold(24))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 4.f, 40.f, 0.f)
						[
							SNew(STextBlock)
							.Text(Body)
							.Font(FT66Style::Tokens::FontRegular(13))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.AutoWrapText(true)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(280.f)
						.HeightOverride(40.f)
						[
							MakeMinigamesGeneratedPanel(
								MakeSettingsAssetPath(bClickable ? TEXT("settings_toggle_on_normal.png") : TEXT("settings_toggle_inactive_normal.png")),
								SNew(STextBlock)
								.Text(Tag)
								.Font(FT66Style::Tokens::FontBold(bClickable ? 22 : 13))
								.ColorAndOpacity(bClickable ? FLinearColor(0.99f, 0.93f, 0.74f, 1.f) : FT66Style::Tokens::TextMuted)
								.Justification(ETextJustify::Center),
								FMargin(18.f, 8.f),
								FLinearColor::White,
								bClickable ? Accent : FLinearColor(0.18f, 0.20f, 0.24f, 1.f))
						]
					],
					FMargin(24.f, 10.f),
					bClickable ? FLinearColor::White : FLinearColor(0.72f, 0.76f, 0.82f, 1.f),
					FLinearColor(0.018f, 0.020f, 0.026f, 1.0f))
			];

		if (!bClickable)
		{
			return SliceContent;
		}

		return FT66Style::MakeBareButton(
			FT66BareButtonParams(ClickDelegate, SliceContent)
			.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder")));
	};

	const TSharedRef<SWidget> Root = SNew(SBox)
		.Padding(FMargin(14.f, TopInset, 14.f, 0.f))
		[
			MakeMinigamesGeneratedPanel(
				MakeSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					.Padding(SafeContentInsets)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.f, 0.f, 0.f, 8.f)
						[
							SNew(STextBlock)
							.Text(MinigamesText)
							.Font(T66ScreenSlateHelpers::MakeFrontendChromeTitleFont())
							.ColorAndOpacity(FLinearColor(0.83f, 0.68f, 0.34f, 1.0f))
							.Justification(ETextJustify::Center)
							.ShadowOffset(FVector2D(0.f, 2.f))
							.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.75f))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 14.f)
						[
							MakeMinigamesGeneratedPanel(
								MakeSettingsAssetPath(TEXT("settings_row_shell_split.png")),
								SNew(STextBlock)
								.Text(MinigamesDescriptionText)
								.Font(FT66Style::Tokens::FontRegular(17))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true),
								FMargin(18.f, 7.f),
								FLinearColor::White,
								T66MinigamesInsetFill())
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.Padding(0.f, 0.f, 0.f, 16.f)
						[
							SNew(SScrollBox)
							.ScrollBarVisibility(EVisibility::Visible)
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 10.f)
							[
								MakeSlicePanel(ActiveSliceTitle, ActiveSliceBody, ActiveSliceTag, FT66Style::Success(), true, FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenMiniChadpocalypseClicked))
							]
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 10.f)
							[
								MakeSlicePanel(TDTitle, TDBody, TDTag, FLinearColor(0.88f, 0.34f, 0.22f, 1.0f), true, FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenChadpocalypseTDClicked))
							]
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 10.f)
							[
								MakeSlicePanel(ComingSoonTitle, ComingSoonBody, ComingSoonTag, FT66Style::Accent2(), false)
							]
							+ SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 0.f)
							[
								MakeSlicePanel(ComingSoonTitle, ComingSoonBody, ComingSoonTag, FT66Style::Accent2(), false)
							]
						]
					]
				],
				FMargin(18.f),
				FLinearColor::White,
				T66FrontendShellFill())
		];

	if (const FSlateBrush* SceneBackgroundBrush = ResolveMinigamesGeneratedBrush(TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/main_menu_scene_plate_imagegen_20260425_v1.png")))
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
	if (T66IsPausedGameplayWidget(this) && UIManager)
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

void UT66MinigamesScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}
