// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PartySizePickerScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"

UT66PartySizePickerScreen::UT66PartySizePickerScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PartySizePicker;
	bIsModal = false;
	bIsNewGame = true;
}

TSharedRef<SWidget> UT66PartySizePickerScreen::BuildSlateUI()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	FText SoloText = Loc ? Loc->GetText_Solo() : NSLOCTEXT("T66.PartySize", "Solo", "SOLO");
	FText CoopText = Loc ? Loc->GetText_Coop() : NSLOCTEXT("T66.PartySize", "Coop", "CO-OP");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	// Brushes for theme-dependent party picker images (Content/UI/PartyPicker: SoloDark, SoloLight, CoopDark, CoopLight)
	SoloCardBrush = MakeShared<FSlateBrush>();
	SoloCardBrush->DrawAs = ESlateBrushDrawType::Image;
	SoloCardBrush->ImageSize = FVector2D(560.f, 560.f);
	SoloCardBrush->Tiling = ESlateBrushTileType::NoTile;
	SoloCardBrush->SetResourceObject(nullptr);

	CoopCardBrush = MakeShared<FSlateBrush>();
	CoopCardBrush->DrawAs = ESlateBrushDrawType::Image;
	CoopCardBrush->ImageSize = FVector2D(560.f, 560.f);
	CoopCardBrush->Tiling = ESlateBrushTileType::NoTile;
	CoopCardBrush->SetResourceObject(nullptr);

	const bool bLight = (FT66Style::GetTheme() == ET66UITheme::Light);
	const FLinearColor LabelColor = bLight ? FLinearColor::Black : FLinearColor::White;
	const FString SoloAssetName = bLight ? TEXT("SoloLight") : TEXT("SoloDark");
	const FString CoopAssetName = bLight ? TEXT("CoopLight") : TEXT("CoopDark");
	const TSoftObjectPtr<UTexture2D> SoloSoft(FSoftObjectPath(FString::Printf(TEXT("/Game/UI/PartyPicker/%s.%s"), *SoloAssetName, *SoloAssetName)));
	const TSoftObjectPtr<UTexture2D> CoopSoft(FSoftObjectPath(FString::Printf(TEXT("/Game/UI/PartyPicker/%s.%s"), *CoopAssetName, *CoopAssetName)));

	if (TexPool)
	{
		if (UTexture2D* SoloTex = TexPool->GetLoadedTexture(SoloSoft))
		{
			SoloCardBrush->SetResourceObject(SoloTex);
		}
		else
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, SoloSoft, this, SoloCardBrush, FName("PartyPickerSolo"), false);
		}
		if (UTexture2D* CoopTex = TexPool->GetLoadedTexture(CoopSoft))
		{
			CoopCardBrush->SetResourceObject(CoopTex);
		}
		else
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, CoopSoft, this, CoopCardBrush, FName("PartyPickerCoop"), false);
		}
	}

	// Main menu background (MMDark / MMLight) — same as main menu
	MainMenuBackgroundBrush = MakeShared<FSlateBrush>();
	MainMenuBackgroundBrush->DrawAs = ESlateBrushDrawType::Box;
	MainMenuBackgroundBrush->Tiling = ESlateBrushTileType::NoTile;
	MainMenuBackgroundBrush->SetResourceObject(nullptr);
	if (TexPool)
	{
		const FString BgAssetName = bLight ? TEXT("MMLight") : TEXT("MMDark");
		const TSoftObjectPtr<UTexture2D> BgSoft(FSoftObjectPath(FString::Printf(TEXT("/Game/UI/MainMenu/%s.%s"), *BgAssetName, *BgAssetName)));
		if (UTexture2D* Cached = TexPool->GetLoadedTexture(BgSoft))
		{
			MainMenuBackgroundBrush->SetResourceObject(Cached);
		}
		else
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, BgSoft, this, MainMenuBackgroundBrush, FName("PartyPickerBg"), false);
		}
	}

	const FButtonStyle& NoBorderStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
	FOnClicked SafeSoloClick = FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66PartySizePickerScreen::HandleSoloClicked));
	FOnClicked SafeCoopClick = FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66PartySizePickerScreen::HandleCoopClicked));

	auto MakePartySizeCard = [this, &NoBorderStyle, LabelColor](const TSharedPtr<FSlateBrush>& ImageBrush, const FText& LabelText, FOnClicked OnClick) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(8.0f, 0.0f))
			[
				SNew(SButton)
				.ButtonStyle(&NoBorderStyle)
				.OnClicked(OnClick)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.ContentPadding(0.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 12.0f)
					[
						FT66Style::MakePanel(
							SNew(SBox)
								.WidthOverride(560.0f)
								.HeightOverride(560.0f)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SImage)
									.Image(ImageBrush.IsValid() ? ImageBrush.Get() : nullptr)
									.ColorAndOpacity(FLinearColor::White)
								],
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(4.f))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						FT66Style::MakePanel(
							SNew(STextBlock)
								.Text(LabelText)
								.Font(FT66Style::Tokens::FontBold(32))
								.ColorAndOpacity(LabelColor),
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f, 8.f)))
					]
				]
			];
	};

	return SNew(SOverlay)
		// Theme-colored underlay (fallback if main menu texture not loaded yet)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			FT66Style::MakePanel(SNullWidget::NullWidget, FT66PanelParams(ET66PanelType::Bg).SetPadding(0.f))
		]
		// Main menu background image (MMDark / MMLight)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(MainMenuBackgroundImage, SImage)
			.Image(TAttribute<const FSlateBrush*>::Create([this]() -> const FSlateBrush* { return MainMenuBackgroundBrush.IsValid() ? MainMenuBackgroundBrush.Get() : nullptr; }))
		]
		// Party size cards
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(20.0f, 380.0f, 20.0f, 60.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.5f)
				[
					MakePartySizeCard(SoloCardBrush, SoloText, SafeSoloClick)
				]
				+ SHorizontalBox::Slot().FillWidth(0.5f)
				[
					MakePartySizeCard(CoopCardBrush, CoopText, SafeCoopClick)
				]
			]
		]
		// Back button
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(20.0f, 0.0f, 0.0f, 20.0f)
		[
			FT66Style::MakeButton(BackText, FOnClicked::CreateUObject(this, &UT66PartySizePickerScreen::HandleBackClicked), ET66ButtonType::Neutral, 120.f)
		];
}

