// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MainMenuScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateBrush.h"
#include "TimerManager.h"

UT66MainMenuScreen::UT66MainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MainMenu;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66MainMenuScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66MainMenuScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;

	// Get localized text
	FText NewGameText = Loc ? Loc->GetText_NewGame() : NSLOCTEXT("T66.MainMenu", "NewGame", "NEW GAME");
	FText LoadGameText = Loc ? Loc->GetText_LoadGame() : NSLOCTEXT("T66.MainMenu", "LoadGame", "LOAD GAME");
	FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.MainMenu", "Settings", "SETTINGS");
	FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.MainMenu", "Achievements", "ACHIEVEMENTS");
	FText QuitText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");

	// Button: 30% smaller (63 -> 44), equal padding on all 4 sides so text isn't glued to edges
	const float ButtonPadding = 14.0f; // same on left, top, right, bottom
	const float ButtonSpacing = 14.0f;
	const int32 ButtonFontSize = 44;   // 63 * 0.7

	auto MakeMenuButton = [this, ButtonPadding, ButtonFontSize](const FText& Text, FReply (UT66MainMenuScreen::*ClickFunc)(), const FLinearColor& BgColor = FT66Style::Tokens::Panel2) -> TSharedRef<SWidget>
	{
		const ISlateStyle& Style = FT66Style::Get();
		const FButtonStyle& Btn = Style.GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");

		return SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked(FT66Style::DebounceClick(FOnClicked::CreateUObject(this, ClickFunc)))
			.ButtonStyle(&Btn)
			.ContentPadding(FMargin(ButtonPadding, ButtonPadding, ButtonPadding, ButtonPadding))
			[
				SNew(STextBlock)
				.Text(Text)
				.Font(FT66Style::Tokens::FontBold(ButtonFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			];
	};

	// Background image brush (filled by texture pool when MainMenuBackground texture loads)
	MainMenuBackgroundBrush = MakeShared<FSlateBrush>();
	MainMenuBackgroundBrush->DrawAs = ESlateBrushDrawType::Box;
	MainMenuBackgroundBrush->Tiling = ESlateBrushTileType::NoTile;
	MainMenuBackgroundBrush->SetResourceObject(nullptr);

	// Filter icons are now loaded and displayed inside the leaderboard panel (pure Slate).

	return SNew(SBorder)
		.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
		[
			SNew(SOverlay)
			// Full-screen background image (Content/UI/MainMenu/MainMenuBackgroundV3)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(MainMenuBackgroundImage, SImage)
				.Image(TAttribute<const FSlateBrush*>::Create([this]() -> const FSlateBrush* { return MainMenuBackgroundBrush.IsValid() ? MainMenuBackgroundBrush.Get() : nullptr; }))
			]
			// Menu buttons: left side, top aligned with leaderboard panel (moved up to match panel start)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(40.0f, 380.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, ButtonSpacing)
					[ MakeMenuButton(NewGameText, &UT66MainMenuScreen::HandleNewGameClicked, FT66Style::Tokens::Accent2) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, ButtonSpacing)
					[ MakeMenuButton(LoadGameText, &UT66MainMenuScreen::HandleLoadGameClicked, FT66Style::Tokens::Accent2) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, ButtonSpacing)
					[ MakeMenuButton(SettingsText, &UT66MainMenuScreen::HandleSettingsClicked) ]
					+ SVerticalBox::Slot().AutoHeight()
					[ MakeMenuButton(AchievementsText, &UT66MainMenuScreen::HandleAchievementsClicked) ]
				]
			]
			// Right: Leaderboard panel (top aligned with NEW GAME button)
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.0f, 380.0f, 40.0f, 40.0f)
			[
				SNew(SBox)
				.WidthOverride(720.0f)
				[
					SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
					.LocalizationSubsystem(Loc)
					.LeaderboardSubsystem(LB)
					.UIManager(UIManager)
				]
			]
			// Quit button (top-right)
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.0f, 15.0f, 15.0f, 0.0f)
			[
				FT66Style::MakeButton(QuitText, FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleQuitClicked), ET66ButtonType::Danger, 100.f, 40.f)
			]
			// Bottom-left row: Language button + Dark/Light theme toggle
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(15.0f, 0.0f, 0.0f, 15.0f)
			[
				SNew(SHorizontalBox)
				// Language globe button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(50.0f)
					.HeightOverride(50.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.OnClicked(FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleLanguageClicked)))
						.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
						.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Common", "GlobeIcon", "\xF0\x9F\x8C\x90"))
							.Font(FT66Style::Tokens::FontBold(20))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
				]
				// Dark theme button (active = inverted look when Dark is selected)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SBox)
					.HeightOverride(50.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.OnClicked(FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleDarkThemeClicked)))
						.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>(
							FT66Style::GetTheme() == ET66UITheme::Dark ? "T66.Button.ToggleActive" : "T66.Button.Neutral"))
						.ContentPadding(FMargin(12.f, 8.f))
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_ThemeDark() : NSLOCTEXT("T66.Theme", "Dark", "DARK"))
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::GetTheme() == ET66UITheme::Dark ? FT66Style::Tokens::Panel : FT66Style::Tokens::Text)
						]
					]
				]
				// Light theme button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.HeightOverride(50.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.OnClicked(FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleLightThemeClicked)))
						.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>(
							FT66Style::GetTheme() == ET66UITheme::Light ? "T66.Button.ToggleActive" : "T66.Button.Neutral"))
						.ContentPadding(FMargin(12.f, 8.f))
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_ThemeLight() : NSLOCTEXT("T66.Theme", "Light", "LIGHT"))
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::GetTheme() == ET66UITheme::Light ? FT66Style::Tokens::Panel : FT66Style::Tokens::Text)
						]
					]
				]
			]
		];
}

