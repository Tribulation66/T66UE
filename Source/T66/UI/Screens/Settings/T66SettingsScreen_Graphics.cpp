// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "Engine/Engine.h"
#include "TimerManager.h"
#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"

using namespace T66SettingsScreenPrivate;

TSharedRef<SWidget> UT66SettingsScreen::BuildFlatGraphicsSettingsUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	InitializeGraphicsFromUserSettingsIfNeeded();

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

	auto WindowModeToText = [Loc](EWindowMode::Type Mode) -> FText
	{
		if (!Loc)
		{
			switch (Mode)
			{
			case EWindowMode::Windowed: return NSLOCTEXT("T66.Settings", "WindowedFlat", "Windowed");
			case EWindowMode::WindowedFullscreen: return NSLOCTEXT("T66.Settings", "BorderlessWindowedFlat", "Borderless Windowed");
			default: return NSLOCTEXT("T66.Settings", "FullscreenFlat", "Fullscreen");
			}
		}
		switch (Mode)
		{
		case EWindowMode::Windowed: return Loc->GetText_Windowed();
		case EWindowMode::WindowedFullscreen: return Loc->GetText_BorderlessWindowed();
		default: return Loc->GetText_Fullscreen();
		}
	};

	auto DisplayModeToText = [Loc](ET66DisplayMode Mode) -> FText
	{
		if (!Loc)
		{
			return Mode == ET66DisplayMode::Widescreen ? NSLOCTEXT("T66.Settings", "WidescreenFlat", "Widescreen") : NSLOCTEXT("T66.Settings", "StandardFlat", "Standard");
		}
		return Mode == ET66DisplayMode::Widescreen ? Loc->GetText_DisplayModeWidescreen() : Loc->GetText_DisplayModeStandard();
	};

	auto FpsCapToText = [Loc](int32 Index) -> FText
	{
		switch (Index)
		{
		case 0: return FText::AsNumber(30);
		case 1: return FText::AsNumber(60);
		case 2: return FText::AsNumber(90);
		case 3: return FText::AsNumber(120);
		default: return Loc ? Loc->GetText_Unlimited() : NSLOCTEXT("T66.Settings", "UnlimitedFlat", "Unlimited");
		}
	};

	auto ResToText = [](const FIntPoint R) -> FText
	{
		return FText::Format(NSLOCTEXT("T66.Settings", "ResolutionFormatFlat", "{0} x {1}"), FText::AsNumber(R.X), FText::AsNumber(R.Y));
	};

	auto MakeOption = [this](const FName Tag, const FText& Label, TFunction<void()> Action, const bool bSelected) -> TSharedRef<SWidget>
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
			430.f,
			44.f,
			true,
			18,
			Tag);
	};

	auto MakeDropdownRow = [&MakeLabel, &ChildTag](const FName RowTag, const FText& Label, TAttribute<FText> CurrentText, TFunction<TSharedRef<SWidget>()> OptionsProvider) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.40f)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Label")), Label, 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.60f)
			.VAlign(VAlign_Center)
			.Padding(18.f, 0.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatDropdown(
					ET66FlatState::Selected,
					MoveTemp(CurrentText),
					MoveTemp(OptionsProvider),
					true,
					840.f,
					58.f,
					20,
					ChildTag(RowTag, TEXT("Dropdown")))
			];
		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(28.f, 18.f, 22.f, 18.f), Row, nullptr, RowTag);
	};

	auto MakeStaticRow = [&MakeLabel, &ChildTag](const FName RowTag, const FText& Label, const FText& Value) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Label")), Label, 24, FT66FlatStyle::SecondaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Value")), Value, 22, FT66FlatStyle::SecondaryText(), true, ETextJustify::Right, false)
			];
		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Disabled, FMargin(22.f, 16.f), Row, nullptr, RowTag);
	};

	auto MakeToggleButton = [&MakeLabel, &ChildTag](const FName Tag, const FName ToggleGroup, const ET66FlatState State, const FText& Text, TFunction<void()> Action) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(Tag, TEXT("Label")), Text, 22, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center, false)
			],
			FOnClicked::CreateLambda([Action = MoveTemp(Action)]()
			{
				Action();
				return FReply::Handled();
			}),
			FMargin(0.f),
			120.f,
			58.f,
			true,
			Tag,
			ToggleGroup);
	};

	auto MakeFogRow = [this, Loc, PS, &MakeLabel, &ChildTag, &MakeToggleButton](const FName RowTag) -> TSharedRef<SWidget>
	{
		const bool bValue = PS ? PS->GetFogEnabled() : false;
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Label")), Loc ? Loc->GetText_SettingsFog() : NSLOCTEXT("T66.Settings", "FogFlat", "Fog"), 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 0.f, 0.f)
			[
				MakeToggleButton(ChildTag(RowTag, TEXT("OnButton")), RowTag, bValue ? ET66FlatState::Selected : ET66FlatState::Default, GetSettingsOnText(Loc), [this, PS]() { if (PS) PS->SetFogEnabled(true); ForceRebuildSlate(); })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
			[
				MakeToggleButton(ChildTag(RowTag, TEXT("OffButton")), RowTag, bValue ? ET66FlatState::Default : ET66FlatState::Selected, GetSettingsOffText(Loc), [this, PS]() { if (PS) PS->SetFogEnabled(false); ForceRebuildSlate(); })
			];
		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(28.f, 18.f, 22.f, 18.f), Row, nullptr, RowTag);
	};

	auto MakeQualityRow = [this, Loc, &MakeLabel, &ChildTag](const FName RowTag) -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Box = SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					MakeLabel(ChildTag(RowTag, TEXT("BestPerformanceLabel")), Loc ? Loc->GetText_BestPerformance() : NSLOCTEXT("T66.Settings.Fallback", "Best Performance Flat", "Best Performance"), 18, FT66FlatStyle::SecondaryText(), true, ETextJustify::Left, false)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeLabel(ChildTag(RowTag, TEXT("BestQualityLabel")), Loc ? Loc->GetText_BestQuality() : NSLOCTEXT("T66.Settings.Fallback", "Best Quality Flat", "Best Quality"), 18, FT66FlatStyle::SecondaryText(), true, ETextJustify::Right, false)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatSlider(
					ET66FlatState::Default,
					0.f,
					3.f,
					TAttribute<float>::CreateLambda([this]() { return static_cast<float>(PendingGraphics.QualityNotch); }),
					FOnFloatValueChanged::CreateLambda([this](float Value)
					{
						PendingGraphics.QualityNotch = FMath::Clamp(FMath::RoundToInt(Value), 0, 3);
						PendingGraphics.bDirty = true;
					}),
					nullptr,
					ChildTag(RowTag, TEXT("Slider")))
			];
		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(22.f, 14.f), Box, nullptr, RowTag);
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

	TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	auto AddRow = [&Rows](const TSharedRef<SWidget>& Widget, const float Height, const float BottomPadding = 12.f)
	{
		Rows->AddSlot().AutoHeight().Padding(0.f, 0.f, 16.f, BottomPadding)
		[
			SNew(SBox).HeightOverride(Height)[Widget]
		];
	};

	AddRow(MakeDropdownRow(
		DTag(TEXT("SettingsGraphics.Rows.Monitor")),
		Loc ? Loc->GetText_Monitor() : NSLOCTEXT("T66.Settings", "MonitorFlat", "Monitor"),
		TAttribute<FText>::CreateLambda([this, Loc]()
		{
			FDisplayMetrics Metrics;
			FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
			const int32 Idx = FMath::Clamp(PendingGraphics.MonitorIndex, 0, FMath::Max(0, Metrics.MonitorInfo.Num() - 1));
			return Idx == 0 ? (Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitorFlat", "Primary Monitor")) : FText::Format(NSLOCTEXT("T66.Settings", "MonitorNFlat", "Monitor {0}"), FText::AsNumber(Idx + 1));
		}),
		[this, Loc, &MakeOption]()
		{
			FDisplayMetrics Metrics;
			FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
			for (int32 Index = 0; Index < Metrics.MonitorInfo.Num(); ++Index)
			{
				const int32 MonitorIdx = Index;
				const FText Label = MonitorIdx == 0 ? (Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitorOptionFlat", "Primary Monitor")) : FText::Format(NSLOCTEXT("T66.Settings", "MonitorNOptionFlat", "Monitor {0}"), FText::AsNumber(MonitorIdx + 1));
				Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeOption(FName(*FString::Printf(TEXT("SettingsGraphics.Rows.Monitor.Option.%d"), MonitorIdx)), Label, [this, MonitorIdx]() { PendingGraphics.MonitorIndex = MonitorIdx; PendingGraphics.bDirty = true; }, PendingGraphics.MonitorIndex == MonitorIdx)
				];
			}
			return Box;
		}),
		130.f);

	AddRow(MakeDropdownRow(
		DTag(TEXT("SettingsGraphics.Rows.Resolution")),
		Loc ? Loc->GetText_Resolution() : NSLOCTEXT("T66.Settings.Fallback", "Resolution Flat", "Resolution"),
		TAttribute<FText>::CreateLambda([this, ResToText]() { return ResToText(PendingGraphics.Resolution); }),
		[this, ResToText, &MakeOption]()
		{
			static const FIntPoint ResList[] = { FIntPoint(1280, 720), FIntPoint(1600, 900), FIntPoint(1920, 1080), FIntPoint(2560, 1440), FIntPoint(3840, 2160) };
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
			for (const FIntPoint& R : ResList)
			{
				Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeOption(FName(*FString::Printf(TEXT("SettingsGraphics.Rows.Resolution.Option.%dx%d"), R.X, R.Y)), ResToText(R), [this, R]() { PendingGraphics.Resolution = R; PendingGraphics.bDirty = true; }, PendingGraphics.Resolution == R)
				];
			}
			return Box;
		}),
		130.f);

	AddRow(MakeDropdownRow(
		DTag(TEXT("SettingsGraphics.Rows.WindowMode")),
		Loc ? Loc->GetText_WindowMode() : NSLOCTEXT("T66.Settings.Fallback", "Window Mode Flat", "Window Mode"),
		TAttribute<FText>::CreateLambda([this, WindowModeToText]() { return WindowModeToText(PendingGraphics.WindowMode); }),
		[this, WindowModeToText, &MakeOption]()
		{
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
			auto Add = [this, &Box, WindowModeToText, &MakeOption](const TCHAR* Suffix, EWindowMode::Type Mode)
			{
				Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeOption(FName(*(FString(TEXT("SettingsGraphics.Rows.WindowMode.Option.")) + Suffix)), WindowModeToText(Mode), [this, Mode]() { PendingGraphics.WindowMode = Mode; PendingGraphics.bDirty = true; }, PendingGraphics.WindowMode == Mode)
				];
			};
			Add(TEXT("Windowed"), EWindowMode::Windowed);
			Add(TEXT("Fullscreen"), EWindowMode::Fullscreen);
			Add(TEXT("Borderless"), EWindowMode::WindowedFullscreen);
			return Box;
		}),
		130.f);

	AddRow(MakeDropdownRow(
		DTag(TEXT("SettingsGraphics.Rows.DisplayMode")),
		Loc ? Loc->GetText_DisplayMode() : NSLOCTEXT("T66.Settings.Fallback", "Display Mode Flat", "Display Mode"),
		TAttribute<FText>::CreateLambda([this, DisplayModeToText]() { return DisplayModeToText(PendingGraphics.DisplayMode); }),
		[this, DisplayModeToText, &MakeOption]()
		{
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
			auto Add = [this, &Box, DisplayModeToText, &MakeOption](const TCHAR* Suffix, ET66DisplayMode Mode)
			{
				Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeOption(FName(*(FString(TEXT("SettingsGraphics.Rows.DisplayMode.Option.")) + Suffix)), DisplayModeToText(Mode), [this, Mode]() { PendingGraphics.DisplayMode = Mode; PendingGraphics.bDirty = true; }, PendingGraphics.DisplayMode == Mode)
				];
			};
			Add(TEXT("Standard"), ET66DisplayMode::Standard);
			Add(TEXT("Widescreen"), ET66DisplayMode::Widescreen);
			return Box;
		}),
		130.f);

	AddRow(MakeStaticRow(DTag(TEXT("SettingsGraphics.Rows.UIStyle")), NSLOCTEXT("T66.Settings", "UiThemeFlat", "UI Style"), NSLOCTEXT("T66.Settings", "UiThemeUnifiedFlat", "Unified")), 76.f);
	AddRow(MakeQualityRow(DTag(TEXT("SettingsGraphics.Rows.Quality"))), 124.f);

	AddRow(MakeDropdownRow(
		DTag(TEXT("SettingsGraphics.Rows.FPSCap")),
		Loc ? Loc->GetText_FpsCap() : NSLOCTEXT("T66.Settings.Fallback", "FPS Cap Flat", "FPS Cap"),
		TAttribute<FText>::CreateLambda([this, FpsCapToText]() { return FpsCapToText(PendingGraphics.FpsCapIndex); }),
		[this, FpsCapToText, &MakeOption]()
		{
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
			for (int32 Index = 0; Index < 5; ++Index)
			{
				const int32 FpsIndex = Index;
				Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeOption(FName(*FString::Printf(TEXT("SettingsGraphics.Rows.FPSCap.Option.%d"), FpsIndex)), FpsCapToText(FpsIndex), [this, FpsIndex]() { PendingGraphics.FpsCapIndex = FpsIndex; PendingGraphics.bDirty = true; }, PendingGraphics.FpsCapIndex == FpsIndex)
				];
			}
			return Box;
		}),
		130.f);

	AddRow(MakeFogRow(DTag(TEXT("SettingsGraphics.Rows.Fog"))), 102.f);

	AddRow(
		FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Selected,
			Loc ? Loc->GetText_Apply() : NSLOCTEXT("T66.Settings.Fallback", "APPLY Flat", "APPLY"),
			FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleApplyGraphicsClicked),
			nullptr,
			nullptr,
			FMargin(18.f, 8.f),
			180.f,
			58.f,
			true,
			22,
			DTag(TEXT("SettingsGraphics.ApplyButton"))),
		72.f,
		0.f);

	AddN(0.000f, 0.000f, 1.000f, 1.000f,
		FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black),
			DTag(TEXT("SettingsGraphics.Background")),
			TEXT("Background"),
			ET66FlatState::Default));

	AddN(0.000f, 0.095f, 1.000f, 0.905f, MakeMetadataRegion(DTag(TEXT("SettingsGraphics.Root")), TEXT("Root")));
	AddN(0.003f, 0.094f, 0.994f, 0.079f, MakeMetadataRegion(DTag(TEXT("SettingsGraphics.SettingsTabs")), TEXT("ToggleGroup.SettingsTabs")));

	AddN(0.003f, 0.094f, 0.119f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGraphics.SettingsTabs.GameplayButton")), ET66SettingsTab::Gameplay, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabGameplayFlatGraphics", "GAMEPLAY"), 0.119f * CanvasW));
	AddN(0.129f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGraphics.SettingsTabs.GraphicsButton")), ET66SettingsTab::Graphics, ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "TabGraphicsFlatGraphics", "GRAPHICS"), 0.118f * CanvasW));
	AddN(0.253f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGraphics.SettingsTabs.ControlsButton")), ET66SettingsTab::Controls, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabControlsFlatGraphics", "CONTROLS"), 0.118f * CanvasW));
	AddN(0.379f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGraphics.SettingsTabs.HUDButton")), ET66SettingsTab::HUD, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabHUDFlatGraphics", "HUD"), 0.118f * CanvasW));
	AddN(0.503f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGraphics.SettingsTabs.MediaViewerButton")), ET66SettingsTab::MediaViewer, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabMediaViewerFlatGraphics", "MEDIA VIEWER"), 0.118f * CanvasW, 18));
	AddN(0.628f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGraphics.SettingsTabs.AudioButton")), ET66SettingsTab::Audio, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabAudioFlatGraphics", "AUDIO"), 0.118f * CanvasW));
	AddN(0.754f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGraphics.SettingsTabs.CrashingButton")), ET66SettingsTab::Crashing, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabCrashingFlatGraphics", "CRASHING"), 0.118f * CanvasW, 20));
	AddN(0.879f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGraphics.SettingsTabs.RetroFXButton")), ET66SettingsTab::RetroFX, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabRetroFXFlatGraphics", "RETRO FX"), 0.118f * CanvasW, 20));

	AddN(0.002f, 0.194f, 0.978f, 0.753f,
		FT66FlatStyle::AttachMetadata(
			SNew(SScrollBox)
			.ScrollBarVisibility(EVisibility::Visible)
			+ SScrollBox::Slot()
			[
				Rows
			],
			DTag(TEXT("SettingsGraphics.ContentScroll")),
			TEXT("ScrollBox"),
			ET66FlatState::Default));

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

