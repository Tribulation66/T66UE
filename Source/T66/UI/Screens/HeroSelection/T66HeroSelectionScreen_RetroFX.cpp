// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"
#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66HeroSelectionRetroFX, Log, All);

using namespace T66HeroSelectionPrivate;

namespace
{
	constexpr float T66InlineRetroFXApplyWidth = 118.f;

	TSharedRef<SWidget> MakeInlineRetroFXSectionHeader(const FText& Text)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(T66SettingsScreenPrivate::SettingsBoldFont(18))
			.ColorAndOpacity(T66SettingsScreenPrivate::GetSettingsPageMuted())
			.AutoWrapText(true);
	}

	TSharedRef<SWidget> MakeInlineRetroFXActionButton(const FText& Label, FOnClicked OnClicked, float MinWidth = 96.f)
	{
		return MakeHeroSelectionButton(
			FT66ButtonParams(Label, MoveTemp(OnClicked), ET66ButtonType::Neutral)
			.SetMinWidth(MinWidth)
			.SetHeight(38.f)
			.SetFontSize(16)
			.SetPadding(FMargin(8.f, 5.f)));
	}

	TSharedRef<SWidget> MakeInlineRetroFXPercentRow(
		const FText& Label,
		const FText& Description,
		TFunction<float()> GetPercent,
		TFunction<void(float)> SetPercent)
	{
		auto GetSnappedPercent = [GetPercent]()
		{
			return FMath::Clamp(FMath::RoundToInt(GetPercent ? GetPercent() : 0.f), 0, 100);
		};

		auto CommitSliderValue = [SetPercent](float NormalizedValue)
		{
			const int32 SnappedPercent = FMath::Clamp(FMath::RoundToInt(NormalizedValue * 100.0f), 0, 100);
			if (SetPercent)
			{
				SetPercent(static_cast<float>(SnappedPercent));
			}
		};

		return MakeHeroSelectionParchmentRowShell(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(T66SettingsScreenPrivate::SettingsRegularFont(18))
					.ColorAndOpacity(GetHeroSelectionParchmentText())
					.AutoWrapText(true)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.MinDesiredWidth(34.f)
					[
						SNew(STextBlock)
						.Text_Lambda([GetSnappedPercent]()
						{
							return FText::AsNumber(GetSnappedPercent());
						})
						.Font(T66SettingsScreenPrivate::SettingsBoldFont(20))
						.ColorAndOpacity(GetHeroSelectionParchmentText())
						.Justification(ETextJustify::Right)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Description)
				.Font(T66SettingsScreenPrivate::SettingsRegularFont(13))
				.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(SSlider)
				.Value_Lambda([GetSnappedPercent]()
				{
					return static_cast<float>(GetSnappedPercent()) / 100.0f;
				})
				.StepSize(0.01f)
				.OnValueChanged_Lambda([CommitSliderValue](float Value)
				{
					CommitSliderValue(Value);
				})
			],
			FMargin(10.f, 8.f));
	}

	TSharedRef<SWidget> MakeInlineRetroFXToggleRow(
		UT66LocalizationSubsystem* Loc,
		const FText& Label,
		const FText& Description,
		TFunction<bool()> GetValue,
		TFunction<void(bool)> SetValue)
	{
		(void)Loc;

		auto MakeToggleButton = [GetValue, SetValue](const bool bTargetValue) -> TSharedRef<SWidget>
		{
			const bool bSelected = GetValue ? GetValue() == bTargetValue : false;
			const FText LabelText = bTargetValue
				? NSLOCTEXT("T66.Settings", "SettingsToggleOn", "ON")
				: NSLOCTEXT("T66.Settings", "SettingsToggleOff", "OFF");
			return MakeHeroSelectionButton(
				FT66ButtonParams(
					LabelText,
					FOnClicked::CreateLambda([SetValue, bTargetValue]()
					{
						if (SetValue)
						{
							SetValue(bTargetValue);
						}
						return FReply::Handled();
					}),
					bSelected ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
				.SetMinWidth(76.f)
				.SetHeight(34.f)
				.SetFontSize(15)
				.SetPadding(FMargin(8.f, 5.f)));
		};

		return MakeHeroSelectionParchmentRowShell(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(T66SettingsScreenPrivate::SettingsRegularFont(18))
				.ColorAndOpacity(GetHeroSelectionParchmentText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Description)
				.Font(T66SettingsScreenPrivate::SettingsRegularFont(13))
				.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 7.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeToggleButton(true)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f, 0.f, 0.f)
				[
					MakeToggleButton(false)
				]
			],
			FMargin(10.f, 8.f));
	}
}

