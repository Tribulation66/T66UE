// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66Style.h"

#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Fonts/CompositeFont.h"

TSharedPtr<FSlateStyleSet> FT66Style::StyleInstance;

// --- Colors ---
const FLinearColor FT66Style::Tokens::Bg(0.015f, 0.015f, 0.020f, 1.0f);
const FLinearColor FT66Style::Tokens::Panel(0.045f, 0.045f, 0.065f, 1.0f);
const FLinearColor FT66Style::Tokens::Panel2(0.075f, 0.075f, 0.105f, 1.0f);
const FLinearColor FT66Style::Tokens::Stroke(0.18f, 0.18f, 0.24f, 1.0f);
const FLinearColor FT66Style::Tokens::Scrim(0.f, 0.f, 0.f, 0.70f);
const FLinearColor FT66Style::Tokens::Text(1.0f, 1.0f, 1.0f, 1.0f);
const FLinearColor FT66Style::Tokens::TextMuted(0.78f, 0.78f, 0.86f, 1.0f);
const FLinearColor FT66Style::Tokens::Accent(0.85f, 0.20f, 0.25f, 1.0f);   // blood red
const FLinearColor FT66Style::Tokens::Accent2(0.20f, 0.55f, 0.90f, 1.0f);  // cold blue
const FLinearColor FT66Style::Tokens::Danger(0.95f, 0.15f, 0.15f, 1.0f);
const FLinearColor FT66Style::Tokens::Success(0.20f, 0.80f, 0.35f, 1.0f);

namespace
{
	// Optional "cartoony" font support without editor imports:
	// If you drop files into Content/Slate/Fonts/, we use them.
	// - Content/Slate/Fonts/T66_Cartoon.ttf
	// - Content/Slate/Fonts/T66_Cartoon_Bold.ttf
	FSlateFontInfo MakeFont(const TCHAR* Weight, int32 Size)
	{
		const FString FontsDir = FPaths::ProjectContentDir() / TEXT("Slate/Fonts");
		const FString Cartoon = FontsDir / TEXT("T66_Cartoon.ttf");
		const FString CartoonBold = FontsDir / TEXT("T66_Cartoon_Bold.ttf");

		const bool bWantBold = FCString::Stricmp(Weight, TEXT("Bold")) == 0;
		const FString& Candidate = (bWantBold && FPaths::FileExists(CartoonBold)) ? CartoonBold : Cartoon;

		if (FPaths::FileExists(Candidate))
		{
			// Avoid deprecated filename-based FSlateFontInfo ctors; build a tiny runtime composite font instead.
			static TMap<FString, TSharedPtr<const FCompositeFont>> FontCache;
			TSharedPtr<const FCompositeFont>& Cached = FontCache.FindOrAdd(Candidate);
			if (!Cached.IsValid())
			{
				Cached = MakeShared<FStandaloneCompositeFont>(
					FName(TEXT("Default")),
					Candidate,
					EFontHinting::Default,
					EFontLoadingPolicy::LazyLoad
				);
			}

			return FSlateFontInfo(Cached, Size);
		}

		// Fallback (engine default)
		return FCoreStyle::GetDefaultFontStyle(Weight, Size);
	}
}

// --- Fonts (cartoon if present; else Roboto) ---
FSlateFontInfo FT66Style::Tokens::FontRegular(int32 Size)
{
	return MakeFont(TEXT("Regular"), Size);
}
FSlateFontInfo FT66Style::Tokens::FontBold(int32 Size)
{
	return MakeFont(TEXT("Bold"), Size);
}
FSlateFontInfo FT66Style::Tokens::FontTitle()
{
	return MakeFont(TEXT("Bold"), 52);
}
FSlateFontInfo FT66Style::Tokens::FontHeading()
{
	return MakeFont(TEXT("Bold"), 24);
}
FSlateFontInfo FT66Style::Tokens::FontBody()
{
	return MakeFont(TEXT("Regular"), 14);
}
FSlateFontInfo FT66Style::Tokens::FontSmall()
{
	return MakeFont(TEXT("Regular"), 11);
}
FSlateFontInfo FT66Style::Tokens::FontChip()
{
	return MakeFont(TEXT("Bold"), 11);
}
FSlateFontInfo FT66Style::Tokens::FontButton()
{
	return MakeFont(TEXT("Bold"), 16);
}

FName FT66Style::GetStyleSetName()
{
	static const FName Name(TEXT("T66Style"));
	return Name;
}

const ISlateStyle& FT66Style::Get()
{
	check(StyleInstance.IsValid());
	return *StyleInstance;
}

