// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SButton.h"  // FOnClicked used by MakeButton helper
#include "Widgets/Layout/SBorder.h" // SBorder used by MakePanel OutBorder parameter

/** UI color theme (Dark / Light). */
enum class ET66UITheme : uint8
{
	Dark,    // Default: black panels, gold text
	Light,   // Grey panels, white text
};

/** Button semantic types for MakeButton. */
enum class ET66ButtonType : uint8
{
	Neutral,       // Default: dark panel background (Panel2)
	Primary,       // Action/confirm: accent color   (Accent2)
	Danger,        // Destructive/warning: red        (Danger)
	Success,       // Positive confirm: green         (Success)
	ToggleActive,  // Active toggle state (inverted colors)
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
	float MinWidth      = 120.f;
	float Height        = 0.f;            // 0 = content-driven (recommended), >0 = explicit override
	int32 FontSize      = 0;              // 0 = use T66.Text.Button default (16pt bold)
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

	// === Custom Content ===
	TSharedPtr<SWidget> CustomContent;               // If set, replaces text block entirely

	// Constructors
	FT66ButtonParams() = default;
	FT66ButtonParams(const FText& InLabel, FOnClicked InOnClicked, ET66ButtonType InType = ET66ButtonType::Neutral)
		: Label(InLabel), OnClicked(MoveTemp(InOnClicked)), Type(InType) {}

	// Builder-style setters (return *this for chaining)
	FT66ButtonParams& SetMinWidth(float W)                                { MinWidth = W; return *this; }
	FT66ButtonParams& SetHeight(float H)                                  { Height = H; return *this; }
	FT66ButtonParams& SetFontSize(int32 S)                                { FontSize = S; return *this; }
	FT66ButtonParams& SetPadding(const FMargin& M)                        { Padding = M; return *this; }
	FT66ButtonParams& SetColor(const TAttribute<FSlateColor>& C)          { ColorOverride = C; bHasColorOverride = true; return *this; }
	FT66ButtonParams& SetColor(const FLinearColor& C)                     { ColorOverride = FSlateColor(C); bHasColorOverride = true; return *this; }
	FT66ButtonParams& SetTextColor(const TAttribute<FSlateColor>& C)      { TextColorOverride = C; bHasTextColorOverride = true; return *this; }
	FT66ButtonParams& SetTextColor(const FLinearColor& C)                 { TextColorOverride = FSlateColor(C); bHasTextColorOverride = true; return *this; }
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
	FMargin Padding    = FMargin(16.f);
	TAttribute<EVisibility> Visibility = EVisibility::Visible;

	// Color override (tints the BorderBackgroundColor)
	bool bHasColorOverride = false;
	TAttribute<FSlateColor> ColorOverride;

	// Constructors
	FT66PanelParams() = default;
	FT66PanelParams(ET66PanelType InType) : Type(InType) {}

	// Builder-style setters
	FT66PanelParams& SetPadding(const FMargin& P)                        { Padding = P; return *this; }
	FT66PanelParams& SetPadding(float P)                                  { Padding = FMargin(P); return *this; }
	FT66PanelParams& SetColor(const TAttribute<FSlateColor>& C)          { ColorOverride = C; bHasColorOverride = true; return *this; }
	FT66PanelParams& SetColor(const FLinearColor& C)                     { ColorOverride = FSlateColor(C); bHasColorOverride = true; return *this; }
	FT66PanelParams& SetVisibility(const TAttribute<EVisibility>& V)     { Visibility = V; return *this; }
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
class FT66Style
{
public:
	static void Initialize();
	static void Shutdown();

	/** Set the UI color theme (Dark/Light). Re-initializes all styles so widgets rebuilt after this call use the new palette. */
	static void SetTheme(ET66UITheme NewTheme);
	static ET66UITheme GetTheme();

	/** Cycle to the next UI font (0–4). Call after changing to refresh style. */
	static void CycleToNextFont();

	/** Toggle force-bold: when on, all UI text uses the bold variant of the current font (if available). Console: T66Bold */
	static void ToggleBoldFont();

	static const ISlateStyle& Get();
	static FName GetStyleSetName();

	// ----- Design tokens (mutable: updated by SetTheme) -----
	struct Tokens
	{
		// Colors (non-const so SetTheme can swap palettes at runtime)
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
		static constexpr float CornerRadius = 10.f;
		static constexpr float CornerRadiusSmall = 8.f;
		static constexpr float StrokeWidth = 1.f;
		static constexpr float BorderWidth = 1.f;   // White outline around panels/buttons

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

		// Font: all text uses the selected font. In T66Style.cpp set GThemeFontIndex (0–5: Caesar Dressing, Cinzel, Cormorant SC, Germania One, Grenze, Alagard).
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
	 * Create a standard T66 themed button (SBox + SButton + STextBlock).
	 * All buttons in the game should use one of these overloads so behavior
	 * (debounce, texture backgrounds, sizing, font) is driven from one place.
	 */

	/** Full params overload — handles every button variant in the game. */
	static TSharedRef<SWidget> MakeButton(const FT66ButtonParams& Params);

	/** Convenience overload for simple buttons (wraps the params version). */
	static TSharedRef<SWidget> MakeButton(
		const FText& Label,
		FOnClicked OnClicked,
		ET66ButtonType Type = ET66ButtonType::Neutral,
		float MinWidth = 120.f);

	/**
	 * Create a standard T66 themed panel (SBorder wrapping Content).
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

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};