TSharedRef<SWidget> UT66SettingsScreen::BuildGraphicsTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	InitializeGraphicsFromUserSettingsIfNeeded();

	auto FpsCapToText = [Loc](int32 Index) -> FText
	{
		switch (Index)
		{
		case 0: return FText::AsNumber(30);
		case 1: return FText::AsNumber(60);
		case 2: return FText::AsNumber(90);
		case 3: return FText::AsNumber(120);
		default: return Loc ? Loc->GetText_Unlimited() : NSLOCTEXT("T66.Settings", "Unlimited", "Unlimited");
		}
	};

	auto WindowModeToText = [Loc](EWindowMode::Type Mode) -> FText
	{
		if (!Loc)
		{
			switch (Mode)
			{
			case EWindowMode::Windowed: return NSLOCTEXT("T66.Settings", "Windowed", "Windowed");
			case EWindowMode::WindowedFullscreen: return NSLOCTEXT("T66.Settings", "BorderlessWindowed", "Borderless Windowed");
			default: return NSLOCTEXT("T66.Settings", "Fullscreen", "Fullscreen");
			}
		}
		switch (Mode)
		{
		case EWindowMode::Windowed: return Loc->GetText_Windowed();
		case EWindowMode::WindowedFullscreen: return Loc->GetText_BorderlessWindowed();
		default: return Loc->GetText_Fullscreen();
		}
	};

	auto DisplayModeToText = [Loc](ET66DisplayMode Mode) -> FText
	{
		if (!Loc)
		{
			return Mode == ET66DisplayMode::Widescreen ? NSLOCTEXT("T66.Settings", "Widescreen", "Widescreen") : NSLOCTEXT("T66.Settings", "Standard", "Standard");
		}
		return Mode == ET66DisplayMode::Widescreen ? Loc->GetText_DisplayModeWidescreen() : Loc->GetText_DisplayModeStandard();
	};

	auto ResToText = [](FIntPoint R) -> FText
	{
		return FText::Format(NSLOCTEXT("T66.Settings", "ResolutionFormat", "{0} x {1}"), FText::AsNumber(R.X), FText::AsNumber(R.Y));
	};

	TSharedRef<SOverlay> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_Monitor() : NSLOCTEXT("T66.Settings", "Monitor", "Monitor"),
				[this, Loc]()
				{
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					const int32 Idx = FMath::Clamp(PendingGraphics.MonitorIndex, 0, FMath::Max(0, Metrics.MonitorInfo.Num() - 1));
					if (Idx == 0) return Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor");
					return FText::Format(NSLOCTEXT("T66.Settings", "MonitorN", "Monitor {0}"), FText::AsNumber(Idx + 1));
				},
				[this, Loc](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (int32 i = 0; i < Metrics.MonitorInfo.Num(); ++i)
					{
						const FText Label = (i == 0) ? (Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor"))
							: FText::Format(NSLOCTEXT("T66.Settings", "MonitorN", "Monitor {0}"), FText::AsNumber(i + 1));
						const int32 MonitorIdx = i;
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								Label,
								FOnClicked::CreateLambda([this, MonitorIdx, CurrentValueText, Loc]()
								{
									PendingGraphics.MonitorIndex = MonitorIdx;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(MonitorIdx == 0
											? (Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor"))
											: FText::Format(NSLOCTEXT("T66.Settings", "MonitorN", "Monitor {0}"), FText::AsNumber(MonitorIdx + 1)));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.MonitorIndex == MonitorIdx,
								0.f,
								34.f,
								14)
						];
					}
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_Resolution() : NSLOCTEXT("T66.Settings.Fallback", "Resolution", "Resolution"),
				[this, ResToText]() { return ResToText(PendingGraphics.Resolution); },
				[this, ResToText](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					static const FIntPoint ResList[] = {
						FIntPoint(1280, 720),
						FIntPoint(1600, 900),
						FIntPoint(1920, 1080),
						FIntPoint(2560, 1440),
						FIntPoint(3840, 2160)
					};
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (const FIntPoint& R : ResList)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								ResToText(R),
								FOnClicked::CreateLambda([this, R, CurrentValueText, ResToText]()
								{
									PendingGraphics.Resolution = R;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(ResToText(R));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.Resolution == R,
								0.f,
								34.f,
								14)
						];
					}
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_WindowMode() : NSLOCTEXT("T66.Settings.Fallback", "Window Mode", "Window Mode"),
				[this, WindowModeToText]() { return WindowModeToText(PendingGraphics.WindowMode); },
				[this, WindowModeToText](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					auto Add = [this, &Box, WindowModeToText, CurrentValueText](EWindowMode::Type Mode)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								WindowModeToText(Mode),
								FOnClicked::CreateLambda([this, Mode, CurrentValueText, WindowModeToText]()
								{
									PendingGraphics.WindowMode = Mode;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(WindowModeToText(Mode));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.WindowMode == Mode,
								0.f,
								34.f,
								14)
						];
					};
					Add(EWindowMode::Windowed);
					Add(EWindowMode::Fullscreen);
					Add(EWindowMode::WindowedFullscreen);
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_DisplayMode() : NSLOCTEXT("T66.Settings.Fallback", "Display Mode", "Display Mode"),
				[this, DisplayModeToText]() { return DisplayModeToText(PendingGraphics.DisplayMode); },
				[this, DisplayModeToText](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					auto Add = [this, &Box, DisplayModeToText, CurrentValueText](ET66DisplayMode Mode)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								DisplayModeToText(Mode),
								FOnClicked::CreateLambda([this, Mode, CurrentValueText, DisplayModeToText]()
								{
									PendingGraphics.DisplayMode = Mode;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(DisplayModeToText(Mode));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.DisplayMode == Mode,
								0.f,
								34.f,
								14)
						];
					};
					Add(ET66DisplayMode::Standard);
					Add(ET66DisplayMode::Widescreen);
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(SBox).MinDesiredWidth(200.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Settings", "UiTheme", "UI Style"))
						.Font(SettingsRegularFont(22))
						.ColorAndOpacity(GetSettingsPageText())
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Settings", "UiThemeUnified", "Unified"))
					.Font(SettingsRegularFont(18))
					.ColorAndOpacity(GetSettingsPageMuted())
				]
			]
		]
		// Quality slider
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(STextBlock).Text(Loc ? Loc->GetText_BestPerformance() : NSLOCTEXT("T66.Settings.Fallback", "Best Performance", "Best Performance"))
						.Font(SettingsRegularFont(16))
						.ColorAndOpacity(GetSettingsPageMuted())
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)
					[
						SNew(STextBlock).Text(Loc ? Loc->GetText_BestQuality() : NSLOCTEXT("T66.Settings.Fallback", "Best Quality", "Best Quality"))
						.Font(SettingsRegularFont(16))
						.ColorAndOpacity(GetSettingsPageMuted())
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					MakeSettingsReferenceSlider(
						TAttribute<float>::CreateLambda([this]() -> float
						{
							return static_cast<float>(PendingGraphics.QualityNotch) / 3.0f;
						}),
						1.0f / 3.0f,
						FOnFloatValueChanged::CreateLambda([this](float V)
						{
							PendingGraphics.QualityNotch = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3);
							PendingGraphics.bDirty = true;
						}),
						FSimpleDelegate(),
						FSimpleDelegate(),
						true)
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_FpsCap() : NSLOCTEXT("T66.Settings.Fallback", "FPS Cap", "FPS Cap"),
				[this, FpsCapToText]() { return FpsCapToText(PendingGraphics.FpsCapIndex); },
				[this, FpsCapToText](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (int32 i = 0; i < 5; ++i)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								FpsCapToText(i),
								FOnClicked::CreateLambda([this, i, CurrentValueText, FpsCapToText]()
								{
									PendingGraphics.FpsCapIndex = i;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(FpsCapToText(i));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.FpsCapIndex == i,
								0.f,
								34.f,
								14)
						];
					}
					return Box;
				}
			)
		]
		// Fog toggle
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsToggleRow(
				Loc,
				Loc ? Loc->GetText_SettingsFog() : NSLOCTEXT("T66.Settings", "Fog", "Fog"),
				[PS]() { return PS ? PS->GetFogEnabled() : false; },
				[PS](bool bEnabled)
				{
					if (PS)
					{
						PS->SetFogEnabled(bEnabled);
					}
				},
				FText(),
				FSettingsBoolToggleStyle())
		]
		// Apply button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f)
		[
			MakeSettingsButton(
				FT66ButtonParams(Loc ? Loc->GetText_Apply() : NSLOCTEXT("T66.Settings.Fallback", "APPLY", "APPLY"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleApplyGraphicsClicked), ET66ButtonType::Success)
				.SetFontSize(20).SetMinWidth(110.f))
		]
	];

	// Confirm overlay (10s auto-revert)
	Root->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SAssignNew(VideoModeConfirmOverlay, SBorder)
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
		.Visibility(bVideoModeConfirmActive ? EVisibility::Visible : EVisibility::Collapsed)
		.Padding(FMargin(30.f))
		[
			SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(FMargin(25.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_KeepTheseSettingsTitle() : NSLOCTEXT("T66.Settings.Fallback", "Keep these settings?", "Keep these settings?"))
					.Font(SettingsBoldFont(24))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 15.f)
				[
					SAssignNew(VideoModeConfirmCountdownText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(SettingsRegularFont(18))
					.ColorAndOpacity(GetSettingsPageMuted())
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(6.f)
					[
					MakeSettingsButton(Loc ? Loc->GetText_Keep() : NSLOCTEXT("T66.Settings.Fallback", "KEEP", "KEEP"),
						FOnClicked::CreateLambda([this]() { EndVideoModeConfirmPrompt(true); return FReply::Handled(); }),
						ET66ButtonType::Success, 100.f)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(6.f)
					[
					MakeSettingsButton(Loc ? Loc->GetText_Revert() : NSLOCTEXT("T66.Settings.Fallback", "REVERT", "REVERT"),
						FOnClicked::CreateLambda([this]() { EndVideoModeConfirmPrompt(false); return FReply::Handled(); }),
						ET66ButtonType::Danger, 100.f)
					]
				]
			]
		]
	];

	return Root;
}

