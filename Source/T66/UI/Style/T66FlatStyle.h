// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSlider.h"

enum class ET66FlatState : uint8
{
	Disabled,
	Default,
	Selected,
	Ready,
};

enum class ET66FlatLabelRole : uint8
{
	Title,
	Header,
	SubHeader,
	Body,
	Caption,
	PurpleAccent,
	StatLabel,
	StatValue,
	Button,
};

enum class ET66FlatActionAlignment : uint8
{
	Left,
	Center,
	Right,
	Fill,
};

struct FT66FlatStatRowData
{
	TAttribute<FText> Label;
	TAttribute<FText> Value;
	ET66FlatLabelRole LabelRole = ET66FlatLabelRole::StatLabel;
	ET66FlatLabelRole ValueRole = ET66FlatLabelRole::StatValue;

	FT66FlatStatRowData() = default;
	FT66FlatStatRowData(const FText& InLabel, const FText& InValue)
		: Label(InLabel)
		, Value(InValue)
	{
	}
	FT66FlatStatRowData(
		TAttribute<FText> InLabel,
		TAttribute<FText> InValue,
		ET66FlatLabelRole InLabelRole = ET66FlatLabelRole::StatLabel,
		ET66FlatLabelRole InValueRole = ET66FlatLabelRole::StatValue)
		: Label(MoveTemp(InLabel))
		, Value(MoveTemp(InValue))
		, LabelRole(InLabelRole)
		, ValueRole(InValueRole)
	{
	}
};

struct FT66FlatButtonParams
{
	static constexpr bool bUseGlow = false;

	ET66FlatState State = ET66FlatState::Default;
	TAttribute<FText> Label;
	FOnClicked OnClicked;
	TSharedPtr<SWidget> OptionalLeftIcon;
	TSharedPtr<SWidget> OptionalRightIcon;
	FMargin Padding = FMargin(14.f, 8.f);
	float MinWidth = 120.f;
	float Height = 0.f;
	TAttribute<bool> IsEnabled = true;
	int32 FontSize = 20;
	FName Tag = NAME_None;
	FName ToggleGroup = NAME_None;

	FT66FlatButtonParams() = default;
	FT66FlatButtonParams(const ET66FlatState InState, const FText& InLabel, FOnClicked InOnClicked)
		: State(InState)
		, Label(InLabel)
		, OnClicked(MoveTemp(InOnClicked))
	{
	}
	FT66FlatButtonParams(const ET66FlatState InState, TAttribute<FText> InLabel, FOnClicked InOnClicked)
		: State(InState)
		, Label(MoveTemp(InLabel))
		, OnClicked(MoveTemp(InOnClicked))
	{
	}
};

struct FT66FlatToggleGroupItem
{
	ET66FlatState State = ET66FlatState::Default;
	TAttribute<bool> bIsSelected = false;
	TAttribute<FText> Label;
	FOnClicked OnClicked;
	TSharedPtr<SWidget> OptionalLeftIcon;
	TSharedPtr<SWidget> OptionalRightIcon;
	FMargin Padding = FMargin(14.f, 8.f);
	float MinWidth = 120.f;
	float Height = 44.f;
	TAttribute<bool> IsEnabled = true;
	int32 FontSize = 20;
	FName Tag = NAME_None;
};

struct FT66FlatToggleGroupParams
{
	FName GroupName = NAME_None;
	bool bMutuallyExclusive = true;
	TArray<FT66FlatToggleGroupItem> Items;
};

struct FT66FlatTransparentRegionParams
{
	ET66FlatState State = ET66FlatState::Default;
	FMargin Padding = FMargin(10.f);
	FName Tag = NAME_None;

	FT66FlatTransparentRegionParams() = default;
	FT66FlatTransparentRegionParams(
		const ET66FlatState InState,
		const FMargin& InPadding,
		const FName InTag = NAME_None)
		: State(InState)
		, Padding(InPadding)
		, Tag(InTag)
	{
	}
};

/**
 * Flat V3 UI helper surface.
 *
 * Quick reference:
 * - MakeFlatPanel / MakeFlatSubPanel / MakeFlatOuterContainer / MakeFlatHeaderedPanel
 * - MakeFlatButton / MakeFlatIconButton / MakeFlatTabButton / MakeFlatActionRow
 * - MakeFlatDropdown / MakeFlatSlider / MakeFlatCheckbox / MakeFlatToggleButton
 * - MakeFlatLabel / MakeFlatStatRow / MakeFlatStatsTable / MakeFlatProgressBar / MakeFlatDivider / MakeFlatPaginationIndicator
 * - MakeFlatPortraitSlot / MakeFlatRankingRow / MakeFlatTopBar / MakeFlatSlimTopBar
 *
 * These helpers draw axis-aligned Slate rectangles with solid fills and clean 2 px borders.
 * They intentionally do not call the PNG chrome path, M_UI_Glow, or ST66RetroUIRetainedSurface.
 */
