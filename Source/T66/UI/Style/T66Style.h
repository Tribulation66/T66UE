// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

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

	static const ISlateStyle& Get();
	static FName GetStyleSetName();

	// ----- Design tokens -----
	struct Tokens
	{
		// Colors
		static const FLinearColor Bg;
		static const FLinearColor Panel;
		static const FLinearColor Panel2;
		static const FLinearColor Stroke;
		static const FLinearColor Scrim;
		static const FLinearColor Text;
		static const FLinearColor TextMuted;
		static const FLinearColor Accent;
		static const FLinearColor Accent2;
		static const FLinearColor Danger;
		static const FLinearColor Success;

		// Sizing
		static constexpr float CornerRadius = 10.f;
		static constexpr float CornerRadiusSmall = 8.f;
		static constexpr float StrokeWidth = 1.f;

		// Spacing scale (4pt baseline)
		static constexpr float Space2 = 8.f;
		static constexpr float Space3 = 12.f;
		static constexpr float Space4 = 16.f;
		static constexpr float Space5 = 20.f;
		static constexpr float Space6 = 24.f;
		static constexpr float Space8 = 32.f;

		// Fonts (Roboto fallback)
		static FSlateFontInfo FontRegular(int32 Size);
		static FSlateFontInfo FontBold(int32 Size);
		static FSlateFontInfo FontTitle();
		static FSlateFontInfo FontHeading();
		static FSlateFontInfo FontBody();
		static FSlateFontInfo FontSmall();
		static FSlateFontInfo FontChip();
		static FSlateFontInfo FontButton();
	};

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};

