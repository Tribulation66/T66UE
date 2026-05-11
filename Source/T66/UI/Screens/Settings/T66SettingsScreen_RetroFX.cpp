// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RetroFXUI, Log, All);

using namespace T66SettingsScreenPrivate;

void UT66SettingsScreen::ConfigureAsRetroFXPreviewPopup(UT66UIManager* InUIManager)
{
	UIManager = InUIManager;
	ScreenType = ET66ScreenType::Settings;
	CurrentTab = ET66SettingsTab::RetroFX;
	bIsModal = false;
	bRetroFXPreviewPopup = true;
	bRetroFXPreviewMode = true;
	bRetroFXInitialized = false;
	bRetroFXDirty = false;
}

TSharedRef<SWidget> UT66SettingsScreen::BuildRetroFXTab()
{
	InitializeRetroFXFromUserSettingsIfNeeded();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	auto MakeSectionHeader = [](const FText& Text) -> TSharedRef<SWidget>
	{
		return MakeSettingsSectionHeader(Text, 22);
	};

	auto MakeRetroButton = [](const FText& Label, TFunction<bool()> IsSelected, FOnClicked OnClicked, float MinWidth = 88.0f) -> TSharedRef<SWidget>
	{
		return MakeSelectableSettingsButton(
			FT66ButtonParams(Label, MoveTemp(OnClicked), ET66ButtonType::Neutral)
			.SetMinWidth(MinWidth)
			.SetHeight(40.0f)
			.SetFontSize(18)
			.SetPadding(FMargin(12.0f, 6.0f))
			.SetTextColor(GetRetroButtonText()),
			MoveTemp(IsSelected),
			FSlateColor(GetRetroButtonSelectedBackground()),
			FSlateColor(GetRetroButtonBackground()));
	};

	auto MakeNumericRow = [](const FText& Label, const FText& Description, TFunction<float()> GetPercent, TFunction<void(float)> SetPercent) -> TSharedRef<SWidget>
	{
		return MakeSettingsPercentSliderRow(
			Label,
			Description,
			MoveTemp(GetPercent),
			MoveTemp(SetPercent),
			NSLOCTEXT("T66.Settings", "RetroFXSliderHint", "Slide from 0 to 100."));
	};

	FSettingsBoolToggleStyle RetroToggleStyle;
	RetroToggleStyle.ButtonMinWidth = 100.0f;
	RetroToggleStyle.ButtonFontSize = 18;
	RetroToggleStyle.ButtonPadding = FMargin(12.0f, 6.0f);
	RetroToggleStyle.OnSelectedColor = FSlateColor(GetRetroButtonSelectedBackground());
	RetroToggleStyle.OffSelectedColor = FSlateColor(GetRetroButtonSelectedBackground());
	RetroToggleStyle.UnselectedColor = FSlateColor(GetRetroButtonBackground());
	RetroToggleStyle.TextColor = FSlateColor(GetRetroButtonText());

	auto MakeToggleRow = [Loc, RetroToggleStyle](const FText& Label, const FText& Description, TFunction<bool()> GetValue, TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		return MakeSettingsToggleRow(Loc, Label, MoveTemp(GetValue), MoveTemp(SetValue), Description, RetroToggleStyle);
	};

	auto MakeFloatGetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<float()>
	{
		return [this, Field]() -> float
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			return PendingRetroFXSettings.*Field;
		};
	};

	auto MakeFloatSetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<void(float)>
	{
		return [this, Field](float Value)
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			PendingRetroFXSettings.*Field = FMath::Clamp(Value, 0.0f, 100.0f);
			MarkRetroFXEdited();
		};
	};

	auto MakeBoolGetter = [this](bool FT66RetroFXSettings::* Field) -> TFunction<bool()>
	{
		return [this, Field]() -> bool
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			return PendingRetroFXSettings.*Field;
		};
	};

	auto MakeBoolSetter = [this](bool FT66RetroFXSettings::* Field) -> TFunction<void(bool)>
	{
		return [this, Field](bool bValue)
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			PendingRetroFXSettings.*Field = bValue;
			MarkRetroFXEdited();
		};
	};

	auto MakePercentToggleGetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<bool()>
	{
		return [this, Field]() -> bool
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			return PendingRetroFXSettings.*Field > KINDA_SMALL_NUMBER;
		};
	};

	auto MakePercentToggleSetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<void(bool)>
	{
		return [this, Field](bool bValue)
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			PendingRetroFXSettings.*Field = bValue ? 100.0f : 0.0f;
			MarkRetroFXEdited();
		};
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
		const TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		if (bIncludeReset)
		{
			Row->AddSlot().AutoWidth()
			[
				MakeRetroButton(
					NSLOCTEXT("T66.Settings", "RetroFXResetDefaults", "RESET TO DEFAULTS"),
					[]() { return false; },
					FOnClicked::CreateLambda([this]()
					{
						return HandleResetRetroFXClicked();
					}),
					220.0f)
			];
		}

		Row->AddSlot().AutoWidth().Padding(bIncludeReset ? FMargin(8.0f, 0.0f, 0.0f, 0.0f) : FMargin(0.0f))
		[
			MakeApplyButton()
		];

		return Row;
	};

	const TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	auto AddRow = [](const TSharedRef<SVerticalBox>& Box, const TSharedRef<SWidget>& Row, float BottomPadding = 8.0f)
	{
		Box->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, BottomPadding)
		[
			Row
		];
	};
	auto AddSection = [&Rows, &MakeSectionHeader, &AddRow](const FText& Title, const FText& Body)
	{
		AddRow(Rows, MakeSectionHeader(Title), 6.0f);
		AddRow(Rows,
			FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
			.Text(Body)
			.Font(SettingsRegularFont(15))
			.ColorAndOpacity(GetSettingsPageMuted())
			.AutoWrapText(true))),
			10.0f);
	};
	auto AddSubsection = [&Rows, &AddRow](const FText& Title, const FText& Body)
	{
		AddRow(Rows, MakeSettingsSectionHeader(Title, 20), 4.0f);
		AddRow(Rows,
			FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
			.Text(Body)
			.Font(SettingsRegularFont(14))
			.ColorAndOpacity(GetSettingsPageMuted())
			.AutoWrapText(true))),
			8.0f);
	};

	AddRow(Rows, MakeSettingsSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXHeader", "Retro FX"), 28), 12.0f);
	AddRow(Rows,
		FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
		.Text(NSLOCTEXT("T66.Settings", "RetroFXBodyDomains", "Retro FX is grouped by what it should visually affect: UI chrome and background images, character-facing geometry, and world/scene treatment for ground, walls, fog, resolution, and screen-space style. Sliders remain 0-100 so you can tune quickly."))
		.Font(SettingsRegularFont(16))
		.ColorAndOpacity(GetSettingsPageMuted())
		.AutoWrapText(true))),
		14.0f);
	AddRow(Rows,
		MakeToggleRow(
			NSLOCTEXT("T66.Settings", "RetroFXPreviewModeLabel", "Preview Mode"),
			NSLOCTEXT("T66.Settings", "RetroFXPreviewModeBody", "Opens the floating Retro FX bar and saves/applies changes as soon as you move a control."),
			[this]() { return ShouldLiveApplyRetroFX(); },
			[this](bool bValue) { HandleRetroFXPreviewModeClicked(bValue); }));
	AddRow(Rows,
		MakeToggleRow(
			NSLOCTEXT("T66.Settings", "RetroFXMasterEnableLabel", "Retro FX Master Enable"),
			NSLOCTEXT("T66.Settings", "RetroFXMasterEnableBody", "Turns the entire Retro FX stack on or off without changing the individual values below."),
			MakeBoolGetter(&FT66RetroFXSettings::bEnableRetroFXMaster),
			MakeBoolSetter(&FT66RetroFXSettings::bEnableRetroFXMaster)));
	AddRow(Rows,
		FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
		.Text_Lambda([this]()
		{
			if (ShouldLiveApplyRetroFX())
			{
				return NSLOCTEXT("T66.Settings", "RetroFXPreviewLive", "Preview mode is live. Changes save and apply immediately.");
			}
			return bRetroFXDirty
				? NSLOCTEXT("T66.Settings", "RetroFXPendingDirty", "Pending changes have not been applied yet.")
				: NSLOCTEXT("T66.Settings", "RetroFXPendingClean", "Pending values match the saved Retro FX profile.");
		})
		.Font(SettingsRegularFont(16))
		.ColorAndOpacity_Lambda([this]() -> FSlateColor
		{
			return FSlateColor((ShouldLiveApplyRetroFX() || bRetroFXDirty) ? FT66Style::ButtonPrimary() : GetSettingsPageMuted());
		})
		.AutoWrapText(true))),
		12.0f);
	AddRow(Rows, MakeActionRow(false), 14.0f);

	AddSection(
		NSLOCTEXT("T66.Settings", "RetroFXSectionUI", "UI"),
		NSLOCTEXT("T66.Settings", "RetroFXSectionUIBody", "Controls for the UI treatment split. Chrome is panels, buttons, icons, portraits, and standard framed images; Background Image is full-screen and screen-art imagery. Text remains supported in code but is hidden here so readable copy stays crisp."));
	AddSubsection(
		NSLOCTEXT("T66.Settings", "RetroFXSectionUIChrome", "UI Chrome"),
		NSLOCTEXT("T66.Settings", "RetroFXSectionUIChromeBody", "Panel, button, portrait, icon, and image treatment."));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIChromePixelationLabel", "Chrome Pixelation"), NSLOCTEXT("T66.Settings", "RetroFXUIChromePixelationBody", "Adds chunky pixel-cell breakup on non-text UI surfaces."), MakeFloatGetter(&FT66RetroFXSettings::UIChromePixelationPercent), MakeFloatSetter(&FT66RetroFXSettings::UIChromePixelationPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIChromeDitheringLabel", "Chrome Dithering"), NSLOCTEXT("T66.Settings", "RetroFXUIChromeDitheringBody", "Adds a dotted dither mask over UI chrome."), MakeFloatGetter(&FT66RetroFXSettings::UIChromeDitheringPercent), MakeFloatSetter(&FT66RetroFXSettings::UIChromeDitheringPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIChromeVertexSnapLabel", "Chrome Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXUIChromeVertexSnapBody", "Offsets and steps non-text UI chrome for a snapped retro surface feel."), MakeFloatGetter(&FT66RetroFXSettings::UIChromeVertexSnapPercent), MakeFloatSetter(&FT66RetroFXSettings::UIChromeVertexSnapPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIChromeVertexSnapResLabel", "Chrome Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXUIChromeVertexSnapResBody", "Higher values make chrome snapping use larger stepped intervals."), MakeFloatGetter(&FT66RetroFXSettings::UIChromeVertexSnapResolutionPercent), MakeFloatSetter(&FT66RetroFXSettings::UIChromeVertexSnapResolutionPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIChromeScanlineLabel", "Chrome Scanlines"), NSLOCTEXT("T66.Settings", "RetroFXUIChromeScanlineBody", "Adds horizontal scanline breakup to panel and button chrome."), MakeFloatGetter(&FT66RetroFXSettings::UIChromeScanlinePercent), MakeFloatSetter(&FT66RetroFXSettings::UIChromeScanlinePercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIChromeChromaticLabel", "Chrome Chromatic Split"), NSLOCTEXT("T66.Settings", "RetroFXUIChromeChromaticBody", "Adds subtle cyan and red edge splitting on UI chrome."), MakeFloatGetter(&FT66RetroFXSettings::UIChromeChromaticAberrationPercent), MakeFloatSetter(&FT66RetroFXSettings::UIChromeChromaticAberrationPercent)), 10.0f);
	AddSubsection(
		NSLOCTEXT("T66.Settings", "RetroFXSectionUIBackgroundImage", "UI Background Image"),
		NSLOCTEXT("T66.Settings", "RetroFXSectionUIBackgroundImageBody", "Full-screen and screen-art background image treatment, separate from chrome and text."));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundPixelationLabel", "Background Image Pixelation"), NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundPixelationBody", "Adds chunky pixel-cell breakup on wrapped UI background imagery."), MakeFloatGetter(&FT66RetroFXSettings::UIBackgroundImagePixelationPercent), MakeFloatSetter(&FT66RetroFXSettings::UIBackgroundImagePixelationPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundDitheringLabel", "Background Image Dithering"), NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundDitheringBody", "Adds a dotted dither mask over wrapped UI background imagery."), MakeFloatGetter(&FT66RetroFXSettings::UIBackgroundImageDitheringPercent), MakeFloatSetter(&FT66RetroFXSettings::UIBackgroundImageDitheringPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundVertexSnapLabel", "Background Image Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundVertexSnapBody", "Offsets and steps background images for a snapped retro surface feel."), MakeFloatGetter(&FT66RetroFXSettings::UIBackgroundImageVertexSnapPercent), MakeFloatSetter(&FT66RetroFXSettings::UIBackgroundImageVertexSnapPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundVertexSnapResLabel", "Background Image Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundVertexSnapResBody", "Higher values make background image snapping use larger stepped intervals."), MakeFloatGetter(&FT66RetroFXSettings::UIBackgroundImageVertexSnapResolutionPercent), MakeFloatSetter(&FT66RetroFXSettings::UIBackgroundImageVertexSnapResolutionPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundScanlineLabel", "Background Image Scanlines"), NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundScanlineBody", "Adds horizontal scanline breakup to UI background imagery."), MakeFloatGetter(&FT66RetroFXSettings::UIBackgroundImageScanlinePercent), MakeFloatSetter(&FT66RetroFXSettings::UIBackgroundImageScanlinePercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundChromaticLabel", "Background Image Chromatic Split"), NSLOCTEXT("T66.Settings", "RetroFXUIBackgroundChromaticBody", "Adds subtle cyan and red edge splitting on UI background imagery."), MakeFloatGetter(&FT66RetroFXSettings::UIBackgroundImageChromaticAberrationPercent), MakeFloatSetter(&FT66RetroFXSettings::UIBackgroundImageChromaticAberrationPercent)), 14.0f);

	AddSection(
		NSLOCTEXT("T66.Settings", "RetroFXSectionCharacters", "Characters"),
		NSLOCTEXT("T66.Settings", "RetroFXSectionCharactersBody", "Applies to hero, enemy, NPC, companion, and other character-facing materials that use T66's shared unlit masters."));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterGeometryEnableLabel", "Character Geometry Enable"), NSLOCTEXT("T66.Settings", "RetroFXCharacterGeometryBody", "Turns the safe runtime retro-geometry swap on or off for character-facing materials."), MakeBoolGetter(&FT66RetroFXSettings::bEnableCharacterGeometry), MakeBoolSetter(&FT66RetroFXSettings::bEnableCharacterGeometry)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterPixelationLabel", "Character Pixelation"), NSLOCTEXT("T66.Settings", "RetroFXCharacterPixelationBody", "Pixelation strength for character-facing scene pixels."), MakeFloatGetter(&FT66RetroFXSettings::CharacterPixelationPercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterPixelationPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapLabel", "Character Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapBody", "Strength for geometry snapping on character-facing unlit materials."), MakeFloatGetter(&FT66RetroFXSettings::CharacterVertexSnapPercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterVertexSnapPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapResLabel", "Character Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapResBody", "Higher values lower the target snap resolution, making hero and enemy geometry wobble more aggressively."), MakeFloatGetter(&FT66RetroFXSettings::CharacterVertexSnapResolutionPercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterVertexSnapResolutionPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexNoiseLabel", "Character Vertex Noise"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexNoiseBody", "Adds extra character-space noise on top of snapping. Keep it lower for readable combat silhouettes."), MakeFloatGetter(&FT66RetroFXSettings::CharacterVertexNoisePercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterVertexNoisePercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineBlendLabel", "Character Affine Blend"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineBlendBody", "Blends character UVs into the affine mapping path."), MakeFloatGetter(&FT66RetroFXSettings::CharacterAffineBlendPercent), MakeFloatSetter(&FT66RetroFXSettings::CharacterAffineBlendPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD1Label", "Character Affine Distance 1"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD1Body", "Near-distance threshold for character affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::CharacterAffineDistance1Percent), MakeFloatSetter(&FT66RetroFXSettings::CharacterAffineDistance1Percent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD2Label", "Character Affine Distance 2"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD2Body", "Mid-distance threshold for character affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::CharacterAffineDistance2Percent), MakeFloatSetter(&FT66RetroFXSettings::CharacterAffineDistance2Percent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD3Label", "Character Affine Distance 3"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD3Body", "Far-distance threshold for character affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::CharacterAffineDistance3Percent), MakeFloatSetter(&FT66RetroFXSettings::CharacterAffineDistance3Percent)), 14.0f);

	AddSection(
		NSLOCTEXT("T66.Settings", "RetroFXSectionWorld", "World"),
		NSLOCTEXT("T66.Settings", "RetroFXSectionWorldBody", "World geometry rows target ground, walls, and environment materials. Screen-space rows below are still the current world/scene render path and can affect the whole rendered scene."));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXWorldGeometryEnableLabel", "World Geometry Enable"), NSLOCTEXT("T66.Settings", "RetroFXWorldGeometryBody", "Turns the safe runtime retro-geometry swap on or off for ground, walls, and environment materials that inherit from T66's shared unlit masters."), MakeBoolGetter(&FT66RetroFXSettings::bEnableWorldGeometry), MakeBoolSetter(&FT66RetroFXSettings::bEnableWorldGeometry)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapLabel", "World Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapBody", "Strength for geometry snapping on ground, wall, and environment materials patched into the retro stack."), MakeFloatGetter(&FT66RetroFXSettings::WorldVertexSnapPercent), MakeFloatSetter(&FT66RetroFXSettings::WorldVertexSnapPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapResLabel", "World Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapResBody", "Higher values lower the target snap resolution, making world surfaces wobble and step more aggressively."), MakeFloatGetter(&FT66RetroFXSettings::WorldVertexSnapResolutionPercent), MakeFloatSetter(&FT66RetroFXSettings::WorldVertexSnapResolutionPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexNoiseLabel", "World Vertex Noise"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexNoiseBody", "Adds extra world-position noise on top of snapping for rougher retro surfaces."), MakeFloatGetter(&FT66RetroFXSettings::WorldVertexNoisePercent), MakeFloatSetter(&FT66RetroFXSettings::WorldVertexNoisePercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineBlendLabel", "World Affine Blend"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineBlendBody", "Blends world UVs into the affine mapping path."), MakeFloatGetter(&FT66RetroFXSettings::WorldAffineBlendPercent), MakeFloatSetter(&FT66RetroFXSettings::WorldAffineBlendPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD1Label", "World Affine Distance 1"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD1Body", "Near-distance threshold for world affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::WorldAffineDistance1Percent), MakeFloatSetter(&FT66RetroFXSettings::WorldAffineDistance1Percent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD2Label", "World Affine Distance 2"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD2Body", "Mid-distance threshold for world affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::WorldAffineDistance2Percent), MakeFloatSetter(&FT66RetroFXSettings::WorldAffineDistance2Percent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD3Label", "World Affine Distance 3"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD3Body", "Far-distance threshold for world affine mapping."), MakeFloatGetter(&FT66RetroFXSettings::WorldAffineDistance3Percent), MakeFloatSetter(&FT66RetroFXSettings::WorldAffineDistance3Percent)), 12.0f);
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1BlendLabel", "PS1 Scene Blend"), NSLOCTEXT("T66.Settings", "RetroFXPs1BlendBody", "Overall blend weight for the UE5RFX PS1 post-process stack."), MakeFloatGetter(&FT66RetroFXSettings::PS1BlendPercent), MakeFloatSetter(&FT66RetroFXSettings::PS1BlendPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1DitherLabel", "PS1 Dithering"), NSLOCTEXT("T66.Settings", "RetroFXPs1DitherBody", "Controls the strength of the imported UE5RFX PS1 dithering pattern."), MakeFloatGetter(&FT66RetroFXSettings::PS1DitheringPercent), MakeFloatSetter(&FT66RetroFXSettings::PS1DitheringPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1BayerLabel", "PS1 Bayer Dithering"), NSLOCTEXT("T66.Settings", "RetroFXPs1BayerBody", "Switches the PS1 post-process stack to the imported Bayer dithering path."), MakePercentToggleGetter(&FT66RetroFXSettings::PS1BayerDitheringPercent), MakePercentToggleSetter(&FT66RetroFXSettings::PS1BayerDitheringPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1ColorLUTLabel", "PS1 Color LUT"), NSLOCTEXT("T66.Settings", "RetroFXPs1ColorLUTBody", "Enables or disables the imported UE5RFX color LUT stage."), MakePercentToggleGetter(&FT66RetroFXSettings::PS1ColorLUTPercent), MakePercentToggleSetter(&FT66RetroFXSettings::PS1ColorLUTPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1ColorBoostLabel", "PS1 Color Boost"), NSLOCTEXT("T66.Settings", "RetroFXPs1ColorBoostBody", "Strength of the imported UE5RFX PS1 LUT color boost."), MakeFloatGetter(&FT66RetroFXSettings::PS1ColorBoostPercent), MakeFloatSetter(&FT66RetroFXSettings::PS1ColorBoostPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPixelationLabel", "World Pixelation"), NSLOCTEXT("T66.Settings", "RetroFXPixelationBody", "Pixelation strength for the world scene pass. UI treatment is controlled separately above."), MakeFloatGetter(&FT66RetroFXSettings::WorldPixelationPercent), MakeFloatSetter(&FT66RetroFXSettings::WorldPixelationPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXN64BlendLabel", "N64 Blur Blend"), NSLOCTEXT("T66.Settings", "RetroFXN64BlendBody", "Overall blend weight for the UE5RFX N64 blur material."), MakeFloatGetter(&FT66RetroFXSettings::N64BlurBlendPercent), MakeFloatSetter(&FT66RetroFXSettings::N64BlurBlendPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXN64StepsLabel", "N64 Blur Steps"), NSLOCTEXT("T66.Settings", "RetroFXN64StepsBody", "Higher values increase the blur sample count in the N64 pass."), MakeFloatGetter(&FT66RetroFXSettings::N64BlurStepsPercent), MakeFloatSetter(&FT66RetroFXSettings::N64BlurStepsPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXN64LowResLabel", "N64 Low Fake Resolution"), NSLOCTEXT("T66.Settings", "RetroFXN64LowResBody", "Enables or disables the low-fake-resolution path used by the N64 blur pass."), MakePercentToggleGetter(&FT66RetroFXSettings::N64LowFakeResolutionPercent), MakePercentToggleSetter(&FT66RetroFXSettings::N64LowFakeResolutionPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXN64ReplaceTMLabel", "N64 Replace Tonemapper"), NSLOCTEXT("T66.Settings", "RetroFXN64ReplaceTMBody", "Switch between the standard N64 blur material and the replace-tonemapper variant."), MakeBoolGetter(&FT66RetroFXSettings::bUseUE5RFXN64BlurReplaceTonemapper), MakeBoolSetter(&FT66RetroFXSettings::bUseUE5RFXN64BlurReplaceTonemapper)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticStrengthLabel", "Chromatic Aberration Strength"), NSLOCTEXT("T66.Settings", "RetroFXChromaticStrengthBody", "Controls the radial RGB split strength for the custom chromatic-aberration post-process pass."), MakeFloatGetter(&FT66RetroFXSettings::ChromaticAberrationPercent), MakeFloatSetter(&FT66RetroFXSettings::ChromaticAberrationPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticDistortionLabel", "Distortion Strength"), NSLOCTEXT("T66.Settings", "RetroFXChromaticDistortionBody", "Controls the radial screen distortion strength used by the chromatic-aberration pass."), MakeFloatGetter(&FT66RetroFXSettings::ChromaticDistortionPercent), MakeFloatSetter(&FT66RetroFXSettings::ChromaticDistortionPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticInvertLabel", "Invert Distortion"), NSLOCTEXT("T66.Settings", "RetroFXChromaticInvertBody", "Flips the radial distortion direction used by the chromatic-aberration pass."), MakeBoolGetter(&FT66RetroFXSettings::bInvertChromaticDistortion), MakeBoolSetter(&FT66RetroFXSettings::bInvertChromaticDistortion)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXRealLowResLabel", "Real Low Resolution"), NSLOCTEXT("T66.Settings", "RetroFXRealLowResBody", "Lowers the actual runtime screen percentage for a stronger low-resolution look."), MakeBoolGetter(&FT66RetroFXSettings::bUseRealLowResolution), MakeBoolSetter(&FT66RetroFXSettings::bUseRealLowResolution)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXResSizeLabel", "Fake Resolution Size Switch"), NSLOCTEXT("T66.Settings", "RetroFXResSizeBody", "Enables or disables the fake screen-size resolution switch."), MakePercentToggleGetter(&FT66RetroFXSettings::FakeResolutionSwitchSizePercent), MakePercentToggleSetter(&FT66RetroFXSettings::FakeResolutionSwitchSizePercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXResUVLabel", "Fake Resolution UV Switch"), NSLOCTEXT("T66.Settings", "RetroFXResUVBody", "Enables or disables the fake UV resolution switch."), MakePercentToggleGetter(&FT66RetroFXSettings::FakeResolutionSwitchUVPercent), MakePercentToggleSetter(&FT66RetroFXSettings::FakeResolutionSwitchUVPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXTargetResLabel", "Target Resolution Height"), NSLOCTEXT("T66.Settings", "RetroFXTargetResBody", "Higher values drive the target height lower, which makes the scene feel more aggressively low-res."), MakeFloatGetter(&FT66RetroFXSettings::TargetResolutionHeightPercent), MakeFloatSetter(&FT66RetroFXSettings::TargetResolutionHeightPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogLabel", "PS1 Fog Enable"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogBody", "Enables or disables the visible PS1 fog contribution by gating its density."), MakePercentToggleGetter(&FT66RetroFXSettings::PS1FogPercent), MakePercentToggleSetter(&FT66RetroFXSettings::PS1FogPercent)));
	AddRow(Rows, MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1SceneDepthFogLabel", "PS1 Scene Depth Fog"), NSLOCTEXT("T66.Settings", "RetroFXPs1SceneDepthFogBody", "Switches the fog calculation to the imported scene-depth-based PS1 fog path."), MakePercentToggleGetter(&FT66RetroFXSettings::PS1SceneDepthFogPercent), MakePercentToggleSetter(&FT66RetroFXSettings::PS1SceneDepthFogPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogDensityLabel", "PS1 Fog Density"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogDensityBody", "Controls how thick the UE5RFX fog becomes once fog is enabled."), MakeFloatGetter(&FT66RetroFXSettings::PS1FogDensityPercent), MakeFloatSetter(&FT66RetroFXSettings::PS1FogDensityPercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogStartLabel", "PS1 Fog Start Distance"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogStartBody", "Higher values pull the fog closer to the camera for a heavier retro atmosphere."), MakeFloatGetter(&FT66RetroFXSettings::PS1FogStartDistancePercent), MakeFloatSetter(&FT66RetroFXSettings::PS1FogStartDistancePercent)));
	AddRow(Rows, MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogFalloffLabel", "PS1 Fog Falloff"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogFalloffBody", "Higher values tighten the fog falloff distance for a denser wall of haze."), MakeFloatGetter(&FT66RetroFXSettings::PS1FogFallOffDistancePercent), MakeFloatSetter(&FT66RetroFXSettings::PS1FogFallOffDistancePercent)), 14.0f);
	AddRow(Rows, MakeActionRow(true), 0.0f);

	return SNew(SScrollBox)
		.ScrollBarStyle(GetSettingsReferenceScrollBarStyle())
		.ScrollBarVisibility(EVisibility::Visible)
		.ScrollBarThickness(FVector2D(14.0f, 14.0f))
		.ScrollBarPadding(FMargin(10.0f, 0.0f, 2.0f, 0.0f))
		+ SScrollBox::Slot()
		[
			Rows
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildRetroFXPreviewPopupUI()
{
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float PanelWidth = FMath::Clamp(SafeFrameSize.X * 0.32f, 500.0f, 620.0f);
	const float PanelHeight = FMath::Clamp(SafeFrameSize.Y - 90.0f, 420.0f, 940.0f);

	const TSharedRef<SWidget> Header =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
		[
			FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
			.Text(NSLOCTEXT("T66.Settings", "RetroFXPreviewPopupTitle", "RETRO FX PREVIEW"))
			.Font(SettingsBoldFont(22))
			.ColorAndOpacity(GetSettingsPageText())))
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			MakeSettingsButton(
				FT66ButtonParams(NSLOCTEXT("T66.Common", "CloseX", "X"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseRetroFXPreviewPopupClicked), ET66ButtonType::Danger)
				.SetFontSize(18)
				.SetMinWidth(38.0f)
				.SetHeight(38.0f)
				.SetPadding(FMargin(0.0f)))
		];

	return SNew(SOverlay)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 36.0f, 28.0f, 36.0f))
		[
			SNew(SBox)
			.WidthOverride(PanelWidth)
			.HeightOverride(PanelHeight)
			[
				MakeSettingsContentShell(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
					[
						Header
					]
					+ SVerticalBox::Slot().FillHeight(1.0f)
					[
						BuildRetroFXTab()
					],
					FMargin(18.0f))
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
	if (ShouldLiveApplyRetroFX())
	{
		ApplyPendingRetroFX();
	}
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleRetroFXPreviewModeClicked(bool bEnabled)
{
	if (bRetroFXPreviewPopup && !bEnabled)
	{
		if (UIManager)
		{
			UIManager->HideRetroFXPreviewPopup();
		}
		return FReply::Handled();
	}

	bRetroFXPreviewMode = bEnabled;
	if (UIManager)
	{
		if (bEnabled)
		{
			UIManager->ShowRetroFXPreviewPopup();
		}
		else
		{
			UIManager->HideRetroFXPreviewPopup();
		}
	}

	if (bEnabled)
	{
		ApplyPendingRetroFX();
	}
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleCloseRetroFXPreviewPopupClicked()
{
	if (UIManager)
	{
		UIManager->HideRetroFXPreviewPopup();
	}
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
		TEXT("ApplyPendingRetroFX: dirty=%s world=%s MasterEnabled=%s PS1Blend=%.2f WorldPixelation=%.2f CharacterPixelation=%.2f ChromePixel=%.2f ChromeDither=%.2f ChromeSnap=%.2f BackgroundPixel=%.2f BackgroundDither=%.2f BackgroundSnap=%.2f"),
		bRetroFXDirty ? TEXT("true") : TEXT("false"),
		*GetNameSafe(GetWorld()),
		PendingRetroFXSettings.bEnableRetroFXMaster ? TEXT("true") : TEXT("false"),
		PendingRetroFXSettings.PS1BlendPercent,
		PendingRetroFXSettings.WorldPixelationPercent,
		PendingRetroFXSettings.CharacterPixelationPercent,
		PendingRetroFXSettings.UIChromePixelationPercent,
		PendingRetroFXSettings.UIChromeDitheringPercent,
		PendingRetroFXSettings.UIChromeVertexSnapPercent,
		PendingRetroFXSettings.UIBackgroundImagePixelationPercent,
		PendingRetroFXSettings.UIBackgroundImageDitheringPercent,
		PendingRetroFXSettings.UIBackgroundImageVertexSnapPercent);

	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->SetRetroFXSettings(PendingRetroFXSettings);
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().InvalidateAllWidgets(false);
		}

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
			{
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
	}
	else
	{
		UE_LOG(LogT66RetroFXUI, Warning, TEXT("ApplyPendingRetroFX: Player settings subsystem was null"));
	}
}

void UT66SettingsScreen::CommitPendingRetroFXOnClose()
{
	if (!bRetroFXInitialized || !bRetroFXDirty)
	{
		return;
	}

	UE_LOG(LogT66RetroFXUI, Log, TEXT("CommitPendingRetroFXOnClose: applying unsaved Retro FX changes before leaving Settings"));
	ApplyPendingRetroFX();
}

void UT66SettingsScreen::ResetPendingRetroFXToDefaults()
{
	InitializeRetroFXFromUserSettingsIfNeeded();
	PendingRetroFXSettings = FT66RetroFXSettings();
	bRetroFXDirty = true;
}

void UT66SettingsScreen::MarkRetroFXEdited()
{
	bRetroFXDirty = true;
	if (ShouldLiveApplyRetroFX())
	{
		ApplyPendingRetroFX();
	}
}

bool UT66SettingsScreen::ShouldLiveApplyRetroFX() const
{
	return bRetroFXPreviewPopup || bRetroFXPreviewMode;
}
