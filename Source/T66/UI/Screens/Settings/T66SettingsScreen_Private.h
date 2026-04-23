// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
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
#include "Misc/DefaultValueHelper.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace T66SettingsScreenPrivate
{
	constexpr int32 SettingsFontDelta = -10;
	constexpr int32 SettingsCompactButtonFontSize = 18;
	constexpr float SettingsCompactButtonHeight = 44.f;

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
		return FT66Style::Background();
	}

	inline FLinearColor T66SettingsPageFill()
	{
		return FT66Style::PanelOuter();
	}

	inline FLinearColor T66SettingsRowFill()
	{
		return FT66Style::PanelInner();
	}

	inline FLinearColor T66SettingsButtonNeutralFill()
	{
		return FT66Style::ButtonNeutral();
	}

	inline FLinearColor T66SettingsButtonOutline()
	{
		return FT66Style::Border();
	}

	inline FLinearColor GetSettingsPageBackground()
	{
		return T66SettingsPageFill();
	}

	inline FLinearColor GetSettingsPageText()
	{
		return FT66Style::Text();
	}

	inline FLinearColor GetSettingsPageMuted()
	{
		return FT66Style::TextMuted();
	}

	inline FLinearColor GetRetroButtonBackground()
	{
		return T66SettingsButtonNeutralFill();
	}

	inline FLinearColor GetRetroButtonSelectedBackground()
	{
		return FT66Style::ButtonPrimary();
	}

	inline FLinearColor GetRetroButtonOutline()
	{
		return T66SettingsButtonOutline();
	}

	inline FLinearColor GetRetroButtonText()
	{
		return FT66Style::Text();
	}

	inline FText FormatRetroPercent(float Value)
	{
		const float ClampedValue = FMath::Clamp(Value, 0.0f, 100.0f);
		if (FMath::IsNearlyEqual(ClampedValue, FMath::RoundToFloat(ClampedValue)))
		{
			return FText::AsNumber(FMath::RoundToInt(ClampedValue));
		}
		return FText::FromString(FString::Printf(TEXT("%.2f"), ClampedValue));
	}

	inline FText GetSettingsOnText(UT66LocalizationSubsystem* Loc)
	{
		return Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON");
	}

	inline FText GetSettingsOffText(UT66LocalizationSubsystem* Loc)
	{
		return Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF");
	}

	inline TAttribute<FSlateColor> MakeSelectedButtonColor(
		TFunction<bool()> IsSelected,
		const FSlateColor& SelectedColor = FSlateColor(FT66Style::ButtonPrimary()),
		const FSlateColor& UnselectedColor = FSlateColor(T66SettingsButtonNeutralFill()))
	{
		return TAttribute<FSlateColor>::CreateLambda([IsSelected, SelectedColor, UnselectedColor]() -> FSlateColor
		{
			return IsSelected() ? SelectedColor : UnselectedColor;
		});
	}

	inline TSharedRef<SWidget> MakeSettingsPanel(const TSharedRef<SWidget>& Content, ET66PanelType Type, const FLinearColor& FillColor, const FMargin& Padding)
	{
		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(FillColor)
				.SetPadding(Padding));
	}

	inline TSharedRef<SWidget> MakeSettingsButton(const FT66ButtonParams& Params)
	{
		FT66ButtonParams FlatParams = Params;
		FlatParams
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetUseGlow(false);

		if (FlatParams.FontSize <= 0)
		{
			FlatParams.SetFontSize(AdjustSettingsFontSize(SettingsCompactButtonFontSize));
		}
		else
		{
			FlatParams.SetFontSize(AdjustSettingsFontSize(static_cast<int32>(FlatParams.FontSize)));
		}
		if (FlatParams.Height <= 0.f)
		{
			FlatParams.SetHeight(SettingsCompactButtonHeight);
		}
		if (FlatParams.Padding.Left < 0.f)
		{
			FlatParams.SetPadding(FMargin(10.f, 5.f));
		}

		if (!FlatParams.bHasColorOverride && FlatParams.Type == ET66ButtonType::Neutral)
		{
			FlatParams.SetColor(T66SettingsButtonNeutralFill());
		}

		return FT66Style::MakeButton(FlatParams);
	}

	inline TSharedRef<SWidget> MakeSettingsButton(const FText& Label, FOnClicked OnClicked, ET66ButtonType Type = ET66ButtonType::Neutral, float MinWidth = 120.f)
	{
		return MakeSettingsButton(FT66ButtonParams(Label, MoveTemp(OnClicked), Type).SetMinWidth(MinWidth));
	}

	inline TSharedRef<SWidget> MakeSelectableSettingsButton(
		const FT66ButtonParams& Params,
		TFunction<bool()> IsSelected,
		const FSlateColor& SelectedColor = FSlateColor(FT66Style::ButtonPrimary()),
		const FSlateColor& UnselectedColor = FSlateColor(T66SettingsButtonNeutralFill()))
	{
		FT66ButtonParams ButtonParams = Params;
		ButtonParams.SetColor(MakeSelectedButtonColor(MoveTemp(IsSelected), SelectedColor, UnselectedColor));
		return MakeSettingsButton(ButtonParams);
	}

	inline TSharedRef<SWidget> MakeSettingsRow(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(12.f, 10.f))
	{
		return SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(Padding)
			[
				Content
			];
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
		FSlateColor TextColor = FSlateColor(GetSettingsPageText());
	};

	inline TSharedRef<SWidget> MakeSettingsToggleButtons(
		UT66LocalizationSubsystem* Loc,
		TFunction<bool()> GetValue,
		TFunction<void(bool)> SetValue,
		const FSettingsBoolToggleStyle& Style = FSettingsBoolToggleStyle())
	{
		FT66ButtonParams OnButtonParams(GetSettingsOnText(Loc), FOnClicked::CreateLambda([SetValue]()
		{
			SetValue(true);
			return FReply::Handled();
		}), ET66ButtonType::Neutral);
		OnButtonParams
			.SetMinWidth(Style.ButtonMinWidth)
			.SetPadding(Style.ButtonPadding)
			.SetTextColor(TAttribute<FSlateColor>::CreateLambda([Color = Style.TextColor]() -> FSlateColor
			{
				return Color;
			}));
		if (Style.ButtonFontSize > 0)
		{
			OnButtonParams.SetFontSize(Style.ButtonFontSize);
		}

		FT66ButtonParams OffButtonParams(GetSettingsOffText(Loc), FOnClicked::CreateLambda([SetValue]()
		{
			SetValue(false);
			return FReply::Handled();
		}), ET66ButtonType::Neutral);
		OffButtonParams
			.SetMinWidth(Style.ButtonMinWidth)
			.SetPadding(Style.ButtonPadding)
			.SetTextColor(TAttribute<FSlateColor>::CreateLambda([Color = Style.TextColor]() -> FSlateColor
			{
				return Color;
			}));
		if (Style.ButtonFontSize > 0)
		{
			OffButtonParams.SetFontSize(Style.ButtonFontSize);
		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeSelectableSettingsButton(
					OnButtonParams,
					[GetValue]() { return GetValue(); },
					Style.OnSelectedColor,
					Style.UnselectedColor)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f, 0.f, 0.f)
			[
				MakeSelectableSettingsButton(
					OffButtonParams,
					[GetValue]() { return !GetValue(); },
					Style.OffSelectedColor,
					Style.UnselectedColor)
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
					SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 18.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Description)
					.Font(SettingsRegularFont(16))
					.ColorAndOpacity(GetSettingsPageMuted())
					.AutoWrapText(true)
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.MinDesiredWidth(Style.LabelMinWidth)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())
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
			]);
	}

	inline TSharedRef<SWidget> MakeDynamicOnOffButton(
		UT66LocalizationSubsystem* Loc,
		TFunction<bool()> GetValue,
		FOnClicked OnClicked)
	{
		return MakeSettingsButton(
			FT66ButtonParams(FText::GetEmpty(), MoveTemp(OnClicked), ET66ButtonType::Neutral)
			.SetMinWidth(0.f)
			.SetColor(TAttribute<FSlateColor>::CreateLambda([GetValue]() -> FSlateColor
			{
				return GetValue() ? FT66Style::Tokens::Success : FT66Style::Tokens::Danger;
			}))
			.SetDynamicLabel(TAttribute<FText>::CreateLambda([Loc, GetValue]() -> FText
			{
				return GetValue() ? GetSettingsOnText(Loc) : GetSettingsOffText(Loc);
			})));
	}

	inline TSharedRef<SWidget> MakeSettingsPercentEntryRow(
		const FText& Label,
		const FText& Description,
		TFunction<float()> GetPercent,
		TFunction<void(float)> SetPercent,
		const FText& HintText,
		const FText& HelpText)
	{
		auto CommitNumericValue = [SetPercent](const FText& NewText)
		{
			const FString TrimmedText = NewText.ToString().TrimStartAndEnd();
			float ParsedValue = 0.0f;
			if (TrimmedText.IsEmpty())
			{
				SetPercent(0.0f);
			}
			else if (FDefaultValueHelper::ParseFloat(TrimmedText, ParsedValue))
			{
				SetPercent(FMath::Clamp(ParsedValue, 0.0f, 100.0f));
			}
		};

		return MakeSettingsRow(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.46f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 18.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Description)
					.Font(SettingsRegularFont(16))
					.ColorAndOpacity(GetSettingsPageMuted())
					.AutoWrapText(true)
				]
			]
			+ SHorizontalBox::Slot().FillWidth(0.54f).VAlign(VAlign_Center).Padding(10.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(140.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(GetRetroButtonOutline())
						.Padding(1.f)
						[
							SNew(SEditableTextBox)
							.Text(FormatRetroPercent(GetPercent()))
							.Font(SettingsRegularFont(18))
							.ForegroundColor(GetRetroButtonText())
							.BackgroundColor(FLinearColor::White)
							.Justification(ETextJustify::Center)
							.HintText(HintText)
							.OnTextCommitted_Lambda([CommitNumericValue](const FText& NewText, ETextCommit::Type)
							{
								CommitNumericValue(NewText);
							})
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(HelpText)
					.Font(SettingsRegularFont(14))
					.ColorAndOpacity(GetSettingsPageMuted())
					.AutoWrapText(true)
				]
			]);
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
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Common", "DropdownArrow", "???"))
				.Font(SettingsRegularFont(16))
				.ColorAndOpacity(GetSettingsPageMuted())
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
				SNew(SSlider)
				.Value(Params.InitialValue)
				.OnValueChanged_Lambda([Params, ValueText](float Value)
				{
					if (Params.OnValueChanged)
					{
						Params.OnValueChanged(Value);
					}
					if ((*ValueText).IsValid())
					{
						(*ValueText)->SetText(Params.FormatValueText ? Params.FormatValueText(Value) : FText::GetEmpty());
					}
				})
				.OnMouseCaptureEnd_Lambda([Params]()
				{
					if (Params.OnMouseCaptureEnd)
					{
						Params.OnMouseCaptureEnd();
					}
				})
				.OnControllerCaptureEnd_Lambda([Params]()
				{
					if (Params.OnControllerCaptureEnd)
					{
						Params.OnControllerCaptureEnd();
					}
				})
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
			FComboButtonStyle Style = FT66Style::GetDropdownComboButtonStyle();
			Style.ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			return Style;
		}();

		TSharedRef<SComboButton> Combo = SNew(SComboButton)
			.ComboButtonStyle(&FlatComboStyle)
			.OnGetMenuContent_Lambda([OnGet = Params.OnGetMenuContent]()
			{
				return MakeSettingsPanel(OnGet(), ET66PanelType::Panel2, T66SettingsRowFill(), FMargin(0.f));
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
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(T66SettingsButtonNeutralFill())
				.Padding(FMargin(1.f))
				[
					Combo
				]
			];
	}
}
