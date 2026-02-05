// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66Style.h"
#include "T66.h"

#include "Blueprint/UserWidget.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66ScreenBase.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "UObject/WeakObjectPtr.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Engine/GameInstance.h"

TSharedPtr<FSlateStyleSet> FT66Style::StyleInstance;

// 9-slice brushes for obsidian texture (filled when texture loads). Margin = fixed edge size in slate units.
static constexpr float ObsidianSliceMargin = 24.f;
// Semi-transparent so obsidian is the visible background and the game scene shows through (borders stay visible).
static const FLinearColor ObsidianTint(1.f, 1.f, 1.f, 0.82f);
// Panel brush: heap-allocated so the style set can own and delete it on Shutdown (passing &static would cause the set to delete static memory and crash).
static FSlateBrush* GObsidianPanelBrushPtr = nullptr;
// Button brushes: static, not registered with the style set (only copied into FButtonStyle when texture loads).
static FSlateBrush GObsidianButtonN;
static FSlateBrush GObsidianButtonH;
static FSlateBrush GObsidianButtonP;

// --- Colors (pitch black panels/buttons, strong bold gold-yellow lettering) ---
const FLinearColor FT66Style::Tokens::Bg(0.08f, 0.08f, 0.10f, 1.0f);         // darker grey
const FLinearColor FT66Style::Tokens::Panel(0.0f, 0.0f, 0.0f, 1.0f);         // pitch black
const FLinearColor FT66Style::Tokens::Panel2(0.0f, 0.0f, 0.0f, 1.0f);        // pitch black
const FLinearColor FT66Style::Tokens::Stroke(0.18f, 0.18f, 0.20f, 1.0f);    // dark grey
const FLinearColor FT66Style::Tokens::Scrim(0.f, 0.f, 0.f, 0.70f);
const FLinearColor FT66Style::Tokens::Text(1.0f, 0.84f, 0.0f, 1.0f);         // strong bold gold yellow
const FLinearColor FT66Style::Tokens::TextMuted(0.95f, 0.78f, 0.1f, 1.0f);   // bold gold (muted)
const FLinearColor FT66Style::Tokens::Accent(0.0f, 0.0f, 0.0f, 1.0f);        // pitch black (primary button)
const FLinearColor FT66Style::Tokens::Accent2(0.0f, 0.0f, 0.0f, 1.0f);       // pitch black (secondary button)
const FLinearColor FT66Style::Tokens::Danger(0.95f, 0.15f, 0.15f, 1.0f);
const FLinearColor FT66Style::Tokens::Success(0.20f, 0.80f, 0.35f, 1.0f);

namespace
{
	// ----- Theme: change this to switch fonts for the entire project -----
	enum class EFontTheme { Aztec, Almendra, Uncial, NewFonts };
	static const EFontTheme GFontTheme = EFontTheme::NewFonts;
	// 0=Caesar Dressing, 1=Cinzel, 2=Cormorant SC, 3=Germania One, 4=Grenze. Change to cycle.
	static const int32 GThemeFontIndex = 0;

	static const TCHAR* GThemeFontPaths[] = {
		TEXT("New Fonts/Caesar_Dressing/CaesarDressing-Regular.ttf"),   // 0 Caesar Dressing
		TEXT("New Fonts/Cinzel/static/Cinzel-Regular.ttf"),             // 1 Cinzel
		TEXT("New Fonts/Cormorant_SC/CormorantSC-Regular.ttf"),         // 2 Cormorant SC
		TEXT("New Fonts/Germania_One/GermaniaOne-Regular.ttf"),         // 3 Germania One
		TEXT("New Fonts/Grenze/Grenze-Regular.ttf"),                    // 4 Grenze
	};
	static const int32 GThemeFontCount = UE_ARRAY_COUNT(GThemeFontPaths);

	const FString& FontsDir()
	{
		static const FString Dir = FPaths::ProjectContentDir() / TEXT("Slate/Fonts");
		return Dir;
	}

	// Aztec: Content/Slate/Fonts/Aztec.ttf (optional Aztec_Bold.ttf).
	FSlateFontInfo MakeFontAztec(const TCHAR* Weight, int32 Size)
	{
		const FString Aztec = FontsDir() / TEXT("Aztec.ttf");
		const FString AztecBold = FontsDir() / TEXT("Aztec_Bold.ttf");
		const bool bWantBold = FCString::Stricmp(Weight, TEXT("Bold")) == 0;
		const FString& Candidate = (bWantBold && FPaths::FileExists(AztecBold)) ? AztecBold : Aztec;
		if (FPaths::FileExists(Candidate))
			return FSlateFontInfo(Candidate, Size);
		return FCoreStyle::GetDefaultFontStyle(Weight, Size);
	}

