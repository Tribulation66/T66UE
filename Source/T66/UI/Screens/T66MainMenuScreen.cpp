// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MainMenuScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
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
#include "Widgets/SNullWidget.h"
#include "Styling/SlateBrush.h"

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
	FText PowerUpText = NSLOCTEXT("T66.MainMenu", "PowerUp", "POWER UP");
	FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.MainMenu", "Achievements", "ACHIEVEMENTS");
	FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.MainMenu", "Settings", "SETTINGS");
	FText QuitText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");

	// Button: 30% smaller (63 -> 44), equal padding on all 4 sides so text isn't glued to edges
	const float ButtonSpacing = 14.0f;

	auto MakeMenuButton = [this](const FText& Text, FReply (UT66MainMenuScreen::*ClickFunc)(), const FLinearColor& BgColor = FT66Style::Tokens::Panel2) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(
			FT66ButtonParams(Text, FOnClicked::CreateUObject(this, ClickFunc))
			.SetFontSize(44)
			.SetPadding(FMargin(14.f))
			.SetColor(BgColor)
			.SetMinWidth(0.f));
	};

	// Background image brush (filled by texture pool when MainMenuBackground texture loads)
	MainMenuBackgroundBrush = MakeShared<FSlateBrush>();
	MainMenuBackgroundBrush->DrawAs = ESlateBrushDrawType::Box;
	MainMenuBackgroundBrush->Tiling = ESlateBrushTileType::NoTile;
	MainMenuBackgroundBrush->SetResourceObject(nullptr);

	// Use cached texture immediately if preloaded (avoids white flash on PIE and theme switch)
	if (UGameInstance* GIPtr = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66UITexturePoolSubsystem* TexPool = GIPtr->GetSubsystem<UT66UITexturePoolSubsystem>())
		{
			const FString BgAssetName = (FT66Style::GetTheme() == ET66UITheme::Light) ? TEXT("MMLight") : TEXT("MMDark");
			const TSoftObjectPtr<UTexture2D> MainMenuBgSoft(FSoftObjectPath(FString::Printf(TEXT("/Game/UI/MainMenu/%s.%s"), *BgAssetName, *BgAssetName)));
			if (UTexture2D* Cached = TexPool->GetLoadedTexture(MainMenuBgSoft))
			{
				MainMenuBackgroundBrush->SetResourceObject(Cached);
			}
		}
	}

	// Filter icons are now loaded and displayed inside the leaderboard panel (pure Slate).

	return FT66Style::MakePanel(
		SNew(SOverlay)
			// Theme-colored underlay so we never show white before the image loads
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				FT66Style::MakePanel(SNullWidget::NullWidget, FT66PanelParams(ET66PanelType::Bg).SetPadding(0.f))
			]
			// Full-screen background image (Content/UI/MainMenu/MMDark or MMLight)
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
					[ MakeMenuButton(PowerUpText, &UT66MainMenuScreen::HandlePowerUpClicked, FT66Style::Tokens::Accent2) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, ButtonSpacing)
					[ MakeMenuButton(AchievementsText, &UT66MainMenuScreen::HandleAchievementsClicked) ]
					+ SVerticalBox::Slot().AutoHeight()
					[ MakeMenuButton(SettingsText, &UT66MainMenuScreen::HandleSettingsClicked) ]
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
				FT66Style::MakeButton(QuitText, FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleQuitClicked), ET66ButtonType::Danger, 100.f)
			]
			// Bottom-left: Language button
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(15.0f, 0.0f, 0.0f, 15.0f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(Loc ? Loc->GetText_LangButton() : NSLOCTEXT("T66.LanguageSelect", "LangButton", "Lang"),
						FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleLanguageClicked))
					.SetFontSize(20)
					.SetMinWidth(50.f).SetHeight(50.f))
			]
		,
		ET66PanelType::Panel);
}

void UT66MainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	UE_LOG(LogTemp, Log, TEXT("MainMenuScreen activated!"));

	RequestBackgroundTexture();

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
	// Force rebuild UI with current language (deferred to next tick for safety)
	ForceRebuildSlate();

	// Re-request background texture after rebuild schedules a fresh widget tree
	RequestBackgroundTexture();

	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}
}

void UT66MainMenuScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	// Rebuild UI when language changes
	ForceRebuildSlate();
	RequestBackgroundTexture();
	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}
}

void UT66MainMenuScreen::RequestBackgroundTexture()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>())
		{
			const FString BgAssetName = (FT66Style::GetTheme() == ET66UITheme::Light) ? TEXT("MMLight") : TEXT("MMDark");
			const TSoftObjectPtr<UTexture2D> MainMenuBgSoft(FSoftObjectPath(FString::Printf(TEXT("/Game/UI/MainMenu/%s.%s"), *BgAssetName, *BgAssetName)));
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

FReply UT66MainMenuScreen::HandlePowerUpClicked()
{
	OnPowerUpClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleAchievementsClicked()
{
	OnAchievementsClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleSettingsClicked()
{
	OnSettingsClicked();
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

void UT66MainMenuScreen::OnPowerUpClicked()
{
	NavigateTo(ET66ScreenType::PowerUp);
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
