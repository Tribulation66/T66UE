// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TemporaryBuffPresetCreateScreen.h"
#include "Core/T66BuffSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

UT66TemporaryBuffPresetCreateScreen::UT66TemporaryBuffPresetCreateScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::TemporaryBuffPresetCreate;
	bIsModal = true;
}

UT66BuffSubsystem* UT66TemporaryBuffPresetCreateScreen::GetBuffSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66BuffSubsystem>();
	}

	return nullptr;
}

void UT66TemporaryBuffPresetCreateScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66TemporaryBuffPresetCreateScreen::BuildSlateUI()
{
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float ModalWidth = FMath::Min(SafeFrameSize.X * 0.40f, 460.0f);
	const float ModalHeight = 240.0f;
	const FText TitleText = NSLOCTEXT("T66.TempBuffPreset", "CreateTitle", "CREATE PRESET");
	const FText HintText = NSLOCTEXT("T66.TempBuffPreset", "CreateHint", "Name the preset, then fill its 5 slots from the temp-buff selector.");
	const FText NameLabelText = NSLOCTEXT("T66.TempBuffPreset", "NameLabel", "Preset Name");
	const FText NameHintText = NSLOCTEXT("T66.TempBuffPreset", "NameHint", "Preset 2");
	const FText CreateText = NSLOCTEXT("T66.TempBuffPreset", "CreateButton", "CREATE");
	const FText CancelText = NSLOCTEXT("T66.Common", "Back", "BACK");

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Scrim())
		[
			SNew(SBox)
			.WidthOverride(ModalWidth)
			.HeightOverride(ModalHeight)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Panel())
				.Padding(FMargin(24.f, 20.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(22))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 6.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(HintText)
						.Font(FT66Style::Tokens::FontRegular(10))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 18.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Text(NameLabelText)
						.Font(FT66Style::Tokens::FontBold(11))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(PresetNameTextBox, SEditableTextBox)
						.Text(FText::FromString(DraftPresetName))
						.HintText(NameHintText)
						.OnTextChanged(FOnTextChanged::CreateUObject(this, &UT66TemporaryBuffPresetCreateScreen::HandlePresetNameChanged))
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SBox)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 8.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(CancelText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffPresetCreateScreen::HandleCancelClicked), ET66ButtonType::Neutral)
								.SetMinWidth(116.f)
								.SetHeight(38.f)
								.SetFontSize(12))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							FT66Style::MakeButton(
								FT66ButtonParams(CreateText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffPresetCreateScreen::HandleCreateClicked), ET66ButtonType::Primary)
								.SetMinWidth(132.f)
								.SetHeight(38.f)
								.SetFontSize(12))
						]
					]
				]
			]
		];
}

void UT66TemporaryBuffPresetCreateScreen::HandlePresetNameChanged(const FText& NewText)
{
	DraftPresetName = NewText.ToString();
}

FReply UT66TemporaryBuffPresetCreateScreen::HandleCreateClicked()
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		Buffs->CreateTemporaryBuffPreset(DraftPresetName);
	}

	CloseModal();
	return FReply::Handled();
}

FReply UT66TemporaryBuffPresetCreateScreen::HandleCancelClicked()
{
	CloseModal();
	return FReply::Handled();
}