FReply UT66SettingsScreen::HandleApplyGraphicsClicked()
{
	OnApplyGraphicsClicked();
	return FReply::Handled();
}

void UT66SettingsScreen::OnApplyGraphicsClicked()
{
	ApplyPendingGraphics(false);
}

void UT66SettingsScreen::InitializeGraphicsFromUserSettingsIfNeeded()
{
	if (bGraphicsInitialized) return;
	bGraphicsInitialized = true;

	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		PendingGraphics.Resolution = GUS->GetScreenResolution();
		PendingGraphics.WindowMode = GUS->GetFullscreenMode();
		PendingGraphics.QualityNotch = FMath::Clamp(GUS->GetOverallScalabilityLevel(), 0, 3);
		const float Limit = GUS->GetFrameRateLimit();
		if (Limit <= 0.0f) PendingGraphics.FpsCapIndex = 4;
		else if (Limit <= 30.0f) PendingGraphics.FpsCapIndex = 0;
		else if (Limit <= 60.0f) PendingGraphics.FpsCapIndex = 1;
		else if (Limit <= 90.0f) PendingGraphics.FpsCapIndex = 2;
		else PendingGraphics.FpsCapIndex = 3;
	}
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PendingGraphics.DisplayMode = (PS->GetDisplayModeIndex() == 1) ? ET66DisplayMode::Widescreen : ET66DisplayMode::Standard;
		PendingGraphics.MonitorIndex = PS->GetMonitorIndex();
	}
	else
	{
		PendingGraphics.DisplayMode = ET66DisplayMode::Standard;
		PendingGraphics.MonitorIndex = 0;
	}
	PendingGraphics.bDirty = false;
}

