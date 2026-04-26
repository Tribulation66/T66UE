// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"     // FComboButtonStyle for GetDropdownComboButtonStyle
#include "Widgets/Input/SButton.h"  // FOnClicked used by MakeButton helper
#include "Widgets/Layout/SBorder.h" // SBorder used by MakePanel OutBorder parameter

/** Button semantic types for MakeButton. */
enum class ET66ButtonType : uint8
{
	Neutral,       // Default: dark panel background (Panel2)
	Primary,       // Action/confirm: accent color   (Accent2)
	Danger,        // Destructive/warning: red        (Danger)
	Success,       // Positive confirm: green         (Success)
	ToggleActive,  // Active toggle state (inverted colors)
	Row,           // Leaderboard/list row: transparent bg, thin border, content-driven layout
};

/** Optional decorative border treatment layered on top of the base button behavior. */
enum class ET66ButtonBorderVisual : uint8
{
	Default,   // Resolve from the active shared style theme
	None,      // Use the base button chrome only (no decorative overlay)
	RetroSky,  // Legacy full-rect retro border treatment
	RetroWood, // Thick rectangular border with a banded wood-grain trim
};

/** Optional background treatment layered under the base button body. */
enum class ET66ButtonBackgroundVisual : uint8
{
	Default,        // Resolve from the active shared style theme
	None,           // Use the base button body only (no decorative fill)
};

/**
 * Parameters for the centralized MakeButton factory.
 * Every button in the game should be built through this struct so that
 * debounce, texture backgrounds, padding, and font are driven from one place.
 *
 * Usage (simple):
 *   FT66Style::MakeButton(FT66ButtonParams(Label, OnClicked, ET66ButtonType::Primary).SetMinWidth(200.f))
 *
 * Usage (advanced):
 *   FT66Style::MakeButton(
 *       FT66ButtonParams(Label, OnClicked)
 *       .SetFontSize(44)
 *       .SetPadding(FMargin(14.f))
 *       .SetColor(FSlateColor(SomeColor))
 *       .SetEnabled(TAttribute<bool>::CreateLambda([](){ return ...; }))
 *   )
 */
struct FT66ButtonParams
{
	// === Required ===
	FText Label;
	FOnClicked OnClicked;

	// === Visual Defaults ===
	ET66ButtonType Type = ET66ButtonType::Neutral;
	ET66ButtonBorderVisual BorderVisual = ET66ButtonBorderVisual::Default;
	ET66ButtonBackgroundVisual BackgroundVisual = ET66ButtonBackgroundVisual::Default;
	float MinWidth      = 120.f;
	float Height        = 0.f;            // 0 = content-driven (recommended), >0 = explicit override
	int32 FontSize      = 0;              // 0 = use T66.Text.Button default (16pt bold)
	FString FontWeight  = TEXT("Bold");   // Uses the locked UI font; weight is kept for call-site intent.
	FMargin Padding     = FMargin(-1.f);  // Negative = use ButtonStyle default padding

	// === Dynamic State ===
	TAttribute<bool> IsEnabled            = true;
	TAttribute<EVisibility> Visibility    = EVisibility::Visible;
	TAttribute<FText> DynamicLabel;       // If bound, replaces Label at runtime

	// === Color Overrides ===
	bool bHasColorOverride     = false;
	TAttribute<FSlateColor> ColorOverride;          // Overrides ButtonColorAndOpacity

