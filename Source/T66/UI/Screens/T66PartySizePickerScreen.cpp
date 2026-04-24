// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PartySizePickerScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"

namespace
{
	constexpr float T66PartyPickerCardWidth = 560.f;
	constexpr float T66PartyPickerCardHeight = 560.f;
	constexpr float T66PartyPickerButtonHeight = 48.f;

	FLinearColor T66PartyPickerBrightText()
	{
		return FLinearColor(0.97f, 0.94f, 0.86f, 1.0f);
	}

	FLinearColor T66PartyPickerGoldText()
	{
		return FLinearColor(0.94f, 0.76f, 0.34f, 1.0f);
	}

	FLinearColor T66PartyPickerMutedText()
	{
		return FLinearColor(0.72f, 0.67f, 0.56f, 1.0f);
	}

	const FSlateBrush* ResolvePartyPickerBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	const FSlateBrush* GetPartyPickerSceneBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolvePartyPickerBrush(
			Entry,
			TEXT("SourceAssets/UI/MainMenuReference/scene_background_1920x1080.png"),
			FMargin(0.f),
			TEXT("PartyPickerScene"));
	}

	const FSlateBrush* GetPartyPickerShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolvePartyPickerBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_content_shell_frame.png"),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f),
			TEXT("PartyPickerShell"));
	}

	const FSlateBrush* GetPartyPickerSlotFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolvePartyPickerBrush(
			Entry,
			TEXT("SourceAssets/UI/MainMenuReference/LeftPanel/party_slot_frame.png"),
			FMargin(0.f),
			TEXT("PartyPickerSlotFrame"));
	}

	const FSlateBrush* GetPartyPickerButtonPlateBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolvePartyPickerBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_compact_neutral_normal.png"),
			FMargin(0.16f, 0.28f, 0.16f, 0.28f),
			TEXT("PartyPickerButtonNeutral"));
	}

	TSharedRef<SWidget> MakePartyPickerShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* ShellBrush = GetPartyPickerShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(ShellBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel2).SetPadding(Padding));
	}

	TSharedRef<SWidget> MakePartyPickerPlateButton(FT66ButtonParams Params)
	{
		Params
			.SetUseGlow(false)
			.SetUseDotaPlateOverlay(true)
			.SetDotaPlateOverrideBrush(GetPartyPickerButtonPlateBrush())
			.SetStateTextColors(T66PartyPickerBrightText(), T66PartyPickerGoldText(), T66PartyPickerBrightText())
			.SetStateTextShadowColors(FLinearColor(0.f, 0.f, 0.f, 0.70f), FLinearColor(0.f, 0.f, 0.f, 0.78f), FLinearColor(0.f, 0.f, 0.f, 0.85f))
			.SetTextShadowOffset(FVector2D(1.f, 1.f));
		return FT66Style::MakeButton(Params);
	}

	TSharedRef<SWidget> MakePartyPickerSlot(const int32 Index, const bool bActive)
	{
		return SNew(SBox)
			.WidthOverride(76.f)
			.HeightOverride(84.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(GetPartyPickerSlotFrameBrush())
					.ColorAndOpacity(bActive ? FLinearColor::White : FLinearColor(0.55f, 0.55f, 0.55f, 0.78f))
				]
				+ SOverlay::Slot()
				.Padding(12.f, 12.f, 12.f, 14.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(bActive ? FLinearColor(0.10f, 0.14f, 0.11f, 0.96f) : FLinearColor(0.06f, 0.07f, 0.06f, 0.82f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::AsNumber(Index + 1))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(bActive ? T66PartyPickerGoldText() : T66PartyPickerMutedText())
						.Justification(ETextJustify::Center)
					]
				]
			];
	}
}

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

	const FText TitleText = Loc ? Loc->GetText_SelectPartySize() : NSLOCTEXT("T66.PartySize", "Title", "SELECT PARTY SIZE");
	const FText SoloText = Loc ? Loc->GetText_Solo() : NSLOCTEXT("T66.PartySize", "Solo", "SOLO");
	const FText CoopText = Loc ? Loc->GetText_Coop() : NSLOCTEXT("T66.PartySize", "Coop", "CO-OP");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText SoloDetailText = NSLOCTEXT("T66.PartySize", "SoloDetail", "One saved party slot. Fastest route into a run.");
	const FText CoopDetailText = NSLOCTEXT("T66.PartySize", "CoopDetail", "Create a party lobby and continue with allies.");

	const FButtonStyle& NoBorderStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
	FOnClicked SafeSoloClick = FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66PartySizePickerScreen::HandleSoloClicked));
	FOnClicked SafeCoopClick = FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66PartySizePickerScreen::HandleCoopClicked));

	auto MakePartySizeCard = [&NoBorderStyle](const FText& LabelText, const FText& DetailText, const int32 ActiveSlots, FOnClicked OnClick) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> SlotRow = SNew(SHorizontalBox);
		for (int32 Index = 0; Index < 4; ++Index)
		{
			SlotRow->AddSlot()
				.AutoWidth()
				.Padding(Index > 0 ? FMargin(12.f, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					MakePartyPickerSlot(Index, Index < ActiveSlots)
				];
		}

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
					SNew(SBox)
					.WidthOverride(T66PartyPickerCardWidth)
					.HeightOverride(T66PartyPickerCardHeight)
					[
						MakePartyPickerShell(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.f, 0.f, 0.f, 20.f)
							[
								SNew(STextBlock)
								.Text(LabelText)
								.Font(FT66Style::Tokens::FontBold(34))
								.ColorAndOpacity(T66PartyPickerGoldText())
								.Justification(ETextJustify::Center)
								.ShadowOffset(FVector2D(1.f, 1.f))
								.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.72f))
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.f, 0.f, 0.f, 34.f)
							[
								SNew(SBox)
								.WidthOverride(410.f)
								[
									SNew(STextBlock)
									.Text(DetailText)
									.Font(FT66Style::Tokens::FontRegular(16))
									.ColorAndOpacity(T66PartyPickerMutedText())
									.AutoWrapText(true)
									.Justification(ETextJustify::Center)
								]
							]
							+ SVerticalBox::Slot()
							.FillHeight(1.f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SlotRow
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.f, 34.f, 0.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(220.f)
								.HeightOverride(T66PartyPickerButtonHeight)
								[
									SNew(SBorder)
									.BorderImage(GetPartyPickerButtonPlateBrush())
									.BorderBackgroundColor(FLinearColor::White)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.Padding(FMargin(18.f, 8.f, 18.f, 7.f))
									[
										SNew(STextBlock)
										.Text(LabelText)
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(T66PartyPickerBrightText())
										.Justification(ETextJustify::Center)
										.ShadowOffset(FVector2D(1.f, 1.f))
										.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.72f))
									]
								]
							],
							FMargin(42.f, 40.f, 42.f, 42.f))
					]
				]
			];
	};

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.015f, 0.014f, 0.011f, 1.0f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(MainMenuBackgroundImage, SImage)
			.Image(GetPartyPickerSceneBrush())
			.ColorAndOpacity(FLinearColor(0.76f, 0.76f, 0.76f, 1.0f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.f, 240.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(TitleText)
			.Font(FT66Style::Tokens::FontBold(38))
			.ColorAndOpacity(T66PartyPickerGoldText())
			.Justification(ETextJustify::Center)
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.78f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(20.0f, 330.0f, 20.0f, 74.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.5f)
				[
					MakePartySizeCard(SoloText, SoloDetailText, 1, SafeSoloClick)
				]
				+ SHorizontalBox::Slot().FillWidth(0.5f)
				[
					MakePartySizeCard(CoopText, CoopDetailText, 4, SafeCoopClick)
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(20.0f, 0.0f, 0.0f, 20.0f)
		[
			MakePartyPickerPlateButton(
				FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66PartySizePickerScreen::HandleBackClicked), ET66ButtonType::Neutral)
				.SetMinWidth(126.f)
				.SetHeight(T66PartyPickerButtonHeight)
				.SetFontSize(11))
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
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
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
			if (UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr)
			{
				SessionSubsystem->EnsurePartySessionReady(PartySize, ET66ScreenType::MainMenu);
			}

			NavigateTo(ET66ScreenType::HeroSelection);
		}
	}
	else
	{
		NavigateTo(ET66ScreenType::SaveSlots);
	}
}

void UT66PartySizePickerScreen::OnSoloClicked() { SelectPartySize(ET66PartySize::Solo); }
void UT66PartySizePickerScreen::OnCoopClicked() { SelectPartySize(ET66PartySize::Duo); }
void UT66PartySizePickerScreen::OnBackClicked() { NavigateBack(); }