class T66_API FT66FlatStyle
{
public:
	static constexpr float FlatStroke = 2.0f;
	static constexpr float FlatGap = 12.0f;

	static FLinearColor BackgroundColor();
	static FLinearColor DisabledFill();
	static FLinearColor DisabledBorder();
	static FLinearColor DisabledText();
	static FLinearColor DefaultFill();
	static FLinearColor DefaultBorder();
	static FLinearColor DefaultText();
	static FLinearColor PurpleAccent();
	static FLinearColor SelectedFill();
	static FLinearColor SelectedBorder();
	static FLinearColor SelectedText();
	static FLinearColor ProgressFill();
	static FLinearColor GoodStandingGreen();
	static FLinearColor ReadyBorder();
	static FLinearColor HoverBorder();
	static FLinearColor HoverText();
	static FLinearColor HoverFill();
	static FLinearColor PrimaryText();
	static FLinearColor SecondaryText();
	static FLinearColor DataAccent();

	static FLinearColor FillForState(ET66FlatState State);
	static FLinearColor BorderForState(ET66FlatState State);
	static FSlateColor TextColorForState(ET66FlatState State);
	static FSlateFontInfo MakeFont(int32 Size);
	static FSlateFontInfo MakeBoldFont(int32 Size);

	static TSharedRef<SWidget> AttachMetadata(
		const TSharedRef<SWidget>& Widget,
		FName Tag,
		const FString& IntendedRole,
		ET66FlatState IntendedState = ET66FlatState::Default,
		const TOptional<FLinearColor>& BorderColor = TOptional<FLinearColor>(),
		bool bHasClickHandler = false,
		FName ToggleGroup = NAME_None,
		bool bIsLabel = false,
		bool bHoverCapable = false);