	bool bHasTextColorOverride = false;
	TAttribute<FSlateColor> TextColorOverride;      // Overrides text ColorAndOpacity
	bool bHasStateTextColors = false;
	FSlateColor NormalStateTextColor = FSlateColor::UseForeground();
	FSlateColor HoveredStateTextColor = FSlateColor::UseForeground();
	FSlateColor PressedStateTextColor = FSlateColor::UseForeground();
	bool bHasStateTextSecondaryColors = false;
	FSlateColor NormalStateTextSecondaryColor = FSlateColor::UseForeground();
	FSlateColor HoveredStateTextSecondaryColor = FSlateColor::UseForeground();
	FSlateColor PressedStateTextSecondaryColor = FSlateColor::UseForeground();
	float TextDualToneSplit = 0.58f;
	bool bHasStateTextShadowColors = false;
	FLinearColor NormalStateTextShadowColor = FLinearColor::Transparent;
	FLinearColor HoveredStateTextShadowColor = FLinearColor::Transparent;
	FLinearColor PressedStateTextShadowColor = FLinearColor::Transparent;
	bool bHasTextShadowOffset = false;
	FVector2D TextShadowOffset = FVector2D::ZeroVector;
	bool bUseGlow = true;
	bool bUseDotaPlateOverlay = false;
	const FSlateBrush* DotaPlateOverrideBrush = nullptr;

	// === Custom Content ===
	TSharedPtr<SWidget> CustomContent;               // If set, replaces text block entirely

	// Constructors
	FT66ButtonParams() = default;
	FT66ButtonParams(const FText& InLabel, FOnClicked InOnClicked, ET66ButtonType InType = ET66ButtonType::Neutral)
		: Label(InLabel), OnClicked(MoveTemp(InOnClicked)), Type(InType) {}

	// Builder-style setters (return *this for chaining)
	FT66ButtonParams& SetBorderVisual(ET66ButtonBorderVisual V)          { BorderVisual = V; return *this; }
	FT66ButtonParams& SetBackgroundVisual(ET66ButtonBackgroundVisual V)  { BackgroundVisual = V; return *this; }
	FT66ButtonParams& SetMinWidth(float W)                                { MinWidth = W; return *this; }
	FT66ButtonParams& SetHeight(float H)                                  { Height = H; return *this; }
	FT66ButtonParams& SetFontSize(int32 S)                                { FontSize = S; return *this; }
	FT66ButtonParams& SetFontWeight(const TCHAR* Weight)                  { FontWeight = Weight; return *this; }
	FT66ButtonParams& SetPadding(const FMargin& M)                        { Padding = M; return *this; }
	FT66ButtonParams& SetColor(const TAttribute<FSlateColor>& C)          { ColorOverride = C; bHasColorOverride = true; return *this; }
	FT66ButtonParams& SetColor(const FLinearColor& C)                     { ColorOverride = FSlateColor(C); bHasColorOverride = true; return *this; }
	FT66ButtonParams& SetTextColor(const TAttribute<FSlateColor>& C)      { TextColorOverride = C; bHasTextColorOverride = true; return *this; }
	FT66ButtonParams& SetTextColor(const FLinearColor& C)                 { TextColorOverride = FSlateColor(C); bHasTextColorOverride = true; return *this; }
	FT66ButtonParams& SetStateTextColors(const FLinearColor& Normal, const FLinearColor& Hovered, const FLinearColor& Pressed)
	{
		NormalStateTextColor = FSlateColor(Normal);
		HoveredStateTextColor = FSlateColor(Hovered);
		PressedStateTextColor = FSlateColor(Pressed);
		bHasStateTextColors = true;
		return *this;
	}
	FT66ButtonParams& SetStateTextSecondaryColors(const FLinearColor& Normal, const FLinearColor& Hovered, const FLinearColor& Pressed)
	{
		NormalStateTextSecondaryColor = FSlateColor(Normal);
		HoveredStateTextSecondaryColor = FSlateColor(Hovered);
		PressedStateTextSecondaryColor = FSlateColor(Pressed);
		bHasStateTextSecondaryColors = true;
		return *this;
	}
	FT66ButtonParams& SetTextDualToneSplit(float InSplit)                { TextDualToneSplit = FMath::Clamp(InSplit, 0.1f, 0.9f); return *this; }
	FT66ButtonParams& SetStateTextShadowColors(const FLinearColor& Normal, const FLinearColor& Hovered, const FLinearColor& Pressed)
	{
		NormalStateTextShadowColor = Normal;
		HoveredStateTextShadowColor = Hovered;
		PressedStateTextShadowColor = Pressed;
		bHasStateTextShadowColors = true;
		return *this;
	}
	FT66ButtonParams& SetTextShadowOffset(const FVector2D& Offset)       { TextShadowOffset = Offset; bHasTextShadowOffset = true; return *this; }
	FT66ButtonParams& SetUseGlow(bool bInUseGlow)                         { bUseGlow = bInUseGlow; return *this; }
	FT66ButtonParams& SetUseDotaPlateOverlay(bool bInUseDotaPlateOverlay) { bUseDotaPlateOverlay = bInUseDotaPlateOverlay; return *this; }
	FT66ButtonParams& SetDotaPlateOverrideBrush(const FSlateBrush* InDotaPlateOverrideBrush) { DotaPlateOverrideBrush = InDotaPlateOverrideBrush; return *this; }
	FT66ButtonParams& SetEnabled(const TAttribute<bool>& E)               { IsEnabled = E; return *this; }
	FT66ButtonParams& SetVisibility(const TAttribute<EVisibility>& V)     { Visibility = V; return *this; }
	FT66ButtonParams& SetDynamicLabel(const TAttribute<FText>& L)         { DynamicLabel = L; return *this; }
	FT66ButtonParams& SetContent(const TSharedRef<SWidget>& W)            { CustomContent = W; return *this; }
};

