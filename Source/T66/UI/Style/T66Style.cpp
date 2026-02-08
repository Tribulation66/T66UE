// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66Style.h"
#include "T66.h"

#include "Engine/Texture2D.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

TSharedPtr<FSlateStyleSet> FT66Style::StyleInstance;

// --- Colors (defaults = Dark palette; SetTheme() overwrites at runtime) ---
FLinearColor FT66Style::Tokens::Bg(0.08f, 0.08f, 0.10f, 1.0f);
FLinearColor FT66Style::Tokens::Panel(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::Panel2(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::Stroke(0.18f, 0.18f, 0.20f, 1.0f);
FLinearColor FT66Style::Tokens::Scrim(0.f, 0.f, 0.f, 0.70f);
FLinearColor FT66Style::Tokens::Text(1.0f, 0.84f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::TextMuted(0.95f, 0.78f, 0.1f, 1.0f);
FLinearColor FT66Style::Tokens::Accent(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::Accent2(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::Danger(0.95f, 0.15f, 0.15f, 1.0f);
FLinearColor FT66Style::Tokens::Success(0.20f, 0.80f, 0.35f, 1.0f);
FLinearColor FT66Style::Tokens::Border(1.0f, 1.0f, 1.0f, 1.0f);
const FMargin FT66Style::Tokens::ButtonPadding(12.f, 4.f);
const FMargin FT66Style::Tokens::ButtonPaddingPressed(12.f, 5.f, 12.f, 3.f);

namespace
{
	// Keep the most recent old style set alive so existing Slate widgets' brush/style
	// raw pointers remain valid until the UI is rebuilt with fresh widgets.
	// Without this, calling StyleInstance.Reset() would free the FButtonStyle and
	// FSlateBrush objects that live Slate widgets still reference, causing access violations.
	static TSharedPtr<FSlateStyleSet> GPreviousStyleSet;

	// ----- Active UI theme -----
	static ET66UITheme GActiveTheme = ET66UITheme::Dark;

	// Button background textures (Content/UI/Assets/ButtonDark, ButtonLight). Kept alive so brushes stay valid.
	static TObjectPtr<UTexture2D> GButtonDarkTex = nullptr;
	static TObjectPtr<UTexture2D> GButtonLightTex = nullptr;

	static UTexture2D* GetButtonTextureForCurrentTheme()
	{
		return (GActiveTheme == ET66UITheme::Light) ? GButtonLightTex : GButtonDarkTex;
	}

	static FSlateBrush MakeButtonTextureBrush(UTexture2D* Tex, const FLinearColor& TintColor)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.SetResourceObject(Tex);
		Brush.TintColor = FSlateColor(TintColor);
		return Brush;
	}

	/** Overwrite Tokens:: colors with the palette for the given theme. */
	void ApplyThemePalette(ET66UITheme Theme)
	{
		using T = FT66Style::Tokens;
		if (Theme == ET66UITheme::Light)
		{
			// Light: grey panels, white text, dark borders
			T::Bg          = FLinearColor(0.70f, 0.70f, 0.73f, 1.0f);
			T::Panel       = FLinearColor(0.45f, 0.45f, 0.48f, 1.0f);
			T::Panel2      = FLinearColor(0.45f, 0.45f, 0.48f, 1.0f);
			T::Stroke      = FLinearColor(0.35f, 0.35f, 0.38f, 1.0f);
			T::Scrim       = FLinearColor(0.60f, 0.60f, 0.60f, 0.70f);
			T::Text        = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
			T::TextMuted   = FLinearColor(0.88f, 0.88f, 0.88f, 1.0f);
			T::Accent      = FLinearColor(0.45f, 0.45f, 0.48f, 1.0f);
			T::Accent2     = FLinearColor(0.45f, 0.45f, 0.48f, 1.0f);
			T::Danger      = FLinearColor(0.95f, 0.15f, 0.15f, 1.0f);
			T::Success     = FLinearColor(0.20f, 0.80f, 0.35f, 1.0f);
			T::Border      = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
		}
		else // Dark (default)
		{
			// Dark: pitch black panels, gold-yellow text, white borders
			T::Bg          = FLinearColor(0.08f, 0.08f, 0.10f, 1.0f);
			T::Panel       = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
			T::Panel2      = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
			T::Stroke      = FLinearColor(0.18f, 0.18f, 0.20f, 1.0f);
			T::Scrim       = FLinearColor(0.0f, 0.0f, 0.0f, 0.70f);
			T::Text        = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
			T::TextMuted   = FLinearColor(0.95f, 0.78f, 0.1f, 1.0f);
			T::Accent      = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
			T::Accent2     = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
			T::Danger      = FLinearColor(0.95f, 0.15f, 0.15f, 1.0f);
			T::Success     = FLinearColor(0.20f, 0.80f, 0.35f, 1.0f);
			T::Border      = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	// ----- Font options: 0=Caesar Dressing, 1=Cinzel, 2=Cormorant SC, 3=Germania One, 4=Grenze -----
	static int32 GThemeFontIndex = 1;  // Cinzel (default); cycle with T66NextFont console command
	static bool GForceBoldFont = false; // When true, use bold variant for all text (T66Bold console command)

	static const TCHAR* GThemeFontPaths[] = {
		TEXT("New Fonts/Caesar_Dressing/CaesarDressing-Regular.ttf"),   // 0 Caesar Dressing
		TEXT("New Fonts/Cinzel/static/Cinzel-Regular.ttf"),             // 1 Cinzel
		TEXT("New Fonts/Cormorant_SC/CormorantSC-Regular.ttf"),         // 2 Cormorant SC
		TEXT("New Fonts/Germania_One/GermaniaOne-Regular.ttf"),         // 3 Germania One
		TEXT("New Fonts/Grenze/Grenze-Regular.ttf"),                    // 4 Grenze
	};
	// Bold variant per font (nullptr = no bold asset, use Regular). Caesar Dressing & Germania One have no bold.
	static const TCHAR* GThemeFontPathsBold[] = {
		nullptr,                                                       // 0 Caesar Dressing
		TEXT("New Fonts/Cinzel/static/Cinzel-Bold.ttf"),               // 1 Cinzel
		TEXT("New Fonts/Cormorant_SC/CormorantSC-Bold.ttf"),            // 2 Cormorant SC
		nullptr,                                                       // 3 Germania One
		TEXT("New Fonts/Grenze/Grenze-Bold.ttf"),                      // 4 Grenze
	};
	static const int32 GThemeFontCount = UE_ARRAY_COUNT(GThemeFontPaths);

	const FString& FontsDir()
	{
		static const FString Dir = FPaths::ProjectContentDir() / TEXT("Slate/Fonts");
		return Dir;
	}

	// Single TTF from Content/Slate/Fonts (path relative to FontsDir).
	FSlateFontInfo MakeFontFromFile(const TCHAR* RelativePath, int32 Size)
	{
		const FString Path = FontsDir() / RelativePath;
		if (!FPaths::FileExists(Path))
			return FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size);
		return FSlateFontInfo(Path, Size);
	}

	// All UI font tokens use the selected font from GThemeFontPaths. Bold uses GThemeFontPathsBold when available.
	FSlateFontInfo ThemeFont(const TCHAR* Weight, int32 Size)
	{
		const int32 Idx = FMath::Clamp(GThemeFontIndex, 0, GThemeFontCount - 1);
		const bool bWantBold = GForceBoldFont || (FCString::Stricmp(Weight, TEXT("Bold")) == 0);
		const TCHAR* Path = GThemeFontPaths[Idx];
		if (bWantBold && GThemeFontPathsBold[Idx] != nullptr)
		{
			const FString BoldPath = FontsDir() / GThemeFontPathsBold[Idx];
			if (FPaths::FileExists(BoldPath))
				Path = GThemeFontPathsBold[Idx];
		}
		return MakeFontFromFile(Path, Size);
	}

	int32 AdvanceToNextFontIndex()
	{
		GThemeFontIndex = (GThemeFontIndex + 1) % GThemeFontCount;
		return GThemeFontIndex;
	}

	void ToggleForceBoldFont()
	{
		GForceBoldFont = !GForceBoldFont;
	}
}

// --- Font theme (all text uses GFontTheme in this file) ---
FSlateFontInfo FT66Style::Tokens::FontRegular(int32 Size)
{
	return ThemeFont(TEXT("Regular"), Size);
}
FSlateFontInfo FT66Style::Tokens::FontBold(int32 Size)
{
	return ThemeFont(TEXT("Bold"), Size);
}
FSlateFontInfo FT66Style::Tokens::FontTitle()
{
	return ThemeFont(TEXT("Bold"), 52);
}
FSlateFontInfo FT66Style::Tokens::FontHeading()
{
	return ThemeFont(TEXT("Bold"), 24);
}
FSlateFontInfo FT66Style::Tokens::FontBody()
{
	return ThemeFont(TEXT("Regular"), 14);
}
FSlateFontInfo FT66Style::Tokens::FontSmall()
{
	return ThemeFont(TEXT("Regular"), 11);
}
FSlateFontInfo FT66Style::Tokens::FontChip()
{
	return ThemeFont(TEXT("Bold"), 11);
}
FSlateFontInfo FT66Style::Tokens::FontButton()
{
	return ThemeFont(TEXT("Bold"), 16);
}

void FT66Style::CycleToNextFont()
{
	AdvanceToNextFontIndex();
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		GPreviousStyleSet = MoveTemp(StyleInstance);
	}
	Initialize();
}

void FT66Style::ToggleBoldFont()
{
	ToggleForceBoldFont();
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		GPreviousStyleSet = MoveTemp(StyleInstance);
	}
	Initialize();
}

ET66UITheme FT66Style::GetTheme()
{
	return GActiveTheme;
}

void FT66Style::SetTheme(ET66UITheme NewTheme)
{
	GActiveTheme = NewTheme;
	ApplyThemePalette(NewTheme);

	// Re-initialize the style set so new brushes/button styles use updated tokens.
	// Keep old set alive (GPreviousStyleSet) so existing widgets' raw pointers stay valid.
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		GPreviousStyleSet = MoveTemp(StyleInstance);
	}
	Initialize();
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

	// Load button background textures once (kept in static refs so brushes stay valid)
	if (!GButtonDarkTex)
	{
		GButtonDarkTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Assets/ButtonDark.ButtonDark"));
	}
	if (!GButtonLightTex)
	{
		GButtonLightTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Assets/ButtonLight.ButtonLight"));
	}
	UTexture2D* const ButtonTex = GetButtonTextureForCurrentTheme();

	// Rounded panel brushes with white outline so each element is clearly distinct
	const float BorderW = Tokens::BorderWidth;
	const FLinearColor BorderColor = Tokens::Border;
	StyleInstance->Set("T66.Brush.Bg", new FSlateRoundedBoxBrush(Tokens::Bg, Tokens::CornerRadius, BorderColor, BorderW));
	StyleInstance->Set("T66.Brush.Panel", new FSlateRoundedBoxBrush(Tokens::Panel, Tokens::CornerRadius, BorderColor, BorderW));
	StyleInstance->Set("T66.Brush.Panel2", new FSlateRoundedBoxBrush(Tokens::Panel2, Tokens::CornerRadius, BorderColor, BorderW));
	StyleInstance->Set("T66.Brush.Stroke", new FSlateRoundedBoxBrush(Tokens::Stroke, Tokens::CornerRadiusSmall, BorderColor, BorderW));
	// Circle-ish brush (use BorderBackgroundColor for actual tint)
	StyleInstance->Set("T66.Brush.Circle", new FSlateRoundedBoxBrush(FLinearColor::White, 110.f, BorderColor, BorderW));
	// Full-screen scrim overlay (no corner radius, no border)
	StyleInstance->Set("T66.Brush.Scrim", new FSlateRoundedBoxBrush(Tokens::Scrim, 0.f));

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

	// Button styles: texture background (ButtonDark / ButtonLight) when loaded, else solid rounded box
	{
		const FLinearColor PrimaryN = Tokens::Accent2 * 0.55f + Tokens::Panel2 * 0.45f;
		const FLinearColor PrimaryH = Tokens::Accent2 * 0.70f + Tokens::Panel2 * 0.30f;
		const FLinearColor PrimaryP = Tokens::Accent2 * 0.80f + Tokens::Panel2 * 0.20f;

		const FLinearColor NeutralN = Tokens::Panel2;
		const FLinearColor NeutralH = Tokens::Panel2 + FLinearColor(0.05f, 0.05f, 0.07f, 0.f);
		const FLinearColor NeutralP = Tokens::Panel2 + FLinearColor(0.08f, 0.08f, 0.10f, 0.f);

		const FLinearColor DangerN = Tokens::Danger * 0.55f + Tokens::Panel2 * 0.45f;
		const FLinearColor DangerH = Tokens::Danger * 0.70f + Tokens::Panel2 * 0.30f;
		const FLinearColor DangerP = Tokens::Danger * 0.80f + Tokens::Panel2 * 0.20f;

		const FLinearColor ActiveN = Tokens::Text;
		FLinearColor ActiveH = Tokens::Text * 0.85f; ActiveH.A = 1.0f;
		FLinearColor ActiveP = Tokens::Text * 0.70f; ActiveP.A = 1.0f;

		if (ButtonTex)
		{
			FButtonStyle Primary = FButtonStyle()
				.SetNormal(MakeButtonTextureBrush(ButtonTex, PrimaryN))
				.SetHovered(MakeButtonTextureBrush(ButtonTex, PrimaryH))
				.SetPressed(MakeButtonTextureBrush(ButtonTex, PrimaryP))
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed);
			StyleInstance->Set("T66.Button.Primary", Primary);

			FButtonStyle Neutral = FButtonStyle()
				.SetNormal(MakeButtonTextureBrush(ButtonTex, NeutralN))
				.SetHovered(MakeButtonTextureBrush(ButtonTex, NeutralH))
				.SetPressed(MakeButtonTextureBrush(ButtonTex, NeutralP))
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed);
			StyleInstance->Set("T66.Button.Neutral", Neutral);

			FButtonStyle Danger = FButtonStyle()
				.SetNormal(MakeButtonTextureBrush(ButtonTex, DangerN))
				.SetHovered(MakeButtonTextureBrush(ButtonTex, DangerH))
				.SetPressed(MakeButtonTextureBrush(ButtonTex, DangerP))
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed);
			StyleInstance->Set("T66.Button.Danger", Danger);

			FButtonStyle ToggleActive = FButtonStyle()
				.SetNormal(MakeButtonTextureBrush(ButtonTex, ActiveN))
				.SetHovered(MakeButtonTextureBrush(ButtonTex, ActiveH))
				.SetPressed(MakeButtonTextureBrush(ButtonTex, ActiveP))
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed);
			StyleInstance->Set("T66.Button.ToggleActive", ToggleActive);
		}
		else
		{
			FButtonStyle Primary = FButtonStyle()
				.SetNormal(FSlateRoundedBoxBrush(PrimaryN, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetHovered(FSlateRoundedBoxBrush(PrimaryH, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetPressed(FSlateRoundedBoxBrush(PrimaryP, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed);
			StyleInstance->Set("T66.Button.Primary", Primary);

			FButtonStyle Neutral = FButtonStyle()
				.SetNormal(FSlateRoundedBoxBrush(NeutralN, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetHovered(FSlateRoundedBoxBrush(NeutralH, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetPressed(FSlateRoundedBoxBrush(NeutralP, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed);
			StyleInstance->Set("T66.Button.Neutral", Neutral);

			FButtonStyle Danger = FButtonStyle()
				.SetNormal(FSlateRoundedBoxBrush(DangerN, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetHovered(FSlateRoundedBoxBrush(DangerH, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetPressed(FSlateRoundedBoxBrush(DangerP, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed);
			StyleInstance->Set("T66.Button.Danger", Danger);

			FButtonStyle ToggleActive = FButtonStyle()
				.SetNormal(FSlateRoundedBoxBrush(ActiveN, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetHovered(FSlateRoundedBoxBrush(ActiveH, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetPressed(FSlateRoundedBoxBrush(ActiveP, Tokens::CornerRadiusSmall, BorderColor, BorderW))
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed);
			StyleInstance->Set("T66.Button.ToggleActive", ToggleActive);
		}
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);

	static bool bConsoleRegistered = false;
	if (!bConsoleRegistered)
	{
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("T66NextFont"),
			TEXT("Cycle to the next UI font (Caesar Dressing, Cinzel, Cormorant SC, Germania One, Grenze). Reopens style so new UI uses it."),
			FConsoleCommandDelegate::CreateStatic(&FT66Style::CycleToNextFont),
			ECVF_Default);
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("T66Bold"),
			TEXT("Toggle force-bold: all UI text uses the bold variant of the current font (if the font has a bold asset)."),
			FConsoleCommandDelegate::CreateStatic(&FT66Style::ToggleBoldFont),
			ECVF_Default);
		bConsoleRegistered = true;
	}
}

FOnClicked FT66Style::DebounceClick(const FOnClicked& InnerDelegate)
{
	// Global timestamp shared by ALL debounced buttons.
	// Any click within 150 ms of the previous one is silently dropped.
	// 150 ms is imperceptible for intentional clicks but prevents:
	//   - double-fire from rapid/spam clicking
	//   - UI rebuild crashes from overlapping widget teardowns
	//   - accidental double-navigation
	return FOnClicked::CreateLambda([InnerDelegate]() -> FReply
	{
		static double GLastClickTime = 0.0;
		const double Now = FPlatformTime::Seconds();
		if (Now - GLastClickTime < 0.15)
		{
			return FReply::Handled();
		}
		GLastClickTime = Now;
		return InnerDelegate.Execute();
	});
}

TSharedRef<SWidget> FT66Style::MakeButton(
	const FText& Label,
	FOnClicked OnClicked,
	ET66ButtonType Type,
	float MinWidth,
	float Height)
{
	FName StyleName;
	FLinearColor BtnColor;
	switch (Type)
	{
	case ET66ButtonType::Primary:
		StyleName = "T66.Button.Primary";
		BtnColor = Tokens::Accent2;
		break;
	case ET66ButtonType::Danger:
		StyleName = "T66.Button.Danger";
		BtnColor = Tokens::Danger;
		break;
	case ET66ButtonType::Success:
		StyleName = "T66.Button.Primary";
		BtnColor = Tokens::Success;
		break;
	default: // Neutral
		StyleName = "T66.Button.Neutral";
		BtnColor = Tokens::Panel2;
		break;
	}

	const FButtonStyle& BtnStyle = Get().GetWidgetStyle<FButtonStyle>(StyleName);
	const FTextBlockStyle& TxtStyle = Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");

	// All centralized buttons get automatic debounce protection.
	FOnClicked SafeClick = DebounceClick(OnClicked);

	return SNew(SBox)
		.MinDesiredWidth(MinWidth)
		.HeightOverride(Height)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked(SafeClick)
			.ButtonStyle(&BtnStyle)
			.ButtonColorAndOpacity(BtnColor)
			[
				SNew(STextBlock)
				.Text(Label)
				.TextStyle(&TxtStyle)
				.Justification(ETextJustify::Center)
			]
		];
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