void UT66SettingsScreen::ApplyPendingGraphics(bool bForceConfirmPrompt)
{
	if (!PendingGraphics.bDirty && !bForceConfirmPrompt) return;

	UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GUS) return;

	const FIntPoint OldRes = GUS->GetScreenResolution();
	const EWindowMode::Type OldMode = GUS->GetFullscreenMode();

	GUS->SetScreenResolution(PendingGraphics.Resolution);
	GUS->SetFullscreenMode(PendingGraphics.WindowMode);
	GUS->SetOverallScalabilityLevel(FMath::Clamp(PendingGraphics.QualityNotch, 0, 3));

	float NewLimit = 60.0f;
	switch (PendingGraphics.FpsCapIndex)
	{
	case 0: NewLimit = 30.0f; break;
	case 1: NewLimit = 60.0f; break;
	case 2: NewLimit = 90.0f; break;
	case 3: NewLimit = 120.0f; break;
	default: NewLimit = 0.0f; break; // Unlimited
	}
	GUS->SetFrameRateLimit(NewLimit);

	GUS->ApplySettings(false);

	// Persist display mode (Standard/Widescreen) for future viewport/camera use.
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->SetDisplayModeIndex(static_cast<int32>(PendingGraphics.DisplayMode));
		PS->SetMonitorIndex(PendingGraphics.MonitorIndex);
	}

	// Move game window to selected monitor when multiple monitors are available.
	FDisplayMetrics Metrics;
	FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
	if (Metrics.MonitorInfo.IsValidIndex(PendingGraphics.MonitorIndex))
	{
		const FMonitorInfo& Mon = Metrics.MonitorInfo[PendingGraphics.MonitorIndex];
		TArray<TSharedRef<SWindow>> TopLevelWindows = FSlateApplication::Get().GetTopLevelWindows();
		if (TopLevelWindows.Num() > 0)
		{
			TSharedRef<SWindow> GameWindow = TopLevelWindows[0];
			const FVector2D Pos(static_cast<float>(Mon.WorkArea.Left), static_cast<float>(Mon.WorkArea.Top));
			GameWindow->MoveWindowTo(Pos);
		}
	}

	const bool bNeedsConfirm = bForceConfirmPrompt || (OldRes != PendingGraphics.Resolution) || (OldMode != PendingGraphics.WindowMode);
	if (bNeedsConfirm)
	{
		StartVideoModeConfirmPrompt();
	}
	else
	{
		GUS->SaveSettings();
		PendingGraphics.bDirty = false;
	}
}