void FT66Style::Initialize()
{
	if (StyleInstance.IsValid())
	{
		return;
	}

	StyleInstance = MakeShared<FSlateStyleSet>(GetStyleSetName());

	// Rounded panel brushes
	StyleInstance->Set("T66.Brush.Bg", new FSlateRoundedBoxBrush(Tokens::Bg, Tokens::CornerRadius));
	StyleInstance->Set("T66.Brush.Panel", new FSlateRoundedBoxBrush(Tokens::Panel, Tokens::CornerRadius));
	StyleInstance->Set("T66.Brush.Panel2", new FSlateRoundedBoxBrush(Tokens::Panel2, Tokens::CornerRadius));
	StyleInstance->Set("T66.Brush.Stroke", new FSlateRoundedBoxBrush(Tokens::Stroke, Tokens::CornerRadiusSmall));

	// Text styles
	{
		const FLinearColor Shadow(0.f, 0.f, 0.f, 0.85f);

		FTextBlockStyle Title = FTextBlockStyle()
			.SetFont(Tokens::FontTitle())
			.SetColorAndOpacity(Tokens::Text)
			.SetShadowOffset(FVector2D(1.f, 1.f))
			.SetShadowColorAndOpacity(Shadow);
		StyleInstance->Set("T66.Text.Title", Title);

		FTextBlockStyle Heading = FTextBlockStyle()
			.SetFont(Tokens::FontHeading())
			.SetColorAndOpacity(Tokens::Text)
			.SetShadowOffset(FVector2D(1.f, 1.f))
			.SetShadowColorAndOpacity(Shadow);
		StyleInstance->Set("T66.Text.Heading", Heading);

		FTextBlockStyle Body = FTextBlockStyle()
			.SetFont(Tokens::FontBody())
			.SetColorAndOpacity(Tokens::TextMuted);
		StyleInstance->Set("T66.Text.Body", Body);

		FTextBlockStyle Chip = FTextBlockStyle()
			.SetFont(Tokens::FontChip())
			.SetColorAndOpacity(Tokens::Text);
		StyleInstance->Set("T66.Text.Chip", Chip);

		FTextBlockStyle ButtonText = FTextBlockStyle()
			.SetFont(Tokens::FontButton())
			.SetColorAndOpacity(Tokens::Text);
		StyleInstance->Set("T66.Text.Button", ButtonText);
	}

	// Button styles (use rounded box brushes; no asset dependency)
	{
		const FLinearColor PrimaryN = Tokens::Accent2 * 0.55f + Tokens::Panel2 * 0.45f;
		const FLinearColor PrimaryH = Tokens::Accent2 * 0.70f + Tokens::Panel2 * 0.30f;
		const FLinearColor PrimaryP = Tokens::Accent2 * 0.80f + Tokens::Panel2 * 0.20f;

		FButtonStyle Primary = FButtonStyle()
			.SetNormal(FSlateRoundedBoxBrush(PrimaryN, Tokens::CornerRadiusSmall))
			.SetHovered(FSlateRoundedBoxBrush(PrimaryH, Tokens::CornerRadiusSmall))
			.SetPressed(FSlateRoundedBoxBrush(PrimaryP, Tokens::CornerRadiusSmall));
		StyleInstance->Set("T66.Button.Primary", Primary);

		const FLinearColor NeutralN = Tokens::Panel2;
		const FLinearColor NeutralH = Tokens::Panel2 + FLinearColor(0.05f, 0.05f, 0.07f, 0.f);
		const FLinearColor NeutralP = Tokens::Panel2 + FLinearColor(0.08f, 0.08f, 0.10f, 0.f);

		FButtonStyle Neutral = FButtonStyle()
			.SetNormal(FSlateRoundedBoxBrush(NeutralN, Tokens::CornerRadiusSmall))
			.SetHovered(FSlateRoundedBoxBrush(NeutralH, Tokens::CornerRadiusSmall))
			.SetPressed(FSlateRoundedBoxBrush(NeutralP, Tokens::CornerRadiusSmall));
		StyleInstance->Set("T66.Button.Neutral", Neutral);

		const FLinearColor DangerN = Tokens::Danger * 0.55f + Tokens::Panel2 * 0.45f;
		const FLinearColor DangerH = Tokens::Danger * 0.70f + Tokens::Panel2 * 0.30f;
		const FLinearColor DangerP = Tokens::Danger * 0.80f + Tokens::Panel2 * 0.20f;

		FButtonStyle Danger = FButtonStyle()
			.SetNormal(FSlateRoundedBoxBrush(DangerN, Tokens::CornerRadiusSmall))
			.SetHovered(FSlateRoundedBoxBrush(DangerH, Tokens::CornerRadiusSmall))
			.SetPressed(FSlateRoundedBoxBrush(DangerP, Tokens::CornerRadiusSmall));
		StyleInstance->Set("T66.Button.Danger", Danger);
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
}

void FT66Style::Shutdown()
{
	if (!StyleInstance.IsValid())
	{
		return;
	}

	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	StyleInstance.Reset();
}

