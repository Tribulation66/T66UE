// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSlider.h"

class UUserWidget;
enum class ET66ButtonType : uint8;
enum class ET66PanelType : uint8;
struct FT66BareButtonParams;
struct FT66ButtonParams;
struct FT66PanelParams;

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

enum class ET66FlatOverlayChromeBrush : uint8
{
	OverlayModalPanel,
	CasinoShellPanel,
	ContentPanelWide,
	ContentPanelTall,
	InnerPanel,
	HeaderSummaryBar,
	CrateStripFrame,
	SlotNormal,
	SlotHover,
	SlotSelected,
	SlotDisabled,
	OfferCardNormal,
	OfferCardHover,
	OfferCardSelected,
	OfferCardDisabled,
	CrateWinnerMarker,
};

enum class ET66FlatOverlayChromeButtonFamily : uint8
{
	Neutral,
	Primary,
	Danger,
	Tab,
	Select,
	DuoLeft,
	DuoRight,
	Central,
	DropdownOption,
	BorderlessIcon,
};

struct T66_API FT66FlatOverlayChromeButtonParams
{
	FText Label;
	FOnClicked OnClicked;
	ET66FlatOverlayChromeButtonFamily Family = ET66FlatOverlayChromeButtonFamily::Neutral;
	float MinWidth = 120.f;
	float MinHeight = 44.f;
	int32 FontSize = 14;
	FMargin Padding = FMargin(12.f, 5.f);
	TAttribute<bool> IsEnabled = true;
	TAttribute<bool> IsSelected = false;
	TSharedPtr<SWidget> CustomContent;

	FT66FlatOverlayChromeButtonParams() = default;
	FT66FlatOverlayChromeButtonParams(
		const FText& InLabel,
		FOnClicked InOnClicked,
		const ET66FlatOverlayChromeButtonFamily InFamily = ET66FlatOverlayChromeButtonFamily::Neutral)
		: Label(InLabel)
		, OnClicked(MoveTemp(InOnClicked))
		, Family(InFamily)
	{
	}

	FT66FlatOverlayChromeButtonParams& SetMinWidth(const float InMinWidth) { MinWidth = InMinWidth; return *this; }
	FT66FlatOverlayChromeButtonParams& SetMinHeight(const float InMinHeight) { MinHeight = InMinHeight; return *this; }
	FT66FlatOverlayChromeButtonParams& SetFontSize(const int32 InFontSize) { FontSize = InFontSize; return *this; }
	FT66FlatOverlayChromeButtonParams& SetPadding(const FMargin& InPadding) { Padding = InPadding; return *this; }
	FT66FlatOverlayChromeButtonParams& SetEnabled(const TAttribute<bool>& InIsEnabled) { IsEnabled = InIsEnabled; return *this; }
	FT66FlatOverlayChromeButtonParams& SetSelected(const TAttribute<bool>& InIsSelected) { IsSelected = InIsSelected; return *this; }
	FT66FlatOverlayChromeButtonParams& SetContent(const TSharedRef<SWidget>& InContent) { CustomContent = InContent; return *this; }
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
	using ET66ReferenceChromePreset = T66ScreenSlateHelpers::ET66ReferenceChromePreset;
	using FFrontendChromeMetrics = T66ScreenSlateHelpers::FFrontendChromeMetrics;
	using FResponsiveGridModalMetrics = T66ScreenSlateHelpers::FResponsiveGridModalMetrics;
	using FTopBarScreenLayoutMetrics = T66ScreenSlateHelpers::FTopBarScreenLayoutMetrics;

	static constexpr float FlatStroke = 2.0f;
	static constexpr float FlatGap = 12.0f;

	struct Tokens
	{
		static FLinearColor Bg;
		static FLinearColor Panel;
		static FLinearColor Panel2;
		static FLinearColor Stroke;
		static FLinearColor Scrim;
		static FLinearColor Text;
		static FLinearColor TextMuted;
		static FLinearColor Accent;
		static FLinearColor Accent2;
		static FLinearColor Danger;
		static FLinearColor Success;
		static FLinearColor Border;