FReply UT66PartySizePickerScreen::HandleSoloClicked() { OnSoloClicked(); return FReply::Handled(); }
FReply UT66PartySizePickerScreen::HandleCoopClicked() { OnCoopClicked(); return FReply::Handled(); }
FReply UT66PartySizePickerScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

void UT66PartySizePickerScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		bIsNewGame = GI->bIsNewGameFlow;
	}
}

void UT66PartySizePickerScreen::SelectPartySize(ET66PartySize PartySize)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedPartySize = PartySize;
	}

	if (bIsNewGame) // set from GI->bIsNewGameFlow in OnScreenActivated
	{
		if (PartySize == ET66PartySize::Solo)
		{
			NavigateTo(ET66ScreenType::HeroSelection);
		}
		else
		{
			// Co-op: use Trio (3 slots) for lobby; Duo/Trio only for leaderboards/saves
			NavigateTo(ET66ScreenType::Lobby);
		}
	}
	else
	{
		NavigateTo(ET66ScreenType::SaveSlots);
	}
}

void UT66PartySizePickerScreen::OnSoloClicked() { SelectPartySize(ET66PartySize::Solo); }
void UT66PartySizePickerScreen::OnCoopClicked() { SelectPartySize(ET66PartySize::Trio); } // Co-op UI → Trio lobby (3 slots)
void UT66PartySizePickerScreen::OnBackClicked() { NavigateBack(); }
