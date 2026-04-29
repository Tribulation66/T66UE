// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

using namespace T66SettingsScreenPrivate;
TSharedRef<SWidget> UT66SettingsScreen::BuildAudioTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	struct FAudioSliderDefinition
	{
		FText Label;
		TFunction<float()> GetValue;
		TFunction<void(float)> SetValue;
	};

	const TArray<FAudioSliderDefinition> SliderDefinitions = {
		{
			Loc ? Loc->GetText_MasterVolume() : NSLOCTEXT("T66.Settings.Fallback", "Master Volume", "Master Volume"),
			[PS]() { return PS ? PS->GetMasterVolume() : 0.8f; },
			[PS](float Value) { if (PS) PS->SetMasterVolume(Value); }
		},
		{
			Loc ? Loc->GetText_MusicVolume() : NSLOCTEXT("T66.Settings.Fallback", "Music Volume", "Music Volume"),
			[PS]() { return PS ? PS->GetMusicVolume() : 0.6f; },
			[PS](float Value) { if (PS) PS->SetMusicVolume(Value); }
		},
		{
			Loc ? Loc->GetText_SfxVolume() : NSLOCTEXT("T66.Settings.Fallback", "SFX Volume", "SFX Volume"),
			[PS]() { return PS ? PS->GetSfxVolume() : 0.8f; },
			[PS](float Value) { if (PS) PS->SetSfxVolume(Value); }
		}
	};

	const TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
	for (const FAudioSliderDefinition& SliderDefinition : SliderDefinitions)
	{
		FSettingsSliderRowParams SliderParams;
		SliderParams.Label = SliderDefinition.Label;
		SliderParams.InitialValue = SliderDefinition.GetValue();
		SliderParams.FormatValueText = [](float Value)
		{
			return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.f)));
		};
		SliderParams.OnValueChanged = SliderDefinition.SetValue;

		Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeSettingsSliderRow(SliderParams)
		];
	}

	Content->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 8.f)
	[
		MakeSettingsRow(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_MuteWhenUnfocused() : NSLOCTEXT("T66.Settings.Fallback", "Mute when unfocused", "Mute when unfocused"))
				.Font(SettingsRegularFont(22))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeDynamicOnOffButton(
					Loc,
					[PS]() { return PS ? PS->GetMuteWhenUnfocused() : false; },
					FOnClicked::CreateLambda([PS]()
					{
						if (PS)
						{
							PS->SetMuteWhenUnfocused(!PS->GetMuteWhenUnfocused());
						}
						return FReply::Handled();
					}))
			],
			FMargin(15.f, 12.f))
	];

	Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
	[
		MakeSettingsDropdownRow(
			Loc ? Loc->GetText_OutputDevice() : NSLOCTEXT("T66.Settings.Fallback", "Output Device", "Output Device"),
			[PS]()
			{
				return (!PS || PS->GetOutputDeviceId().IsEmpty())
					? NSLOCTEXT("T66.Settings.Fallback", "Default", "Default")
					: FText::FromString(PS->GetOutputDeviceId());
			},
			[PS](const TSharedPtr<STextBlock>& CurrentValueText)
			{
				const FText DefaultLabel = NSLOCTEXT("T66.Settings.Fallback", "Default", "Default");
				return SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						FT66Style::MakeDropdownOptionButton(
							DefaultLabel,
							FOnClicked::CreateLambda([PS, CurrentValueText, DefaultLabel]()
							{
								if (PS)
								{
									PS->SetOutputDeviceId(FString());
								}
								if (CurrentValueText.IsValid())
								{
									CurrentValueText->SetText(DefaultLabel);
								}
								FSlateApplication::Get().DismissAllMenus();
								return FReply::Handled();
							}),
							!PS || PS->GetOutputDeviceId().IsEmpty(),
							0.f,
							34.f,
							14)
					];
			})
	];

	Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
	[
		MakeSettingsRow(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SubtitlesAlwaysOn() : NSLOCTEXT("T66.Settings.Fallback", "Subtitles: always on", "Subtitles: always on"))
				.Font(SettingsRegularFont(22))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeDynamicOnOffButton(
					Loc,
					[PS]() { return PS ? PS->GetSubtitlesAlwaysOn() : false; },
					FOnClicked::CreateLambda([PS]()
					{
						if (PS)
						{
							PS->SetSubtitlesAlwaysOn(!PS->GetSubtitlesAlwaysOn());
						}
						return FReply::Handled();
					}))
			],
			FMargin(15.f, 12.f))
	];

	return Content;
}


