// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "Misc/Paths.h"
#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"

using namespace T66SettingsScreenPrivate;

namespace
{
	TMap<FString, TStrongObjectPtr<UTexture2D>> GSettingsRetroFXTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GSettingsRetroFXBrushCache;

	UTexture2D* LoadSettingsRetroFXTexture(const FString& RelativePath)
	{
		if (TStrongObjectPtr<UTexture2D>* Existing = GSettingsRetroFXTextureCache.Find(RelativePath))
		{
			return Existing->Get();
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				TextureFilter::TF_Nearest,
				true,
				TEXT("SettingsRetroFXFlatUI"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Nearest,
					TEXT("SettingsRetroFXFlatUI"));
			}

			if (Texture)
			{
				GSettingsRetroFXTextureCache.Add(RelativePath, TStrongObjectPtr<UTexture2D>(Texture));
				return Texture;
			}
		}

		GSettingsRetroFXTextureCache.Add(RelativePath, TStrongObjectPtr<UTexture2D>(nullptr));
		return nullptr;
	}

	const FSlateBrush* ResolveSettingsRetroFXBrush(const FString& RelativePath, const FVector2D& SizeHint)
	{
		if (TSharedPtr<FSlateBrush>* Existing = GSettingsRetroFXBrushCache.Find(RelativePath))
		{
			return Existing->Get();
		}

		UTexture2D* Texture = LoadSettingsRetroFXTexture(RelativePath);
		if (!Texture)
		{
			return nullptr;
		}

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->SetResourceObject(Texture);
		Brush->ImageSize = SizeHint;
		Brush->DrawAs = ESlateBrushDrawType::Image;
		GSettingsRetroFXBrushCache.Add(RelativePath, Brush);
		return Brush.Get();
	}
}