/** Panel semantic types for MakePanel. */
enum class ET66PanelType : uint8
{
	Bg,       // Full-screen/section background
	Panel,    // Primary content panel
	Panel2,   // Secondary/inner panel (item tiles, inventory slots, etc.)
};

/**
 * Parameters for the centralized MakePanel factory.
 * Every panel in the game should be built through this struct so that
 * texture backgrounds, padding, and borders are driven from one place.
 *
 * Usage (simple):
 *   FT66Style::MakePanel(Content)
 *
 * Usage (advanced):
 *   FT66Style::MakePanel(Content,
 *       FT66PanelParams(ET66PanelType::Panel2)
 *       .SetPadding(FMargin(8.f))
 *       .SetVisibility(TAttribute<EVisibility>::CreateLambda([](){ return ...; }))
 *   )
 *
 * Usage (SAssignNew — pass pointer to capture the SBorder):
 *   TSharedPtr<SBorder> MyBorder;
 *   FT66Style::MakePanel(Content, FT66PanelParams(), &MyBorder)
 */
struct FT66PanelParams
{
	ET66PanelType Type = ET66PanelType::Panel;
	ET66ButtonBorderVisual BorderVisual = ET66ButtonBorderVisual::Default;
	ET66ButtonBackgroundVisual BackgroundVisual = ET66ButtonBackgroundVisual::Default;
	FMargin Padding    = FMargin(16.f);
	TAttribute<EVisibility> Visibility = EVisibility::Visible;

	// Color override (tints the BorderBackgroundColor)
	bool bHasColorOverride = false;
	TAttribute<FSlateColor> ColorOverride;

	// Constructors
	FT66PanelParams() = default;
	FT66PanelParams(ET66PanelType InType) : Type(InType) {}

	// Builder-style setters
	FT66PanelParams& SetBorderVisual(ET66ButtonBorderVisual V)          { BorderVisual = V; return *this; }
	FT66PanelParams& SetBackgroundVisual(ET66ButtonBackgroundVisual V)  { BackgroundVisual = V; return *this; }
	FT66PanelParams& SetPadding(const FMargin& P)                        { Padding = P; return *this; }
	FT66PanelParams& SetPadding(float P)                                  { Padding = FMargin(P); return *this; }
	FT66PanelParams& SetColor(const TAttribute<FSlateColor>& C)          { ColorOverride = C; bHasColorOverride = true; return *this; }
	FT66PanelParams& SetColor(const FLinearColor& C)                     { ColorOverride = FSlateColor(C); bHasColorOverride = true; return *this; }
	FT66PanelParams& SetVisibility(const TAttribute<EVisibility>& V)     { Visibility = V; return *this; }
};

