// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"

using namespace T66SettingsScreenPrivate;

TSharedRef<SWidget> UT66SettingsScreen::BuildFlatAudioSettingsUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;
	const FName SettingsTabsGroup(TEXT("SettingsTabs"));

	auto DTag = [](const TCHAR* Text) -> FName
	{
		return FName(Text);
	};

	auto ChildTag = [](const FName& BaseTag, const TCHAR* Suffix) -> FName
	{
		return FName(*(BaseTag.ToString() + FString(TEXT(".")) + Suffix));
	};

	auto MakeLabel = [](const FName Tag, const TAttribute<FText>& Text, const int32 FontSize, const FLinearColor& Color, const bool bBold = false, const ETextJustify::Type Justify = ETextJustify::Left, const bool bAutoWrap = true) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justify)
			.AutoWrapText(bAutoWrap)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	auto MakeMetadataRegion = [](const FName Tag, const FString& Role) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(SNew(SBox), Tag, Role, ET66FlatState::Default);
	};

	auto MakeFlatTab = [this, &MakeLabel, &ChildTag, SettingsTabsGroup](const FName Tag, const ET66SettingsTab Tab, const ET66FlatState State, const FText& Text, const float Width, const int32 FontSize = 22) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(Tag, TEXT("Label")), Text, FontSize, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center, false)
			],
			FOnClicked::CreateLambda([this, Tab]()
			{
				SwitchToTab(Tab);
				ForceRebuildSlate();
				return FReply::Handled();
			}),
			FMargin(0.f),
			Width,
			0.079f * CanvasH,
			true,
			Tag,
			SettingsTabsGroup);
	};

	auto MakeValueText = [&MakeLabel](const FName Tag, TFunction<float()> Getter) -> TSharedRef<SWidget>
	{
		return MakeLabel(
			Tag,
			TAttribute<FText>::CreateLambda([Getter = MoveTemp(Getter)]()
			{
				return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(FMath::Clamp(Getter(), 0.f, 1.f) * 100.f)));
			}),
			26,
			FT66FlatStyle::PrimaryText(),
			true,
			ETextJustify::Right,
			false);
	};

	auto MakeSliderRow = [&MakeLabel, &MakeValueText, &ChildTag](const FName RowTag, const FText& Label, TFunction<float()> Getter, TFunction<void(float)> Setter) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.30f).VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Label")), Label, 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot().FillWidth(0.56f).VAlign(VAlign_Center).Padding(18.f, 0.f)
			[
				FT66FlatStyle::MakeFlatSlider(
					ET66FlatState::Default,
					0.f,
					1.f,
					TAttribute<float>::CreateLambda([Getter]() { return FMath::Clamp(Getter(), 0.f, 1.f); }),
					FOnFloatValueChanged::CreateLambda([Setter](float Value)
					{
						Setter(FMath::Clamp(Value, 0.f, 1.f));
					}),
					nullptr,
					ChildTag(RowTag, TEXT("Slider")))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(92.f).HAlign(HAlign_Right)
				[
					MakeValueText(ChildTag(RowTag, TEXT("Value")), Getter)
				]
			];

		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(34.f, 0.f, 28.f, 0.f), Row, nullptr, RowTag);
	};

	auto MakeToggleRow = [this, Loc, &MakeLabel, &ChildTag](const FName RowTag, const FText& Label, TFunction<bool()> Getter, TFunction<void()> ToggleAction) -> TSharedRef<SWidget>
	{
		const bool bEnabled = Getter();
		const FText ButtonLabel = bEnabled ? GetSettingsOnText(Loc) : GetSettingsOffText(Loc);
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Label")), Label, 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Selected,
					ButtonLabel,
					FOnClicked::CreateLambda([this, ToggleAction = MoveTemp(ToggleAction)]()
					{
						ToggleAction();
						ForceRebuildSlate();
						return FReply::Handled();
					}),
					nullptr,
					nullptr,
					FMargin(12.f, 8.f),
					130.f,
					64.f,
					true,
					22,
					ChildTag(RowTag, TEXT("ToggleButton")))
			];
		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(18.f, 0.f, 22.f, 0.f), Row, nullptr, RowTag);
	};

	auto MakeDropdownOption = [this](const FName Tag, const FText& Label, TFunction<void()> Action, const bool bSelected) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatButton(
			bSelected ? ET66FlatState::Selected : ET66FlatState::Default,
			Label,
			FOnClicked::CreateLambda([this, Action = MoveTemp(Action)]()
			{
				Action();
				FSlateApplication::Get().DismissAllMenus();
				ForceRebuildSlate();
				return FReply::Handled();
			}),
			nullptr,
			nullptr,
			FMargin(12.f, 8.f),
			640.f,
			44.f,
			true,
			18,
			Tag);
	};

	auto MakeOutputRow = [this, PS, Loc, &MakeLabel, &ChildTag, &MakeDropdownOption](const FName RowTag) -> TSharedRef<SWidget>
	{
		auto CurrentOutputText = [PS]()
		{
			return (!PS || PS->GetOutputDeviceId().IsEmpty())
				? NSLOCTEXT("T66.Settings.Fallback", "DefaultFlatAudio", "Default")
				: FText::FromString(PS->GetOutputDeviceId());
		};

		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.36f).VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Label")), Loc ? Loc->GetText_OutputDevice() : NSLOCTEXT("T66.Settings.Fallback", "Output Device Flat", "Output Device"), 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot().FillWidth(0.64f).VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatDropdown(
					ET66FlatState::Selected,
					TAttribute<FText>::CreateLambda(CurrentOutputText),
					[PS, &MakeDropdownOption]()
					{
						const FText DefaultLabel = NSLOCTEXT("T66.Settings.Fallback", "DefaultOutputOptionFlat", "Default");
						return SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								MakeDropdownOption(
									FName(TEXT("SettingsAudio.Rows.OutputDevice.Option.Default")),
									DefaultLabel,
									[PS]()
									{
										if (PS)
										{
											PS->SetOutputDeviceId(FString());
										}
									},
									!PS || PS->GetOutputDeviceId().IsEmpty())
							];
					},
					true,
					1090.f,
					64.f,
					22,
					ChildTag(RowTag, TEXT("Dropdown")))
			];
		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(34.f, 0.f, 32.f, 0.f), Row, nullptr, RowTag);
	};

	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
		.Anchors(FAnchors(X, Y, X, Y))
		.Alignment(FVector2D::ZeroVector)
		.Offset(FMargin(0.f, 0.f, W * CanvasW, H * CanvasH))
		[
			Widget
		];
	};

	AddN(0.000f, 0.000f, 1.000f, 1.000f,
		FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black),
			DTag(TEXT("SettingsAudio.Background")),
			TEXT("Background"),
			ET66FlatState::Default));

	AddN(0.000f, 0.095f, 1.000f, 0.905f, MakeMetadataRegion(DTag(TEXT("SettingsAudio.Root")), TEXT("Root")));
	AddN(0.003f, 0.094f, 0.994f, 0.079f, MakeMetadataRegion(DTag(TEXT("SettingsAudio.SettingsTabs")), TEXT("ToggleGroup.SettingsTabs")));

	AddN(0.003f, 0.094f, 0.119f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsAudio.SettingsTabs.GameplayButton")), ET66SettingsTab::Gameplay, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabGameplayFlatAudio", "GAMEPLAY"), 0.119f * CanvasW));
	AddN(0.129f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsAudio.SettingsTabs.GraphicsButton")), ET66SettingsTab::Graphics, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabGraphicsFlatAudio", "GRAPHICS"), 0.118f * CanvasW));
	AddN(0.253f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsAudio.SettingsTabs.ControlsButton")), ET66SettingsTab::Controls, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabControlsFlatAudio", "CONTROLS"), 0.118f * CanvasW));
	AddN(0.379f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsAudio.SettingsTabs.HUDButton")), ET66SettingsTab::HUD, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabHUDFlatAudio", "HUD"), 0.118f * CanvasW));
	AddN(0.503f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsAudio.SettingsTabs.MediaViewerButton")), ET66SettingsTab::MediaViewer, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabMediaViewerFlatAudio", "MEDIA VIEWER"), 0.118f * CanvasW, 18));
	AddN(0.628f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsAudio.SettingsTabs.AudioButton")), ET66SettingsTab::Audio, ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "TabAudioFlatAudio", "AUDIO"), 0.118f * CanvasW));
	AddN(0.754f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsAudio.SettingsTabs.CrashingButton")), ET66SettingsTab::Crashing, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabCrashingFlatAudio", "CRASHING"), 0.118f * CanvasW, 20));
	AddN(0.879f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsAudio.SettingsTabs.RetroFXButton")), ET66SettingsTab::RetroFX, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabRetroFXFlatAudio", "RETRO FX"), 0.118f * CanvasW, 20));

	AddN(0.002f, 0.193f, 0.978f, 0.108f,
		MakeSliderRow(DTag(TEXT("SettingsAudio.Rows.MasterVolume")), Loc ? Loc->GetText_MasterVolume() : NSLOCTEXT("T66.Settings.Fallback", "Master Volume Flat", "Master Volume"), [PS]() { return PS ? PS->GetMasterVolume() : 0.8f; }, [PS](float Value) { if (PS) PS->SetMasterVolume(Value); }));
	AddN(0.002f, 0.313f, 0.978f, 0.108f,
		MakeSliderRow(DTag(TEXT("SettingsAudio.Rows.MusicVolume")), Loc ? Loc->GetText_MusicVolume() : NSLOCTEXT("T66.Settings.Fallback", "Music Volume Flat", "Music Volume"), [PS]() { return PS ? PS->GetMusicVolume() : 0.6f; }, [PS](float Value) { if (PS) PS->SetMusicVolume(Value); }));
	AddN(0.002f, 0.433f, 0.978f, 0.108f,
		MakeSliderRow(DTag(TEXT("SettingsAudio.Rows.SFXVolume")), Loc ? Loc->GetText_SfxVolume() : NSLOCTEXT("T66.Settings.Fallback", "SFX Volume Flat", "SFX Volume"), [PS]() { return PS ? PS->GetSfxVolume() : 0.8f; }, [PS](float Value) { if (PS) PS->SetSfxVolume(Value); }));
	AddN(0.002f, 0.567f, 0.978f, 0.092f,
		MakeToggleRow(DTag(TEXT("SettingsAudio.Rows.MuteWhenUnfocused")), Loc ? Loc->GetText_MuteWhenUnfocused() : NSLOCTEXT("T66.Settings.Fallback", "Mute when unfocused Flat", "Mute when unfocused"), [PS]() { return PS ? PS->GetMuteWhenUnfocused() : false; }, [PS]() { if (PS) PS->SetMuteWhenUnfocused(!PS->GetMuteWhenUnfocused()); }));
	AddN(0.002f, 0.672f, 0.978f, 0.120f,
		MakeOutputRow(DTag(TEXT("SettingsAudio.Rows.OutputDevice"))));
	AddN(0.002f, 0.802f, 0.978f, 0.092f,
		MakeToggleRow(DTag(TEXT("SettingsAudio.Rows.SubtitlesAlwaysOn")), Loc ? Loc->GetText_SubtitlesAlwaysOn() : NSLOCTEXT("T66.Settings.Fallback", "Subtitles always on Flat", "Subtitles: always on"), [PS]() { return PS ? PS->GetSubtitlesAlwaysOn() : false; }, [PS]() { if (PS) PS->SetSubtitlesAlwaysOn(!PS->GetSubtitlesAlwaysOn()); }));

	return SNew(SScaleBox)
		.Stretch(EStretch::ScaleToFit)
		.StretchDirection(EStretchDirection::Both)
		[
			SNew(SBox)
			.WidthOverride(CanvasW)
			.HeightOverride(CanvasH)
			[
				Canvas
			]
		];
}

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


