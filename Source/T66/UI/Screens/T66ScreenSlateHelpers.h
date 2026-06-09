// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Input/SButton.h"

class SGridPanel;
class SWidget;
class UT66UIManager;

namespace T66ScreenSlateHelpers
{
	enum class ET66ReferenceChromePreset : uint8
	{
		SquareVariant,
		BloodyRetro
	};

	struct FFrontendChromeMetrics
	{
		int32 TitleFontSize = 54;
		int32 TabFontSize = 30;
		float TabMinWidth = 380.0f;
		float TabHeight = 72.0f;
		float TopBarOverlapPx = 18.0f;
		FMargin HeaderPadding = FMargin(0.0f, 0.0f, 0.0f, 8.0f);
		FMargin TabPadding = FMargin(26.0f, 10.0f, 26.0f, 9.0f);
	};

	struct FResponsiveGridModalMetrics
	{
		int32 Columns = 1, Rows = 1;
		float ModalWidth = 0.0f, ModalHeight = 0.0f;
		float ModalPaddingX = 30.0f, ModalPaddingY = 25.0f;
		float TitleSectionHeight = 58.0f, FooterSectionHeight = 78.0f;
		float TileGap = 6.0f, TileSize = 64.0f;
		float GridWidth = 64.0f, GridHeight = 64.0f;
	};

	struct FTopBarScreenLayoutMetrics
	{
		FVector2D ViewportSize = FVector2D::ZeroVector;
		FVector2D ContentSize = FVector2D::ZeroVector;
		FMargin Padding = FMargin(0.0f);
		float TopBarReservedHeight = 0.0f;
		bool bCompact = false;
		bool bStacked = false;
	};

	enum class EFriendslopStandardModalButtonChrome : uint8
	{
		Dark,
		Red,
		Green,
	};

	enum class EFriendslopStandardModalButtonState : uint8
	{
		Default,
		Selected,
		Ready,
		Disabled,
	};

	struct FFriendslopStandardModalButton
	{
		FText Label;
		FOnClicked OnClicked;
		EFriendslopStandardModalButtonState State = EFriendslopStandardModalButtonState::Default;
		EFriendslopStandardModalButtonChrome Chrome = EFriendslopStandardModalButtonChrome::Red;
		TAttribute<bool> IsEnabled = true;
		FName Tag = NAME_None;
		int32 FontSize = 16;
		float MinWidth = 300.0f;
		float Height = 58.0f;
	};

	struct FFriendslopStandardModalCheckboxRow
	{
		FText Label;
		FOnClicked OnClicked;
		TAttribute<bool> IsChecked = false;
		TAttribute<bool> IsEnabled = true;
		FName RowTag = NAME_None;
		FName CheckboxTag = NAME_None;
		FName LabelTag = NAME_None;
		int32 FontSize = 16;
		float CheckboxSize = 44.0f;
	};

	struct FFriendslopStandardModalParams
	{
		FText TitleText;
		FText BodyText;
		FText StatusText;
		FFriendslopStandardModalButton LeftButton;
		FFriendslopStandardModalButton RightButton;
		FFriendslopStandardModalCheckboxRow CheckboxRow;
		FName RootTag = NAME_None;
		FName ScrimTag = NAME_None;
		FName PanelTag = NAME_None;
		FName TitleTag = NAME_None;
		FName BodyTag = NAME_None;
		FName StatusTag = NAME_None;
		bool bShowCheckboxRow = false;
	};

