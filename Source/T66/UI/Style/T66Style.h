// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SButton.h"  // FOnClicked used by MakeButton helper

/** UI color theme (Dark / Light). */
enum class ET66UITheme : uint8
{
	Dark,    // Default: black panels, gold text
	Light,   // Grey panels, white text
};

/** Button semantic types for MakeButton. */
enum class ET66ButtonType : uint8
{
	Neutral,   // Default: dark panel background (Panel2)
	Primary,   // Action/confirm: accent color   (Accent2)
	Danger,    // Destructive/warning: red        (Danger)
	Success,   // Positive confirm: green         (Success)
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

		// Font: all text uses the selected font. In T66Style.cpp set GThemeFontIndex (0–4: Caesar Dressing, Cinzel, Cormorant SC, Germania One, Grenze).
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
	 * Type determines the ButtonStyle + ButtonColorAndOpacity.
	 * Uses MinDesiredWidth (so text never clips) and a fixed HeightOverride.
	 * The OnClicked delegate is automatically wrapped with DebounceClick.
	 */
	static TSharedRef<SWidget> MakeButton(
		const FText& Label,
		FOnClicked OnClicked,
		ET66ButtonType Type = ET66ButtonType::Neutral,
		float MinWidth = 120.f,
		float Height = 40.f);

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};

