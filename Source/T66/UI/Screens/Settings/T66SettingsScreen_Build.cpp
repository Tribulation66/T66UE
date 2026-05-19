// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
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

	if (CurrentTab == ET66SettingsTab::HUD)
	{
		return BuildFlatHUDSettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::MediaViewer)
	{
		return BuildFlatMediaViewerSettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::Audio)
	{
		return BuildFlatAudioSettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::Crashing)
	{
		return BuildFlatCrashingSettingsUI();
	}

	if (CurrentTab == ET66SettingsTab::RetroFX)
	{
		InitializeRetroFXFromUserSettingsIfNeeded();

		constexpr float CanvasW = 1920.f;
		constexpr float CanvasH = 1080.f;
		const FName FrontendGroup(TEXT("RetroFXFrontendEnable"));
		const FName GameplayGroup(TEXT("RetroFXGameplayEnable"));

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

		auto SetFrontendRetroFX = [this](const bool bEnabled) -> FReply
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			PendingRetroFXSettings.UIFullScreenCRTEnabled = bEnabled;
			MarkRetroFXEdited();
			ApplyPendingRetroFX();
			ForceRebuildSlate();
			return FReply::Handled();
		};

		auto SetGameplayRetroFX = [this](const bool bEnabled) -> FReply
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			PendingRetroFXSettings.bEnableRetroFXMaster = bEnabled;
			MarkRetroFXEdited();
			ApplyPendingRetroFX();
			ForceRebuildSlate();
			return FReply::Handled();
		};

		AddN(0.000f, 0.000f, 1.000f, 1.000f,
			FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor::Black),
				DTag(TEXT("SettingsRetroFX.Background")),
				TEXT("Background"),
				ET66FlatState::Default));

		AddN(0.000f, 0.095f, 1.000f, 0.905f, MakeMetadataRegion(DTag(TEXT("SettingsRetroFX.Root")), TEXT("Root")));
		AddStableSettingsTabRow(Canvas, this, TEXT("SettingsRetroFX"), CurrentTab, CanvasW, CanvasH);

		const bool bFrontendEnabled = PendingRetroFXSettings.UIFullScreenCRTEnabled;
		const bool bGameplayEnabled = PendingRetroFXSettings.bEnableRetroFXMaster;

		AddN(0.013f, 0.223f, 0.974f, 0.160f, MakePanel(DTag(TEXT("SettingsRetroFX.FrontendPanel"))));
		AddN(0.033f, 0.260f, 0.420f, 0.042f, MakeLabel(DTag(TEXT("SettingsRetroFX.FrontendPanel.Label")), NSLOCTEXT("T66.Settings", "RetroFXFrontendLabelFlat", "FRONTEND RETRO FX"), 30, FT66FlatStyle::PrimaryText(), true));
		AddN(0.598f, 0.260f, 0.160f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.FrontendPanel.OnButton")), FrontendGroup, bFrontendEnabled ? ET66FlatState::Selected : ET66FlatState::Default, NSLOCTEXT("T66.Settings", "RetroFXOnFlat", "ON"), FOnClicked::CreateLambda([SetFrontendRetroFX]() { return SetFrontendRetroFX(true); }), 0.160f * CanvasW, 0.064f * CanvasH));
		AddN(0.774f, 0.260f, 0.160f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.FrontendPanel.OffButton")), FrontendGroup, bFrontendEnabled ? ET66FlatState::Default : ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "RetroFXOffFlat", "OFF"), FOnClicked::CreateLambda([SetFrontendRetroFX]() { return SetFrontendRetroFX(false); }), 0.160f * CanvasW, 0.064f * CanvasH));

		AddN(0.013f, 0.411f, 0.974f, 0.160f, MakePanel(DTag(TEXT("SettingsRetroFX.GameplayPanel"))));
		AddN(0.033f, 0.448f, 0.420f, 0.042f, MakeLabel(DTag(TEXT("SettingsRetroFX.GameplayPanel.Label")), NSLOCTEXT("T66.Settings", "RetroFXGameplayLabelFlat", "GAMEPLAY RETRO FX"), 30, FT66FlatStyle::PrimaryText(), true));
		AddN(0.598f, 0.448f, 0.160f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.GameplayPanel.OnButton")), GameplayGroup, bGameplayEnabled ? ET66FlatState::Selected : ET66FlatState::Default, NSLOCTEXT("T66.Settings", "RetroFXOnFlat", "ON"), FOnClicked::CreateLambda([SetGameplayRetroFX]() { return SetGameplayRetroFX(true); }), 0.160f * CanvasW, 0.064f * CanvasH));
		AddN(0.774f, 0.448f, 0.160f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.GameplayPanel.OffButton")), GameplayGroup, bGameplayEnabled ? ET66FlatState::Default : ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "RetroFXOffFlat", "OFF"), FOnClicked::CreateLambda([SetGameplayRetroFX]() { return SetGameplayRetroFX(false); }), 0.160f * CanvasW, 0.064f * CanvasH));

		AddN(0.448f, 0.860f, 0.160f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.ResetButton")), NAME_None, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "RetroFXResetFlat", "RESET"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleResetRetroFXClicked), 0.160f * CanvasW, 0.064f * CanvasH));
		AddN(0.624f, 0.860f, 0.160f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.CancelButton")), NAME_None, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "RetroFXCancelFlat", "CANCEL"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseClicked), 0.160f * CanvasW, 0.064f * CanvasH));
		AddN(0.800f, 0.860f, 0.160f, 0.064f, MakeButton(DTag(TEXT("SettingsRetroFX.ApplyButton")), NAME_None, ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "RetroFXApplyFlat", "APPLY"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleApplyRetroFXClicked), 0.160f * CanvasW, 0.064f * CanvasH));

		const FVector2D ViewportSize = FT66Style::GetViewportLogicalSize();
		const float RootW = FMath::Max(CanvasW, ViewportSize.X);
		const float RootH = FMath::Max(CanvasH, ViewportSize.Y);

		return SNew(SBox)
			.WidthOverride(RootW)
			.HeightOverride(RootH)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor::Black)
					.Padding(0.f)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
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
		.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
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