/**
 * Parameters for the centralized MakeDropdown factory.
 * Every dropdown (combo button) uses the same dark presentation.
 *
 * Usage (simple):
 *   FT66Style::MakeDropdown(FT66DropdownParams(TriggerContent, OnGetMenuContent))
 *
 * Usage (advanced):
 *   FT66Style::MakeDropdown(FT66DropdownParams(TriggerContent, OnGetMenuContent)
 *       .SetMinWidth(200.f).SetHeight(32.f))
 */
struct FT66DropdownParams
{
	TSharedRef<SWidget> Content;
	TFunction<TSharedRef<SWidget>()> OnGetMenuContent;

	float MinWidth   = 0.f;
	float Height     = 32.f;
	FMargin Padding  = FMargin(8.f, 4.f);
	TAttribute<EVisibility> Visibility = EVisibility::Visible;

	FT66DropdownParams() = default;
	FT66DropdownParams(const TSharedRef<SWidget>& InContent, TFunction<TSharedRef<SWidget>()> InOnGetMenuContent)
		: Content(InContent), OnGetMenuContent(MoveTemp(InOnGetMenuContent)) {}

	FT66DropdownParams& SetMinWidth(float W)      { MinWidth = W; return *this; }
	FT66DropdownParams& SetHeight(float H)       { Height = H; return *this; }
	FT66DropdownParams& SetPadding(const FMargin& M) { Padding = M; return *this; }
	FT66DropdownParams& SetVisibility(const TAttribute<EVisibility>& V) { Visibility = V; return *this; }
};

/**
 * T66 UI style system (Slate).
 *
 * Goals:
 * - Consistent look across all Slate screens/widgets
 * - Centralized "design tokens" (colors, font sizes, corner radius, spacing)
 * - No per-frame work; pure data/styles
 *
 * Note: This intentionally avoids requiring editor-created assets.
 * If we later want a custom font/icon set, we can load raw files from Content/Slate/ at runtime.
 */
class T66_API FT66Style
{
public:
	static void Initialize();
	static void Shutdown();

	static bool IsDotaTheme();
	static FSlateFontInfo MakeFont(const TCHAR* Weight, int32 Size);
	static FLinearColor Background();
	static FLinearColor PanelOuter();
	static FLinearColor Panel();
	static FLinearColor PanelInner();
	static FLinearColor Stroke();
	static FLinearColor Scrim();
	static FLinearColor Text();
	static FLinearColor TextMuted();
	static FLinearColor Accent();
	static FLinearColor Accent2();
	static FLinearColor Border();
	static FLinearColor HeaderBar();
	static FLinearColor HeaderAccent();
	static FLinearColor Success();
	static FLinearColor Danger();
	static FLinearColor ScreenBackground();
	static FLinearColor ScreenText();
	static FLinearColor ScreenMuted();
	static FLinearColor SlotOuter();
	static FLinearColor SlotInner();
	static FLinearColor SlotFill();
	static FLinearColor BossBarBackground();
	static FLinearColor BossBarFill();
	static FLinearColor PromptBackground();
	static FLinearColor SelectionFill();
	static FLinearColor MinimapBackground();
	static FLinearColor MinimapTerrain();
	static FLinearColor MinimapGrid();
	static FLinearColor MinimapFriendly();
	static FLinearColor MinimapEnemy();
	static FLinearColor MinimapNeutral();
	static FLinearColor ButtonNeutral();
	static FLinearColor ButtonHovered();
	static FLinearColor ButtonPressed();
	static FLinearColor ButtonPrimary();
	static FLinearColor ButtonPrimaryHovered();
	static FLinearColor ButtonPrimaryPressed();
	static FLinearColor DangerButton();
	static FLinearColor DangerButtonHovered();
	static FLinearColor DangerButtonPressed();
	static FLinearColor SuccessButton();
	static FLinearColor SuccessButtonHovered();
	static FLinearColor SuccessButtonPressed();
	static FLinearColor ToggleButton();
	static FLinearColor ToggleButtonHovered();
	static FLinearColor ToggleButtonPressed();
	static float CornerRadius();
	static float CornerRadiusSmall();

	static const ISlateStyle& Get();
	static FName GetStyleSetName();