TSharedRef<SWidget> UT66HeroSelectionScreen::BuildInlineRetroFXPanel()
{
	InitializeInlineRetroFXFromUserSettingsIfNeeded();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	auto MakeFloatGetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<float()>
	{
		return [this, Field]() -> float
		{
			InitializeInlineRetroFXFromUserSettingsIfNeeded();
			return PendingInlineRetroFXSettings.*Field;
		};
	};

	auto MakeFloatSetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<void(float)>
	{
		return [this, Field](float Value)
		{
			InitializeInlineRetroFXFromUserSettingsIfNeeded();
			PendingInlineRetroFXSettings.*Field = FMath::Clamp(Value, 0.f, 100.f);
			bInlineRetroFXDirty = true;
		};
	};

	auto MakeBoolGetter = [this](bool FT66RetroFXSettings::* Field) -> TFunction<bool()>
	{
		return [this, Field]() -> bool
		{
			InitializeInlineRetroFXFromUserSettingsIfNeeded();
			return PendingInlineRetroFXSettings.*Field;
		};
	};

	auto MakeBoolSetter = [this](bool FT66RetroFXSettings::* Field) -> TFunction<void(bool)>
	{
		return [this, Field](bool bValue)
		{
			InitializeInlineRetroFXFromUserSettingsIfNeeded();
			PendingInlineRetroFXSettings.*Field = bValue;
			bInlineRetroFXDirty = true;
		};
	};

	auto MakePercentToggleGetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<bool()>
	{
		return [this, Field]() -> bool
		{
			InitializeInlineRetroFXFromUserSettingsIfNeeded();
			return PendingInlineRetroFXSettings.*Field > KINDA_SMALL_NUMBER;
		};
	};

	auto MakePercentToggleSetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<void(bool)>
	{
		return [this, Field](bool bValue)
		{
			InitializeInlineRetroFXFromUserSettingsIfNeeded();
			PendingInlineRetroFXSettings.*Field = bValue ? 100.f : 0.f;
			bInlineRetroFXDirty = true;
		};
	};

	auto MakeNumericRow = [](const FText& Label, const FText& Description, TFunction<float()> GetPercent, TFunction<void(float)> SetPercent) -> TSharedRef<SWidget>
	{
		return MakeInlineRetroFXPercentRow(Label, Description, MoveTemp(GetPercent), MoveTemp(SetPercent));
	};

	auto MakeToggleRow = [Loc](const FText& Label, const FText& Description, TFunction<bool()> GetValue, TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		return MakeInlineRetroFXToggleRow(Loc, Label, Description, MoveTemp(GetValue), MoveTemp(SetValue));
	};

	auto AddRow = [](const TSharedRef<SVerticalBox>& Box, const TSharedRef<SWidget>& Row, const float BottomPadding = 7.f)
	{
		Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, BottomPadding)
		[
			Row
		];
	};

	const TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);

	AddRow(Rows,
		MakeToggleRow(
			NSLOCTEXT("T66.Settings", "RetroFXMasterEnableLabel", "Retro FX Master Enable"),
			NSLOCTEXT("T66.Settings", "RetroFXMasterEnableBody", "Turns the entire Retro FX stack on or off without changing the individual values below."),
			MakeBoolGetter(&FT66RetroFXSettings::bEnableRetroFXMaster),
			MakeBoolSetter(&FT66RetroFXSettings::bEnableRetroFXMaster)));

	AddRow(Rows,
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return bInlineRetroFXDirty
					? NSLOCTEXT("T66.Settings", "RetroFXPendingDirty", "Pending changes have not been applied yet.")
					: NSLOCTEXT("T66.Settings", "RetroFXPendingClean", "Pending values match the saved Retro FX profile.");
			})
			.Font(T66SettingsScreenPrivate::SettingsRegularFont(13))
			.ColorAndOpacity_Lambda([this]()
			{
				return FSlateColor(bInlineRetroFXDirty ? FT66Style::ButtonPrimary() : T66SettingsScreenPrivate::GetSettingsPageMuted());
			})
			.AutoWrapText(true)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
		[
			MakeInlineRetroFXActionButton(
				NSLOCTEXT("T66.Settings", "RetroFXApply", "APPLY"),
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleApplyInlineRetroFXClicked),
				T66InlineRetroFXApplyWidth)
		],
		12.f);

	AddRow(Rows, MakeInlineRetroFXSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionPS1", "PS1 Stack")), 6.f);
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1BlendLabel", "PS1 Master Blend"), NSLOCTEXT("T66.Settings", "RetroFXPs1BlendBody", "Overall blend weight for the UE5RFX PS1 post-process stack."), MakeFloatGetter(&FT66RetroFXSettings::PS1BlendPercent), MakeFloatSetter(&FT66RetroFXSettings::PS1BlendPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1DitherLabel", "PS1 Dithering"), NSLOCTEXT("T66.Settings", "RetroFXPs1DitherBody", "Controls the strength of the imported UE5RFX PS1 dithering pattern."), MakeFloatGetter(&FT66RetroFXSettings::PS1DitheringPercent), MakeFloatSetter(&FT66RetroFXSettings::PS1DitheringPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1BayerLabel", "PS1 Bayer Dithering"), NSLOCTEXT("T66.Settings", "RetroFXPs1BayerBody", "Switches the PS1 post-process stack to the imported Bayer dithering path."), MakePercentToggleGetter(&FT66RetroFXSettings::PS1BayerDitheringPercent), MakePercentToggleSetter(&FT66RetroFXSettings::PS1BayerDitheringPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1ColorLUTLabel", "PS1 Color LUT"), NSLOCTEXT("T66.Settings", "RetroFXPs1ColorLUTBody", "Enables or disables the imported UE5RFX color LUT stage. Color Boost only shows when this is enabled."), MakePercentToggleGetter(&FT66RetroFXSettings::PS1ColorLUTPercent), MakePercentToggleSetter(&FT66RetroFXSettings::PS1ColorLUTPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1ColorBoostLabel", "PS1 Color Boost"), NSLOCTEXT("T66.Settings", "RetroFXPs1ColorBoostBody", "Strength of the imported UE5RFX PS1 LUT color boost."), MakeFloatGetter(&FT66RetroFXSettings::PS1ColorBoostPercent), MakeFloatSetter(&FT66RetroFXSettings::PS1ColorBoostPercent)), 12.f);

	AddRow(Rows, MakeInlineRetroFXSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionWorldGeometry", "World Geometry")), 6.f);
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXWorldGeometryEnableLabel", "World Geometry Enable"), NSLOCTEXT("T66.Settings", "RetroFXWorldGeometryEnableBody", "Turns the safe runtime retro-geometry swap on or off for world and environment materials that inherit from T66's shared unlit masters."), MakeBoolGetter(&FT66RetroFXSettings::bEnableWorldGeometry), MakeBoolSetter(&FT66RetroFXSettings::bEnableWorldGeometry)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapLabel", "World Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapBody", "Strength for geometry snapping on world and environment materials patched into the retro stack."), MakeFloatGetter(&FT66RetroFXSettings::WorldVertexSnapPercent), MakeFloatSetter(&FT66RetroFXSettings::WorldVertexSnapPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapResLabel", "World Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapResBody", "Higher values lower the target snap resolution, making the world wobble and step more aggressively."), MakeFloatGetter(&FT66RetroFXSettings::WorldVertexSnapResolutionPercent), MakeFloatSetter(&FT66RetroFXSettings::WorldVertexSnapResolutionPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexNoiseLabel", "World Vertex Noise"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexNoiseBody", "Adds extra world-position noise on top of snapping for rougher retro surfaces."), MakeFloatGetter(&FT66RetroFXSettings::WorldVertexNoisePercent), MakeFloatSetter(&FT66RetroFXSettings::WorldVertexNoisePercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineBlendLabel", "World Affine Blend"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineBlendBody", "Blends world UVs into the affine mapping path. 0 keeps stable UVs, 100 fully commits to the retro perspective error."), MakeFloatGetter(&FT66RetroFXSettings::WorldAffineBlendPercent), MakeFloatSetter(&FT66RetroFXSettings::WorldAffineBlendPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD1Label", "World Affine Distance 1"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD1Body", "Near-distance threshold for world affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::WorldAffineDistance1Percent), MakeFloatSetter(&FT66RetroFXSettings::WorldAffineDistance1Percent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD2Label", "World Affine Distance 2"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD2Body", "Mid-distance threshold for world affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::WorldAffineDistance2Percent), MakeFloatSetter(&FT66RetroFXSettings::WorldAffineDistance2Percent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD3Label", "World Affine Distance 3"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD3Body", "Far-distance threshold for world affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::WorldAffineDistance3Percent), MakeFloatSetter(&FT66RetroFXSettings::WorldAffineDistance3Percent)), 12.f);

	AddRow(Rows, MakeInlineRetroFXSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionCharacterGeometry", "Character Geometry")), 6.f);
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterGeometryEnableLabel", "Character Geometry Enable"), NSLOCTEXT("T66.Settings", "RetroFXCharacterGeometryEnableBody", "Turns the safe runtime retro-geometry swap on or off for character-facing materials that inherit from T66's shared unlit masters."), MakeBoolGetter(&FT66RetroFXSettings::bEnableCharacterGeometry), MakeBoolSetter(&FT66RetroFXSettings::bEnableCharacterGeometry)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapLabel", "Character Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapBody", "Strength for geometry snapping on character-facing unlit materials."), MakeFloatGetter(&FT66RetroFXSettings::CharacterVertexSnapPercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterVertexSnapPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapResLabel", "Character Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapResBody", "Higher values lower the target snap resolution, making hero and enemy geometry wobble more aggressively."), MakeFloatGetter(&FT66RetroFXSettings::CharacterVertexSnapResolutionPercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterVertexSnapResolutionPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexNoiseLabel", "Character Vertex Noise"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexNoiseBody", "Adds extra character-space noise on top of snapping. Keep it lower if you want readable combat silhouettes."), MakeFloatGetter(&FT66RetroFXSettings::CharacterVertexNoisePercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterVertexNoisePercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineBlendLabel", "Character Affine Blend"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineBlendBody", "Blends character UVs into the affine mapping path. Push this carefully to avoid readability loss in combat."), MakeFloatGetter(&FT66RetroFXSettings::CharacterAffineBlendPercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterAffineBlendPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD1Label", "Character Affine Distance 1"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD1Body", "Near-distance threshold for character affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::CharacterAffineDistance1Percent), MakeFloatSetter(&FT66RetroFXSettings::CharacterAffineDistance1Percent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD2Label", "Character Affine Distance 2"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD2Body", "Mid-distance threshold for character affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::CharacterAffineDistance2Percent), MakeFloatSetter(&FT66RetroFXSettings::CharacterAffineDistance2Percent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD3Label", "Character Affine Distance 3"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD3Body", "Far-distance threshold for character affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::CharacterAffineDistance3Percent), MakeFloatSetter(&FT66RetroFXSettings::CharacterAffineDistance3Percent)), 12.f);

	AddRow(Rows, MakeInlineRetroFXSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionN64", "N64 Blur")), 6.f);
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXN64BlendLabel", "N64 Blur Blend"), NSLOCTEXT("T66.Settings", "RetroFXN64BlendBody", "Overall blend weight for the UE5RFX N64 blur material."), MakeFloatGetter(&FT66RetroFXSettings::N64BlurBlendPercent), MakeFloatSetter(&FT66RetroFXSettings::N64BlurBlendPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXN64StepsLabel", "N64 Blur Steps"), NSLOCTEXT("T66.Settings", "RetroFXN64StepsBody", "Higher values increase the blur sample count in the N64 pass."), MakeFloatGetter(&FT66RetroFXSettings::N64BlurStepsPercent), MakeFloatSetter(&FT66RetroFXSettings::N64BlurStepsPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXN64LowResLabel", "N64 Low Fake Resolution"), NSLOCTEXT("T66.Settings", "RetroFXN64LowResBody", "Enables or disables the low-fake-resolution path used by the N64 blur pass."), MakePercentToggleGetter(&FT66RetroFXSettings::N64LowFakeResolutionPercent), MakePercentToggleSetter(&FT66RetroFXSettings::N64LowFakeResolutionPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXN64ReplaceTMLabel", "N64 Replace Tonemapper"), NSLOCTEXT("T66.Settings", "RetroFXN64ReplaceTMBody", "Switch between the standard N64 blur material and the replace-tonemapper variant."), MakeBoolGetter(&FT66RetroFXSettings::bUseUE5RFXN64BlurReplaceTonemapper), MakeBoolSetter(&FT66RetroFXSettings::bUseUE5RFXN64BlurReplaceTonemapper)), 12.f);

	AddRow(Rows, MakeInlineRetroFXSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionChromatic", "Chromatic Aberration And Pixelation")), 6.f);
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticStrengthLabel", "Chromatic Aberration Strength"), NSLOCTEXT("T66.Settings", "RetroFXChromaticStrengthBody", "Controls the radial RGB split strength for the custom chromatic-aberration post-process pass."), MakeFloatGetter(&FT66RetroFXSettings::ChromaticAberrationPercent), MakeFloatSetter(&FT66RetroFXSettings::ChromaticAberrationPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticDistortionLabel", "Distortion Strength"), NSLOCTEXT("T66.Settings", "RetroFXChromaticDistortionBody", "Controls the radial screen distortion strength used by the chromatic-aberration pass."), MakeFloatGetter(&FT66RetroFXSettings::ChromaticDistortionPercent), MakeFloatSetter(&FT66RetroFXSettings::ChromaticDistortionPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticInvertLabel", "Invert Distortion"), NSLOCTEXT("T66.Settings", "RetroFXChromaticInvertBody", "Flips the radial distortion direction used by the chromatic-aberration pass."), MakeBoolGetter(&FT66RetroFXSettings::bInvertChromaticDistortion), MakeBoolSetter(&FT66RetroFXSettings::bInvertChromaticDistortion)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPixelationLabel", "T66 Pixelation"), NSLOCTEXT("T66.Settings", "RetroFXPixelationBody", "Drives the existing T66 pixelation subsystem from Off to full strength."), MakeFloatGetter(&FT66RetroFXSettings::T66PixelationPercent), MakeFloatSetter(&FT66RetroFXSettings::T66PixelationPercent)), 12.f);

	AddRow(Rows, MakeInlineRetroFXSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionResolution", "Shared Resolution")), 6.f);
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXRealLowResLabel", "Real Low Resolution"), NSLOCTEXT("T66.Settings", "RetroFXRealLowResBody", "Lowers the actual runtime screen percentage for a stronger low-resolution look. This is the most aggressive shared-resolution mode."), MakeBoolGetter(&FT66RetroFXSettings::bUseRealLowResolution), MakeBoolSetter(&FT66RetroFXSettings::bUseRealLowResolution)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXResSizeLabel", "Fake Resolution Size Switch"), NSLOCTEXT("T66.Settings", "RetroFXResSizeBody", "Enables or disables the fake screen-size resolution switch."), MakePercentToggleGetter(&FT66RetroFXSettings::FakeResolutionSwitchSizePercent), MakePercentToggleSetter(&FT66RetroFXSettings::FakeResolutionSwitchSizePercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXResUVLabel", "Fake Resolution UV Switch"), NSLOCTEXT("T66.Settings", "RetroFXResUVBody", "Enables or disables the fake UV resolution switch."), MakePercentToggleGetter(&FT66RetroFXSettings::FakeResolutionSwitchUVPercent), MakePercentToggleSetter(&FT66RetroFXSettings::FakeResolutionSwitchUVPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXTargetResLabel", "Target Resolution Height"), NSLOCTEXT("T66.Settings", "RetroFXTargetResBody", "Higher values drive the target height lower, which makes the scene feel more aggressively low-res."), MakeFloatGetter(&FT66RetroFXSettings::TargetResolutionHeightPercent), MakeFloatSetter(&FT66RetroFXSettings::TargetResolutionHeightPercent)), 12.f);

	AddRow(Rows, MakeInlineRetroFXSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionFog", "PS1 Fog")), 6.f);
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogLabel", "PS1 Fog Enable"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogBody", "Enables or disables the visible PS1 fog contribution by gating its density."), MakePercentToggleGetter(&FT66RetroFXSettings::PS1FogPercent), MakePercentToggleSetter(&FT66RetroFXSettings::PS1FogPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1SceneDepthFogLabel", "PS1 Scene Depth Fog"), NSLOCTEXT("T66.Settings", "RetroFXPs1SceneDepthFogBody", "Switches the fog calculation to the imported scene-depth-based PS1 fog path."), MakePercentToggleGetter(&FT66RetroFXSettings::PS1SceneDepthFogPercent), MakePercentToggleSetter(&FT66RetroFXSettings::PS1SceneDepthFogPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogDensityLabel", "PS1 Fog Density"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogDensityBody", "Controls how thick the UE5RFX fog becomes once fog is enabled."), MakeFloatGetter(&FT66RetroFXSettings::PS1FogDensityPercent), MakeFloatSetter(&FT66RetroFXSettings::PS1FogDensityPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogStartLabel", "PS1 Fog Start Distance"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogStartBody", "Higher values pull the fog closer to the camera for a heavier retro atmosphere."), MakeFloatGetter(&FT66RetroFXSettings::PS1FogStartDistancePercent), MakeFloatSetter(&FT66RetroFXSettings::PS1FogStartDistancePercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogFalloffLabel", "PS1 Fog Falloff"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogFalloffBody", "Higher values tighten the fog falloff distance for a denser wall of haze."), MakeFloatGetter(&FT66RetroFXSettings::PS1FogFallOffDistancePercent), MakeFloatSetter(&FT66RetroFXSettings::PS1FogFallOffDistancePercent)), 12.f);

	AddRow(Rows,
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			MakeInlineRetroFXActionButton(
				NSLOCTEXT("T66.Settings", "RetroFXResetDefaults", "RESET TO DEFAULTS"),
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleResetInlineRetroFXClicked),
				0.f)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(7.f, 0.f, 0.f, 0.f)
		[
			MakeInlineRetroFXActionButton(
				NSLOCTEXT("T66.Settings", "RetroFXApply", "APPLY"),
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleApplyInlineRetroFXClicked),
				T66InlineRetroFXApplyWidth)
		],
		0.f);

	return SNew(SScrollBox)
		.ScrollBarStyle(GetHeroSelectionReferenceScrollBarStyle())
		.ScrollBarThickness(FVector2D(14.f, 14.f))
		.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
		+ SScrollBox::Slot()
		[
			Rows
		];
}