		static constexpr float ReferenceLayoutWidth = 1280.f;
		static constexpr float ReferenceLayoutHeight = 720.f;
		static constexpr float SafeFrameAspectRatio = 16.f / 9.f;
		static constexpr float CornerRadius = 10.f;
		static constexpr float CornerRadiusSmall = 8.f;
		static constexpr float StrokeWidth = 1.f;
		static constexpr float BorderWidth = 1.f;
		static constexpr float DefaultPanelPadding = 16.f;
		static constexpr float ButtonMinWidth = 120.f;
		static constexpr float ButtonMaxWidth = 420.f;
		static constexpr float ButtonMinHeight = 44.f;
		static constexpr float ButtonTallHeight = 56.f;
		static constexpr float TopBarReservedHeight = 146.f;
		static constexpr float TopBarSurfaceHeight = 118.f;
		static constexpr float ModalMaxWidth = 1120.f;
		static constexpr float ModalMaxHeight = 640.f;
		static constexpr float ReadableLineWidth = 760.f;
		static constexpr float HUDSafeInset = 20.f;

		static constexpr float InventorySlotSize = 160.f;
		static constexpr float ItemPanelIconSize = 200.f;
		static constexpr float GameCardMinWidth = 260.f;
		static constexpr float GameCardHeight = 200.f;
		static constexpr float NPCCenterPanelTotalWidth = 920.f;
		static constexpr float NPCRightPanelWidth = 380.f;
		static constexpr float NPCMainRowHeight = 600.f;
		static constexpr float NPCInventoryPanelHeight = 180.f;
		static constexpr float NPCGamblerInventoryPanelHeight = 252.f;
		static constexpr float NPCStatsPanelWidth = 300.f;
		static constexpr float NPCStatsPanelContentHeight = 400.f;
		static constexpr float NPCShopStatsPanelWidth = NPCStatsPanelWidth;
		static constexpr float NPCGamblerStatsPanelWidth = NPCStatsPanelWidth;
		static constexpr float NPCShopCardWidth = 248.f;
		static constexpr float NPCShopCardHeight = 500.f;
		static constexpr float NPCCompactShopCardWidth = 148.f;
		static constexpr float NPCCompactShopCardHeight = 264.f;
		static constexpr float NPCAngerCircleSize = 170.f;
		static constexpr float NPCBankSpinBoxWidth = 110.f;
		static constexpr float NPCBankSpinBoxHeight = 44.f;
		static constexpr float NPCOverlayPadding = 24.f;

		static constexpr float Space2 = 8.f;
		static constexpr float Space3 = 12.f;
		static constexpr float Space4 = 16.f;
		static constexpr float Space5 = 20.f;
		static constexpr float Space6 = 24.f;
		static constexpr float Space8 = 32.f;

		static const FMargin ButtonPadding;
		static const FMargin ButtonPaddingPressed;
		static constexpr float ButtonGlowPadding = 6.f;
		static constexpr float ButtonHoverGlowIntensity = 0.75f;
		static constexpr float ButtonPressedGlowIntensity = 1.10f;
		static constexpr float ButtonGlowFallbackOpacity = 0.28f;

