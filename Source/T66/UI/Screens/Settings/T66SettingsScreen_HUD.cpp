// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

using namespace T66SettingsScreenPrivate;
TSharedRef<SWidget> UT66SettingsScreen::BuildHUDTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	InitializeUIScaleFromPlayerSettingsIfNeeded();
	FSettingsSliderRowParams UIScaleSliderParams;
	UIScaleSliderParams.Label = NSLOCTEXT("T66.Settings", "UIScale", "UI Scale");
	UIScaleSliderParams.InitialValue = (FMath::Clamp(PendingUIScale, 0.75f, 1.50f) - 0.75f) / 0.75f;
	UIScaleSliderParams.FormatValueText = [](float Value)
	{
		const float UIScale = FMath::Clamp(FMath::Lerp(0.75f, 1.50f, Value), 0.75f, 1.50f);
		return FText::FromString(FString::Printf(TEXT("%.2fx"), UIScale));
	};
	UIScaleSliderParams.OnValueChanged = [this](float Value)
	{
		PendingUIScale = FMath::Clamp(FMath::Lerp(0.75f, 1.50f, Value), 0.75f, 1.50f);
	};
	UIScaleSliderParams.OnMouseCaptureEnd = [this, PS]()
	{
		if (PS)
		{
			PS->SetUIScale(PendingUIScale);
			bUIScaleInitialized = false;
		}
	};
	UIScaleSliderParams.OnControllerCaptureEnd = UIScaleSliderParams.OnMouseCaptureEnd;
	UIScaleSliderParams.HelpText = NSLOCTEXT("T66.Settings", "UIScaleHelp", "Scales the entire HUD and menu UI on top of the automatic DPI scale.");

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsSliderRow(UIScaleSliderParams)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SettingsHudToggleIntro() : NSLOCTEXT("T66.Settings", "HudToggleIntro", "When you press the HUD toggle key, the following elements will show or hide:"))
				.Font(SettingsRegularFont(18))
				.ColorAndOpacity(GetSettingsPageText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsHudInventory() : NSLOCTEXT("T66.Settings", "HudInventory", "Inventory"),
					[PS]() { return PS ? PS->GetHudToggleAffectsInventory() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsInventory(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsHudMinimap() : NSLOCTEXT("T66.Settings", "HudMinimap", "Minimap"),
					[PS]() { return PS ? PS->GetHudToggleAffectsMinimap() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsMinimap(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsHudIdolSlots() : NSLOCTEXT("T66.Settings", "HudIdolSlots", "Idol slots"),
					[PS]() { return PS ? PS->GetHudToggleAffectsIdolSlots() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsIdolSlots(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsHudPortraitStats() : NSLOCTEXT("T66.Settings", "HudPortraitStats", "Portrait & stats"),
					[PS]() { return PS ? PS->GetHudToggleAffectsPortraitStats() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsPortraitStats(b); }
				)
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildMediaViewerTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	struct FMediaSourceDefinition
	{
		FText Label;
		ET66MediaViewerSource Source = ET66MediaViewerSource::TikTok;
	};

	const TArray<FMediaSourceDefinition> SourceDefinitions = {
		{ NSLOCTEXT("T66.Settings", "MediaSourceTikTok", "TIKTOK"), ET66MediaViewerSource::TikTok },
		{ NSLOCTEXT("T66.Settings", "MediaSourceShorts", "YOUTUBE SHORTS"), ET66MediaViewerSource::Shorts },
		{ NSLOCTEXT("T66.Settings", "MediaSourceReels", "INSTAGRAM REELS"), ET66MediaViewerSource::Reels }
	};

	const TSharedRef<SHorizontalBox> SourceButtons = SNew(SHorizontalBox);
	for (int32 Index = 0; Index < SourceDefinitions.Num(); ++Index)
	{
		const FMediaSourceDefinition& SourceDefinition = SourceDefinitions[Index];
		SourceButtons->AddSlot().AutoWidth().Padding(Index + 1 < SourceDefinitions.Num() ? FMargin(0.f, 0.f, 6.f, 0.f) : FMargin(0.f))
		[
			MakeSelectableSettingsButton(
				FT66ButtonParams(
					SourceDefinition.Label,
					FOnClicked::CreateLambda([PS, Source = SourceDefinition.Source]()
					{
						if (PS)
						{
							PS->SetMediaViewerSource(Source);
						}
						return FReply::Handled();
					}),
					ET66ButtonType::Neutral)
				.SetMinWidth(152.f)
				.SetPadding(FMargin(10.f, 5.f)),
				[PS, Source = SourceDefinition.Source]()
				{
					return PS && PS->GetMediaViewerSource() == Source;
				})
		];
	}

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SettingsMediaViewerPrivacyBody() : NSLOCTEXT("T66.Settings", "MediaViewerPrivacyBody", "The Media Viewer runs only on your computer. We do not receive or store the videos you watch or any data from TikTok, YouTube, or Instagram. Choose which feed opens here, then use the key bound to \"Toggle Media Viewer\" in the Controls tab."))
				.Font(SettingsRegularFont(18))
				.ColorAndOpacity(GetSettingsPageText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 16.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsMediaViewerEnable() : NSLOCTEXT("T66.Settings", "MediaViewerEnable", "Enable Media Viewer"),
					[PS]() { return PS ? PS->GetMediaViewerEnabled() : true; },
					[PS](bool bEnabled)
					{
						if (PS)
						{
							PS->SetMediaViewerEnabled(bEnabled);
						}
					})
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsRow(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.32f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_SettingsMediaViewerSource() : NSLOCTEXT("T66.Settings", "MediaViewerSource", "Default Feed"))
						.Font(SettingsRegularFont(22))
						.ColorAndOpacity(GetSettingsPageText())
					]
					+ SHorizontalBox::Slot().FillWidth(0.68f).VAlign(VAlign_Center)
					[
						SourceButtons
					])
			]
		];
}

void UT66SettingsScreen::InitializeUIScaleFromPlayerSettingsIfNeeded()
{
	if (bUIScaleInitialized)
	{
		return;
	}

	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PendingUIScale = PS->GetUIScale();
	}
	else
	{
		PendingUIScale = 1.0f;
	}

	bUIScaleInitialized = true;
}