TSharedRef<SWidget> UT66SettingsScreen::BuildSlateUI()
{
	if (bRetroFXPreviewPopup)
	{
		return BuildRetroFXPreviewPopupUI();
	}

	ApplyCommandLineTabOverride();

	if (CurrentTab == ET66SettingsTab::Gameplay)
	{
		return BuildFlatGameplaySettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::Graphics)
	{
		return BuildFlatGraphicsSettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::Controls)
	{
		return BuildFlatControlsSettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::MediaViewer)
	{
		return BuildFlatMediaViewerSettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::Audio)
	{
		return BuildFlatAudioSettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::RetroFX)
	{
		InitializeRetroFXFromUserSettingsIfNeeded();

		constexpr float CanvasW = 1920.f;
		constexpr float CanvasH = 1080.f;
		const FName SettingsTabsGroup(TEXT("SettingsTabs"));
		const FName ThemeModeGroup(TEXT("ThemeMode"));
		const FName MasterEnableGroup(TEXT("RetroFXMasterEnable"));

		auto DTag = [](const TCHAR* Text) -> FName
		{
			return FName(Text);
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

		auto MakeLabel = [](const FName Tag, const FText& Text, const int32 FontSize, const FLinearColor& Color, const bool bBold = false, const ETextJustify::Type Justify = ETextJustify::Left, const bool bAutoWrap = true) -> TSharedRef<SWidget>
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

		auto MakePanel = [](const FName Tag) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(0.f),
				SNew(SSpacer).Size(FVector2D(1.f, 1.f)),
				nullptr,
				Tag);
		};

		auto MakeMetadataRegion = [](const FName Tag, const FString& Role) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBox),
				Tag,
				Role,
				ET66FlatState::Default);
		};

		auto MakeButton = [&MakeLabel](const FName Tag, const FName ToggleGroup, const ET66FlatState State, const FText& Label, FOnClicked OnClicked, const float Width, const float Height, const int32 FontSize = 22) -> TSharedRef<SWidget>
		{
			const FName LabelTag = Tag.IsNone() ? NAME_None : FName(*(Tag.ToString() + TEXT(".Label")));
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					MakeLabel(LabelTag, Label, FontSize, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center, false)
				],
				MoveTemp(OnClicked),
				FMargin(0.f),
				Width,
				Height,
				true,
				Tag,
				ToggleGroup);
		};

		auto MakeIconToggle = [&MakeButton](const FName Tag, const FName ToggleGroup, const ET66FlatState State, const FString& IconPath, const FText& FallbackText, FOnClicked OnClicked, const float Width, const float Height) -> TSharedRef<SWidget>
		{
			const FSlateBrush* Brush = ResolveSettingsRetroFXBrush(IconPath, FVector2D(46.f, 46.f));
			TSharedRef<SWidget> Content = Brush
				? StaticCastSharedRef<SWidget>(SNew(SImage).Image(Brush).ColorAndOpacity(State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PurpleAccent()))
				: StaticCastSharedRef<SWidget>(FT66FlatStyle::MakeFlatLabel(FallbackText, ET66FlatLabelRole::Button, ETextJustify::Center));
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)[Content],
				MoveTemp(OnClicked),
				FMargin(0.f),
				Width,
				Height,
				true,
				Tag,
				ToggleGroup);
		};

		auto MakeSliderValueText = [](TFunction<float()> Getter) -> TSharedRef<SWidget>
		{
			return SNew(STextBlock)
				.Text_Lambda([Getter = MoveTemp(Getter)]()
				{
					return FText::AsNumber(FMath::RoundToInt(Getter()));
				})
				.Font(FT66FlatStyle::MakeBoldFont(28))
				.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				.Justification(ETextJustify::Center);
		};

		auto MakeSlider = [this, &MakeSliderValueText](const FName Tag, float FT66RetroFXSettings::* Field) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatSlider(
				ET66FlatState::Default,
				0.f,
				100.f,
				TAttribute<float>::CreateLambda([this, Field]()
				{
					InitializeRetroFXFromUserSettingsIfNeeded();
					return PendingRetroFXSettings.*Field;
				}),
				FOnFloatValueChanged::CreateLambda([this, Field](float Value)
				{
					InitializeRetroFXFromUserSettingsIfNeeded();
					PendingRetroFXSettings.*Field = FMath::Clamp(Value, 0.f, 100.f);
					MarkRetroFXEdited();
				}),
				MakeSliderValueText([this, Field]()
				{
					InitializeRetroFXFromUserSettingsIfNeeded();
					return PendingRetroFXSettings.*Field;
				}),
				Tag);
		};

		AddN(0.000f, 0.000f, 1.000f, 1.000f,
			FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor::Black),
				DTag(TEXT("SettingsRetroFX.Background")),
				TEXT("Background"),
				ET66FlatState::Default));

		AddN(0.013f, 0.157f, 0.974f, 0.821f, MakeMetadataRegion(DTag(TEXT("SettingsRetroFX.Root")), TEXT("Root")));
		AddN(0.014f, 0.157f, 0.109f, 0.067f, MakeMetadataRegion(DTag(TEXT("SettingsRetroFX.Theme")), TEXT("ToggleGroup.ThemeMode")));
		AddN(0.154f, 0.157f, 0.824f, 0.067f, MakeMetadataRegion(DTag(TEXT("SettingsRetroFX.SettingsTabs")), TEXT("ToggleGroup.SettingsTabs")));

		AddN(0.014f, 0.157f, 0.050f, 0.067f, MakeIconToggle(DTag(TEXT("SettingsRetroFX.Theme.SunButton")), ThemeModeGroup, ET66FlatState::Default, TEXT("RuntimeDependencies/T66/UI/Icons/Flat/sun.png"), NSLOCTEXT("T66.Settings", "FlatSun", "SUN"), FOnClicked::CreateLambda([]() { return FReply::Handled(); }), 0.050f * CanvasW, 0.067f * CanvasH));
		AddN(0.073f, 0.157f, 0.050f, 0.067f, MakeIconToggle(DTag(TEXT("SettingsRetroFX.Theme.MoonButton")), ThemeModeGroup, ET66FlatState::Selected, TEXT("RuntimeDependencies/T66/UI/Icons/Flat/moon.png"), NSLOCTEXT("T66.Settings", "FlatMoon", "MOON"), FOnClicked::CreateLambda([]() { return FReply::Handled(); }), 0.050f * CanvasW, 0.067f * CanvasH));

		AddN(0.154f, 0.157f, 0.127f, 0.067f, MakeButton(DTag(TEXT("SettingsRetroFX.SettingsTabs.GameplayButton")), SettingsTabsGroup, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabGameplayFlat", "GAMEPLAY"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, ET66SettingsTab::Gameplay), 0.127f * CanvasW, 0.067f * CanvasH));
		AddN(0.295f, 0.157f, 0.126f, 0.067f, MakeButton(DTag(TEXT("SettingsRetroFX.SettingsTabs.GraphicsButton")), SettingsTabsGroup, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabGraphicsFlat", "GRAPHICS"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, ET66SettingsTab::Graphics), 0.126f * CanvasW, 0.067f * CanvasH));
		AddN(0.434f, 0.157f, 0.124f, 0.067f, MakeButton(DTag(TEXT("SettingsRetroFX.SettingsTabs.ControlsButton")), SettingsTabsGroup, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabControlsFlat", "CONTROLS"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, ET66SettingsTab::Controls), 0.124f * CanvasW, 0.067f * CanvasH));
		AddN(0.572f, 0.157f, 0.132f, 0.067f, MakeButton(DTag(TEXT("SettingsRetroFX.SettingsTabs.MediaViewerButton")), SettingsTabsGroup, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabMediaViewerFlat", "MEDIA VIEWER"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, ET66SettingsTab::MediaViewer), 0.132f * CanvasW, 0.067f * CanvasH, 20));
		AddN(0.717f, 0.157f, 0.117f, 0.067f, MakeButton(DTag(TEXT("SettingsRetroFX.SettingsTabs.AudioButton")), SettingsTabsGroup, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabAudioFlat", "AUDIO"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, ET66SettingsTab::Audio), 0.117f * CanvasW, 0.067f * CanvasH));
		AddN(0.848f, 0.157f, 0.130f, 0.067f, MakeButton(DTag(TEXT("SettingsRetroFX.SettingsTabs.RetroFXButton")), SettingsTabsGroup, ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "TabRetroFXFlat", "RETRO FX"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, ET66SettingsTab::RetroFX), 0.130f * CanvasW, 0.067f * CanvasH));

		AddN(0.013f, 0.247f, 0.974f, 0.181f, MakePanel(DTag(TEXT("SettingsRetroFX.MasterPanel"))));
		AddN(0.033f, 0.268f, 0.440f, 0.040f, MakeLabel(DTag(TEXT("SettingsRetroFX.MasterPanel.Header")), NSLOCTEXT("T66.Settings", "RetroFXMasterEnableLabelFlat", "RETRO FX MASTER ENABLE"), 30, FT66FlatStyle::PrimaryText(), true));
		AddN(0.033f, 0.318f, 0.615f, 0.034f, MakeLabel(DTag(TEXT("SettingsRetroFX.MasterPanel.Description")), NSLOCTEXT("T66.Settings", "RetroFXMasterEnableBodyFlat", "Turns the entire Retro FX stack on or off without changing the individual values below."), 22, FT66FlatStyle::PrimaryText()));
		AddN(0.033f, 0.374f, 0.520f, 0.034f, MakeLabel(DTag(TEXT("SettingsRetroFX.MasterPanel.StatusNote")), NSLOCTEXT("T66.Settings", "RetroFXPendingCleanFlat", "Pending values match the saved Retro FX profile."), 22, FT66FlatStyle::PrimaryText()));
		AddN(0.712f, 0.268f, 0.255f, 0.143f, MakeMetadataRegion(DTag(TEXT("SettingsRetroFX.MasterPanel.Controls")), TEXT("ToggleGroup.RetroFXMasterEnable")));
		AddN(0.712f, 0.268f, 0.128f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.MasterPanel.OnButton")), MasterEnableGroup, ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "RetroFXOnFlat", "ON"), FOnClicked::CreateLambda([this]() { PendingRetroFXSettings.bEnableRetroFXMaster = true; MarkRetroFXEdited(); ApplyPendingRetroFX(); return FReply::Handled(); }), 0.128f * CanvasW, 0.064f * CanvasH));
		AddN(0.849f, 0.268f, 0.118f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.MasterPanel.OffButton")), MasterEnableGroup, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "RetroFXOffFlat", "OFF"), FOnClicked::CreateLambda([this]() { PendingRetroFXSettings.bEnableRetroFXMaster = false; MarkRetroFXEdited(); ApplyPendingRetroFX(); return FReply::Handled(); }), 0.118f * CanvasW, 0.064f * CanvasH));
		AddN(0.712f, 0.347f, 0.255f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.MasterPanel.ApplyButton")), NAME_None, ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "RetroFXApplyFlat", "APPLY"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleApplyRetroFXClicked), 0.255f * CanvasW, 0.064f * CanvasH));

		AddN(0.013f, 0.442f, 0.974f, 0.159f, MakePanel(DTag(TEXT("SettingsRetroFX.UIPanel"))));
		AddN(0.034f, 0.462f, 0.120f, 0.042f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIPanel.Header")), NSLOCTEXT("T66.Settings", "RetroFXSectionUIFlat", "UI"), 30, FT66FlatStyle::PrimaryText(), true));
		AddN(0.034f, 0.505f, 0.860f, 0.082f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIPanel.Description")), NSLOCTEXT("T66.Settings", "RetroFXSectionUIBodyFlat", "Controls for the UI treatment split. Chrome is panels, buttons, icons, portraits, and standard framed images.\nBackground image is full-screen and screen-art imagery.\nText remains supported in code but is hidden here so readable copy stays crisp."), 21, FT66FlatStyle::PrimaryText()));

		AddN(0.013f, 0.615f, 0.974f, 0.363f, MakePanel(DTag(TEXT("SettingsRetroFX.UIChromePanel"))));
		AddN(0.034f, 0.636f, 0.180f, 0.042f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIChromePanel.Header")), NSLOCTEXT("T66.Settings", "RetroFXSectionUIChromeFlat", "UI CHROME"), 30, FT66FlatStyle::PrimaryText(), true));
		AddN(0.034f, 0.678f, 0.560f, 0.034f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIChromePanel.Description")), NSLOCTEXT("T66.Settings", "RetroFXSectionUIChromeBodyFlat", "Panel, button, portrait, icon, and image treatment."), 21, FT66FlatStyle::PrimaryText()));

		AddN(0.035f, 0.706f, 0.932f, 0.110f, FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(0.f), SNew(SSpacer).Size(FVector2D(1.f, 1.f)), nullptr, DTag(TEXT("SettingsRetroFX.UIChromePanel.PixelationSubPanel"))));
		AddN(0.061f, 0.727f, 0.260f, 0.036f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIChromePanel.Pixelation.Label")), NSLOCTEXT("T66.Settings", "RetroFXUIChromePixelationLabelFlat", "CHROME PIXELATION"), 26, FT66FlatStyle::PrimaryText(), true));
		AddN(0.061f, 0.771f, 0.300f, 0.030f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIChromePanel.Pixelation.Description")), NSLOCTEXT("T66.Settings", "RetroFXUIChromePixelationBodyFlat", "Adds chunky pixel-cell breakup on non-text UI surfaces."), 18, FT66FlatStyle::PrimaryText()));
		AddN(0.374f, 0.737f, 0.566f, 0.042f, MakeSlider(DTag(TEXT("SettingsRetroFX.UIChromePanel.Pixelation.Slider")), &FT66RetroFXSettings::UIChromePixelationPercent));
		AddN(0.374f, 0.788f, 0.150f, 0.026f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIChromePanel.Pixelation.Caption")), NSLOCTEXT("T66.Settings", "RetroFXSliderHintFlat", "Slide from 0 to 100."), 16, FT66FlatStyle::PrimaryText()));

		AddN(0.035f, 0.826f, 0.932f, 0.121f, FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(0.f), SNew(SSpacer).Size(FVector2D(1.f, 1.f)), nullptr, DTag(TEXT("SettingsRetroFX.UIChromePanel.DitheringSubPanel"))));
		AddN(0.061f, 0.847f, 0.260f, 0.036f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIChromePanel.Dithering.Label")), NSLOCTEXT("T66.Settings", "RetroFXUIChromeDitheringLabelFlat", "CHROME DITHERING"), 26, FT66FlatStyle::PrimaryText(), true));
		AddN(0.061f, 0.891f, 0.300f, 0.030f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIChromePanel.Dithering.Description")), NSLOCTEXT("T66.Settings", "RetroFXUIChromeDitheringBodyFlat", "Adds a dotted dither mask over UI chrome."), 18, FT66FlatStyle::PrimaryText()));
		AddN(0.374f, 0.858f, 0.566f, 0.042f, MakeSlider(DTag(TEXT("SettingsRetroFX.UIChromePanel.Dithering.Slider")), &FT66RetroFXSettings::UIChromeDitheringPercent));
		AddN(0.374f, 0.910f, 0.150f, 0.026f, MakeLabel(DTag(TEXT("SettingsRetroFX.UIChromePanel.Dithering.Caption")), NSLOCTEXT("T66.Settings", "RetroFXSliderHintFlat2", "Slide from 0 to 100."), 16, FT66FlatStyle::PrimaryText()));

		return SNew(SBox)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::Both)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				[
					SNew(SBox)
					.WidthOverride(CanvasW)
					.HeightOverride(CanvasH)
					[
						Canvas
					]
				]
			];
	}

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	bool bShowMediaViewerTab = true;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (const UT66RuntimePlatformSubsystem* RuntimePlatform = GI->GetSubsystem<UT66RuntimePlatformSubsystem>())
		{
			bShowMediaViewerTab = RuntimePlatform->ShouldShowMediaViewer();
		}
	}

	struct FSettingsTabDefinition
	{
		FText Label;
		ET66SettingsTab Tab = ET66SettingsTab::Gameplay;
	};

	TArray<FSettingsTabDefinition> TabDefinitions = {
		{ Loc ? Loc->GetText_SettingsTabGameplay() : NSLOCTEXT("T66.Settings", "TabGameplay", "GAMEPLAY"), ET66SettingsTab::Gameplay },
		{ Loc ? Loc->GetText_SettingsTabGraphics() : NSLOCTEXT("T66.Settings", "TabGraphics", "GRAPHICS"), ET66SettingsTab::Graphics },
		{ Loc ? Loc->GetText_SettingsTabControls() : NSLOCTEXT("T66.Settings", "TabControls", "CONTROLS"), ET66SettingsTab::Controls },
		{ Loc ? Loc->GetText_SettingsTabHUD() : NSLOCTEXT("T66.Settings", "TabHUD", "HUD"), ET66SettingsTab::HUD }
	};
	if (bShowMediaViewerTab)
	{
		TabDefinitions.Add({ Loc ? Loc->GetText_SettingsTabMediaViewer() : NSLOCTEXT("T66.Settings", "TabMediaViewer", "MEDIA VIEWER"), ET66SettingsTab::MediaViewer });
	}
	TabDefinitions.Add({ Loc ? Loc->GetText_SettingsTabAudio() : NSLOCTEXT("T66.Settings", "TabAudio", "AUDIO"), ET66SettingsTab::Audio });
	TabDefinitions.Add({ Loc ? Loc->GetText_SettingsTabCrashing() : NSLOCTEXT("T66.Settings", "TabCrashing", "CRASHING"), ET66SettingsTab::Crashing });
	TabDefinitions.Add({ NSLOCTEXT("T66.Settings", "TabRetroFX", "RETRO FX"), ET66SettingsTab::RetroFX });

	const TSharedRef<SHorizontalBox> TabRow = SNew(SHorizontalBox);
	for (const FSettingsTabDefinition& TabDefinition : TabDefinitions)
	{
		TabRow->AddSlot().FillWidth(1.f)
		[
			SNew(SBox).Padding(FMargin(2.f))
			[
				MakeSelectableSettingsButton(
					FT66ButtonParams(TabDefinition.Label, FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, TabDefinition.Tab), ET66ButtonType::Neutral)
					.SetFontSize(18)
					.SetHeight(62.f)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(10.f, 16.f, 10.f, 13.f))
					.SetContent(
						SNew(STextBlock)
						.Text(TabDefinition.Label)
						.Font(SettingsBoldFont(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)),
					[this, Tab = TabDefinition.Tab]() { return CurrentTab == Tab; })
			]
		];
	}

	// Build the widget switcher with all tab content (stored as class member)
	const bool bModalPresentation = (UIManager && UIManager->GetCurrentModalType() == ScreenType) || (!UIManager && GetOwningPlayer() && GetOwningPlayer()->IsPaused());
	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 18.f;
	const float TopInset = bModalPresentation
		? 0.f
		: FMath::Max(0.f, ((UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f) - TopBarOverlapPx) / ResponsiveScale);
	const FVector2D ViewportSize = FT66Style::GetViewportLogicalSize();
	const float SurfaceW = FMath::Max(1.f, ViewportSize.X);
	const float SurfaceH = FMath::Max(1.f, ViewportSize.Y - TopInset);
	const FMargin ContentAreaPadding = bModalPresentation
		? FMargin(8.f)
		: FMargin(0.f);

	const TSharedRef<SWidget> SettingsContent =
		SNew(SVerticalBox)
		// Tab bar + close button (extra vertical padding so tab buttons are not clipped)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Padding(FMargin(0.0f, 10.0f, 0.0f, 10.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					TabRow
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.Padding(6.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.Visibility(bModalPresentation ? EVisibility::Visible : EVisibility::Collapsed)
					[
						MakeSettingsButton(
							FT66ButtonParams(NSLOCTEXT("T66.Common", "CloseX", "X"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseClicked), ET66ButtonType::Danger)
							.SetFontSize(18)
							.SetMinWidth(36.f).SetHeight(36.f)
							.SetPadding(FMargin(0.f))
							.SetColor(FT66Style::Tokens::Danger)
							.SetContent(SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "CloseX", "X"))
								.Font(SettingsBoldFont(20))
								.ColorAndOpacity(FT66Style::Tokens::Text))
						)
					]
				]
			]
		]
		// Content area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(ContentAreaPadding)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SAssignNew(ContentSwitcher, SWidgetSwitcher)
				.WidgetIndex(static_cast<int32>(CurrentTab))
				+ SWidgetSwitcher::Slot()
				[
					BuildGameplayTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildGraphicsTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildControlsTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildHUDTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildMediaViewerTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildAudioTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildCrashingTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildRetroFXTab()
				]
			]
		];

	const TSharedRef<SWidget> SettingsSurface =
		MakeSettingsContentShell(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Clipping(EWidgetClipping::ClipToBounds)
			.Padding(FMargin(0.f, 0.f, 0.f, 8.f))
			[
				SettingsContent
			],
			FMargin(0.f, 0.f, 0.f, 30.f));

	TSharedRef<SOverlay> Root = SNew(SOverlay);

	Root->AddSlot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::White)
	];

	if (bModalPresentation)
	{
		Root->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Scrim)
		];
	}

	Root->AddSlot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
	[
		SNew(SBox)
		.WidthOverride(SurfaceW)
		.HeightOverride(SurfaceH)
		[
			SettingsSurface
		]
	];

	return Root;
}

FReply UT66SettingsScreen::HandleTabClicked(ET66SettingsTab Tab)
{
	SwitchToTab(Tab);
	return FReply::Handled();
}