		static FSlateFontInfo FontRegular(int32 Size);
		static FSlateFontInfo FontBold(int32 Size);
		static FSlateFontInfo FontTitle();
		static FSlateFontInfo FontHeading();
		static FSlateFontInfo FontBody();
		static FSlateFontInfo FontSmall();
		static FSlateFontInfo FontChip();
		static FSlateFontInfo FontButton();
	};

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
	static FLinearColor Text();
	static FLinearColor TextMuted();
	static FLinearColor Accent2();
	static FLinearColor Danger();
	static FLinearColor Border();
	static FLinearColor Background();
	static FLinearColor PanelOuter();
	static FLinearColor PanelInner();
	static FLinearColor BossBarBackground();
	static FLinearColor BossBarFill();
	static FLinearColor MinimapBackground();
	static FLinearColor MinimapTerrain();
	static FLinearColor MinimapGrid();
	static FLinearColor MinimapFriendly();
	static FLinearColor MinimapEnemy();
	static FLinearColor MinimapNeutral();
	static float GetGlobalUIScale();
	static FVector2D GetViewportLogicalSize();
	static FVector2D GetSafeFrameSize(float AspectRatio = Tokens::SafeFrameAspectRatio);
	static FMargin GetSafeFrameInsets(float AspectRatio = Tokens::SafeFrameAspectRatio);
	static FMargin GetSafePadding(const FMargin& Padding, float AspectRatio = Tokens::SafeFrameAspectRatio);
	static TSharedRef<SWidget> MakeResponsiveRoot(
		const TSharedRef<SWidget>& Content,
		const FVector2D& ReferenceResolution = FVector2D(Tokens::ReferenceLayoutWidth, Tokens::ReferenceLayoutHeight),
		bool bAllowUpscale = false);
	static void DeferRebuild(UUserWidget* Widget, int32 ZOrder = 0);
	static TSharedRef<SWidget> MakeButton(const FT66ButtonParams& Params);
	static FT66ButtonParams MakeInRunButtonParams(
		const FText& Label,
		FOnClicked OnClicked,
		ET66ButtonType Type = ET66ButtonType::Neutral);
	static TSharedRef<SWidget> MakeBareButton(const FT66BareButtonParams& Params, TSharedPtr<SButton>* OutButton = nullptr);
	static TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		const FT66PanelParams& Params,
		TSharedPtr<class SBorder>* OutBorder = nullptr);
	static TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		ET66PanelType Type,
		FMargin Padding = FMargin(16.f));
	static FSlateFontInfo MakeFont(const TCHAR* Weight, int32 Size);
	static const FTextBlockStyle& GetTextBlockStyle(FName StyleName);
	static const FSlateBrush* GetBrush(FName BrushName);
	static const FButtonStyle& GetButtonStyle(FName StyleName);
	static const FComboButtonStyle& GetDropdownComboButtonStyle();
	static FLinearColor Scrim();
	static TSharedRef<SWidget> MakeHudPanel(
		const TSharedRef<SWidget>& Content,
		const FText& Title,
		const FMargin& Padding = FMargin(12.f, 10.f));
	static TSharedRef<SWidget> MakeHudPanel(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding = FMargin(12.f, 10.f));

	static const FFrontendChromeMetrics& GetFrontendChromeMetrics();
	static float GetFrontendChromeTopInset(const UT66UIManager* UIManager);
	static FTopBarScreenLayoutMetrics MakeTopBarScreenLayoutMetrics(
		const UT66UIManager* UIManager,
		const FMargin& ExtraPadding = FMargin(0.0f));
	static TSharedRef<SWidget> MakeTopBarScreenRoot(
		const UT66UIManager* UIManager,
		const TSharedRef<SWidget>& Content,
		const TSharedRef<SWidget>& BackgroundContent,
		const FLinearColor& OverlayTint = FLinearColor::Transparent,
		const FMargin& ExtraPadding = FMargin(0.0f));
	static FSlateFontInfo MakeFrontendChromeTitleFont();
	static FSlateFontInfo MakeFrontendChromeTabFont();
	static TSharedPtr<FSlateBrush> MakeSlateBrush(
		const FVector2D& ImageSize = FVector2D::ZeroVector,
		ESlateBrushDrawType::Type DrawAs = ESlateBrushDrawType::Image);
	static FResponsiveGridModalMetrics MakeResponsiveGridModalMetrics(int32 ItemCount, const FVector2D& SafeFrameSize);
	static void AddUniformGridPaddingSlots(SGridPanel& GridPanel, int32 FilledSlotCount, const FResponsiveGridModalMetrics& Metrics);
	static TSharedRef<SWidget> MakeFilledButtonText(
		const FT66ButtonParams& Params,
		float ButtonHeight,
		const TAttribute<FSlateColor>& DefaultTextColor,
		const TAttribute<FLinearColor>& DefaultShadowColor);
	static TSharedRef<SWidget> BuildFlatHorizontalSlicedImage(
		TAttribute<const FSlateBrush*> Brush,
		const FVector2D& DesiredSize = FVector2D(1.0f, 1.0f),
		float SourceCapFraction = 0.105f);
	static float NormalizeFlatSlicedButtonMinWidth(float RequestedMinWidth, float Height);
	static ET66ReferenceChromePreset GetFlatChromePreset();
	static const TCHAR* GetFlatChromePresetName();
	static void SetFlatChromePresetForSession(ET66ReferenceChromePreset Preset);
	static FString GetFlatMainMenuElementAssetPath(const TCHAR* FileName);
	static FString GetFlatChromeElementAssetPath(const TCHAR* FileName);
	static FString GetFlatLongPanelAssetPath(const TCHAR* State = TEXT("normal"));
	static FString GetFlatRedSquareButtonAssetPath(const TCHAR* State);
	static FString GetFlatChromeButtonAssetPath(const TCHAR* Family, const TCHAR* State);
	static FString GetFlatButtonAssetPath(const TCHAR* FamilyStem, const TCHAR* State);
	static FString GetFlatSharedAssetPath(const TCHAR* RelativeAssetPath);
	static const FSlateBrush* GetFlatSharedBrush(
		const TCHAR* RelativeAssetPath,
		const FMargin& Margin,
		const TCHAR* DebugLabel);
	static bool IsFlatChromeButtonAssetPath(const FString& SourceRelativePath);
	static bool IsFlatChromePillButtonAssetPath(const FString& SourceRelativePath);
	static bool IsFlatChromeCTAButtonAssetPath(const FString& SourceRelativePath);
	static TSharedRef<SWidget> BuildFlatSharedBorder(
		const TCHAR* RelativeAssetPath,
		const TSharedRef<SWidget>& Content,
		const FMargin& BrushMargin,
		const FMargin& Padding,
		const TCHAR* DebugLabel,
		const FLinearColor& FallbackColor);
	static TSharedRef<SWidget> BuildFlatSlicedPlateButton(
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
	static TSharedRef<SWidget> BuildFlatProgressBar(
		TAttribute<TOptional<float>> Percent,
		const FVector2D& DesiredSize,
		const FLinearColor& FallbackFill = FLinearColor(0.10f, 0.64f, 0.96f, 1.0f),
		const FMargin& Padding = FMargin(4.0f, 3.0f));
	static TSharedRef<SWidget> BuildFlatProgressBar(
		float Percent,
		const FVector2D& DesiredSize,
		const FLinearColor& FallbackFill = FLinearColor(0.10f, 0.64f, 0.96f, 1.0f),
		const FMargin& Padding = FMargin(4.0f, 3.0f));
	static TSharedRef<SWidget> MakeResponsiveGridTile(
		const FT66ButtonParams& ButtonParams,
		const FLinearColor& BackgroundColor,
		const TSharedRef<SWidget>& Content,
		const FResponsiveGridModalMetrics& Metrics);
	static TSharedRef<SWidget> MakeResponsiveGridModal(
		const FText& TitleText,
		const TSharedRef<SWidget>& GridContent,
		const TSharedRef<SWidget>& FooterContent,
		const FResponsiveGridModalMetrics& Metrics);
	static TSharedRef<SWidget> MakeCenteredScrimModal(
		const TSharedRef<SWidget>& Content,
		const FMargin& OuterPadding = FMargin(0.0f),
		float WidthOverride = 0.0f,
		float HeightOverride = 0.0f,
		bool bUseWhiteBrush = false);
	static TSharedRef<SWidget> MakeTwoButtonRow(
		const TSharedRef<SWidget>& LeftButton,
		const TSharedRef<SWidget>& RightButton,
		const FMargin& LeftPadding = FMargin(10.0f, 0.0f),
		const FMargin& RightPadding = FMargin(10.0f, 0.0f),
		EVisibility Visibility = EVisibility::Visible);

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

	static const FSlateBrush* GetFlatOverlayBrush(ET66FlatOverlayChromeBrush Brush);
	static FT66FlatOverlayChromeButtonParams MakeFlatOverlayButtonParams(
		const FText& Label,
		FOnClicked OnClicked,
		ET66FlatOverlayChromeButtonFamily Family = ET66FlatOverlayChromeButtonFamily::Neutral);
	static TSharedRef<SWidget> MakeFlatOverlayPanel(
		const TSharedRef<SWidget>& Content,
		ET66FlatOverlayChromeBrush Brush = ET66FlatOverlayChromeBrush::ContentPanelWide,
		const FMargin& Padding = FMargin(16.f),
		TSharedPtr<class SBorder>* OutBorder = nullptr);
	static TSharedRef<SWidget> MakeFlatOverlayButton(const FT66FlatOverlayChromeButtonParams& Params);
	static TSharedRef<SWidget> MakeFlatOverlaySlotPanel(
		const TSharedRef<SWidget>& Content,
		const TAttribute<bool>& IsSelected = false,
		const TAttribute<bool>& IsEnabled = true,
		const FMargin& Padding = FMargin(6.f));

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