	// Single TTF from Content/Slate/Fonts (path relative to FontsDir).
	FSlateFontInfo MakeFontFromFile(const TCHAR* RelativePath, int32 Size)
	{
		const FString Path = FontsDir() / RelativePath;
		if (!FPaths::FileExists(Path))
			return FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size);
		return FSlateFontInfo(Path, Size);
	}

	// One theme entry point: all UI font tokens go through here.
	FSlateFontInfo ThemeFont(const TCHAR* Weight, int32 Size)
	{
		switch (GFontTheme)
		{
		case EFontTheme::Almendra:
			return MakeFontFromFile(TEXT("AlmendraSC-Regular.ttf"), Size);
		case EFontTheme::Uncial:
			return MakeFontFromFile(TEXT("UncialAntiqua-Regular.ttf"), Size);
		case EFontTheme::NewFonts:
		{
			const int32 Idx = FMath::Clamp(GThemeFontIndex, 0, GThemeFontCount - 1);
			return MakeFontFromFile(GThemeFontPaths[Idx], Size);
		}
		case EFontTheme::Aztec:
		default:
			return MakeFontAztec(Weight, Size);
		}
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
	// Circle-ish brush (use BorderBackgroundColor for actual tint)
	StyleInstance->Set("T66.Brush.Circle", new FSlateRoundedBoxBrush(FLinearColor::White, 110.f));

	// Obsidian 9-slice brushes (heap-allocated; style set owns and deletes them on Shutdown).
	GObsidianPanelBrushPtr = new FSlateBrush();
	GObsidianPanelBrushPtr->DrawAs = ESlateBrushDrawType::Box;
	GObsidianPanelBrushPtr->Tiling = ESlateBrushTileType::NoTile;
	GObsidianPanelBrushPtr->Margin = FMargin(ObsidianSliceMargin);
	GObsidianPanelBrushPtr->TintColor = FSlateColor(Tokens::Panel);
	StyleInstance->Set("T66.Brush.ObsidianPanel", GObsidianPanelBrushPtr);

	GObsidianButtonN = FSlateBrush();
	GObsidianButtonN.DrawAs = ESlateBrushDrawType::Box;
	GObsidianButtonN.Tiling = ESlateBrushTileType::NoTile;
	GObsidianButtonN.Margin = FMargin(ObsidianSliceMargin);
	GObsidianButtonN.TintColor = FSlateColor(Tokens::Panel);
	GObsidianButtonH = GObsidianButtonN;
	GObsidianButtonP = GObsidianButtonN;

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

		// Placeholder until obsidian texture loads (replaced in EnsureObsidianBrushes)
		const FLinearColor PlaceholderN = Tokens::Panel2;
		const FLinearColor PlaceholderH = Tokens::Panel2 + FLinearColor(0.05f, 0.05f, 0.07f, 0.f);
		const FLinearColor PlaceholderP = Tokens::Panel2 + FLinearColor(0.08f, 0.08f, 0.10f, 0.f);
		FButtonStyle ObsidianPlaceholder = FButtonStyle()
			.SetNormal(FSlateRoundedBoxBrush(PlaceholderN, Tokens::CornerRadiusSmall))
			.SetHovered(FSlateRoundedBoxBrush(PlaceholderH, Tokens::CornerRadiusSmall))
			.SetPressed(FSlateRoundedBoxBrush(PlaceholderP, Tokens::CornerRadiusSmall));
		StyleInstance->Set("T66.Button.Obsidian", ObsidianPlaceholder);
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
}