void UT66MainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	UE_LOG(LogTemp, Log, TEXT("MainMenuScreen activated!"));

	// Request main menu background texture
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>())
		{
			const TSoftObjectPtr<UTexture2D> MainMenuBgSoft(FSoftObjectPath(TEXT("/Game/UI/MainMenu/MainMenuBackgroundV3.MainMenuBackgroundV3")));
			TSharedPtr<FSlateBrush> Brush = MainMenuBackgroundBrush;
			TSharedPtr<SImage> Image = MainMenuBackgroundImage;
			TexPool->RequestTexture(MainMenuBgSoft, this, TEXT("MainMenuBackground"), [Brush, Image](UTexture2D* Loaded)
			{
				if (Brush.IsValid())
				{
					Brush->SetResourceObject(Loaded);
					if (Image.IsValid() && Brush.IsValid())
					{
						Image->SetImage(Brush.Get());
					}
				}
			});
		}
	}

	// Important: Screen UI can be built before UIManager is assigned by UT66UIManager.
	// Inject it here so the leaderboard panel can open modals on row click.
	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}

	// Subscribe to language changes
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66MainMenuScreen::OnLanguageChanged);
	}
}

void UT66MainMenuScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	// Force rebuild UI with current language
	ForceRebuildSlate();
}

void UT66MainMenuScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	// Rebuild UI when language changes
	ForceRebuildSlate();
}

// Slate click handlers (return FReply)
FReply UT66MainMenuScreen::HandleNewGameClicked()
{
	OnNewGameClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleLoadGameClicked()
{
	OnLoadGameClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleSettingsClicked()
{
	OnSettingsClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleAchievementsClicked()
{
	OnAchievementsClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleLanguageClicked()
{
	OnLanguageClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleQuitClicked()
{
	OnQuitClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleDarkThemeClicked()
{
	// Ignore if already Dark or if a theme change is in flight
	if (bThemeChangeInProgress || FT66Style::GetTheme() == ET66UITheme::Dark)
	{
		return FReply::Handled();
	}
	bThemeChangeInProgress = true;
	PendingTheme = ET66UITheme::Dark;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UT66MainMenuScreen::ApplyPendingTheme));
	}
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleLightThemeClicked()
{
	// Ignore if already Light or if a theme change is in flight
	if (bThemeChangeInProgress || FT66Style::GetTheme() == ET66UITheme::Light)
	{
		return FReply::Handled();
	}
	bThemeChangeInProgress = true;
	PendingTheme = ET66UITheme::Light;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UT66MainMenuScreen::ApplyPendingTheme));
	}
	return FReply::Handled();
}

void UT66MainMenuScreen::ApplyPendingTheme()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->SetLightTheme(PendingTheme == ET66UITheme::Light);
		}
	}
	// Safe to rebuild — no Slate event in flight, old style set kept alive by GPreviousStyleSet.
	ForceRebuildSlate();

	// Re-wire rebuilt widgets that need post-build setup
	// (ForceRebuildSlate creates a completely fresh widget tree).
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>())
		{
			const TSoftObjectPtr<UTexture2D> MainMenuBgSoft(FSoftObjectPath(TEXT("/Game/UI/MainMenu/MainMenuBackgroundV3.MainMenuBackgroundV3")));
			TSharedPtr<FSlateBrush> Brush = MainMenuBackgroundBrush;
			TSharedPtr<SImage> Image = MainMenuBackgroundImage;
			TexPool->RequestTexture(MainMenuBgSoft, this, TEXT("MainMenuBackground"), [Brush, Image](UTexture2D* Loaded)
			{
				if (Brush.IsValid())
				{
					Brush->SetResourceObject(Loaded);
					if (Image.IsValid())
					{
						Image->SetImage(Brush.Get());
					}
				}
			});
		}
	}
	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}

	bThemeChangeInProgress = false;
}

// UFUNCTION handlers (call navigation)
void UT66MainMenuScreen::OnNewGameClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bIsNewGameFlow = true;
	}
	NavigateTo(ET66ScreenType::PartySizePicker);
}

void UT66MainMenuScreen::OnLoadGameClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bIsNewGameFlow = false;
	}
	NavigateTo(ET66ScreenType::PartySizePicker);
}

void UT66MainMenuScreen::OnSettingsClicked()
{
	ShowModal(ET66ScreenType::Settings);
}

void UT66MainMenuScreen::OnAchievementsClicked()
{
	NavigateTo(ET66ScreenType::Achievements);
}

void UT66MainMenuScreen::OnLanguageClicked()
{
	ShowModal(ET66ScreenType::LanguageSelect);
}

void UT66MainMenuScreen::OnQuitClicked()
{
	ShowModal(ET66ScreenType::QuitConfirmation);
}

void UT66MainMenuScreen::OnAccountStatusClicked()
{
	ShowModal(ET66ScreenType::AccountStatus);
}

bool UT66MainMenuScreen::ShouldShowAccountStatus() const
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	return LB && LB->ShouldShowAccountStatusButton();
}
