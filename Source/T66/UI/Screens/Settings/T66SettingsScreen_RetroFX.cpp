// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RetroFXUI, Log, All);

using namespace T66SettingsScreenPrivate;
TSharedRef<SWidget> UT66SettingsScreen::BuildRetroFXTab()
{
	InitializeRetroFXFromUserSettingsIfNeeded();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	auto MakeSectionHeader = [](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(SettingsBoldFont(22))
			.ColorAndOpacity(FT66Style::TextMuted());
	};

	auto MakeRetroButton = [](const FText& Label, TFunction<bool()> IsSelected, FOnClicked OnClicked, float MinWidth = 88.0f) -> TSharedRef<SWidget>
	{
		return MakeSelectableSettingsButton(
			FT66ButtonParams(Label, MoveTemp(OnClicked), ET66ButtonType::Neutral)
			.SetMinWidth(MinWidth)
			.SetFontSize(18)
			.SetPadding(FMargin(12.0f, 6.0f))
			.SetTextColor(GetRetroButtonText()),
			MoveTemp(IsSelected),
			FSlateColor(GetRetroButtonSelectedBackground()),
			FSlateColor(GetRetroButtonBackground()));
	};

	auto MakeNumericRow = [](const FText& Label, const FText& Description, TFunction<float()> GetPercent, TFunction<void(float)> SetPercent) -> TSharedRef<SWidget>
	{
		return MakeSettingsPercentEntryRow(
			Label,
			Description,
			MoveTemp(GetPercent),
			MoveTemp(SetPercent),
			NSLOCTEXT("T66.Settings", "RetroFXPercentHint", "0-100"),
			NSLOCTEXT("T66.Settings", "RetroFXNumericHint", "Enter a value from 0 to 100."));
	};

	FSettingsBoolToggleStyle RetroToggleStyle;
	RetroToggleStyle.ButtonMinWidth = 100.f;
	RetroToggleStyle.ButtonFontSize = 18;
	RetroToggleStyle.ButtonPadding = FMargin(12.f, 6.f);
	RetroToggleStyle.OnSelectedColor = FSlateColor(GetRetroButtonSelectedBackground());
	RetroToggleStyle.OffSelectedColor = FSlateColor(GetRetroButtonSelectedBackground());
	RetroToggleStyle.UnselectedColor = FSlateColor(GetRetroButtonBackground());
	RetroToggleStyle.TextColor = FSlateColor(GetRetroButtonText());

	auto MakeToggleRow = [Loc, RetroToggleStyle](const FText& Label, const FText& Description, TFunction<bool()> GetValue, TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		return MakeSettingsToggleRow(Loc, Label, MoveTemp(GetValue), MoveTemp(SetValue), Description, RetroToggleStyle);
	};

	auto MakeApplyButton = [this, &MakeRetroButton]() -> TSharedRef<SWidget>
	{
		return MakeRetroButton(
			NSLOCTEXT("T66.Settings", "RetroFXApply", "APPLY"),
			[]() { return false; },
			FOnClicked::CreateLambda([this]()
			{
				return HandleApplyRetroFXClicked();
			}),
			180.0f);
	};

	auto MakeActionRow = [this, &MakeRetroButton, &MakeApplyButton](bool bIncludeReset) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				bIncludeReset
					? StaticCastSharedRef<SWidget>(MakeRetroButton(
						NSLOCTEXT("T66.Settings", "RetroFXResetDefaults", "RESET TO DEFAULTS"),
						[]() { return false; },
						FOnClicked::CreateLambda([this]()
						{
							return HandleResetRetroFXClicked();
						}),
						220.0f))
					: SNullWidget::NullWidget
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(bIncludeReset ? FMargin(8.0f, 0.0f, 0.0f, 0.0f) : FMargin(0.0f))
			[
				MakeApplyButton()
			];
	};

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Settings", "RetroFXHeader", "Retro FX"))
				.Font(SettingsBoldFont(24))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 14.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Settings", "RetroFXBodyExpanded", "This panel drives the currently wired UE5RFX screen-space stack and the safe retro geometry swap path for T66's world and character unlit materials. Scalar settings use direct 0-100 numeric input, while binary settings use ON and OFF toggles. The master toggle disables the entire Retro FX stack without erasing your saved values. PS1 sub-settings require PS1 Master Blend above 0 to show on screen."))
				.Font(SettingsRegularFont(16))
				.ColorAndOpacity(GetSettingsPageMuted())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					NSLOCTEXT("T66.Settings", "RetroFXMasterEnableLabel", "Retro FX Master Enable"),
					NSLOCTEXT("T66.Settings", "RetroFXMasterEnableBody", "Turns the entire Retro FX stack on or off without changing the individual values below."),
					[this]() { return PendingRetroFXSettings.bEnableRetroFXMaster; },
					[this](bool bValue) { PendingRetroFXSettings.bEnableRetroFXMaster = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(bRetroFXDirty ? NSLOCTEXT("T66.Settings", "RetroFXPendingDirty", "Pending changes have not been applied yet.") : NSLOCTEXT("T66.Settings", "RetroFXPendingClean", "Pending values match the saved Retro FX profile."))
				.Font(SettingsRegularFont(16))
				.ColorAndOpacity(bRetroFXDirty ? FT66Style::ButtonPrimary() : GetSettingsPageMuted())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 0.0f, 0.0f, 18.0f)
			[
				MakeActionRow(false)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionPS1", "PS1 Stack"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1BlendLabel", "PS1 Master Blend"), NSLOCTEXT("T66.Settings", "RetroFXPs1BlendBody", "Overall blend weight for the UE5RFX PS1 post-process stack."), [this]() { return PendingRetroFXSettings.PS1BlendPercent; }, [this](float V) { PendingRetroFXSettings.PS1BlendPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1DitherLabel", "PS1 Dithering"), NSLOCTEXT("T66.Settings", "RetroFXPs1DitherBody", "Controls the strength of the imported UE5RFX PS1 dithering pattern."), [this]() { return PendingRetroFXSettings.PS1DitheringPercent; }, [this](float V) { PendingRetroFXSettings.PS1DitheringPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1BayerLabel", "PS1 Bayer Dithering"), NSLOCTEXT("T66.Settings", "RetroFXPs1BayerBody", "Switches the PS1 post-process stack to the imported Bayer dithering path."), [this]() { return PendingRetroFXSettings.PS1BayerDitheringPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.PS1BayerDitheringPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1ColorLUTLabel", "PS1 Color LUT"), NSLOCTEXT("T66.Settings", "RetroFXPs1ColorLUTBody", "Enables or disables the imported UE5RFX color LUT stage. Color Boost only shows when this is enabled."), [this]() { return PendingRetroFXSettings.PS1ColorLUTPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.PS1ColorLUTPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1ColorBoostLabel", "PS1 Color Boost"), NSLOCTEXT("T66.Settings", "RetroFXPs1ColorBoostBody", "Strength of the imported UE5RFX PS1 LUT color boost."), [this]() { return PendingRetroFXSettings.PS1ColorBoostPercent; }, [this](float V) { PendingRetroFXSettings.PS1ColorBoostPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionWorldGeometry", "World Geometry"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXWorldGeometryEnableLabel", "World Geometry Enable"), NSLOCTEXT("T66.Settings", "RetroFXWorldGeometryEnableBody", "Turns the safe runtime retro-geometry swap on or off for world and environment materials that inherit from T66's shared unlit masters."), [this]() { return PendingRetroFXSettings.bEnableWorldGeometry; }, [this](bool bValue) { PendingRetroFXSettings.bEnableWorldGeometry = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapLabel", "World Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapBody", "Strength for geometry snapping on world and environment materials patched into the retro stack."), [this]() { return PendingRetroFXSettings.WorldVertexSnapPercent; }, [this](float V) { PendingRetroFXSettings.WorldVertexSnapPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapResLabel", "World Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapResBody", "Higher values lower the target snap resolution, making the world wobble and step more aggressively."), [this]() { return PendingRetroFXSettings.WorldVertexSnapResolutionPercent; }, [this](float V) { PendingRetroFXSettings.WorldVertexSnapResolutionPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexNoiseLabel", "World Vertex Noise"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexNoiseBody", "Adds extra world-position noise on top of snapping for rougher retro surfaces."), [this]() { return PendingRetroFXSettings.WorldVertexNoisePercent; }, [this](float V) { PendingRetroFXSettings.WorldVertexNoisePercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineBlendLabel", "World Affine Blend"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineBlendBody", "Blends world UVs into the affine mapping path. 0 keeps stable UVs, 100 fully commits to the retro perspective error."), [this]() { return PendingRetroFXSettings.WorldAffineBlendPercent; }, [this](float V) { PendingRetroFXSettings.WorldAffineBlendPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD1Label", "World Affine Distance 1"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD1Body", "Near-distance threshold for world affine mapping."), [this]() { return PendingRetroFXSettings.WorldAffineDistance1Percent; }, [this](float V) { PendingRetroFXSettings.WorldAffineDistance1Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD2Label", "World Affine Distance 2"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD2Body", "Mid-distance threshold for world affine mapping."), [this]() { return PendingRetroFXSettings.WorldAffineDistance2Percent; }, [this](float V) { PendingRetroFXSettings.WorldAffineDistance2Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD3Label", "World Affine Distance 3"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD3Body", "Far-distance threshold for world affine mapping."), [this]() { return PendingRetroFXSettings.WorldAffineDistance3Percent; }, [this](float V) { PendingRetroFXSettings.WorldAffineDistance3Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionCharacterGeometry", "Character Geometry"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterGeometryEnableLabel", "Character Geometry Enable"), NSLOCTEXT("T66.Settings", "RetroFXCharacterGeometryEnableBody", "Turns the safe runtime retro-geometry swap on or off for character-facing materials that inherit from T66's shared unlit masters."), [this]() { return PendingRetroFXSettings.bEnableCharacterGeometry; }, [this](bool bValue) { PendingRetroFXSettings.bEnableCharacterGeometry = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapLabel", "Character Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapBody", "Strength for geometry snapping on character-facing unlit materials."), [this]() { return PendingRetroFXSettings.CharacterVertexSnapPercent; }, [this](float V) { PendingRetroFXSettings.CharacterVertexSnapPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapResLabel", "Character Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapResBody", "Higher values lower the target snap resolution, making hero and enemy geometry wobble more aggressively."), [this]() { return PendingRetroFXSettings.CharacterVertexSnapResolutionPercent; }, [this](float V) { PendingRetroFXSettings.CharacterVertexSnapResolutionPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexNoiseLabel", "Character Vertex Noise"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexNoiseBody", "Adds extra character-space noise on top of snapping. Keep it lower if you want readable combat silhouettes."), [this]() { return PendingRetroFXSettings.CharacterVertexNoisePercent; }, [this](float V) { PendingRetroFXSettings.CharacterVertexNoisePercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineBlendLabel", "Character Affine Blend"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineBlendBody", "Blends character UVs into the affine mapping path. Push this carefully to avoid readability loss in combat."), [this]() { return PendingRetroFXSettings.CharacterAffineBlendPercent; }, [this](float V) { PendingRetroFXSettings.CharacterAffineBlendPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD1Label", "Character Affine Distance 1"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD1Body", "Near-distance threshold for character affine mapping."), [this]() { return PendingRetroFXSettings.CharacterAffineDistance1Percent; }, [this](float V) { PendingRetroFXSettings.CharacterAffineDistance1Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD2Label", "Character Affine Distance 2"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD2Body", "Mid-distance threshold for character affine mapping."), [this]() { return PendingRetroFXSettings.CharacterAffineDistance2Percent; }, [this](float V) { PendingRetroFXSettings.CharacterAffineDistance2Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD3Label", "Character Affine Distance 3"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD3Body", "Far-distance threshold for character affine mapping."), [this]() { return PendingRetroFXSettings.CharacterAffineDistance3Percent; }, [this](float V) { PendingRetroFXSettings.CharacterAffineDistance3Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionN64", "N64 Blur"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXN64BlendLabel", "N64 Blur Blend"), NSLOCTEXT("T66.Settings", "RetroFXN64BlendBody", "Overall blend weight for the UE5RFX N64 blur material."), [this]() { return PendingRetroFXSettings.N64BlurBlendPercent; }, [this](float V) { PendingRetroFXSettings.N64BlurBlendPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXN64StepsLabel", "N64 Blur Steps"), NSLOCTEXT("T66.Settings", "RetroFXN64StepsBody", "Higher values increase the blur sample count in the N64 pass."), [this]() { return PendingRetroFXSettings.N64BlurStepsPercent; }, [this](float V) { PendingRetroFXSettings.N64BlurStepsPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXN64LowResLabel", "N64 Low Fake Resolution"), NSLOCTEXT("T66.Settings", "RetroFXN64LowResBody", "Enables or disables the low-fake-resolution path used by the N64 blur pass."), [this]() { return PendingRetroFXSettings.N64LowFakeResolutionPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.N64LowFakeResolutionPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXN64ReplaceTMLabel", "N64 Replace Tonemapper"), NSLOCTEXT("T66.Settings", "RetroFXN64ReplaceTMBody", "Switch between the standard N64 blur material and the replace-tonemapper variant."), [this]() { return PendingRetroFXSettings.bUseUE5RFXN64BlurReplaceTonemapper; }, [this](bool bValue) { PendingRetroFXSettings.bUseUE5RFXN64BlurReplaceTonemapper = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionChromatic", "Chromatic Aberration And Pixelation"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticStrengthLabel", "Chromatic Aberration Strength"), NSLOCTEXT("T66.Settings", "RetroFXChromaticStrengthBody", "Controls the radial RGB split strength for the custom chromatic-aberration post-process pass."), [this]() { return PendingRetroFXSettings.ChromaticAberrationPercent; }, [this](float V) { PendingRetroFXSettings.ChromaticAberrationPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticDistortionLabel", "Distortion Strength"), NSLOCTEXT("T66.Settings", "RetroFXChromaticDistortionBody", "Controls the radial screen distortion strength used by the chromatic-aberration pass."), [this]() { return PendingRetroFXSettings.ChromaticDistortionPercent; }, [this](float V) { PendingRetroFXSettings.ChromaticDistortionPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticInvertLabel", "Invert Distortion"), NSLOCTEXT("T66.Settings", "RetroFXChromaticInvertBody", "Flips the radial distortion direction used by the chromatic-aberration pass."), [this]() { return PendingRetroFXSettings.bInvertChromaticDistortion; }, [this](bool bValue) { PendingRetroFXSettings.bInvertChromaticDistortion = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 18.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPixelationLabel", "T66 Pixelation"), NSLOCTEXT("T66.Settings", "RetroFXPixelationBody", "Drives the existing T66 pixelation subsystem from Off to full strength."), [this]() { return PendingRetroFXSettings.T66PixelationPercent; }, [this](float V) { PendingRetroFXSettings.T66PixelationPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionResolution", "Shared Resolution"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXRealLowResLabel", "Real Low Resolution"), NSLOCTEXT("T66.Settings", "RetroFXRealLowResBody", "Lowers the actual runtime screen percentage for a stronger low-resolution look. This is the most aggressive shared-resolution mode."), [this]() { return PendingRetroFXSettings.bUseRealLowResolution; }, [this](bool bValue) { PendingRetroFXSettings.bUseRealLowResolution = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXResSizeLabel", "Fake Resolution Size Switch"), NSLOCTEXT("T66.Settings", "RetroFXResSizeBody", "Enables or disables the fake screen-size resolution switch."), [this]() { return PendingRetroFXSettings.FakeResolutionSwitchSizePercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.FakeResolutionSwitchSizePercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXResUVLabel", "Fake Resolution UV Switch"), NSLOCTEXT("T66.Settings", "RetroFXResUVBody", "Enables or disables the fake UV resolution switch."), [this]() { return PendingRetroFXSettings.FakeResolutionSwitchUVPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.FakeResolutionSwitchUVPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXTargetResLabel", "Target Resolution Height"), NSLOCTEXT("T66.Settings", "RetroFXTargetResBody", "Higher values drive the target height lower, which makes the scene feel more aggressively low-res."), [this]() { return PendingRetroFXSettings.TargetResolutionHeightPercent; }, [this](float V) { PendingRetroFXSettings.TargetResolutionHeightPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionFog", "PS1 Fog"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogLabel", "PS1 Fog Enable"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogBody", "Enables or disables the visible PS1 fog contribution by gating its density."), [this]() { return PendingRetroFXSettings.PS1FogPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.PS1FogPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1SceneDepthFogLabel", "PS1 Scene Depth Fog"), NSLOCTEXT("T66.Settings", "RetroFXPs1SceneDepthFogBody", "Switches the fog calculation to the imported scene-depth-based PS1 fog path."), [this]() { return PendingRetroFXSettings.PS1SceneDepthFogPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.PS1SceneDepthFogPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogDensityLabel", "PS1 Fog Density"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogDensityBody", "Controls how thick the UE5RFX fog becomes once fog is enabled."), [this]() { return PendingRetroFXSettings.PS1FogDensityPercent; }, [this](float V) { PendingRetroFXSettings.PS1FogDensityPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogStartLabel", "PS1 Fog Start Distance"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogStartBody", "Higher values pull the fog closer to the camera for a heavier retro atmosphere."), [this]() { return PendingRetroFXSettings.PS1FogStartDistancePercent; }, [this](float V) { PendingRetroFXSettings.PS1FogStartDistancePercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 18.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogFalloffLabel", "PS1 Fog Falloff"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogFalloffBody", "Higher values tighten the fog falloff distance for a denser wall of haze."), [this]() { return PendingRetroFXSettings.PS1FogFallOffDistancePercent; }, [this](float V) { PendingRetroFXSettings.PS1FogFallOffDistancePercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
			[
				MakeActionRow(true)
			]
		];
}
FReply UT66SettingsScreen::HandleApplyRetroFXClicked()
{
	UE_LOG(LogT66RetroFXUI, Log, TEXT("HandleApplyRetroFXClicked: dirty=%s world=%s"), bRetroFXDirty ? TEXT("true") : TEXT("false"), *GetNameSafe(GetWorld()));
	ApplyPendingRetroFX();
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleResetRetroFXClicked()
{
	ResetPendingRetroFXToDefaults();
	return FReply::Handled();
}

void UT66SettingsScreen::InitializeRetroFXFromUserSettingsIfNeeded()
{
	if (bRetroFXInitialized) return;
	bRetroFXInitialized = true;

	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PendingRetroFXSettings = PS->GetRetroFXSettings();
	}
	else
	{
		PendingRetroFXSettings = FT66RetroFXSettings();
	}

	bRetroFXDirty = false;
}