	static TSharedRef<SWidget> WrapWithoutRetainer(const TSharedRef<SWidget>& Widget, FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatPanel(
		ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		TSharedPtr<class SBorder>* OutBorder = nullptr,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatInteractivePanel(
		ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		TAttribute<bool> IsEnabled = true,
		FName Tag = NAME_None,
		const FString& IntendedRole = TEXT("InteractivePanel"));

	static TSharedRef<SWidget> MakeFlatSubPanel(
		ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		TSharedPtr<class SBorder>* OutBorder = nullptr,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatTransparentRegion(
		const FT66FlatTransparentRegionParams& Params,
		const TSharedRef<SWidget>& Content);

	static TSharedRef<SWidget> MakeFlatTransparentRegion(
		ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatOuterContainer(
		ET66FlatState State,
		const TArray<TSharedRef<SWidget>>& Children,
		float Gap = FlatGap,
		EOrientation Orientation = Orient_Vertical,
		const FMargin& Padding = FMargin(FlatGap),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatHeaderedPanel(
		ET66FlatState State,
		const TAttribute<FText>& HeaderText,
		const TSharedRef<SWidget>& BodyContent,
		const TSharedPtr<SWidget>& OptionalIcon = nullptr,
		const TOptional<FLinearColor>& OptionalHeaderAccent = TOptional<FLinearColor>(),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatHeaderedPanel(
		ET66FlatState State,
		const FText& HeaderText,
		const TSharedRef<SWidget>& BodyContent,
		const TSharedPtr<SWidget>& OptionalIcon = nullptr,
		const TOptional<FLinearColor>& OptionalHeaderAccent = TOptional<FLinearColor>(),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatButton(const FT66FlatButtonParams& Params);

	static TSharedRef<SWidget> MakeFlatButton(
		ET66FlatState State,
		const TAttribute<FText>& Label,
		FOnClicked OnClicked,
		const TSharedPtr<SWidget>& OptionalLeftIcon = nullptr,
		const TSharedPtr<SWidget>& OptionalRightIcon = nullptr,
		const FMargin& Padding = FMargin(14.f, 8.f),
		float MinWidth = 120.f,
		float Height = 0.f,
		TAttribute<bool> IsEnabled = true,
		int32 FontSize = 20,
		FName Tag = NAME_None,
		FName ToggleGroup = NAME_None);

	static TSharedRef<SWidget> MakeFlatButton(
		ET66FlatState State,
		const FText& Label,
		FOnClicked OnClicked,
		const TSharedPtr<SWidget>& OptionalLeftIcon = nullptr,
		const TSharedPtr<SWidget>& OptionalRightIcon = nullptr,
		const FMargin& Padding = FMargin(14.f, 8.f),
		float MinWidth = 120.f,
		float Height = 0.f,
		TAttribute<bool> IsEnabled = true,
		int32 FontSize = 20,
		FName Tag = NAME_None,
		FName ToggleGroup = NAME_None);

	static TSharedRef<SWidget> MakeFlatToggleGroupButton(
		ET66FlatState State,
		const TSharedRef<SWidget>& Content,
		FOnClicked OnClicked,
		const FMargin& Padding = FMargin(14.f, 8.f),
		float MinWidth = 120.f,
		float Height = 44.f,
		TAttribute<bool> IsEnabled = true,
		FName Tag = NAME_None,
		FName ToggleGroup = NAME_None);

	static TArray<TSharedRef<SWidget>> MakeFlatToggleGroup(const FT66FlatToggleGroupParams& Params);

	static TSharedRef<SWidget> MakeFlatIconButton(
		ET66FlatState State,
		const FSlateBrush* Icon,
		FOnClicked OnClicked,
		const FVector2D& SizeHint = FVector2D(48.f, 48.f),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatTooltipContent(
		const FText& Text,
		float Width = 360.f,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatTooltipIcon(
		ET66FlatState State,
		const FSlateBrush* Icon,
		const FText& TooltipText,
		const FVector2D& SizeHint = FVector2D(31.f, 31.f),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatTabButton(
		ET66FlatState State,
		const TAttribute<FText>& Label,
		FOnClicked OnClicked,
		const TSharedPtr<SWidget>& OptionalLeftIcon,
		const FText& TooltipText,
		float MinWidth = 140.f,
		float Height = 46.f,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatActionRow(
		const TArray<TSharedRef<SWidget>>& Buttons,
		ET66FlatActionAlignment Alignment = ET66FlatActionAlignment::Fill,
		float Gap = FlatGap,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatDropdown(
		ET66FlatState State,
		const TAttribute<FText>& CurrentValueText,
		TFunction<TSharedRef<SWidget>()> OptionsProvider,
		bool bForceSelectedState = false,
		float MinWidth = 160.f,
		float Height = 42.f,
		int32 FontSize = 20,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatSlider(
		ET66FlatState State,
		float Min,
		float Max,
		TAttribute<float> Current,
		FOnFloatValueChanged OnChange,
		const TSharedPtr<SWidget>& OptionalValueDisplay = nullptr,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatCheckbox(
		ET66FlatState State,
		TAttribute<ECheckBoxState> Checked,
		FOnCheckStateChanged OnToggle,
		const TAttribute<FText>& OptionalLabel = TAttribute<FText>(),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatToggleButton(
		ET66FlatState State,
		const TAttribute<FText>& Label,
		TAttribute<bool> bIsActive,
		FOnClicked OnToggle,
		float MinWidth = 120.f,
		float Height = 44.f,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatLabel(
		const TAttribute<FText>& Text,
		ET66FlatLabelRole Role,
		ETextJustify::Type Justification = ETextJustify::Left,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatLabel(
		const FText& Text,
		ET66FlatLabelRole Role,
		ETextJustify::Type Justification = ETextJustify::Left,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatStatRow(
		const TAttribute<FText>& LabelText,
		const TAttribute<FText>& ValueText,
		ET66FlatLabelRole LabelRole = ET66FlatLabelRole::StatLabel,
		ET66FlatLabelRole ValueRole = ET66FlatLabelRole::StatValue,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatStatsTable(
		const TArray<FT66FlatStatRowData>& Rows,
		int32 Columns = 1,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatProgressBar(
		TAttribute<float> Percent,
		const TOptional<FLinearColor>& OptionalColor = TOptional<FLinearColor>(),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatDivider(
		EOrientation Orientation,
		float Length = 0.f,
		const TOptional<FLinearColor>& OptionalColor = TOptional<FLinearColor>(),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatPaginationIndicator(
		int32 Total,
		int32 CurrentIndex,
		const TOptional<FLinearColor>& CurrentColor = TOptional<FLinearColor>(),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatPortraitSlot(
		ET66FlatState State,
		const FSlateBrush* PortraitTexture,
		const FSlateBrush* OptionalRoleIcon = nullptr,
		const FVector2D& OptionalSize = FVector2D(76.f, 76.f),
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatRankingRow(
		int32 Rank,
		const TAttribute<FText>& Name,
		const TAttribute<FText>& Score,
		ET66FlatState OptionalState = ET66FlatState::Default,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatTopBar(
		const TArray<TSharedRef<SWidget>>& Buttons,
		bool bUseOuterContainer = true,
		FName Tag = NAME_None);

	static TSharedRef<SWidget> MakeFlatSlimTopBar(
		const TArray<TSharedRef<SWidget>>& LeftButtons,
		const TSharedRef<SWidget>& CenterButton,
		const TArray<TSharedRef<SWidget>>& RightButtons,
		FName Tag = NAME_None);
};