FReply UT66HeroSelectionScreen::HandleApplyInlineRetroFXClicked()
{
	ApplyPendingInlineRetroFX();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleResetInlineRetroFXClicked()
{
	ResetPendingInlineRetroFXToDefaults();
	return FReply::Handled();
}

void UT66HeroSelectionScreen::InitializeInlineRetroFXFromUserSettingsIfNeeded()
{
	if (bInlineRetroFXInitialized)
	{
		return;
	}

	bInlineRetroFXInitialized = true;

	UGameInstance* GI = GetGameInstance();
	if (UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr)
	{
		PendingInlineRetroFXSettings = PS->GetRetroFXSettings();
	}
	else
	{
		PendingInlineRetroFXSettings = FT66RetroFXSettings();
	}

	bInlineRetroFXDirty = false;
}

void UT66HeroSelectionScreen::ApplyPendingInlineRetroFX()
{
	InitializeInlineRetroFXFromUserSettingsIfNeeded();

	UE_LOG(LogT66HeroSelectionRetroFX, Log,
		TEXT("ApplyPendingInlineRetroFX: dirty=%s world=%s MasterEnabled=%s PS1Blend=%.2f Pixelation=%.2f"),
		bInlineRetroFXDirty ? TEXT("true") : TEXT("false"),
		*GetNameSafe(GetWorld()),
		PendingInlineRetroFXSettings.bEnableRetroFXMaster ? TEXT("true") : TEXT("false"),
		PendingInlineRetroFXSettings.PS1BlendPercent,
		PendingInlineRetroFXSettings.T66PixelationPercent);

	UGameInstance* GI = GetGameInstance();
	if (UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr)
	{
		PS->SetRetroFXSettings(PendingInlineRetroFXSettings);

		if (GI)
		{
			if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
			{
				RetroFX->ApplySettings(PendingInlineRetroFXSettings, GetWorld());
			}
			else
			{
				UE_LOG(LogT66HeroSelectionRetroFX, Warning, TEXT("ApplyPendingInlineRetroFX: Retro FX subsystem was null"));
			}
		}
		else
		{
			UE_LOG(LogT66HeroSelectionRetroFX, Warning, TEXT("ApplyPendingInlineRetroFX: GameInstance was null"));
		}

		bInlineRetroFXDirty = false;
	}
	else
	{
		UE_LOG(LogT66HeroSelectionRetroFX, Warning, TEXT("ApplyPendingInlineRetroFX: Player settings subsystem was null"));
	}
}

void UT66HeroSelectionScreen::ResetPendingInlineRetroFXToDefaults()
{
	InitializeInlineRetroFXFromUserSettingsIfNeeded();
	PendingInlineRetroFXSettings = FT66RetroFXSettings();
	bInlineRetroFXDirty = true;
}
