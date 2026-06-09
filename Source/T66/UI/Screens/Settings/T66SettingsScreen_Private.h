// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RuntimePlatformSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/GameUserSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericApplication.h"
#include "Styling/SlateTypes.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66Style.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace T66SettingsScreenPrivate
{
	constexpr int32 SettingsFontDelta = -2;
	constexpr int32 SettingsCompactButtonFontSize = 19;
	constexpr float SettingsCompactButtonHeight = 46.f;
	inline int32 AdjustSettingsFontSize(int32 BaseSize)
	{
		return FMath::Max(8, BaseSize + SettingsFontDelta);
	}

	inline FSlateFontInfo SettingsRegularFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontRegular(AdjustSettingsFontSize(BaseSize));
	}

	inline FSlateFontInfo SettingsBoldFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontBold(AdjustSettingsFontSize(BaseSize));
	}

	inline FLinearColor T66SettingsShellFill()
	{
		return FT66FlatStyle::BackgroundColor();
	}

	inline FLinearColor T66SettingsPageFill()
	{
		return FT66FlatStyle::DefaultFill();
	}

	inline FLinearColor T66SettingsRowFill()
	{
		return FT66FlatStyle::DefaultFill();
	}

	inline FLinearColor T66SettingsButtonNeutralFill()
	{
		return FT66FlatStyle::DefaultFill();
	}

	inline FLinearColor T66SettingsButtonOutline()
	{
		return FT66FlatStyle::DefaultBorder();
	}

	inline FLinearColor GetSettingsPageBackground()
	{
		return T66SettingsPageFill();
	}

	inline FLinearColor GetSettingsPageText()
	{
		return FT66FlatStyle::PrimaryText();
	}

	inline FLinearColor GetSettingsPageMuted()
	{
		return FT66FlatStyle::SecondaryText();
	}

	inline FLinearColor GetSettingsButtonText()
	{
		return FT66FlatStyle::PrimaryText();
	}

	inline FText GetSettingsOnText(UT66LocalizationSubsystem* Loc)
	{
		return Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON");
	}

	inline FText GetSettingsOffText(UT66LocalizationSubsystem* Loc)
	{
		return Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF");
	}

	inline FName MakeSettingsChildTag(const FName& BaseTag, const TCHAR* Suffix)
	{
		return FName(*(BaseTag.ToString() + FString(TEXT(".")) + Suffix));
	}

	inline TSharedRef<SWidget> MakeStableSettingsTabLabel(
		const FName Tag,
		const FText& Text,
		const int32 FontSize,
		const ET66FlatState State)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Visibility(EVisibility::HitTestInvisible)
			.Text(Text)
			.Font(FT66FlatStyle::MakeBoldFont(FontSize))
			.ColorAndOpacity(State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText())
			.Justification(ETextJustify::Center)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds),
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	}

	inline TSharedRef<SWidget> MakeStableSettingsTabButton(
		UT66SettingsScreen* Screen,
		const FName Tag,
		const ET66SettingsTab TargetTab,
		const ET66SettingsTab ActiveTab,
		const FText& Label,
		const float Width,
		const float Height,
		const int32 FontSize)
	{
		const ET66FlatState State = ActiveTab == TargetTab ? ET66FlatState::Selected : ET66FlatState::Default;
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeStableSettingsTabLabel(MakeSettingsChildTag(Tag, TEXT("Label")), Label, FontSize, State)
			],
			FOnClicked::CreateLambda([Screen, TargetTab]()
			{
				if (Screen)
				{
					Screen->SwitchToTab(TargetTab);
				}
				return FReply::Handled();
			}),
			FMargin(0.f),
			Width,
			Height,
			true,
			Tag,
			FName(TEXT("SettingsTabs")));
	}

	inline void AddStableSettingsTabRow(
		const TSharedRef<SConstraintCanvas>& Canvas,
		UT66SettingsScreen* Screen,
		const TCHAR* TagPrefix,
		const ET66SettingsTab ActiveTab,
		const float CanvasW = 1920.f,
		const float CanvasH = 1080.f)
	{
		struct FStableSettingsTabSpec
		{
			const TCHAR* Suffix;
			ET66SettingsTab Tab;
			FText Label;
			float X;
			float Width;
			int32 FontSize;
		};

		const FString Prefix(TagPrefix);
		auto MakeTag = [&Prefix](const TCHAR* Suffix) -> FName
		{
			return FName(*(Prefix + FString(TEXT(".")) + Suffix));
		};
		auto AddN = [&Canvas, CanvasW, CanvasH](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			Canvas->AddSlot()
			.Anchors(FAnchors(X, Y, X, Y))
			.Alignment(FVector2D::ZeroVector)
			.Offset(FMargin(0.f, 0.f, W * CanvasW, H * CanvasH))
			[
				Widget
			];
		};

		constexpr float StableSettingsTabY = 0.123f;
		AddN(0.003f, StableSettingsTabY, 0.994f, 0.079f,
			FT66FlatStyle::AttachMetadata(
				SNew(SBox),
				MakeTag(TEXT("SettingsTabs")),
				TEXT("ToggleGroup.SettingsTabs"),
				ET66FlatState::Default));

		const FStableSettingsTabSpec Specs[] = {
			{ TEXT("SettingsTabs.GameplayButton"), ET66SettingsTab::Gameplay, NSLOCTEXT("T66.Settings", "StableTabGameplay", "GAMEPLAY"), 0.003f, 0.119f, 22 },
			{ TEXT("SettingsTabs.GraphicsButton"), ET66SettingsTab::Graphics, NSLOCTEXT("T66.Settings", "StableTabGraphics", "GRAPHICS"), 0.129f, 0.118f, 22 },
			{ TEXT("SettingsTabs.ControlsButton"), ET66SettingsTab::Controls, NSLOCTEXT("T66.Settings", "StableTabControls", "CONTROLS"), 0.253f, 0.118f, 22 },
			{ TEXT("SettingsTabs.HUDButton"), ET66SettingsTab::HUD, NSLOCTEXT("T66.Settings", "StableTabHUD", "HUD"), 0.379f, 0.118f, 22 },
			{ TEXT("SettingsTabs.MediaViewerButton"), ET66SettingsTab::MediaViewer, NSLOCTEXT("T66.Settings", "StableTabMediaViewer", "MEDIA VIEWER"), 0.503f, 0.118f, 18 },
			{ TEXT("SettingsTabs.AudioButton"), ET66SettingsTab::Audio, NSLOCTEXT("T66.Settings", "StableTabAudio", "AUDIO"), 0.628f, 0.118f, 22 },
			{ TEXT("SettingsTabs.CrashingButton"), ET66SettingsTab::Crashing, NSLOCTEXT("T66.Settings", "StableTabCrashing", "CRASHING"), 0.754f, 0.118f, 20 },
		};

		for (const FStableSettingsTabSpec& Spec : Specs)
		{
			AddN(Spec.X, StableSettingsTabY, Spec.Width, 0.079f,
				MakeStableSettingsTabButton(
					Screen,
					MakeTag(Spec.Suffix),
					Spec.Tab,
					ActiveTab,
					Spec.Label,
					Spec.Width * CanvasW,
					0.079f * CanvasH,
					Spec.FontSize));
		}
	}

	inline TAttribute<FSlateColor> MakeSelectedButtonColor(
		TFunction<bool()> IsSelected,
		const FSlateColor& SelectedColor = FSlateColor(FT66FlatStyle::SelectedFill()),
		const FSlateColor& UnselectedColor = FSlateColor(T66SettingsButtonNeutralFill()))
	{
		return TAttribute<FSlateColor>::CreateLambda([IsSelected, SelectedColor, UnselectedColor]() -> FSlateColor
		{
			return IsSelected() ? SelectedColor : UnselectedColor;
		});
	}

	inline TSharedRef<SWidget> MakeSettingsPanel(const TSharedRef<SWidget>& Content, ET66PanelType Type, const FLinearColor& FillColor, const FMargin& Padding)
	{
		(void)Type;
		(void)FillColor;
		return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, Padding, Content);
	}

	inline TSharedRef<SWidget> MakeSettingsContentShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, Padding, Content);
	}

	inline TSharedRef<SWidget> MakeSettingsButton(const FT66ButtonParams& Params)
	{
		const int32 FontSize = Params.FontSize > 0 ? static_cast<int32>(Params.FontSize) : SettingsCompactButtonFontSize;
		const float Height = Params.Height > 0.f ? Params.Height : SettingsCompactButtonHeight;
		const FMargin Padding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(10.f, 5.f);
		const ET66FlatState State =
			(Params.Type == ET66ButtonType::Primary || Params.Type == ET66ButtonType::Success || Params.Type == ET66ButtonType::Danger || Params.Type == ET66ButtonType::ToggleActive)
				? ET66FlatState::Selected
				: ET66FlatState::Default;
		const TAttribute<FText> Text = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);

		TSharedRef<SWidget> Button = Params.CustomContent.IsValid()
			? FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				Params.CustomContent.ToSharedRef(),
				Params.OnClicked,
				Padding,
				Params.MinWidth,
				Height,
				Params.IsEnabled)
			: FT66FlatStyle::MakeFlatButton(
				State,
				Text,
				Params.OnClicked,
				nullptr,
				nullptr,
				Padding,
				Params.MinWidth,
				Height,
				Params.IsEnabled,
				FontSize);

		return SNew(SBox)
			.Visibility(Params.Visibility)
			[
				Button
			];
	}

	inline TSharedRef<SWidget> MakeSettingsButton(const FText& Label, FOnClicked OnClicked, ET66ButtonType Type = ET66ButtonType::Neutral, float MinWidth = 120.f)
	{
		return MakeSettingsButton(FT66ButtonParams(Label, MoveTemp(OnClicked), Type).SetMinWidth(MinWidth));
	}

	inline TSharedRef<SWidget> MakeSelectableSettingsButton(
		const FT66ButtonParams& Params,
		TFunction<bool()> IsSelected,
		const FSlateColor& SelectedColor = FSlateColor(FT66FlatStyle::SelectedFill()),
		const FSlateColor& UnselectedColor = FSlateColor(T66SettingsButtonNeutralFill()))
	{
		(void)SelectedColor;
		(void)UnselectedColor;
		const float Height = Params.Height > 0.f ? Params.Height : SettingsCompactButtonHeight;
		const TAttribute<FText> Text = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		return SNew(SBox)
			.Visibility(Params.Visibility)
			[
				FT66FlatStyle::MakeFlatToggleButton(
					ET66FlatState::Default,
					Text,
					TAttribute<bool>::CreateLambda([IsSelected = MoveTemp(IsSelected)]() mutable -> bool
					{
						return IsSelected ? IsSelected() : false;
					}),
					Params.OnClicked,
					Params.MinWidth,
					Height)
			];
	}

	inline TSharedRef<SWidget> MakeSettingsRow(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(26.f, 24.f, 18.f, 24.f))
	{
		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, Padding, Content);
	}

	struct FSettingsBoolToggleStyle
	{
		float LabelMinWidth = 200.f;
		float ButtonMinWidth = 88.f;
		int32 ButtonFontSize = 0;
		FMargin ButtonPadding = FMargin(10.f, 5.f);
		float RightPadding = 10.f;
		FSlateColor OnSelectedColor = FSlateColor(FT66Style::Tokens::Success);
		FSlateColor OffSelectedColor = FSlateColor(FT66Style::Tokens::Danger);
		FSlateColor UnselectedColor = FSlateColor(T66SettingsButtonNeutralFill());
		FSlateColor TextColor = FSlateColor(GetSettingsButtonText());
	};

	inline TSharedRef<SWidget> MakeSettingsToggleButtons(
		UT66LocalizationSubsystem* Loc,
		TFunction<bool()> GetValue,
		TFunction<void(bool)> SetValue,
		const FSettingsBoolToggleStyle& Style = FSettingsBoolToggleStyle())
	{
		const TSharedRef<TFunction<bool()>> GetValueRef = MakeShared<TFunction<bool()>>(MoveTemp(GetValue));
		const TSharedRef<TFunction<void(bool)>> SetValueRef = MakeShared<TFunction<void(bool)>>(MoveTemp(SetValue));
		auto IsOn = [GetValueRef]() -> bool
		{
			return (*GetValueRef)();
		};
		auto IsOff = [GetValueRef]() -> bool
		{
			return !(*GetValueRef)();
		};

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				FT66FlatStyle::MakeFlatToggleButton(
					ET66FlatState::Default,
					TAttribute<FText>(GetSettingsOnText(Loc)),
					TAttribute<bool>::CreateLambda(IsOn),
					FOnClicked::CreateLambda([SetValueRef]()
					{
						(*SetValueRef)(true);
						return FReply::Handled();
					}),
					Style.ButtonMinWidth,
					SettingsCompactButtonHeight)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatToggleButton(
					ET66FlatState::Default,
					TAttribute<FText>(GetSettingsOffText(Loc)),
					TAttribute<bool>::CreateLambda(IsOff),
					FOnClicked::CreateLambda([SetValueRef]()
					{
						(*SetValueRef)(false);
						return FReply::Handled();
					}),
					Style.ButtonMinWidth,
					SettingsCompactButtonHeight)
			];
	}

	inline TSharedRef<SWidget> MakeSettingsToggleRow(
		UT66LocalizationSubsystem* Loc,
		const FText& Label,
		TFunction<bool()> GetValue,
		TFunction<void(bool)> SetValue,
		const FText& Description = FText(),
		const FSettingsBoolToggleStyle& Style = FSettingsBoolToggleStyle())
	{
		const bool bHasDescription = !Description.IsEmpty();
		const TSharedRef<SWidget> LabelContent = bHasDescription
			? StaticCastSharedRef<SWidget>(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 18.f, 0.f)
				[
					FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
					.Text(Description)
					.Font(SettingsRegularFont(16))
					.ColorAndOpacity(GetSettingsPageMuted())
					.AutoWrapText(true)), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.MinDesiredWidth(Style.LabelMinWidth)
				[
					FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
				]);

		return MakeSettingsRow(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				LabelContent
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(Style.RightPadding, 0.f, 0.f, 0.f)
			[
				MakeSettingsToggleButtons(Loc, MoveTemp(GetValue), MoveTemp(SetValue), Style)
			],
			FMargin(24.f, 14.f, 18.f, 12.f));
	}

	inline TSharedRef<SWidget> MakeSettingsSectionHeader(const FText& Text, const int32 FontSize = 22)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
				.Text(Text)
				.Font(SettingsBoldFont(FontSize))
				.ColorAndOpacity(GetSettingsPageText())), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 1.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(145.f)
				.HeightOverride(2.f)
				[
					FT66FlatStyle::MakeFlatDivider(Orient_Horizontal)
				]
			];
	}

	inline TSharedRef<SWidget> MakeDynamicOnOffButton(
		UT66LocalizationSubsystem* Loc,
		TFunction<bool()> GetValue,
		FOnClicked OnClicked)
	{
		return FT66FlatStyle::MakeFlatToggleButton(
			ET66FlatState::Default,
			TAttribute<FText>::CreateLambda([Loc, GetValue]() -> FText
			{
				return GetValue() ? GetSettingsOnText(Loc) : GetSettingsOffText(Loc);
			}),
			TAttribute<bool>::CreateLambda([GetValue]() -> bool
			{
				return GetValue();
			}),
			MoveTemp(OnClicked),
			0.f,
			SettingsCompactButtonHeight);
	}

	inline const FScrollBarStyle* GetSettingsFlatScrollBarStyle()
	{
		static FSlateColorBrush TrackBrush(FT66FlatStyle::DefaultFill());
		static FSlateColorBrush ThumbBrush(FT66FlatStyle::DefaultBorder());
		static FSlateColorBrush HoverBrush(FT66FlatStyle::HoverBorder());
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static bool bInitialized = false;
		if (!bInitialized)
		{
			Style
				.SetVerticalBackgroundImage(TrackBrush)
				.SetVerticalTopSlotImage(TrackBrush)
				.SetVerticalBottomSlotImage(TrackBrush)
				.SetNormalThumbImage(ThumbBrush)
				.SetHoveredThumbImage(HoverBrush)
				.SetDraggedThumbImage(HoverBrush)
				.SetThickness(14.f);
			bInitialized = true;
		}
		return &Style;
	}

	inline TSharedRef<SWidget> MakeSettingsFlatSlider(
		TAttribute<float> Value,
		float StepSize,
		FOnFloatValueChanged OnValueChanged,
		FSimpleDelegate OnMouseCaptureEnd = FSimpleDelegate(),
		FSimpleDelegate OnControllerCaptureEnd = FSimpleDelegate(),
		bool bMouseUsesStep = false)
	{
		const TSharedRef<SWidget> Slider = SNew(SSlider)
			.Value(Value)
			.StepSize(StepSize)
			.IndentHandle(false)
			.MouseUsesStep(bMouseUsesStep)
			.OnValueChanged(OnValueChanged)
			.OnMouseCaptureEnd(OnMouseCaptureEnd)
			.OnControllerCaptureEnd(OnControllerCaptureEnd);

		return FT66FlatStyle::AttachMetadata(
			FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(12.f, 6.f), Slider),
			NAME_None,
			TEXT("Slider"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			OnValueChanged.IsBound(),
			NAME_None,
			false,
			OnValueChanged.IsBound());
	}

	inline TSharedRef<SWidget> MakeSettingsPercentSliderRow(
		const FText& Label,
		const FText& Description,
		TFunction<float()> GetPercent,
		TFunction<void(float)> SetPercent,
		const FText& HelpText)
	{
		auto GetSnappedPercent = [GetPercent]()
		{
			return FMath::Clamp(FMath::RoundToInt(GetPercent()), 0, 100);
		};

		auto CommitSliderValue = [SetPercent](float NormalizedValue)
		{
			const int32 SnappedPercent = FMath::Clamp(FMath::RoundToInt(NormalizedValue * 100.0f), 0, 100);
			SetPercent(static_cast<float>(SnappedPercent));
		};

		return SNew(SBox)
			.HeightOverride(104.f)
			[
				MakeSettingsRow(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.46f).VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
							.Text(Label)
							.Font(SettingsBoldFont(20))
							.ColorAndOpacity(GetSettingsPageText())), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 18.f, 0.f)
						[
							FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
							.Text(Description)
							.Font(SettingsRegularFont(14))
							.ColorAndOpacity(GetSettingsPageMuted())
							.AutoWrapText(true)), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(0.54f).VAlign(VAlign_Center).Padding(10.f, 0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
							.Text_Lambda([GetSnappedPercent]()
							{
								return FText::AsNumber(GetSnappedPercent());
							})
							.Font(SettingsBoldFont(22))
							.ColorAndOpacity(GetSettingsPageText())
							.Justification(ETextJustify::Center)), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
						[
							MakeSettingsFlatSlider(
								TAttribute<float>::CreateLambda([GetSnappedPercent]() -> float
								{
									return static_cast<float>(GetSnappedPercent()) / 100.0f;
								}),
								0.01f,
								FOnFloatValueChanged::CreateLambda([CommitSliderValue](float Value)
								{
									CommitSliderValue(Value);
								}))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
						[
							FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
							.Text(HelpText)
							.Font(SettingsRegularFont(12))
							.ColorAndOpacity(GetSettingsPageMuted())
							.AutoWrapText(true)), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
						]
					],
					FMargin(22.f, 8.f, 16.f, 8.f))
			];
	}

	inline TSharedRef<SWidget> MakeSettingsDropdown(const FT66DropdownParams& Params);

	inline TSharedRef<SWidget> MakeSettingsDropdownRow(
		const FText& Label,
		TFunction<FText()> GetCurrentValue,
		TFunction<TSharedRef<SWidget>(const TSharedPtr<STextBlock>&)> MakeMenuContent,
		float LabelFillWidth = 0.4f,
		float ValueFillWidth = 0.6f,
		bool bUseEllipsis = false)
	{
		const TSharedRef<STextBlock> CurrentValueWidget = SNew(STextBlock)
			.Text(GetCurrentValue())
			.Font(SettingsRegularFont(18))
			.ColorAndOpacity(GetSettingsPageText());
		if (bUseEllipsis)
		{
			CurrentValueWidget->SetOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		}

		const TSharedPtr<STextBlock> CurrentValueText = CurrentValueWidget;
		const TSharedRef<SWidget> TriggerContent =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(FMargin(10.f, 4.f))
			[
				CurrentValueWidget
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(4.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(26.f)
				[
					SNew(SSpacer)
				]
			];

		return MakeSettingsRow(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(LabelFillWidth).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(SettingsRegularFont(22))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SHorizontalBox::Slot().FillWidth(ValueFillWidth)
			[
				MakeSettingsDropdown(FT66DropdownParams(
					TriggerContent,
					[MakeMenuContent, CurrentValueText]()
					{
						return MakeMenuContent(CurrentValueText);
					}).SetHeight(0.f))
			]);
	}

	struct FSettingsSliderRowParams
	{
		FText Label;
		float InitialValue = 0.f;
		TFunction<FText(float)> FormatValueText;
		TFunction<void(float)> OnValueChanged;
		TFunction<void()> OnMouseCaptureEnd;
		TFunction<void()> OnControllerCaptureEnd;
		FText HelpText;
		float LabelFillWidth = 0.3f;
		float SliderFillWidth = 0.55f;
		float ValueFillWidth = 0.15f;
		int32 LabelFontSize = 22;
		int32 ValueFontSize = 26;
		int32 HelpFontSize = 16;
	};

	inline TSharedRef<SWidget> MakeSettingsSliderRow(const FSettingsSliderRowParams& Params)
	{
		const TSharedRef<TSharedPtr<STextBlock>> ValueText = MakeShared<TSharedPtr<STextBlock>>();
		const TSharedRef<float> SliderValue = MakeShared<float>(FMath::Clamp(Params.InitialValue, 0.f, 1.f));
		TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

		Content->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(Params.LabelFillWidth).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(SettingsRegularFont(Params.LabelFontSize))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SHorizontalBox::Slot().FillWidth(Params.SliderFillWidth).VAlign(VAlign_Center).Padding(10.f, 0.f)
			[
				MakeSettingsFlatSlider(
					TAttribute<float>::CreateLambda([SliderValue]() -> float
					{
						return FMath::Clamp(*SliderValue, 0.f, 1.f);
					}),
					0.01f,
					FOnFloatValueChanged::CreateLambda([Params, ValueText, SliderValue](float Value)
					{
						*SliderValue = FMath::Clamp(Value, 0.f, 1.f);
						if (Params.OnValueChanged)
						{
							Params.OnValueChanged(Value);
						}
						if ((*ValueText).IsValid())
						{
							(*ValueText)->SetText(Params.FormatValueText ? Params.FormatValueText(Value) : FText::GetEmpty());
						}
					}),
					FSimpleDelegate::CreateLambda([Params]()
					{
						if (Params.OnMouseCaptureEnd)
						{
							Params.OnMouseCaptureEnd();
						}
					}),
					FSimpleDelegate::CreateLambda([Params]()
					{
						if (Params.OnControllerCaptureEnd)
						{
							Params.OnControllerCaptureEnd();
						}
					}))
			]
			+ SHorizontalBox::Slot().FillWidth(Params.ValueFillWidth).VAlign(VAlign_Center)
			[
				SAssignNew(*ValueText, STextBlock)
				.Text(Params.FormatValueText ? Params.FormatValueText(Params.InitialValue) : FText::GetEmpty())
				.Font(SettingsRegularFont(Params.ValueFontSize))
				.ColorAndOpacity(GetSettingsPageText())
				.Justification(ETextJustify::Right)
			]
		];

		if (!Params.HelpText.IsEmpty())
		{
			Content->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Params.HelpText)
				.Font(SettingsRegularFont(Params.HelpFontSize))
				.ColorAndOpacity(GetSettingsPageMuted())
				.AutoWrapText(true)
			];
		}

		return MakeSettingsRow(Content);
	}

	inline TSharedRef<SWidget> MakeSettingsDropdown(const FT66DropdownParams& Params)
	{
		static FComboButtonStyle FlatComboStyle = []()
		{
			FComboButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton");
			Style.ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			return Style;
		}();

		TSharedRef<SComboButton> Combo = SNew(SComboButton)
			.ComboButtonStyle(&FlatComboStyle)
			.MenuPlacement(MenuPlacement_BelowAnchor)
			.HasDownArrow(false)
			.OnGetMenuContent_Lambda([OnGet = Params.OnGetMenuContent]()
			{
				return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(4.f), OnGet());
			})
			.ContentPadding(Params.Padding)
			.ButtonContent()
			[
				Params.Content
			];

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(4.f, 2.f), Combo)
			];
	}

	inline TSharedRef<SWidget> MakeSettingsDropdownOptionButton(
		const FText& Label,
		FOnClicked OnClicked,
		const bool bSelected,
		const float MinWidth = 0.f,
		const float Height = 34.f,
		const int32 FontSize = 14)
	{
		return FT66FlatStyle::MakeFlatDropdownOptionButton(
			bSelected ? ET66FlatState::Selected : ET66FlatState::Default,
			Label,
			MoveTemp(OnClicked),
			MinWidth,
			Height,
			FontSize);
	}
}