void UT66SettingsScreen::StartVideoModeConfirmPrompt()
{
	if (bVideoModeConfirmActive) return;
	bVideoModeConfirmActive = true;
	VideoModeConfirmSecondsRemaining = 10;
	if (VideoModeConfirmOverlay.IsValid())
	{
		VideoModeConfirmOverlay->SetVisibility(EVisibility::Visible);
	}

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		if (VideoModeConfirmCountdownText.IsValid())
		{
			VideoModeConfirmCountdownText->SetText(FText::Format(Loc->GetText_KeepTheseSettingsBodyFormat(), FText::AsNumber(VideoModeConfirmSecondsRemaining)));
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VideoModeConfirmTimerHandle);
		FTimerDelegate TickDelegate;
		TickDelegate.BindLambda([this]()
		{
			VideoModeConfirmSecondsRemaining = FMath::Max(0, VideoModeConfirmSecondsRemaining - 1);
			if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
			{
				if (VideoModeConfirmCountdownText.IsValid())
				{
					VideoModeConfirmCountdownText->SetText(FText::Format(Loc->GetText_KeepTheseSettingsBodyFormat(), FText::AsNumber(VideoModeConfirmSecondsRemaining)));
				}
			}
			if (VideoModeConfirmSecondsRemaining <= 0)
			{
				EndVideoModeConfirmPrompt(false);
			}
		});
		World->GetTimerManager().SetTimer(VideoModeConfirmTimerHandle, TickDelegate, 1.0f, true);
	}
}

void UT66SettingsScreen::EndVideoModeConfirmPrompt(bool bKeepNewSettings)
{
	if (!bVideoModeConfirmActive) return;
	bVideoModeConfirmActive = false;
	if (VideoModeConfirmOverlay.IsValid())
	{
		VideoModeConfirmOverlay->SetVisibility(EVisibility::Collapsed);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VideoModeConfirmTimerHandle);
	}

	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		if (bKeepNewSettings)
		{
			GUS->ConfirmVideoMode();
			GUS->SaveSettings();
			PendingGraphics.bDirty = false;
		}
		else
		{
			GUS->RevertVideoMode();
			GUS->ApplySettings(false);
			// Sync pending back to current applied.
			PendingGraphics.Resolution = GUS->GetScreenResolution();
			PendingGraphics.WindowMode = GUS->GetFullscreenMode();
			PendingGraphics.bDirty = false;
		}
	}
}


