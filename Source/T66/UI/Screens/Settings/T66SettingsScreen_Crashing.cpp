// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

using namespace T66SettingsScreenPrivate;
TSharedRef<SWidget> UT66SettingsScreen::BuildCrashingTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	return SNew(SVerticalBox)
		// Header
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 20.0f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_CrashingChecklistHeader() : NSLOCTEXT("T66.Settings.Fallback", "CrashingChecklistHeader", "If your game is crashing, try these steps:"))
			.Font(SettingsBoldFont(24))
			.ColorAndOpacity(GetSettingsPageText())
		]
		// Checklist
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(FMargin(20.0f, 15.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_CrashingChecklistBody() : FText::GetEmpty())
					.Font(SettingsRegularFont(18))
					.ColorAndOpacity(GetSettingsPageText())
					.AutoWrapText(true)
				]
			]
		]
		// Safe Mode button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 15.0f, 0.0f, 20.0f)
		[
			MakeSettingsButton(
				FT66ButtonParams(Loc ? Loc->GetText_ApplySafeModeSettings() : NSLOCTEXT("T66.Settings.Fallback", "Apply Safe Mode Settings", "Apply Safe Mode Settings"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleSafeModeClicked), ET66ButtonType::Danger)
				.SetFontSize(20).SetMinWidth(180.f))
		]
		// Separator
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 20.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			[
				SNew(SBox).HeightOverride(1.0f)
			]
		]
		// Report Bug section
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_StillHavingIssues() : NSLOCTEXT("T66.Settings.Fallback", "Still having issues?", "Still having issues?"))
			.Font(SettingsBoldFont(22))
			.ColorAndOpacity(GetSettingsPageText())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_ReportBugDescription() : NSLOCTEXT("T66.Settings.Fallback", "ReportBugDescription", "Report a bug to help us fix it. Your report will include basic system info."))
			.Font(SettingsRegularFont(16))
			.ColorAndOpacity(GetSettingsPageMuted())
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			MakeSettingsButton(
				FT66ButtonParams(Loc ? Loc->GetText_ReportBug() : NSLOCTEXT("T66.ReportBug", "Title", "REPORT BUG"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleReportBugClicked), ET66ButtonType::Primary)
				.SetFontSize(20).SetMinWidth(130.f))
		];
}

FReply UT66SettingsScreen::HandleReportBugClicked()
{
	ShowModal(ET66ScreenType::ReportBug);
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleSafeModeClicked()
{
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->ApplySafeModeSettings();
	}
	return FReply::Handled();
}