void FT66Style::EnsureObsidianBrushes(UGameInstance* GI, UObject* Requester)
{
	UE_LOG(LogT66, Log, TEXT("[Obsidian] EnsureObsidianBrushes called, GI=%p Requester=%s"), GI, Requester ? *Requester->GetName() : TEXT("null"));

	if (!StyleInstance.IsValid() || !GI || !Requester)
	{
		UE_LOG(LogT66, Warning, TEXT("[Obsidian] Early exit: StyleInstance=%d GI=%d Requester=%d"), StyleInstance.IsValid() ? 1 : 0, GI ? 1 : 0, Requester ? 1 : 0);
		return;
	}

	UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
	if (!TexPool)
	{
		UE_LOG(LogT66, Warning, TEXT("[Obsidian] TexPool is null"));
		return;
	}

	const FSoftObjectPath ObsidianPath(TEXT("/Game/UI/Obsidian.Obsidian"));
	const TSoftObjectPtr<UTexture2D> ObsidianSoft(ObsidianPath);
	UE_LOG(LogT66, Log, TEXT("[Obsidian] Requesting texture: %s"), *ObsidianPath.ToString());

	if (ObsidianSoft.IsNull())
	{
		UE_LOG(LogT66, Warning, TEXT("[Obsidian] ObsidianSoft.IsNull()"));
		return;
	}

	if (UTexture2D* Loaded = TexPool->GetLoadedTexture(ObsidianSoft))
	{
		UE_LOG(LogT66, Log, TEXT("[Obsidian] Texture already loaded: %s"), Loaded ? *Loaded->GetName() : TEXT("null"));
		if (GObsidianPanelBrushPtr)
		{
			GObsidianPanelBrushPtr->SetResourceObject(Loaded);
			GObsidianPanelBrushPtr->TintColor = FSlateColor(ObsidianTint);
		}
		GObsidianButtonN.SetResourceObject(Loaded);
		GObsidianButtonN.TintColor = FSlateColor(ObsidianTint);
		GObsidianButtonH.SetResourceObject(Loaded);
		GObsidianButtonH.TintColor = FSlateColor(ObsidianTint);
		GObsidianButtonP.SetResourceObject(Loaded);
		GObsidianButtonP.TintColor = FSlateColor(ObsidianTint);
		FButtonStyle ObsidianBtn = FButtonStyle()
			.SetNormal(GObsidianButtonN)
			.SetHovered(GObsidianButtonH)
			.SetPressed(GObsidianButtonP);
		StyleInstance->Set("T66.Button.Obsidian", ObsidianBtn);
		UE_LOG(LogT66, Log, TEXT("[Obsidian] Brushes updated (sync path). Texture=%s"), Loaded ? *Loaded->GetPathName() : TEXT("null"));
		return;
	}

	UE_LOG(LogT66, Log, TEXT("[Obsidian] Texture not loaded yet, requesting async..."));
	TWeakObjectPtr<UObject> RequesterWeak(Requester);
	TexPool->RequestTexture(ObsidianSoft, Requester, FName(TEXT("Obsidian")), [RequesterWeak](UTexture2D* Loaded)
	{
		UE_LOG(LogT66, Log, TEXT("[Obsidian] Async callback: Loaded=%p StyleInstance=%d"), Loaded, StyleInstance.IsValid() ? 1 : 0);
		if (!Loaded || !StyleInstance.IsValid())
		{
			UE_LOG(LogT66, Warning, TEXT("[Obsidian] Async callback skipped: Loaded=%d StyleInstance=%d"), Loaded ? 1 : 0, StyleInstance.IsValid() ? 1 : 0);
			return;
		}
		if (GObsidianPanelBrushPtr)
		{
			GObsidianPanelBrushPtr->SetResourceObject(Loaded);
			GObsidianPanelBrushPtr->TintColor = FSlateColor(ObsidianTint);
		}
		GObsidianButtonN.SetResourceObject(Loaded);
		GObsidianButtonN.TintColor = FSlateColor(ObsidianTint);
		GObsidianButtonH.SetResourceObject(Loaded);
		GObsidianButtonH.TintColor = FSlateColor(ObsidianTint);
		GObsidianButtonP.SetResourceObject(Loaded);
		GObsidianButtonP.TintColor = FSlateColor(ObsidianTint);
		FButtonStyle ObsidianBtn = FButtonStyle()
			.SetNormal(GObsidianButtonN)
			.SetHovered(GObsidianButtonH)
			.SetPressed(GObsidianButtonP);
		StyleInstance->Set("T66.Button.Obsidian", ObsidianBtn);
		UE_LOG(LogT66, Log, TEXT("[Obsidian] Brushes updated (async). Texture=%s"), Loaded ? *Loaded->GetPathName() : TEXT("null"));

		// Force full Slate rebuild so buttons get the new style (old style was replaced and would be use-after-free).
		if (UT66ScreenBase* Screen = Cast<UT66ScreenBase>(RequesterWeak.Get()))
		{
			Screen->ForceRebuildSlate();
			UE_LOG(LogT66, Log, TEXT("[Obsidian] ForceRebuildSlate on requester so buttons use new style"));
		}
		else if (UUserWidget* Widget = Cast<UUserWidget>(RequesterWeak.Get()))
		{
			Widget->InvalidateLayoutAndVolatility();
			UE_LOG(LogT66, Log, TEXT("[Obsidian] Invalidated requester widget for repaint"));
		}
	});
}

void FT66Style::Shutdown()
{
	if (!StyleInstance.IsValid())
	{
		return;
	}

	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	StyleInstance.Reset();
	// Style set owned and deleted the panel brush; avoid use-after-free (e.g. hot reload).
	GObsidianPanelBrushPtr = nullptr;
}