	// ----- Design tokens -----
	struct Tokens
	{
		// Colors
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

		// Sizing
		static constexpr float ReferenceLayoutWidth = 1280.f;
		static constexpr float ReferenceLayoutHeight = 720.f;
		static constexpr float SafeFrameAspectRatio = 16.f / 9.f;
		static constexpr float CornerRadius = 10.f;
		static constexpr float CornerRadiusSmall = 8.f;
		static constexpr float StrokeWidth = 1.f;
		static constexpr float BorderWidth = 1.f;   // White outline around panels/buttons
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

		// Vendor/Gambler shared layout — all fixed sizes (no FillWidth/FillHeight in main row).
		// See Tokens docs or this file for full width/height list.
		static constexpr float InventorySlotSize = 160.f;   // Inventory slots on Vendor and Gambler screens
		static constexpr float ItemPanelIconSize = 200.f;   // Vendor shop item icon; Gambler game card icon
		static constexpr float GameCardMinWidth = 260.f;
		static constexpr float GameCardHeight = 200.f;
		/** Center panel width for NPC overlays (Vendor shop + Gambler casino). */
		static constexpr float NPCCenterPanelTotalWidth = 920.f;
		/** Right panel width (anger + bank) for NPC overlays. */
		static constexpr float NPCRightPanelWidth = 380.f;
		/** Fixed height of the main 3-column row (Stats | Shop/Casino | Bank) on Vendor and Gambler. */
		static constexpr float NPCMainRowHeight = 600.f;
		/** Fixed height of the inventory panel strip at the bottom on Vendor and Gambler. */
		static constexpr float NPCInventoryPanelHeight = 180.f;
		/** Gambler/Casino inventory panel height (+40% vs base). */
		static constexpr float NPCGamblerInventoryPanelHeight = 252.f;  // 180 * 1.4
		/** Stats panel width (Vendor and Gambler); single source of truth. */
		static constexpr float NPCStatsPanelWidth = 300.f;
		/** Fixed height of the scrollable stats content (primary + secondary); prevents panel from pushing other UI off screen. */
		static constexpr float NPCStatsPanelContentHeight = 400.f;
		/** Vendor stats panel width (same as NPCStatsPanelWidth). */
		static constexpr float NPCVendorStatsPanelWidth = NPCStatsPanelWidth;
		/** Gambler stats panel width (same as NPCStatsPanelWidth). */
		static constexpr float NPCGamblerStatsPanelWidth = NPCStatsPanelWidth;
		/** Vendor shop item card: width and height (each of the 3 cards). */
		static constexpr float NPCShopCardWidth = 248.f;
		static constexpr float NPCShopCardHeight = 500.f;
		/** Compact embedded casino item card footprint shared by vendor, gambling, and alchemy. */
		static constexpr float NPCCompactShopCardWidth = 148.f;
		static constexpr float NPCCompactShopCardHeight = 264.f;
		/** Anger face circle size (Vendor and Gambler right panel). */
		static constexpr float NPCAngerCircleSize = 170.f;
		/** Bank spinbox width (Borrow/Payback amount). */
		static constexpr float NPCBankSpinBoxWidth = 110.f;
		/** Bank spinbox height and similar single-line control height. */
		static constexpr float NPCBankSpinBoxHeight = 44.f;
		/** Root overlay padding (Vendor/Gambler). */
		static constexpr float NPCOverlayPadding = 24.f;

		// Spacing scale (4pt baseline)
		static constexpr float Space2 = 8.f;
		static constexpr float Space3 = 12.f;
		static constexpr float Space4 = 16.f;
		static constexpr float Space5 = 20.f;
		static constexpr float Space6 = 24.f;
		static constexpr float Space8 = 32.f;

		// Default button content padding (prevents text clipping; applies to all T66 button styles).
		// Individual buttons can still override via SButton::ContentPadding().
		static const FMargin ButtonPadding;
		static const FMargin ButtonPaddingPressed;
		static constexpr float ButtonGlowPadding = 6.f;
		static constexpr float ButtonHoverGlowIntensity = 0.75f;
		static constexpr float ButtonPressedGlowIntensity = 1.10f;
		static constexpr float ButtonGlowFallbackOpacity = 0.28f;