	const FFrontendChromeMetrics& GetFrontendChromeMetrics();
	float GetFrontendChromeTopInset(const UT66UIManager* UIManager);
	FTopBarScreenLayoutMetrics MakeTopBarScreenLayoutMetrics(
		const UT66UIManager* UIManager,
		const FMargin& ExtraPadding = FMargin(0.0f));
	TSharedRef<SWidget> MakeTopBarScreenRoot(
		const UT66UIManager* UIManager,
		const TSharedRef<SWidget>& Content,
		const TSharedRef<SWidget>& BackgroundContent,
		const FLinearColor& OverlayTint = FLinearColor::Transparent,
		const FMargin& ExtraPadding = FMargin(0.0f));
	FSlateFontInfo MakeFrontendChromeTitleFont();
	FSlateFontInfo MakeFrontendChromeTabFont();
	TSharedPtr<FSlateBrush> MakeSlateBrush(const FVector2D& ImageSize = FVector2D::ZeroVector, ESlateBrushDrawType::Type DrawAs = ESlateBrushDrawType::Image);
	FResponsiveGridModalMetrics MakeResponsiveGridModalMetrics(int32 ItemCount, const FVector2D& SafeFrameSize);
	void AddUniformGridPaddingSlots(SGridPanel& GridPanel, int32 FilledSlotCount, const FResponsiveGridModalMetrics& Metrics);
	TSharedRef<SWidget> MakeFilledButtonText(
		const FT66ButtonParams& Params,
		float ButtonHeight,
		const TAttribute<FSlateColor>& DefaultTextColor,
		const TAttribute<FLinearColor>& DefaultShadowColor);
	TSharedRef<SWidget> MakeReferenceHorizontalSlicedImage(
		TAttribute<const FSlateBrush*> Brush,
		const FVector2D& DesiredSize = FVector2D(1.0f, 1.0f),
		float SourceCapFraction = 0.105f);
	float NormalizeReferenceSlicedButtonMinWidth(float RequestedMinWidth, float Height);
	T66_API ET66ReferenceChromePreset GetReferenceChromePreset();
	T66_API const TCHAR* GetReferenceChromePresetName();
	T66_API void SetReferenceChromePresetForSession(ET66ReferenceChromePreset Preset);
	T66_API FString MakeReferenceMainMenuElementAssetPath(const TCHAR* FileName);
	T66_API FString MakeReferenceChromeElementAssetPath(const TCHAR* FileName);
	T66_API FString MakeReferenceLongPanelAssetPath(const TCHAR* State = TEXT("normal"));
	T66_API FString MakeReferenceRedSquareButtonAssetPath(const TCHAR* State);
	T66_API FString MakeReferenceChromeButtonAssetPath(
		const TCHAR* Family,
		const TCHAR* State);
	FString MakeReferenceButtonAssetPath(
		const TCHAR* FamilyStem,
		const TCHAR* State);
	FString MakeReferenceSharedAssetPath(const TCHAR* RelativeAssetPath);
	const FSlateBrush* GetReferenceSharedBrush(
		const TCHAR* RelativeAssetPath,
		const FMargin& Margin,
		const TCHAR* DebugLabel);
	bool IsReferenceChromeButtonAssetPath(const FString& SourceRelativePath);
	bool IsReferenceChromePillButtonAssetPath(const FString& SourceRelativePath);
	bool IsReferenceChromeCTAButtonAssetPath(const FString& SourceRelativePath);
	T66_API TSharedRef<SWidget> MakeReferenceSharedBorder(
		const TCHAR* RelativeAssetPath,
		const TSharedRef<SWidget>& Content,
		const FMargin& BrushMargin,
		const FMargin& Padding,
		const TCHAR* DebugLabel,
		const FLinearColor& FallbackColor);
	T66_API TSharedRef<SWidget> MakeReferenceSlicedPlateButton(
		FOnClicked OnClicked,
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* NormalBrush,
		const FSlateBrush* HoveredBrush,
		const FSlateBrush* PressedBrush,
		const FSlateBrush* DisabledBrush,
		float MinWidth,
		float Height,
		const FMargin& ContentPadding,
		const TAttribute<bool>& IsEnabled = TAttribute<bool>(true),
		const TAttribute<EVisibility>& Visibility = TAttribute<EVisibility>(EVisibility::Visible),
		float SourceCapFraction = 0.105f,
		const FSlateBrush* SelectedBrush = nullptr,
		const TAttribute<bool>& IsSelected = TAttribute<bool>(false));
	TSharedRef<SWidget> MakeReferenceProgressBar(
		TAttribute<TOptional<float>> Percent,
		const FVector2D& DesiredSize,
		const FLinearColor& FallbackFill = FLinearColor(0.10f, 0.64f, 0.96f, 1.0f),
		const FMargin& Padding = FMargin(4.0f, 3.0f));
	TSharedRef<SWidget> MakeReferenceProgressBar(
		float Percent,
		const FVector2D& DesiredSize,
		const FLinearColor& FallbackFill = FLinearColor(0.10f, 0.64f, 0.96f, 1.0f),
		const FMargin& Padding = FMargin(4.0f, 3.0f));
	TSharedRef<SWidget> MakeResponsiveGridTile(const FT66ButtonParams& ButtonParams, const FLinearColor& BackgroundColor, const TSharedRef<SWidget>& Content, const FResponsiveGridModalMetrics& Metrics);
	TSharedRef<SWidget> MakeResponsiveGridModal(const FText& TitleText, const TSharedRef<SWidget>& GridContent, const TSharedRef<SWidget>& FooterContent, const FResponsiveGridModalMetrics& Metrics);
	TSharedRef<SWidget> MakeCenteredScrimModal(const TSharedRef<SWidget>& Content, const FMargin& OuterPadding = FMargin(0.0f), float WidthOverride = 0.0f, float HeightOverride = 0.0f, bool bUseWhiteBrush = false);
	T66_API TSharedRef<SWidget> MakeFriendslopStandardModal(const FFriendslopStandardModalParams& Params);
	TSharedRef<SWidget> MakeTwoButtonRow(const TSharedRef<SWidget>& LeftButton, const TSharedRef<SWidget>& RightButton, const FMargin& LeftPadding = FMargin(10.0f, 0.0f), const FMargin& RightPadding = FMargin(10.0f, 0.0f), EVisibility Visibility = EVisibility::Visible);
}