void UT66SettingsScreen::ApplyPendingRetroFX()
{
	InitializeRetroFXFromUserSettingsIfNeeded();

	UE_LOG(LogT66RetroFXUI, Log,
		TEXT("ApplyPendingRetroFX: begin dirty=%s world=%s MasterEnabled=%s PS1Blend=%.2f PS1Dither=%.2f PS1Bayer=%.2f PS1ColorLUT=%.2f PS1ColorBoost=%.2f ChromaticStrength=%.2f DistortionStrength=%.2f InvertDistortion=%s PS1FogEnable=%.2f PS1SceneDepthFog=%.2f PS1FogDensity=%.2f PS1FogStart=%.2f PS1FogFalloff=%.2f RealLowRes=%s FakeSize=%.2f FakeUV=%.2f TargetRes=%.2f"),
		bRetroFXDirty ? TEXT("true") : TEXT("false"),
		*GetNameSafe(GetWorld()),
		PendingRetroFXSettings.bEnableRetroFXMaster ? TEXT("true") : TEXT("false"),
		PendingRetroFXSettings.PS1BlendPercent,
		PendingRetroFXSettings.PS1DitheringPercent,
		PendingRetroFXSettings.PS1BayerDitheringPercent,
		PendingRetroFXSettings.PS1ColorLUTPercent,
		PendingRetroFXSettings.PS1ColorBoostPercent,
		PendingRetroFXSettings.ChromaticAberrationPercent,
		PendingRetroFXSettings.ChromaticDistortionPercent,
		PendingRetroFXSettings.bInvertChromaticDistortion ? TEXT("true") : TEXT("false"),
		PendingRetroFXSettings.PS1FogPercent,
		PendingRetroFXSettings.PS1SceneDepthFogPercent,
		PendingRetroFXSettings.PS1FogDensityPercent,
		PendingRetroFXSettings.PS1FogStartDistancePercent,
		PendingRetroFXSettings.PS1FogFallOffDistancePercent,
		PendingRetroFXSettings.bUseRealLowResolution ? TEXT("true") : TEXT("false"),
		PendingRetroFXSettings.FakeResolutionSwitchSizePercent,
		PendingRetroFXSettings.FakeResolutionSwitchUVPercent,
		PendingRetroFXSettings.TargetResolutionHeightPercent);

	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		UE_LOG(LogT66RetroFXUI, Log, TEXT("ApplyPendingRetroFX: saving pending settings to UT66PlayerSettingsSubsystem"));
		PS->SetRetroFXSettings(PendingRetroFXSettings);

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
			{
				UE_LOG(LogT66RetroFXUI, Log, TEXT("ApplyPendingRetroFX: directly applying pending settings to runtime subsystem"));
				RetroFX->ApplySettings(PendingRetroFXSettings, GetWorld());
			}
			else
			{
				UE_LOG(LogT66RetroFXUI, Warning, TEXT("ApplyPendingRetroFX: Retro FX subsystem was null"));
			}
		}
		else
		{
			UE_LOG(LogT66RetroFXUI, Warning, TEXT("ApplyPendingRetroFX: GameInstance was null"));
		}

		bRetroFXDirty = false;
		UE_LOG(LogT66RetroFXUI, Log, TEXT("ApplyPendingRetroFX: completed successfully"));
	}
	else
	{
		UE_LOG(LogT66RetroFXUI, Warning, TEXT("ApplyPendingRetroFX: Player settings subsystem was null"));
	}
}

void UT66SettingsScreen::ResetPendingRetroFXToDefaults()
{
	InitializeRetroFXFromUserSettingsIfNeeded();
	PendingRetroFXSettings = FT66RetroFXSettings();
	bRetroFXDirty = true;
}