		// Font: all text uses the locked Jersey 10 UI stack.
		static FSlateFontInfo FontRegular(int32 Size);
		static FSlateFontInfo FontBold(int32 Size);
		static FSlateFontInfo FontTitle();
		static FSlateFontInfo FontHeading();
		static FSlateFontInfo FontBody();
		static FSlateFontInfo FontSmall();
		static FSlateFontInfo FontChip();
		static FSlateFontInfo FontButton();
	};

	/**
	 * Wrap an OnClicked delegate with a global debounce guard.
	 * Rapid clicks within a short window (150 ms) are silently dropped.
	 * Best practice: use this for every interactive button to prevent
	 * double-fire, UI rebuild crashes, and accidental double-navigation.
	 */
	static FOnClicked DebounceClick(const FOnClicked& InnerDelegate);

	/**
	 * Create a standard T66 button (SBox + SButton + STextBlock).
	 * All buttons in the game should use one of these overloads so behavior
	 * (debounce, texture backgrounds, sizing, font) is driven from one place.
	 */

	/** Full params overload — handles every button variant in the game. */
	static TSharedRef<SWidget> MakeButton(const FT66ButtonParams& Params);

	static FT66ButtonParams MakeInRunButtonParams(
		const FText& Label,
		FOnClicked OnClicked,
		ET66ButtonType Type = ET66ButtonType::Neutral);
	static const FSlateBrush* GetInRunButtonPlateBrush(ET66ButtonType Type);
	static const FSlateBrush* GetInRunTabPlateBrush(bool bSelected);

	/** Convenience overload for simple buttons (wraps the params version). */
	static TSharedRef<SWidget> MakeButton(
		const FText& Label,
		FOnClicked OnClicked,
		ET66ButtonType Type = ET66ButtonType::Neutral,
		float MinWidth = 120.f);

	/**
	 * Create a standard T66 panel (SBorder wrapping Content).
	 * All panels/cards in the game should use this so texture backgrounds,
	 * border styling, and padding are driven from one place.
	 *
	 * @param Content     Child widget placed inside the panel.
	 * @param Params      Panel configuration (type, padding, color, visibility).
	 * @param OutBorder   If non-null, receives the SBorder pointer for later runtime changes
	 *                    (e.g. highlighting a selected inventory slot).
	 */
	static TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		const FT66PanelParams& Params = FT66PanelParams(),
		TSharedPtr<SBorder>* OutBorder = nullptr);

	/** Convenience overload: Panel type + padding only, no params struct. */
	static TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		ET66PanelType Type,
		FMargin Padding = FMargin(16.f));

	/** Check whether panel textures are loaded. */
	static bool HasPanelTextures();

	/** Shared combo style (Panel bg, Text for arrow). Use for MakeDropdown and for SComboBox::ComboButtonStyle(). */
	static const FComboButtonStyle& GetDropdownComboButtonStyle();

	/** Build a dropdown (SComboButton). All dropdowns should use this or apply GetDropdownComboButtonStyle() to SComboBox. */
	static TSharedRef<SWidget> MakeDropdown(const FT66DropdownParams& Params);
	static TSharedRef<SWidget> MakeScreenSurface(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(14.f));
	static TSharedRef<SWidget> MakeViewportFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(6.f));
	static TSharedRef<SWidget> MakeViewportCutoutFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(6.f));
	static TSharedRef<SWidget> MakeSlotFrame(const TSharedRef<SWidget>& Content, const TAttribute<FSlateColor>& AccentColor, const FMargin& Padding = FMargin(1.f));
	static TSharedRef<SWidget> MakeSlotFrame(const TSharedRef<SWidget>& Content, const FLinearColor& AccentColor, const FMargin& Padding = FMargin(1.f));
	static TSharedRef<SWidget> MakeHudPanel(const TSharedRef<SWidget>& Content, const FText& Title, const FMargin& Padding = FMargin(12.f, 10.f));
	static TSharedRef<SWidget> MakeHudPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(12.f, 10.f));
	static TSharedRef<SWidget> MakeDivider(float Height = 1.f);
	static TSharedRef<SWidget> MakeMinimapFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(10.f));

	/** Wrap a Slate subtree in a viewport-aware DPI scaler using the given design resolution. */
	static TSharedRef<SWidget> MakeResponsiveRoot(
		const TSharedRef<SWidget>& Content,
		const FVector2D& ReferenceResolution = FVector2D(Tokens::ReferenceLayoutWidth, Tokens::ReferenceLayoutHeight),
		bool bAllowUpscale = true);

	/** Compute the current viewport scale relative to the supplied design resolution. */
	static float GetViewportResponsiveScale(
		const FVector2D& ReferenceResolution = FVector2D(Tokens::ReferenceLayoutWidth, Tokens::ReferenceLayoutHeight),
		bool bAllowUpscale = true);

	/** Get the current game viewport size in physical pixels, or the baseline reference size if unavailable. */
	static FVector2D GetViewportSize();

	/** Get the current viewport size expressed in Slate units after engine/player scaling. */
	static FVector2D GetViewportLogicalSize();

	/** Get the current engine DPI scale derived from /Script/Engine.UserInterfaceSettings. */
	static float GetEngineDPIScale();

	/** Get the persisted player UI scale multiplier. */
	static float GetPlayerUIScale();

	/** Get the combined engine DPI scale and player UI scale multiplier. */
	static float GetGlobalUIScale();

	/** Get the centered safe-frame size in Slate units for a target aspect ratio. */
	static FVector2D GetSafeFrameSize(float AspectRatio = Tokens::SafeFrameAspectRatio);

	/** Get the centered safe-frame gutters for a target aspect ratio. */
	static FMargin GetSafeFrameInsets(float AspectRatio = Tokens::SafeFrameAspectRatio);

	/** Apply safe-frame gutters to a local padding value. */
	static FMargin GetSafePadding(
		const FMargin& Padding,
		float AspectRatio = Tokens::SafeFrameAspectRatio);

	/** Center content inside the shared safe frame with optional padding and max logical size caps. */
	static TSharedRef<SWidget> MakeSafeFrame(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding = FMargin(0.f),
		float MaxWidth = 0.f,
		float MaxHeight = 0.f,
		float AspectRatio = Tokens::SafeFrameAspectRatio);

	/**
	 * Schedule a safe widget rebuild for the next tick.
	 *
	 * ALWAYS use this instead of calling ReleaseSlateResources / TakeWidget /
	 * ForceRebuildSlate directly.  Deferring to the next tick ensures that
	 * Slate has finished processing the current click/key event before the
	 * widget tree is torn down, preventing dangling-pointer crashes.
	 *
	 * @param Widget   The UUserWidget whose Slate tree should be rebuilt.
	 * @param ZOrder   Viewport Z-order to restore (0 = screens, 100 = modals).
	 *
	 * Cost: one TFunction push onto the timer queue (small-object-optimized,
	 * no heap alloc).  Fires on the very next tick — imperceptible delay.
	 */
	static void DeferRebuild(UUserWidget* Widget, int32 ZOrder = 0);

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;
	static ET66ButtonBorderVisual ResolveButtonBorderVisual(const FT66ButtonParams& Params);
	static ET66ButtonBackgroundVisual ResolveButtonBackgroundVisual(const FT66ButtonParams& Params);
	static ET66ButtonBorderVisual ResolvePanelBorderVisual(const FT66PanelParams& Params);
	static ET66ButtonBackgroundVisual ResolvePanelBackgroundVisual(const FT66PanelParams& Params);
	static float ResolveButtonDecorativeBorderThickness(const FT66ButtonParams& Params, int32 EffectiveFontSize, float MaxThickness);
	static float ResolvePanelDecorativeBorderThickness(const FT66PanelParams& Params, float MaxThickness);
};
