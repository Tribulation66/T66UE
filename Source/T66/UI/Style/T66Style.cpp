// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66Style.h"
#include "UI/Style/T66ButtonVisuals.h"
#include "UI/Style/T66OverlayChromeStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "Core/T66AudioSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "T66.h"

#include "Engine/Texture2D.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/GameViewportClient.h"
#include "ImageUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Paths.h"
#include "UObject/StrongObjectPtr.h"
#include "Engine/Engine.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Engine/World.h"
#include "UObject/GarbageCollection.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Layout/Visibility.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Async/Async.h"
#include "Framework/Application/SlateApplication.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Style, Log, All);

namespace
{
	TSet<TWeakObjectPtr<UUserWidget>> GDeferredRebuildWidgets;
}

TSharedPtr<FSlateStyleSet> FT66Style::StyleInstance;

// --- Colors (dark presentation) ---
FLinearColor FT66Style::Tokens::Bg(0.08f, 0.08f, 0.10f, 1.0f);
FLinearColor FT66Style::Tokens::Panel(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::Panel2(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::Stroke(0.18f, 0.18f, 0.20f, 1.0f);
FLinearColor FT66Style::Tokens::Scrim(0.f, 0.f, 0.f, 0.70f);
FLinearColor FT66Style::Tokens::Text(244.f / 255.f, 236.f / 255.f, 216.f / 255.f, 1.0f);
FLinearColor FT66Style::Tokens::TextMuted(0.75f, 0.75f, 0.75f, 1.0f);
FLinearColor FT66Style::Tokens::Accent(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::Accent2(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66Style::Tokens::Danger(0.95f, 0.15f, 0.15f, 1.0f);
FLinearColor FT66Style::Tokens::Success(0.20f, 0.80f, 0.35f, 1.0f);
FLinearColor FT66Style::Tokens::Border(1.0f, 1.0f, 1.0f, 1.0f);
const FMargin FT66Style::Tokens::ButtonPadding(12.f, 4.f);
const FMargin FT66Style::Tokens::ButtonPaddingPressed(12.f, 5.f, 12.f, 3.f);

namespace
{
	// MasterLibrary global button textures. Loaded from loose PNGs so shared HUD/Slate
	// chrome follows the same source-art path as frontend screens.
	static TStrongObjectPtr<UTexture2D> GMasterBasicButtonNormal;
	static TStrongObjectPtr<UTexture2D> GMasterBasicButtonHover;
	static TStrongObjectPtr<UTexture2D> GMasterBasicButtonPressed;
	static TStrongObjectPtr<UTexture2D> GMasterBasicButtonDisabled;
	static TStrongObjectPtr<UTexture2D> GMasterSelectButtonNormal;
	static TStrongObjectPtr<UTexture2D> GMasterSelectButtonHover;
	static TStrongObjectPtr<UTexture2D> GMasterSelectButtonPressed;
	static TStrongObjectPtr<UTexture2D> GMasterSelectButtonSelected;
	static TStrongObjectPtr<UTexture2D> GMasterSelectButtonDisabled;

	struct FT66MasterButtonTextureSet
	{
		UTexture2D* BasicNormal = nullptr;
		UTexture2D* BasicHover = nullptr;
		UTexture2D* BasicPressed = nullptr;
		UTexture2D* BasicDisabled = nullptr;
		UTexture2D* SelectNormal = nullptr;
		UTexture2D* SelectHover = nullptr;
		UTexture2D* SelectPressed = nullptr;
		UTexture2D* SelectSelected = nullptr;
		UTexture2D* SelectDisabled = nullptr;
	};

	static void RefreshMouseCursorQuery()
	{
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().QueryCursor();
		}
	}

	static FSimpleDelegate WithMouseCursorRefresh(FSimpleDelegate Delegate)
	{
		return FSimpleDelegate::CreateLambda([Delegate = MoveTemp(Delegate)]() mutable
		{
			Delegate.ExecuteIfBound();
			RefreshMouseCursorQuery();
		});
	}

	static bool HasButtonTextures()
	{
		return GMasterBasicButtonNormal.IsValid()
			&& GMasterBasicButtonHover.IsValid()
			&& GMasterBasicButtonPressed.IsValid()
			&& GMasterBasicButtonDisabled.IsValid()
			&& GMasterSelectButtonNormal.IsValid()
			&& GMasterSelectButtonHover.IsValid()
			&& GMasterSelectButtonPressed.IsValid()
			&& GMasterSelectButtonSelected.IsValid()
			&& GMasterSelectButtonDisabled.IsValid();
	}

	static FT66MasterButtonTextureSet GetButtonTextures()
	{
		return FT66MasterButtonTextureSet{
			GMasterBasicButtonNormal.Get(),
			GMasterBasicButtonHover.Get(),
			GMasterBasicButtonPressed.Get(),
			GMasterBasicButtonDisabled.Get(),
			GMasterSelectButtonNormal.Get(),
			GMasterSelectButtonHover.Get(),
			GMasterSelectButtonPressed.Get(),
			GMasterSelectButtonSelected.Get(),
			GMasterSelectButtonDisabled.Get()
		};
	}

	static FSlateBrush MakeButtonTextureBrush(UTexture2D* Tex)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.SetResourceObject(Tex);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		Brush.ImageSize = FVector2D(270.f, 88.f);
		Brush.Margin = FMargin(0.104f, 0.250f, 0.104f, 0.250f);
		return Brush;
	}

	static bool LoadMasterLibraryTexture(
		TStrongObjectPtr<UTexture2D>& OutTexture,
		const TCHAR* RelativePath,
		const TCHAR* DebugLabel)
	{
		if (OutTexture.IsValid())
		{
			return true;
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
		{
			if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				TextureFilter::TF_Trilinear,
				true,
				DebugLabel))
			{
				OutTexture.Reset(Texture);
				return true;
			}
		}

		UE_LOG(
			LogT66Style,
			Warning,
			TEXT("[T66Style] Missing MasterLibrary texture '%s' for %s"),
			RelativePath,
			DebugLabel ? DebugLabel : TEXT("<unnamed>"));
		return false;
	}

	static void LoadButtonTexturesOnce()
	{
		if (GMasterBasicButtonNormal.IsValid()) return;

		LoadMasterLibraryTexture(GMasterBasicButtonNormal, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/basic_button_normal.png"), TEXT("MasterBasicButtonNormal"));
		LoadMasterLibraryTexture(GMasterBasicButtonHover, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/basic_button_hover.png"), TEXT("MasterBasicButtonHover"));
		LoadMasterLibraryTexture(GMasterBasicButtonPressed, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/basic_button_pressed.png"), TEXT("MasterBasicButtonPressed"));
		LoadMasterLibraryTexture(GMasterBasicButtonDisabled, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/basic_button_disabled.png"), TEXT("MasterBasicButtonDisabled"));
		LoadMasterLibraryTexture(GMasterSelectButtonNormal, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/select_button_normal.png"), TEXT("MasterSelectButtonNormal"));
		LoadMasterLibraryTexture(GMasterSelectButtonHover, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/select_button_hover.png"), TEXT("MasterSelectButtonHover"));
		LoadMasterLibraryTexture(GMasterSelectButtonPressed, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/select_button_pressed.png"), TEXT("MasterSelectButtonPressed"));
		LoadMasterLibraryTexture(GMasterSelectButtonSelected, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/select_button_selected.png"), TEXT("MasterSelectButtonSelected"));
		LoadMasterLibraryTexture(GMasterSelectButtonDisabled, TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/select_button_disabled.png"), TEXT("MasterSelectButtonDisabled"));
	}

	// MasterLibrary global panel textures.
	static TStrongObjectPtr<UTexture2D> GMasterPanel;
	static TStrongObjectPtr<UTexture2D> GMasterInnerPanel;
	static TObjectPtr<UMaterialInterface> GButtonGlowMaterial = nullptr;
	static bool bCheckedButtonGlowMaterial = false;

	enum class ET66InRunPlateKind : uint8
	{
		Neutral,
		Primary,
		Danger,
		TabActive,
		TabInactive,
	};

	T66RuntimeUIBrushAccess::FOptionalTextureBrush& GetInRunPlateEntry(const ET66InRunPlateKind Kind)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Neutral;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Primary;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Danger;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush TabActive;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush TabInactive;

		switch (Kind)
		{
		case ET66InRunPlateKind::Primary:
			return Primary;
		case ET66InRunPlateKind::Danger:
			return Danger;
		case ET66InRunPlateKind::TabActive:
			return TabActive;
		case ET66InRunPlateKind::TabInactive:
			return TabInactive;
		case ET66InRunPlateKind::Neutral:
		default:
			return Neutral;
		}
	}

	FString GetMainMenuReferenceAssetPath(const TCHAR* RelativePath)
	{
		return FPaths::ProjectDir() / TEXT("SourceAssets/UI/MainMenuReference") / RelativePath;
	}

	const FSlateBrush* ResolveMinimapFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush FrameBrush;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			FrameBrush,
			nullptr,
			FPaths::ProjectDir() / TEXT("RuntimeDependencies/T66/UI/Minimap/minimap_frame.png"),
			FMargin(0.f),
			TEXT("MinimapFrame"),
			TextureFilter::TF_Trilinear);
	}

	const TCHAR* GetInRunPlateRelativePath(const ET66InRunPlateKind Kind)
	{
		switch (Kind)
		{
		case ET66InRunPlateKind::Primary:
			return TEXT("Center/cta_button_new_game_plate.png");
		case ET66InRunPlateKind::Danger:
			return TEXT("Center/cta_button_daily_challenge_plate.png");
		case ET66InRunPlateKind::TabActive:
			return TEXT("RightPanel/tab_weekly_active.png");
		case ET66InRunPlateKind::TabInactive:
			return TEXT("RightPanel/tab_all_time_inactive.png");
		case ET66InRunPlateKind::Neutral:
		default:
			return TEXT("Center/cta_button_load_game_plate.png");
		}
	}

	FMargin GetInRunPlateMargin(const ET66InRunPlateKind Kind)
	{
		switch (Kind)
		{
		case ET66InRunPlateKind::TabActive:
		case ET66InRunPlateKind::TabInactive:
			return FMargin(0.18f, 0.34f, 0.18f, 0.34f);
		default:
			return FMargin(0.18f, 0.35f, 0.18f, 0.35f);
		}
	}

	ET66InRunPlateKind ResolveInRunButtonPlateKind(const ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return ET66InRunPlateKind::Primary;
		case ET66ButtonType::Danger:
			return ET66InRunPlateKind::Danger;
		case ET66ButtonType::Neutral:
		default:
			return ET66InRunPlateKind::Neutral;
		}
	}

	const FSlateBrush* GetWhiteBrush()
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	T66RuntimeUIBrushAccess::FOptionalTextureBrush& GetInventorySlotEntry()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush InventorySlot;
		return InventorySlot;
	}

	TSharedRef<SWidget> MakeLayeredSurface(
		const TSharedRef<SWidget>& Content,
		const FLinearColor& Outer,
		const FLinearColor& Middle,
		const FLinearColor& Fill,
		const FLinearColor& TopHighlight,
		const FMargin& Padding)
	{
		return SNew(SBorder)
			.BorderImage(GetWhiteBrush())
			.BorderBackgroundColor(Outer)
			.Padding(1.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(Middle)
				.Padding(1.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(GetWhiteBrush())
						.BorderBackgroundColor(Fill)
						.Padding(Padding)
						[
							Content
						]
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Top)
					.Padding(1.f, 1.f, 1.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(2.f)
						[
							SNew(SBorder)
							.BorderImage(GetWhiteBrush())
							.BorderBackgroundColor(TopHighlight)
						]
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Bottom)
					.Padding(1.f, 0.f, 1.f, 1.f)
					[
						SNew(SBox)
						.HeightOverride(2.f)
						[
							SNew(SBorder)
							.BorderImage(GetWhiteBrush())
							.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
						]
					]
				]
			];
	}

	static bool GPanelTexturesAvailable()
	{
		return GMasterPanel.IsValid() && GMasterInnerPanel.IsValid();
	}

	struct FT66ButtonGlowState
	{
		float Intensity = 0.f;
		FLinearColor Color = FLinearColor::White;
		TSharedPtr<FSlateBrush> Brush;
		TStrongObjectPtr<UMaterialInstanceDynamic> Material;
	};

	static UMaterialInterface* GetButtonGlowMaterial()
	{
		if (!bCheckedButtonGlowMaterial)
		{
			bCheckedButtonGlowMaterial = true;
			GButtonGlowMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/UI/Materials/M_UI_Glow.M_UI_Glow"));
			UE_LOG(LogT66Style, Verbose, TEXT("[T66Style] Load UI glow material -> %s"), GButtonGlowMaterial ? TEXT("OK") : TEXT("MISS"));
		}

		return GButtonGlowMaterial.Get();
	}

	static FLinearColor ResolveButtonGlowColor(ET66ButtonType ButtonType)
	{
		switch (ButtonType)
		{
		case ET66ButtonType::Primary:
			return FT66Style::Accent();
		case ET66ButtonType::Danger:
			return FT66Style::Tokens::Danger;
		case ET66ButtonType::Success:
			return FT66Style::Tokens::Success;
		case ET66ButtonType::ToggleActive:
			return FT66Style::Tokens::Text;
		default:
			return FT66Style::Tokens::Border;
		}
	}

	static TSharedPtr<FT66ButtonGlowState> CreateButtonGlowState(ET66ButtonType ButtonType)
	{
		TSharedPtr<FT66ButtonGlowState> State = MakeShared<FT66ButtonGlowState>();
		State->Color = ResolveButtonGlowColor(ButtonType);

		if (UMaterialInterface* GlowMaterial = GetButtonGlowMaterial())
		{
			State->Material.Reset(UMaterialInstanceDynamic::Create(GlowMaterial, GetTransientPackage()));
			if (State->Material.IsValid())
			{
				State->Material->SetVectorParameterValue(TEXT("GlowColor"), State->Color);
				State->Material->SetScalarParameterValue(TEXT("GlowIntensity"), 0.f);

				State->Brush = MakeShared<FSlateBrush>();
				State->Brush->DrawAs = ESlateBrushDrawType::Image;
				State->Brush->Tiling = ESlateBrushTileType::NoTile;
				// Keep the glow brush from inflating button desired size; the overlay slot stretches it to the button bounds.
				State->Brush->ImageSize = FVector2D(1.f, 1.f);
				State->Brush->TintColor = FSlateColor(FLinearColor::White);
				State->Brush->SetResourceObject(State->Material.Get());
			}
		}

		return State;
	}

	static void SetButtonGlowIntensity(const TSharedPtr<FT66ButtonGlowState>& State, float NewIntensity)
	{
		if (!State.IsValid())
		{
			return;
		}

		State->Intensity = FMath::Max(0.f, NewIntensity);
		if (State->Material.IsValid())
		{
			State->Material->SetVectorParameterValue(TEXT("GlowColor"), State->Color);
			State->Material->SetScalarParameterValue(TEXT("GlowIntensity"), State->Intensity);
		}
	}

	static UTexture2D* GetPanelTexture()
	{
		return GMasterPanel.Get();
	}

	static UTexture2D* GetPanel2Texture()
	{
		return GMasterInnerPanel.IsValid() ? GMasterInnerPanel.Get() : GMasterPanel.Get();
	}

	static FLinearColor WithAlpha(const FLinearColor& Color, float Alpha)
	{
		FLinearColor Out = Color;
		Out.A = Alpha;
		return Out;
	}

	static FLinearColor MixColor(const FLinearColor& A, const FLinearColor& B, float T)
	{
		return FLinearColor::LerpUsingHSV(A, B, FMath::Clamp(T, 0.f, 1.f));
	}

	static FSlateBrush MakePanelTextureBrush(UTexture2D* Tex, const FVector2D& ImageSize)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;   // 9-slice: corners preserved, center stretches
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.SetResourceObject(Tex);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		Brush.ImageSize = ImageSize;
		Brush.Margin = FMargin(0.067f, 0.043f, 0.067f, 0.043f);
		return Brush;
	}

	static void LoadPanelTexturesOnce()
	{
		if (GMasterPanel.IsValid()) return;

		LoadMasterLibraryTexture(GMasterPanel, TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/basic_panel_normal.png"), TEXT("MasterBasicPanel"));
		LoadMasterLibraryTexture(GMasterInnerPanel, TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/inner_panel_normal.png"), TEXT("MasterInnerPanel"));
	}

	void ApplyThemePalette()
	{
		using T = FT66Style::Tokens;
		T::Bg = FT66Style::Background();
		T::Panel = FT66Style::Panel();
		T::Panel2 = FT66Style::PanelInner();
		T::Stroke = FT66Style::Stroke();
		T::Scrim = FT66Style::Scrim();
		T::Text = FT66Style::Text();
		T::TextMuted = FT66Style::TextMuted();
		T::Accent = FT66Style::Accent();
		T::Accent2 = FT66Style::Accent2();
		T::Danger = FT66Style::Danger();
		T::Success = FT66Style::Success();
		T::Border = FT66Style::Border();
	}

	FSlateFontInfo MakeCurrentUIFont(const TCHAR* Weight, int32 Size)
	{
		const FString LockedFontPath = T66RuntimeUIFontAccess::ResolveLockedUIFontPath();
		if (!LockedFontPath.IsEmpty())
		{
			return T66RuntimeUIFontAccess::MakeFontFromAbsoluteFile(LockedFontPath, Size);
		}

		return T66RuntimeUIFontAccess::MakeLocalizedEngineFont(Size, T66RuntimeUIFontAccess::IsBoldWeight(Weight));
	}

	FVector2D GetViewportSizeOrFallback()
	{
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			const FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
			if (Size.X > 0 && Size.Y > 0)
			{
				return FVector2D(static_cast<float>(Size.X), static_cast<float>(Size.Y));
			}
		}

		return FVector2D(FT66Style::Tokens::ReferenceLayoutWidth, FT66Style::Tokens::ReferenceLayoutHeight);
	}

	float GetConfiguredDPIScale(const FVector2D& ViewportSize)
	{
		const UUserInterfaceSettings* UISettings = GetDefault<UUserInterfaceSettings>();
		if (!UISettings)
		{
			return 1.0f;
		}

		const FIntPoint PixelSize(
			FMath::Max(1, FMath::RoundToInt(ViewportSize.X)),
			FMath::Max(1, FMath::RoundToInt(ViewportSize.Y)));
		return FMath::Max(0.01f, UISettings->GetDPIScaleBasedOnSize(PixelSize));
	}

	float GetPlayerUIScaleOrDefault()
	{
		if (GEngine && GEngine->GameViewport)
		{
			if (UWorld* World = GEngine->GameViewport->GetWorld())
			{
				if (UGameInstance* GI = World->GetGameInstance())
				{
					if (UT66PlayerSettingsSubsystem* PlayerSettings = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
					{
						return PlayerSettings->GetUIScale();
					}
				}
			}
		}

		return 1.0f;
	}

	float ComputeResponsiveScale(const FVector2D& ReferenceResolution, bool bAllowUpscale)
	{
		const FVector2D ViewportSize = GetViewportSizeOrFallback();
		float Scale = GetConfiguredDPIScale(ViewportSize) * GetPlayerUIScaleOrDefault();
		if (Scale <= 0.f)
		{
			const FVector2D SafeReference(
				FMath::Max(ReferenceResolution.X, 1.f),
				FMath::Max(ReferenceResolution.Y, 1.f));
			Scale = FMath::Min(ViewportSize.X / SafeReference.X, ViewportSize.Y / SafeReference.Y);
		}

		if (!bAllowUpscale)
		{
			Scale = FMath::Min(Scale, 1.f);
		}

		return FMath::Max(0.01f, Scale);
	}

}

// --- Font selection (all text uses the active font selection in this file) ---
FSlateFontInfo FT66Style::Tokens::FontRegular(int32 Size)
{
	return FT66Style::MakeFont(TEXT("Regular"), Size);
}
FSlateFontInfo FT66Style::Tokens::FontBold(int32 Size)
{
	return FT66Style::MakeFont(TEXT("Bold"), Size);
}
FSlateFontInfo FT66Style::Tokens::FontTitle()
{
	FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), 56);
	Font.LetterSpacing = 180;
	return Font;
}
FSlateFontInfo FT66Style::Tokens::FontHeading()
{
	FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), 26);
	Font.LetterSpacing = 120;
	return Font;
}
FSlateFontInfo FT66Style::Tokens::FontBody()
{
	FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Regular"), 15);
	Font.LetterSpacing = 20;
	return Font;
}
FSlateFontInfo FT66Style::Tokens::FontSmall()
{
	FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Regular"), 12);
	Font.LetterSpacing = 18;
	return Font;
}
FSlateFontInfo FT66Style::Tokens::FontChip()
{
	FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), 12);
	Font.LetterSpacing = 90;
	return Font;
}
FSlateFontInfo FT66Style::Tokens::FontButton()
{
	FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), 18);
	Font.LetterSpacing = 110;
	return Font;
}

bool FT66Style::IsDotaTheme()
{
	return true;
}

FSlateFontInfo FT66Style::MakeFont(const TCHAR* Weight, int32 Size)
{
	return MakeCurrentUIFont(Weight, Size);
}

FLinearColor FT66Style::Background() { return FLinearColor(2.f / 255.f, 3.f / 255.f, 6.f / 255.f, 1.0f); }
FLinearColor FT66Style::PanelOuter() { return FLinearColor(1.f / 255.f, 1.f / 255.f, 3.f / 255.f, 1.0f); }
FLinearColor FT66Style::Panel() { return FLinearColor(6.f / 255.f, 8.f / 255.f, 13.f / 255.f, 0.988f); }
FLinearColor FT66Style::PanelInner() { return FLinearColor(4.f / 255.f, 5.f / 255.f, 9.f / 255.f, 0.992f); }
FLinearColor FT66Style::Stroke() { return FLinearColor(38.f / 255.f, 44.f / 255.f, 54.f / 255.f, 1.0f); }
FLinearColor FT66Style::Scrim() { return FLinearColor(2.f / 255.f, 3.f / 255.f, 5.f / 255.f, 0.93f); }
FLinearColor FT66Style::Text() { return FLinearColor(244.f / 255.f, 236.f / 255.f, 216.f / 255.f, 1.0f); }
FLinearColor FT66Style::TextMuted() { return FLinearColor(126.f / 255.f, 136.f / 255.f, 148.f / 255.f, 1.0f); }
FLinearColor FT66Style::Accent() { return FLinearColor(58.f / 255.f, 96.f / 255.f, 138.f / 255.f, 1.0f); }
FLinearColor FT66Style::Accent2() { return FLinearColor(196.f / 255.f, 167.f / 255.f, 96.f / 255.f, 1.0f); }
FLinearColor FT66Style::Border() { return FLinearColor(44.f / 255.f, 50.f / 255.f, 60.f / 255.f, 1.0f); }
FLinearColor FT66Style::HeaderBar() { return FLinearColor(5.f / 255.f, 7.f / 255.f, 11.f / 255.f, 0.99f); }
FLinearColor FT66Style::HeaderAccent() { return FLinearColor(64.f / 255.f, 104.f / 255.f, 148.f / 255.f, 1.0f); }
FLinearColor FT66Style::Success() { return FLinearColor(78.f / 255.f, 110.f / 255.f, 63.f / 255.f, 1.0f); }
FLinearColor FT66Style::Danger() { return FLinearColor(171.f / 255.f, 64.f / 255.f, 52.f / 255.f, 1.0f); }
FLinearColor FT66Style::ScreenBackground() { return FLinearColor(3.f / 255.f, 4.f / 255.f, 7.f / 255.f, 0.992f); }
FLinearColor FT66Style::ScreenText() { return Text(); }
FLinearColor FT66Style::ScreenMuted() { return TextMuted(); }
FLinearColor FT66Style::SlotOuter() { return FLinearColor(2.f / 255.f, 3.f / 255.f, 5.f / 255.f, 0.99f); }
FLinearColor FT66Style::SlotInner() { return FLinearColor(22.f / 255.f, 27.f / 255.f, 34.f / 255.f, 1.0f); }
FLinearColor FT66Style::SlotFill() { return FLinearColor(4.f / 255.f, 5.f / 255.f, 8.f / 255.f, 0.992f); }
FLinearColor FT66Style::BossBarBackground() { return FLinearColor(0.04f, 0.04f, 0.05f, 0.94f); }
FLinearColor FT66Style::BossBarFill() { return FLinearColor(0.70f, 0.17f, 0.12f, 0.98f); }
FLinearColor FT66Style::PromptBackground() { return FLinearColor(0.015f, 0.020f, 0.028f, 0.92f); }
FLinearColor FT66Style::SelectionFill() { return FLinearColor(42.f / 255.f, 72.f / 255.f, 104.f / 255.f, 0.96f); }
FLinearColor FT66Style::MinimapBackground() { return FLinearColor(22.f / 255.f, 25.f / 255.f, 30.f / 255.f, 0.99f); }
FLinearColor FT66Style::MinimapTerrain() { return FLinearColor(49.f / 255.f, 57.f / 255.f, 52.f / 255.f, 1.0f); }
FLinearColor FT66Style::MinimapGrid() { return FLinearColor(88.f / 255.f, 94.f / 255.f, 91.f / 255.f, 0.30f); }
FLinearColor FT66Style::MinimapFriendly() { return FLinearColor(0.220f, 0.930f, 0.510f, 1.0f); }
FLinearColor FT66Style::MinimapEnemy() { return FLinearColor(0.920f, 0.220f, 0.155f, 1.0f); }
FLinearColor FT66Style::MinimapNeutral() { return FLinearColor(0.665f, 0.595f, 0.340f, 1.0f); }
FLinearColor FT66Style::ButtonNeutral() { return FLinearColor(10.f / 255.f, 13.f / 255.f, 18.f / 255.f, 1.0f); }
FLinearColor FT66Style::ButtonHovered() { return FLinearColor(16.f / 255.f, 20.f / 255.f, 27.f / 255.f, 1.0f); }
FLinearColor FT66Style::ButtonPressed() { return FLinearColor(7.f / 255.f, 9.f / 255.f, 13.f / 255.f, 1.0f); }
FLinearColor FT66Style::ButtonPrimary() { return FLinearColor(59.f / 255.f, 85.f / 255.f, 48.f / 255.f, 1.0f); }
FLinearColor FT66Style::ButtonPrimaryHovered() { return FLinearColor(69.f / 255.f, 98.f / 255.f, 56.f / 255.f, 1.0f); }
FLinearColor FT66Style::ButtonPrimaryPressed() { return FLinearColor(46.f / 255.f, 67.f / 255.f, 38.f / 255.f, 1.0f); }
FLinearColor FT66Style::DangerButton() { return FLinearColor(92.f / 255.f, 37.f / 255.f, 31.f / 255.f, 1.0f); }
FLinearColor FT66Style::DangerButtonHovered() { return FLinearColor(122.f / 255.f, 48.f / 255.f, 40.f / 255.f, 1.0f); }
FLinearColor FT66Style::DangerButtonPressed() { return FLinearColor(69.f / 255.f, 28.f / 255.f, 24.f / 255.f, 1.0f); }
FLinearColor FT66Style::SuccessButton() { return ButtonPrimary(); }
FLinearColor FT66Style::SuccessButtonHovered() { return ButtonPrimaryHovered(); }
FLinearColor FT66Style::SuccessButtonPressed() { return ButtonPrimaryPressed(); }
FLinearColor FT66Style::ToggleButton() { return ButtonPrimary(); }
FLinearColor FT66Style::ToggleButtonHovered() { return ButtonPrimaryHovered(); }
FLinearColor FT66Style::ToggleButtonPressed() { return ButtonPrimaryPressed(); }
float FT66Style::CornerRadius() { return 0.0f; }
float FT66Style::CornerRadiusSmall() { return 0.0f; }

FName FT66Style::GetStyleSetName()
{
	static const FName Name(TEXT("T66Style"));
	return Name;
}

const ISlateStyle& FT66Style::Get()
{
	if (!StyleInstance.IsValid())
	{
		Initialize();
	}
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
	ApplyThemePalette();

	// Load MasterLibrary button/panel textures once for shared HUD/Slate chrome.
	LoadButtonTexturesOnce();
	LoadPanelTexturesOnce();

	bool bUseButtonTextures = HasButtonTextures();
	const FT66MasterButtonTextureSet ButtonTextures = bUseButtonTextures
		? GetButtonTextures()
		: FT66MasterButtonTextureSet{};

	bool bUsePanelTextures = GPanelTexturesAvailable();
	UTexture2D* PanelTex = bUsePanelTextures ? GetPanelTexture() : nullptr;
	UTexture2D* Panel2Tex = bUsePanelTextures ? GetPanel2Texture() : nullptr;
	if (IsDotaTheme())
	{
		bUseButtonTextures = false;
		bUsePanelTextures = false;
		PanelTex = nullptr;
		Panel2Tex = nullptr;
	}

	UE_LOG(LogT66Style, Verbose, TEXT("[T66Style] Resources: ButtonTex=%d  PanelTex=%d"),
		bUseButtonTextures ? 1 : 0, bUsePanelTextures ? 1 : 0);

	// Panel brushes: prefer texture (9-slice with baked border/bevel), fallback to rounded box
	const float BorderW = Tokens::BorderWidth;
	const FLinearColor BorderColor = Tokens::Border;
	const float PanelCornerRadius = IsDotaTheme() ? FT66Style::CornerRadius() : Tokens::CornerRadius;
	const float PanelCornerRadiusSmall = IsDotaTheme() ? FT66Style::CornerRadiusSmall() : Tokens::CornerRadiusSmall;

	if (bUsePanelTextures && PanelTex && Panel2Tex)
	{
		FSlateBrush PanelBrush = MakePanelTextureBrush(PanelTex, FVector2D(480.f, 800.f));
		FSlateBrush Panel2Brush = MakePanelTextureBrush(Panel2Tex, FVector2D(620.f, 340.f));
		StyleInstance->Set("T66.Brush.Bg",     new FSlateBrush(PanelBrush));
		StyleInstance->Set("T66.Brush.Panel",   new FSlateBrush(PanelBrush));
		StyleInstance->Set("T66.Brush.Panel2",  new FSlateBrush(Panel2Brush));
	}
	else
	{
		// Fallback: solid rounded box with border
		StyleInstance->Set("T66.Brush.Bg",     new FSlateRoundedBoxBrush(Tokens::Bg, PanelCornerRadius, BorderColor, BorderW));
		StyleInstance->Set("T66.Brush.Panel",   new FSlateRoundedBoxBrush(Tokens::Panel, PanelCornerRadius, BorderColor, BorderW));
		StyleInstance->Set("T66.Brush.Panel2",  new FSlateRoundedBoxBrush(Tokens::Panel2, PanelCornerRadius, BorderColor, BorderW));
	}

	// Utility brushes (always procedural)
	StyleInstance->Set("T66.Brush.Stroke", new FSlateRoundedBoxBrush(Tokens::Stroke, PanelCornerRadiusSmall, BorderColor, BorderW));
	StyleInstance->Set("T66.Brush.Circle", new FSlateRoundedBoxBrush(FLinearColor::White, 110.f, BorderColor, BorderW));
	StyleInstance->Set("T66.Brush.Scrim",  new FSlateRoundedBoxBrush(Tokens::Scrim, 0.f));

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

	// Button styles: prefer 6-texture set (with effects baked in), else solid rounded box fallback
	{
		const float ButtonCornerRadius = IsDotaTheme() ? FT66Style::CornerRadiusSmall() : Tokens::CornerRadiusSmall;

		if (bUseButtonTextures
			&& ButtonTextures.BasicNormal
			&& ButtonTextures.BasicHover
			&& ButtonTextures.BasicPressed
			&& ButtonTextures.BasicDisabled
			&& ButtonTextures.SelectSelected)
		{
			auto MakeTexStyle = [](UTexture2D* N, UTexture2D* H, UTexture2D* P, UTexture2D* D) {
				return FButtonStyle()
					.SetNormal(MakeButtonTextureBrush(N))
					.SetHovered(MakeButtonTextureBrush(H))
					.SetPressed(MakeButtonTextureBrush(P))
					.SetDisabled(MakeButtonTextureBrush(D))
					.SetNormalPadding(FT66Style::Tokens::ButtonPadding)
					.SetPressedPadding(FT66Style::Tokens::ButtonPaddingPressed);
			};
			StyleInstance->Set("T66.Button.Primary", MakeTexStyle(
				ButtonTextures.BasicNormal,
				ButtonTextures.BasicHover,
				ButtonTextures.BasicPressed,
				ButtonTextures.BasicDisabled));
			StyleInstance->Set("T66.Button.Neutral", MakeTexStyle(
				ButtonTextures.BasicNormal,
				ButtonTextures.BasicHover,
				ButtonTextures.BasicPressed,
				ButtonTextures.BasicDisabled));
			StyleInstance->Set("T66.Button.Danger", MakeTexStyle(
				ButtonTextures.BasicNormal,
				ButtonTextures.BasicHover,
				ButtonTextures.BasicPressed,
				ButtonTextures.BasicDisabled));
			StyleInstance->Set("T66.Button.Success", MakeTexStyle(
				ButtonTextures.BasicNormal,
				ButtonTextures.BasicHover,
				ButtonTextures.BasicPressed,
				ButtonTextures.BasicDisabled));
			StyleInstance->Set("T66.Button.ToggleActive", MakeTexStyle(
				ButtonTextures.SelectSelected,
				ButtonTextures.SelectHover ? ButtonTextures.SelectHover : ButtonTextures.BasicHover,
				ButtonTextures.SelectPressed ? ButtonTextures.SelectPressed : ButtonTextures.BasicPressed,
				ButtonTextures.SelectDisabled ? ButtonTextures.SelectDisabled : ButtonTextures.BasicDisabled));
		}
		else
		{
			const FLinearColor NeutralN = IsDotaTheme() ? FT66Style::ButtonNeutral() : Tokens::Panel2;
			const FLinearColor NeutralH = IsDotaTheme() ? FT66Style::ButtonHovered() : (Tokens::Panel2 + FLinearColor(0.05f, 0.05f, 0.07f, 0.f));
			const FLinearColor NeutralP = IsDotaTheme() ? FT66Style::ButtonPressed() : (Tokens::Panel2 + FLinearColor(0.08f, 0.08f, 0.10f, 0.f));

			auto MakeBoxStyle = [&](const FLinearColor& N, const FLinearColor& H, const FLinearColor& P) {
				return FButtonStyle()
					.SetNormal(FSlateRoundedBoxBrush(N, ButtonCornerRadius, BorderColor, BorderW))
					.SetHovered(FSlateRoundedBoxBrush(H, ButtonCornerRadius, BorderColor, BorderW))
					.SetPressed(FSlateRoundedBoxBrush(P, ButtonCornerRadius, BorderColor, BorderW))
					.SetNormalPadding(Tokens::ButtonPadding)
					.SetPressedPadding(Tokens::ButtonPaddingPressed);
			};

			const FLinearColor PrimaryN = IsDotaTheme() ? FT66Style::ButtonPrimary() : NeutralN;
			const FLinearColor PrimaryH = IsDotaTheme() ? FT66Style::ButtonPrimaryHovered() : NeutralH;
			const FLinearColor PrimaryP = IsDotaTheme() ? FT66Style::ButtonPrimaryPressed() : NeutralP;
			const FLinearColor DangerN = IsDotaTheme() ? FT66Style::DangerButton() : NeutralN;
			const FLinearColor DangerH = IsDotaTheme() ? FT66Style::DangerButtonHovered() : NeutralH;
			const FLinearColor DangerP = IsDotaTheme() ? FT66Style::DangerButtonPressed() : NeutralP;
			const FLinearColor SuccessN = IsDotaTheme() ? FT66Style::SuccessButton() : NeutralN;
			const FLinearColor SuccessH = IsDotaTheme() ? FT66Style::SuccessButtonHovered() : NeutralH;
			const FLinearColor SuccessP = IsDotaTheme() ? FT66Style::SuccessButtonPressed() : NeutralP;

			StyleInstance->Set("T66.Button.Primary",      MakeBoxStyle(PrimaryN, PrimaryH, PrimaryP));
			StyleInstance->Set("T66.Button.Neutral",      MakeBoxStyle(NeutralN, NeutralH, NeutralP));
			StyleInstance->Set("T66.Button.Danger",       MakeBoxStyle(DangerN, DangerH, DangerP));
			StyleInstance->Set("T66.Button.Success",      MakeBoxStyle(SuccessN, SuccessH, SuccessP));
			// ToggleActive: selected/pressed look — brighter fill + visible border so ON/OFF selection is obvious.
			{
				const FLinearColor ToggleN = IsDotaTheme() ? FT66Style::ToggleButton() : Tokens::Text;
				const FLinearColor ToggleH = IsDotaTheme() ? FT66Style::ToggleButtonHovered() : (Tokens::Text * 0.90f + FLinearColor(0,0,0,0.10f));
				const FLinearColor ToggleP = IsDotaTheme() ? FT66Style::ToggleButtonPressed() : (Tokens::Text * 0.75f + FLinearColor(0,0,0,0.25f));
				const float ToggleBorderW = 2.f;
				const FLinearColor ToggleBorder = Tokens::Accent2.A > 0.01f ? Tokens::Accent2 : Tokens::Stroke;
				StyleInstance->Set("T66.Button.ToggleActive", FButtonStyle()
					.SetNormal(FSlateRoundedBoxBrush(ToggleN, ButtonCornerRadius, ToggleBorder, ToggleBorderW))
					.SetHovered(FSlateRoundedBoxBrush(ToggleH, ButtonCornerRadius, ToggleBorder, ToggleBorderW))
					.SetPressed(FSlateRoundedBoxBrush(ToggleP, ButtonCornerRadius, ToggleBorder, ToggleBorderW))
					.SetNormalPadding(Tokens::ButtonPadding)
					.SetPressedPadding(Tokens::ButtonPaddingPressed));
			}
		}

		// Flat rectangular button body for custom border treatments like RetroWood.
		{
			const FSlateRoundedBoxBrush FlatBrush(FLinearColor::White, 0.f, FLinearColor::Transparent, 0.f);
			StyleInstance->Set("T66.Button.FlatRect", FButtonStyle()
				.SetNormal(FlatBrush)
				.SetHovered(FlatBrush)
				.SetPressed(FlatBrush)
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed));
		}

		// Transparent hit target for buttons that render their body from a separate fill overlay.
		{
			const FSlateRoundedBoxBrush FlatTransparentBrush(FLinearColor::Transparent, 0.f, FLinearColor::Transparent, 0.f);
			StyleInstance->Set("T66.Button.FlatTransparent", FButtonStyle()
				.SetNormal(FlatTransparentBrush)
				.SetHovered(FlatTransparentBrush)
				.SetPressed(FlatTransparentBrush)
				.SetNormalPadding(Tokens::ButtonPadding)
				.SetPressedPadding(Tokens::ButtonPaddingPressed));
		}

		// Row style: transparent background, very thin border, subtle hover highlight.
		// Used for leaderboard rows and list items that are clickable but shouldn't look like buttons.
		{
			constexpr float RowBorderW = 0.5f;
			const FLinearColor RowBorderColor = Tokens::Stroke;
			const FLinearColor RowN = FLinearColor(0.f, 0.f, 0.f, 0.f);       // transparent
			const FLinearColor RowH = Tokens::Stroke * FLinearColor(1,1,1,0.5f); // subtle highlight
			const FLinearColor RowP = Tokens::Stroke * FLinearColor(1,1,1,0.7f); // slightly stronger on press
			StyleInstance->Set("T66.Button.Row", FButtonStyle()
				.SetNormal(FSlateRoundedBoxBrush(RowN, ButtonCornerRadius, RowBorderColor, RowBorderW))
				.SetHovered(FSlateRoundedBoxBrush(RowH, ButtonCornerRadius, RowBorderColor, RowBorderW))
				.SetPressed(FSlateRoundedBoxBrush(RowP, ButtonCornerRadius, RowBorderColor, RowBorderW))
				.SetNormalPadding(FMargin(0.f))
				.SetPressedPadding(FMargin(0.f)));
		}

	}

	// Dropdown: use exactly the same button style as T66.Button.Neutral (texture or fallback) + small down arrow
	{
		FComboButtonStyle DropdownStyle;
		DropdownStyle.ButtonStyle = StyleInstance->GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
		FSlateBrush ArrowBrush = FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton").DownArrowImage;
		ArrowBrush.TintColor = FSlateColor(Tokens::Text);
		DropdownStyle.DownArrowImage = ArrowBrush;
		DropdownStyle.MenuBorderBrush = FSlateRoundedBoxBrush(Tokens::Panel, PanelCornerRadiusSmall, BorderColor, BorderW);
		DropdownStyle.MenuBorderPadding = FMargin(0.f);
		StyleInstance->Set("T66.Dropdown.ComboButtonStyle", DropdownStyle);
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
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
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("UI.Click")));
		return InnerDelegate.Execute();
	});
}

ET66ButtonBorderVisual FT66Style::ResolveButtonBorderVisual(const FT66ButtonParams& Params)
{
	if (Params.BorderVisual != ET66ButtonBorderVisual::Default)
	{
		return Params.BorderVisual;
	}

	if (Params.Type == ET66ButtonType::Row)
	{
		return ET66ButtonBorderVisual::None;
	}

	if (IsDotaTheme())
	{
		return ET66ButtonBorderVisual::None;
	}

	return ET66ButtonBorderVisual::RetroWood;
}

ET66ButtonBackgroundVisual FT66Style::ResolveButtonBackgroundVisual(const FT66ButtonParams& Params)
{
	if (Params.BackgroundVisual != ET66ButtonBackgroundVisual::Default)
	{
		return Params.BackgroundVisual;
	}

	if (Params.Type == ET66ButtonType::Row)
	{
		return ET66ButtonBackgroundVisual::None;
	}

	if (IsDotaTheme())
	{
		return ET66ButtonBackgroundVisual::None;
	}

	return ET66ButtonBackgroundVisual::None;
}

ET66ButtonBorderVisual FT66Style::ResolvePanelBorderVisual(const FT66PanelParams& Params)
{
	if (Params.BorderVisual != ET66ButtonBorderVisual::Default)
	{
		return Params.BorderVisual;
	}

	if (IsDotaTheme())
	{
		return ET66ButtonBorderVisual::None;
	}

	return Params.Type == ET66PanelType::Bg
		? ET66ButtonBorderVisual::None
		: ET66ButtonBorderVisual::RetroWood;
}

ET66ButtonBackgroundVisual FT66Style::ResolvePanelBackgroundVisual(const FT66PanelParams& Params)
{
	if (Params.BackgroundVisual != ET66ButtonBackgroundVisual::Default)
	{
		return Params.BackgroundVisual;
	}

	if (IsDotaTheme())
	{
		return ET66ButtonBackgroundVisual::None;
	}

	return ET66ButtonBackgroundVisual::None;
}

float FT66Style::ResolveButtonDecorativeBorderThickness(const FT66ButtonParams& Params, int32 EffectiveFontSize, float MaxThickness)
{
	float Thickness = FMath::Clamp(static_cast<float>(EffectiveFontSize) * 0.25f, 3.5f, MaxThickness);

	if (Params.Height > 0.f)
	{
		Thickness = FMath::Min(Thickness, FMath::Max(3.f, Params.Height * 0.16f));
	}

	return FMath::Clamp(Thickness, 3.f, MaxThickness);
}

float FT66Style::ResolvePanelDecorativeBorderThickness(const FT66PanelParams& Params, float MaxThickness)
{
	switch (Params.Type)
	{
	case ET66PanelType::Bg:
		return 0.f;

	case ET66PanelType::Panel2:
		return FMath::Min(4.f, MaxThickness);

	case ET66PanelType::Panel:
	default:
		break;
	}

	const float MinPadding = FMath::Min(
		FMath::Min(Params.Padding.Left, Params.Padding.Top),
		FMath::Min(Params.Padding.Right, Params.Padding.Bottom));
	float Thickness = MaxThickness;
	if (MinPadding > 0.f)
	{
		Thickness = FMath::Min(Thickness, FMath::Max(4.f, MinPadding * 0.55f));
	}

	return FMath::Clamp(Thickness, 4.f, MaxThickness);
}

const FSlateBrush* FT66Style::GetInRunButtonPlateBrush(const ET66ButtonType Type)
{
	const ET66InRunPlateKind Kind = ResolveInRunButtonPlateKind(Type);
	return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
		GetInRunPlateEntry(Kind),
		nullptr,
		GetMainMenuReferenceAssetPath(GetInRunPlateRelativePath(Kind)),
		GetInRunPlateMargin(Kind),
		TEXT("InRunButtonPlate"));
}

const FSlateBrush* FT66Style::GetInRunTabPlateBrush(const bool bSelected)
{
	const ET66InRunPlateKind Kind = bSelected ? ET66InRunPlateKind::TabActive : ET66InRunPlateKind::TabInactive;
	return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
		GetInRunPlateEntry(Kind),
		nullptr,
		GetMainMenuReferenceAssetPath(GetInRunPlateRelativePath(Kind)),
		GetInRunPlateMargin(Kind),
		TEXT("InRunTabPlate"));
}

FT66ButtonParams FT66Style::MakeInRunButtonParams(const FText& Label, FOnClicked OnClicked, const ET66ButtonType Type)
{
	FT66ButtonParams Params(Label, MoveTemp(OnClicked), Type);
	Params.SetUseDotaPlateOverlay(true)
		.SetDotaPlateOverrideBrush(GetInRunButtonPlateBrush(Type))
		.SetTextColor(FT66Style::Text())
		.SetStateTextShadowColors(
			FLinearColor(0.f, 0.f, 0.f, 0.92f),
			FLinearColor(0.f, 0.f, 0.f, 0.96f),
			FLinearColor(0.f, 0.f, 0.f, 0.98f))
		.SetTextShadowOffset(FVector2D(1.f, 1.f));
	return Params;
}

// ---------------------------------------------------------------------------
// Full params-based MakeButton — every button in the game funnels through here.
// ---------------------------------------------------------------------------
TSharedRef<SWidget> FT66Style::MakeButton(const FT66ButtonParams& Params)
{
	// 1. Resolve ButtonStyle name and default background color from Type.
	FName StyleName;
	FLinearColor DefaultBtnColor;
	switch (Params.Type)
	{
	case ET66ButtonType::Primary:
		StyleName = "T66.Button.Primary";    DefaultBtnColor = Tokens::Accent2; break;
	case ET66ButtonType::Danger:
		StyleName = "T66.Button.Danger";     DefaultBtnColor = Tokens::Danger;  break;
	case ET66ButtonType::Success:
		StyleName = "T66.Button.Success";    DefaultBtnColor = Tokens::Success; break;
	case ET66ButtonType::ToggleActive:
		StyleName = "T66.Button.ToggleActive"; DefaultBtnColor = Tokens::Text;  break;
	case ET66ButtonType::Row:
		StyleName = "T66.Button.Row";        DefaultBtnColor = FLinearColor::Transparent; break;
	default: // Neutral
		StyleName = "T66.Button.Neutral";    DefaultBtnColor = Tokens::Panel2;  break;
	}

	const ET66ButtonBorderVisual ResolvedBorderVisual = ResolveButtonBorderVisual(Params);
	const ET66ButtonBackgroundVisual ResolvedBackgroundVisual = ResolveButtonBackgroundVisual(Params);

	const TSharedPtr<FT66ButtonFillBrushSet> FillBrushSet = FT66ButtonVisuals::CreateFillBrushSet(ResolvedBackgroundVisual);
	const bool bHasCustomFill = FillBrushSet.IsValid() && FillBrushSet->IsValid();

	if (ResolvedBorderVisual == ET66ButtonBorderVisual::RetroWood)
	{
		StyleName = bHasCustomFill ? "T66.Button.FlatTransparent" : "T66.Button.FlatRect";
	}

	const FButtonStyle& BtnStyle = Get().GetWidgetStyle<FButtonStyle>(StyleName);
	const FTextBlockStyle& TxtStyle = Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");

	// 2. Debounce the click delegate.
	FOnClicked SafeClick = DebounceClick(Params.OnClicked);

	// 3. Button background color.
	//    When a color override is set (e.g. ON/OFF toggle selected state), use it so the
	//    selected button is visually distinct. Otherwise with textures use white so the
	//    baked-in border/gloss passes through; with fallback use the style default.
	const bool bUsesTextureChrome =
		!IsDotaTheme()
		&& HasButtonTextures()
		&& ResolvedBorderVisual != ET66ButtonBorderVisual::RetroWood;
	TAttribute<FSlateColor> BtnColor;
	if (bHasCustomFill)
	{
		BtnColor = TAttribute<FSlateColor>(FSlateColor(FLinearColor::White));
	}
	else if (Params.bHasColorOverride)
	{
		BtnColor = Params.ColorOverride;
	}
	else if (bUsesTextureChrome)
	{
		BtnColor = TAttribute<FSlateColor>(FSlateColor(FLinearColor::White));
	}
	else
	{
		BtnColor = TAttribute<FSlateColor>(FSlateColor(DefaultBtnColor));
	}

	// 4. Content padding (negative sentinel = don't override, use FMargin(0)).
	const FMargin ContentPad = (Params.Padding.Left >= 0.f) ? Params.Padding : FMargin(0.f);
	const TSharedPtr<ET66ButtonBorderState> BorderState = MakeShared<ET66ButtonBorderState>(ET66ButtonBorderState::Normal);

	// 5. Build the inner content widget (text block or custom).
	TAttribute<FText> TextAttr = Params.DynamicLabel.IsBound()
		? Params.DynamicLabel
		: TAttribute<FText>(Params.Label);

	const bool bUseBoldButtonText = T66RuntimeUIFontAccess::IsBoldWeight(*Params.FontWeight);
	FSlateFontInfo TextFont = (Params.FontSize > 0)
		? Tokens::FontBold(Params.FontSize)
		: TxtStyle.Font;
	if (!bUseBoldButtonText)
	{
		const int32 ResolvedFontSize = Params.FontSize > 0 ? Params.FontSize : TextFont.Size;
		TextFont = FT66Style::MakeFont(*Params.FontWeight, ResolvedFontSize);
	}
	if (IsDotaTheme() && Params.Type != ET66ButtonType::Row)
	{
		if (bUseBoldButtonText && (Params.Type == ET66ButtonType::Primary || Params.Type == ET66ButtonType::Success))
		{
			TextFont.LetterSpacing = FMath::Max(TextFont.LetterSpacing, FMath::RoundToInt(FMath::Clamp(static_cast<float>(TextFont.Size) * 2.0f, 72.f, 118.f)));
		}
		else if (bUseBoldButtonText)
		{
			TextFont.LetterSpacing = FMath::Max(TextFont.LetterSpacing, FMath::RoundToInt(FMath::Clamp(static_cast<float>(TextFont.Size) * 1.45f, 42.f, 86.f)));
		}
		else
		{
			TextFont.LetterSpacing = FMath::Max(TextFont.LetterSpacing, FMath::RoundToInt(FMath::Clamp(static_cast<float>(TextFont.Size) * 0.35f, 6.f, 18.f)));
		}
	}

	TAttribute<FSlateColor> TextColor;
	if (Params.bHasStateTextColors)
	{
		const FSlateColor NormalStateTextColor = Params.NormalStateTextColor;
		const FSlateColor HoveredStateTextColor = Params.HoveredStateTextColor;
		const FSlateColor PressedStateTextColor = Params.PressedStateTextColor;
		TextColor = TAttribute<FSlateColor>::CreateLambda([BorderState, NormalStateTextColor, HoveredStateTextColor, PressedStateTextColor]() -> FSlateColor
		{
			if (!BorderState.IsValid())
			{
				return NormalStateTextColor;
			}

			switch (*BorderState)
			{
			case ET66ButtonBorderState::Hovered:
				return HoveredStateTextColor;
			case ET66ButtonBorderState::Pressed:
				return PressedStateTextColor;
			case ET66ButtonBorderState::Normal:
			default:
				return NormalStateTextColor;
			}
		});
	}
	else if (Params.bHasTextColorOverride)
	{
		TextColor = Params.TextColorOverride;
	}
	else
	{
		TextColor = TAttribute<FSlateColor>(TxtStyle.ColorAndOpacity);
	}

	TAttribute<FSlateColor> SecondaryTextColor;
	if (Params.bHasStateTextSecondaryColors)
	{
		const FSlateColor NormalStateTextSecondaryColor = Params.NormalStateTextSecondaryColor;
		const FSlateColor HoveredStateTextSecondaryColor = Params.HoveredStateTextSecondaryColor;
		const FSlateColor PressedStateTextSecondaryColor = Params.PressedStateTextSecondaryColor;
		SecondaryTextColor = TAttribute<FSlateColor>::CreateLambda([BorderState, NormalStateTextSecondaryColor, HoveredStateTextSecondaryColor, PressedStateTextSecondaryColor]() -> FSlateColor
		{
			if (!BorderState.IsValid())
			{
				return NormalStateTextSecondaryColor;
			}

			switch (*BorderState)
			{
			case ET66ButtonBorderState::Hovered:
				return HoveredStateTextSecondaryColor;
			case ET66ButtonBorderState::Pressed:
				return PressedStateTextSecondaryColor;
			case ET66ButtonBorderState::Normal:
			default:
				return NormalStateTextSecondaryColor;
			}
		});
	}

	TAttribute<FLinearColor> TextShadowColor;
	if (Params.bHasStateTextShadowColors)
	{
		const FLinearColor NormalStateTextShadowColor = Params.NormalStateTextShadowColor;
		const FLinearColor HoveredStateTextShadowColor = Params.HoveredStateTextShadowColor;
		const FLinearColor PressedStateTextShadowColor = Params.PressedStateTextShadowColor;
		TextShadowColor = TAttribute<FLinearColor>::CreateLambda([BorderState, NormalStateTextShadowColor, HoveredStateTextShadowColor, PressedStateTextShadowColor]() -> FLinearColor
		{
			if (!BorderState.IsValid())
			{
				return NormalStateTextShadowColor;
			}

			switch (*BorderState)
			{
			case ET66ButtonBorderState::Hovered:
				return HoveredStateTextShadowColor;
			case ET66ButtonBorderState::Pressed:
				return PressedStateTextShadowColor;
			case ET66ButtonBorderState::Normal:
			default:
				return NormalStateTextShadowColor;
			}
		});
	}
	else
	{
		TextShadowColor = TAttribute<FLinearColor>(TxtStyle.ShadowColorAndOpacity);
	}

	const TAttribute<FVector2D> TextShadowOffset = Params.bHasTextShadowOffset
		? TAttribute<FVector2D>(Params.TextShadowOffset)
		: TAttribute<FVector2D>(TxtStyle.ShadowOffset);

	const TSharedRef<SWidget> Content = [&]() -> TSharedRef<SWidget>
	{
		if (Params.CustomContent.IsValid())
		{
			return Params.CustomContent.ToSharedRef();
		}

		if (Params.bHasStateTextSecondaryColors)
		{
			const float PrimaryClipHeight = FMath::Max(1.f, FMath::RoundToFloat(static_cast<float>(TextFont.Size) * Params.TextDualToneSplit));

			return StaticCastSharedRef<SWidget>(
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(STextBlock)
					.Text(TextAttr)
					.Font(TextFont)
					.ColorAndOpacity(SecondaryTextColor)
					.Justification(ETextJustify::Center)
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Top)
				[
					SNew(SBox)
					.Clipping(EWidgetClipping::ClipToBounds)
					.HeightOverride(PrimaryClipHeight)
					[
						SNew(STextBlock)
						.Text(TextAttr)
						.Font(TextFont)
						.ColorAndOpacity(TextColor)
						.Justification(ETextJustify::Center)
					]
				]);
		}

		return StaticCastSharedRef<SWidget>(
			SNew(STextBlock)
			.Text(TextAttr)
			.Font(TextFont)
			.ColorAndOpacity(TextColor)
			.ShadowOffset(TextShadowOffset)
			.ShadowColorAndOpacity(TextShadowColor)
			.Justification(ETextJustify::Center)
		);
	}();

	// 6. Assemble: SBox > SButton > Content.
	//    Width/Height of 0 means "no constraint" (FOptionalSize() = unset).
	//    Row buttons use HAlign_Fill so custom content (e.g. column layouts) stretches to full width.
	const EHorizontalAlignment BtnHAlign = (Params.Type == ET66ButtonType::Row) ? HAlign_Fill : HAlign_Center;
	const EVerticalAlignment BtnVAlign = (Params.Type == ET66ButtonType::Row) ? VAlign_Fill : VAlign_Center;
	const bool bEnableButtonGlow = Params.bUseGlow && !IsDotaTheme();
	const TSharedPtr<FT66ButtonGlowState> GlowState = bEnableButtonGlow ? CreateButtonGlowState(Params.Type) : nullptr;
	const TSharedPtr<FT66ButtonBorderBrushSet> BorderBrushSet = FT66ButtonVisuals::CreateBorderBrushSet(ResolvedBorderVisual);
	const int32 EffectiveFontSize = (Params.FontSize > 0) ? Params.FontSize : TextFont.Size;
	const float BorderThickness = (BorderBrushSet.IsValid() && BorderBrushSet->IsValid())
		? ResolveButtonDecorativeBorderThickness(Params, EffectiveFontSize, BorderBrushSet->Thickness)
		: 0.f;
	const auto SetGlow = [GlowState](float Intensity)
	{
		SetButtonGlowIntensity(GlowState, Intensity);
	};
	const auto SetBorderState = [BorderState](ET66ButtonBorderState NewState)
	{
		if (BorderState.IsValid())
		{
			*BorderState = NewState;
		}
	};

	if (ResolvedBorderVisual == ET66ButtonBorderVisual::RetroWood)
	{
		UE_LOG(LogT66Style, Verbose, TEXT("[BORDER] MakeButton: Label='%s' BrushSet=%d BrushSetValid=%d Thickness=%.1f"),
			*Params.Label.ToString(),
			BorderBrushSet.IsValid() ? 1 : 0,
			(BorderBrushSet.IsValid() && BorderBrushSet->IsValid()) ? 1 : 0,
			BorderThickness);
	}

	const TSharedRef<SWidget> GlowWidget =
		!bEnableButtonGlow
		? StaticCastSharedRef<SWidget>(
			SNew(SBox)
			.Visibility(EVisibility::Collapsed))
		: (GlowState.IsValid() && GlowState->Brush.IsValid())
			? StaticCastSharedRef<SWidget>(
				SNew(SImage)
				.Visibility(EVisibility::HitTestInvisible)
				.Image(TAttribute<const FSlateBrush*>::CreateLambda([GlowState]() -> const FSlateBrush*
				{
					return GlowState.IsValid() && GlowState->Brush.IsValid() ? GlowState->Brush.Get() : nullptr;
				})))
			: StaticCastSharedRef<SWidget>(
				SNew(SBorder)
				.Visibility(EVisibility::HitTestInvisible)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([GlowState]() -> FSlateColor
				{
					if (!GlowState.IsValid())
					{
						return FSlateColor(FLinearColor::Transparent);
					}

					FLinearColor Color = GlowState->Color;
					Color.A = GlowState->Intensity * Tokens::ButtonGlowFallbackOpacity;
					return FSlateColor(Color);
				})));

	const TAttribute<const FSlateBrush*> FillBrushAttr = TAttribute<const FSlateBrush*>::CreateLambda([FillBrushSet, BorderState]() -> const FSlateBrush*
	{
		if (!FillBrushSet.IsValid() || !FillBrushSet->IsValid() || !BorderState.IsValid())
		{
			return nullptr;
		}

		return FillBrushSet->GetBrush(*BorderState);
	});

	const TSharedRef<SWidget> FillWidget =
		bHasCustomFill
		? StaticCastSharedRef<SWidget>(
			SNew(SBorder)
			.Visibility(EVisibility::HitTestInvisible)
			.BorderImage(FillBrushAttr)
			.BorderBackgroundColor(FLinearColor::White))
		: StaticCastSharedRef<SWidget>(
			SNew(SBox)
			.Visibility(EVisibility::Collapsed));

	const TAttribute<const FSlateBrush*> HorizontalBorderBrushAttr = TAttribute<const FSlateBrush*>::CreateLambda([BorderBrushSet, BorderState]() -> const FSlateBrush*
	{
		if (!BorderBrushSet.IsValid() || !BorderState.IsValid())
		{
			return nullptr;
		}

		return BorderBrushSet->GetHorizontalBrush(*BorderState);
	});

	const TAttribute<const FSlateBrush*> VerticalBorderBrushAttr = TAttribute<const FSlateBrush*>::CreateLambda([BorderBrushSet, BorderState]() -> const FSlateBrush*
	{
		if (!BorderBrushSet.IsValid() || !BorderState.IsValid())
		{
			return nullptr;
		}

		return BorderBrushSet->GetVerticalBrush(*BorderState);
	});

	const TSharedRef<SWidget> BorderWidget =
		(BorderBrushSet.IsValid() && BorderBrushSet->IsValid() && BorderThickness > 0.f)
		? StaticCastSharedRef<SWidget>(
			SNew(SOverlay)
			.Visibility(EVisibility::HitTestInvisible)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SNew(SBox)
				.HeightOverride(BorderThickness)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(HorizontalBorderBrushAttr)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Bottom)
			[
				SNew(SBox)
				.HeightOverride(BorderThickness)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(HorizontalBorderBrushAttr)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(BorderThickness)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(VerticalBorderBrushAttr)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(BorderThickness)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(VerticalBorderBrushAttr)
				]
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SBox)
			.Visibility(EVisibility::Collapsed));

	const bool bUseFlatBodyOnly =
		!IsDotaTheme() &&
		ResolvedBorderVisual == ET66ButtonBorderVisual::None &&
		ResolvedBackgroundVisual == ET66ButtonBackgroundVisual::None &&
		!Params.bUseDotaPlateOverlay;

	if (bUseFlatBodyOnly && Params.Type != ET66ButtonType::Row)
	{
		const FButtonStyle& FlatStyle = Get().GetWidgetStyle<FButtonStyle>("T66.Button.FlatRect");
		const FMargin FlatContentPad = (Params.Padding.Left >= 0.f)
			? Params.Padding
			: ((Params.FontSize >= 24) ? FMargin(20.f, 12.f, 20.f, 10.f) : FMargin(14.f, 8.f, 14.f, 6.f));

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					GlowWidget
				]
				+ SOverlay::Slot()
				[
					SNew(SButton)
					.Cursor(EMouseCursor::Hand)
					.HAlign(BtnHAlign)
					.VAlign(BtnVAlign)
					.OnClicked(SafeClick)
					.OnHovered(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonHoverGlowIntensity); SetBorderState(ET66ButtonBorderState::Hovered); RefreshMouseCursorQuery(); }))
					.OnUnhovered(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(0.f); SetBorderState(ET66ButtonBorderState::Normal); RefreshMouseCursorQuery(); }))
					.OnPressed(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonPressedGlowIntensity); SetBorderState(ET66ButtonBorderState::Pressed); }))
					.OnReleased(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonHoverGlowIntensity); SetBorderState(ET66ButtonBorderState::Hovered); }))
					.ButtonStyle(&FlatStyle)
					.ButtonColorAndOpacity(BtnColor)
					.ContentPadding(FlatContentPad)
					.IsEnabled(Params.IsEnabled)
					[
						Content
					]
				]
			];
	}

	if (IsDotaTheme() && Params.Type != ET66ButtonType::Row)
	{
		const FButtonStyle& HitStyle = Get().GetWidgetStyle<FButtonStyle>("T66.Button.FlatTransparent");
		const FMargin DotaContentPad = (Params.Padding.Left >= 0.f)
			? Params.Padding
			: ((Params.FontSize >= 24) ? FMargin(20.f, 12.f, 20.f, 10.f) : FMargin(14.f, 8.f, 14.f, 6.f));

		FLinearColor BodyNormal = FT66Style::ButtonNeutral();
		FLinearColor BodyHovered = FT66Style::ButtonHovered();
		FLinearColor BodyPressed = FT66Style::ButtonPressed();

		switch (Params.Type)
		{
		case ET66ButtonType::Primary:
			BodyNormal = FT66Style::ButtonPrimary();
			BodyHovered = FT66Style::ButtonPrimaryHovered();
			BodyPressed = FT66Style::ButtonPrimaryPressed();
			break;
		case ET66ButtonType::Danger:
			BodyNormal = FT66Style::DangerButton();
			BodyHovered = FT66Style::DangerButtonHovered();
			BodyPressed = FT66Style::DangerButtonPressed();
			break;
		case ET66ButtonType::Success:
			BodyNormal = FT66Style::SuccessButton();
			BodyHovered = FT66Style::SuccessButtonHovered();
			BodyPressed = FT66Style::SuccessButtonPressed();
			break;
		case ET66ButtonType::ToggleActive:
			BodyNormal = FT66Style::ToggleButton();
			BodyHovered = FT66Style::ToggleButtonHovered();
			BodyPressed = FT66Style::ToggleButtonPressed();
			break;
		default:
			break;
		}

		if (Params.bHasColorOverride)
		{
			const FLinearColor OverrideBase = Params.ColorOverride.Get().GetSpecifiedColor();
			BodyNormal = OverrideBase;
			BodyHovered = MixColor(OverrideBase, FT66Style::Text(), 0.10f);
			BodyPressed = MixColor(OverrideBase, FT66Style::Background(), 0.30f);
		}

		const TAttribute<FSlateColor> BodyColor = TAttribute<FSlateColor>::CreateLambda([BorderState, BodyNormal, BodyHovered, BodyPressed]() -> FSlateColor
		{
			if (!BorderState.IsValid())
			{
				return FSlateColor(BodyNormal);
			}

			switch (*BorderState)
			{
			case ET66ButtonBorderState::Hovered:
				return FSlateColor(BodyHovered);
			case ET66ButtonBorderState::Pressed:
				return FSlateColor(BodyPressed);
			case ET66ButtonBorderState::Normal:
			default:
				return FSlateColor(BodyNormal);
			}
		});

		const FLinearColor OutlineBaseColor =
			(Params.Type == ET66ButtonType::Primary || Params.Type == ET66ButtonType::Success)
				? FT66Style::Success()
				: (Params.Type == ET66ButtonType::Danger
					? FT66Style::Danger()
					: (Params.Type == ET66ButtonType::ToggleActive ? FT66Style::Success() : FT66Style::Border()));
		const bool bPrimaryLikeButton =
			Params.Type == ET66ButtonType::Primary ||
			Params.Type == ET66ButtonType::Success ||
			Params.Type == ET66ButtonType::ToggleActive;

		const TAttribute<FSlateColor> MidBorderColor = TAttribute<FSlateColor>::CreateLambda([BorderState, BodyNormal, BodyHovered, BodyPressed, OutlineBaseColor, bPrimaryLikeButton]() -> FSlateColor
		{
			const auto ResolveMid = [OutlineBaseColor, bPrimaryLikeButton](const FLinearColor& InColor) -> FLinearColor
			{
				return MixColor(OutlineBaseColor, InColor, bPrimaryLikeButton ? 0.12f : 0.18f);
			};

			if (!BorderState.IsValid())
			{
				return FSlateColor(ResolveMid(BodyNormal));
			}

			switch (*BorderState)
			{
			case ET66ButtonBorderState::Hovered:
				return FSlateColor(ResolveMid(BodyHovered));
			case ET66ButtonBorderState::Pressed:
				return FSlateColor(ResolveMid(BodyPressed));
			case ET66ButtonBorderState::Normal:
			default:
				return FSlateColor(ResolveMid(BodyNormal));
			}
		});

		const TAttribute<FSlateColor> HighlightColor = TAttribute<FSlateColor>::CreateLambda([BorderState, BodyNormal, BodyHovered, BodyPressed, bPrimaryLikeButton]() -> FSlateColor
		{
			const auto ResolveHighlight = [bPrimaryLikeButton](const FLinearColor& InColor) -> FLinearColor
			{
				const float BlendAmount = bPrimaryLikeButton ? 0.10f : 0.24f;
				const float Alpha = bPrimaryLikeButton ? 0.10f : 0.22f;
				return WithAlpha(MixColor(InColor, FT66Style::Text(), BlendAmount), Alpha);
			};

			if (!BorderState.IsValid())
			{
				return FSlateColor(ResolveHighlight(BodyNormal));
			}

			switch (*BorderState)
			{
			case ET66ButtonBorderState::Hovered:
				return FSlateColor(ResolveHighlight(BodyHovered));
			case ET66ButtonBorderState::Pressed:
				return FSlateColor(ResolveHighlight(BodyPressed));
			case ET66ButtonBorderState::Normal:
			default:
				return FSlateColor(ResolveHighlight(BodyNormal));
			}
		});

		T66RuntimeUIBrushAccess::ET66DotaPlateBrushKind PlateType = T66RuntimeUIBrushAccess::ET66DotaPlateBrushKind::Neutral;
		if (Params.Type == ET66ButtonType::Primary || Params.Type == ET66ButtonType::Success || Params.Type == ET66ButtonType::ToggleActive)
		{
			PlateType = T66RuntimeUIBrushAccess::ET66DotaPlateBrushKind::Primary;
		}
		else if (Params.Type == ET66ButtonType::Danger)
		{
			PlateType = T66RuntimeUIBrushAccess::ET66DotaPlateBrushKind::Danger;
		}

		const bool bUseButtonPlateOverlay = Params.bUseDotaPlateOverlay;
		const FSlateBrush* PlateBrush = nullptr;
		if (bUseButtonPlateOverlay)
		{
			PlateBrush = Params.DotaPlateOverrideBrush ? Params.DotaPlateOverrideBrush : T66RuntimeUIBrushAccess::ResolveDotaButtonPlateBrush(PlateType);
		}
		const FLinearColor PlateNormalTint(1.00f, 1.00f, 1.00f, 1.0f);
		const FLinearColor PlateHoverTint(1.05f, 1.05f, 1.05f, 1.0f);
		const FLinearColor PlatePressedTint(0.92f, 0.92f, 0.92f, 1.0f);
		const TAttribute<FSlateColor> PlateTint = TAttribute<FSlateColor>::CreateLambda([BorderState, PlateNormalTint, PlateHoverTint, PlatePressedTint]() -> FSlateColor
		{
			if (!BorderState.IsValid())
			{
				return FSlateColor(PlateNormalTint);
			}

			switch (*BorderState)
			{
			case ET66ButtonBorderState::Hovered:
				return FSlateColor(PlateHoverTint);
			case ET66ButtonBorderState::Pressed:
				return FSlateColor(PlatePressedTint);
			case ET66ButtonBorderState::Normal:
			default:
				return FSlateColor(PlateNormalTint);
			}
		});

		const bool bUseMinimalPlateButton = false;

		if (bUseMinimalPlateButton)
		{
			return SNew(SBox)
				.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
				.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
				.Visibility(Params.Visibility)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						GlowWidget
					]
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Visibility(EVisibility::HitTestInvisible)
						.Image(PlateBrush)
						.ColorAndOpacity(PlateTint)
					]
					+ SOverlay::Slot()
					[
						SNew(SButton)
						.Cursor(EMouseCursor::Hand)
						.HAlign(BtnHAlign)
						.VAlign(BtnVAlign)
						.OnClicked(SafeClick)
						.OnHovered(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonHoverGlowIntensity); SetBorderState(ET66ButtonBorderState::Hovered); RefreshMouseCursorQuery(); }))
						.OnUnhovered(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(0.f); SetBorderState(ET66ButtonBorderState::Normal); RefreshMouseCursorQuery(); }))
						.OnPressed(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonPressedGlowIntensity); SetBorderState(ET66ButtonBorderState::Pressed); }))
						.OnReleased(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonHoverGlowIntensity); SetBorderState(ET66ButtonBorderState::Hovered); }))
						.ButtonStyle(&HitStyle)
						.ButtonColorAndOpacity(FSlateColor(FLinearColor::White))
						.ContentPadding(DotaContentPad)
						.IsEnabled(Params.IsEnabled)
						[
							Content
						]
					]
				];
		}

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					GlowWidget
				]
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FT66Style::PanelOuter())
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(MidBorderColor)
						.Padding(1.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(BodyColor)
							]
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Visibility(PlateBrush ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
								.Image(PlateBrush)
								.ColorAndOpacity(PlateTint)
							]
							+ SOverlay::Slot()
							.VAlign(VAlign_Top)
							.Padding(1.f, 1.f, 1.f, 0.f)
							[
								SNew(SBox)
								.HeightOverride(3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(HighlightColor)
								]
							]
							+ SOverlay::Slot()
							.VAlign(VAlign_Bottom)
							.Padding(1.f, 0.f, 1.f, 1.f)
							[
								SNew(SBox)
								.HeightOverride(2.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.45f))
								]
							]
							+ SOverlay::Slot()
							[
								SNew(SButton)
								.Cursor(EMouseCursor::Hand)
								.HAlign(BtnHAlign)
								.VAlign(BtnVAlign)
								.OnClicked(SafeClick)
								.OnHovered(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonHoverGlowIntensity); SetBorderState(ET66ButtonBorderState::Hovered); RefreshMouseCursorQuery(); }))
								.OnUnhovered(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(0.f); SetBorderState(ET66ButtonBorderState::Normal); RefreshMouseCursorQuery(); }))
								.OnPressed(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonPressedGlowIntensity); SetBorderState(ET66ButtonBorderState::Pressed); }))
								.OnReleased(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonHoverGlowIntensity); SetBorderState(ET66ButtonBorderState::Hovered); }))
								.ButtonStyle(&HitStyle)
								.ButtonColorAndOpacity(FSlateColor(FLinearColor::White))
								.ContentPadding(DotaContentPad)
								.IsEnabled(Params.IsEnabled)
								[
									Content
								]
							]
						]
					]
				]
			];
	}

	return SNew(SBox)
		.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
		.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
		.Visibility(Params.Visibility)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(0.f))
			[
				GlowWidget
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(0.f))
			[
				FillWidget
			]
			+ SOverlay::Slot()
			[
				SNew(SButton)
				.Cursor(EMouseCursor::Hand)
				.HAlign(BtnHAlign)
				.VAlign(BtnVAlign)
				.OnClicked(SafeClick)
				.OnHovered(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonHoverGlowIntensity); SetBorderState(ET66ButtonBorderState::Hovered); RefreshMouseCursorQuery(); }))
				.OnUnhovered(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(0.f); SetBorderState(ET66ButtonBorderState::Normal); RefreshMouseCursorQuery(); }))
				.OnPressed(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonPressedGlowIntensity); SetBorderState(ET66ButtonBorderState::Pressed); }))
				.OnReleased(FSimpleDelegate::CreateLambda([SetGlow, SetBorderState]() { SetGlow(Tokens::ButtonHoverGlowIntensity); SetBorderState(ET66ButtonBorderState::Hovered); }))
				.ButtonStyle(&BtnStyle)
				.ButtonColorAndOpacity(BtnColor)
				.ContentPadding(ContentPad)
				.IsEnabled(Params.IsEnabled)
				[
					Content
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				BorderWidget
			]
		];
}

TSharedRef<SWidget> FT66Style::MakeBareButton(const FT66BareButtonParams& Params, TSharedPtr<SButton>* OutButton)
{
	const FButtonStyle& ResolvedStyle = Params.ButtonStyle
		? *Params.ButtonStyle
		: FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
	const FOnClicked ClickDelegate = Params.bDebounceClick
		? DebounceClick(Params.OnClicked)
		: Params.OnClicked;

	TSharedRef<SButton> Button =
		SNew(SButton)
		.Cursor(Params.Cursor)
		.HAlign(Params.HAlign)
		.VAlign(Params.VAlign)
		.Visibility(Params.Visibility)
		.ToolTipText(Params.ToolTipText)
		.OnClicked(ClickDelegate)
		.OnHovered(WithMouseCursorRefresh(Params.OnHovered))
		.OnUnhovered(WithMouseCursorRefresh(Params.OnUnhovered))
		.OnPressed(Params.OnPressed)
		.OnReleased(Params.OnReleased)
		.ButtonStyle(&ResolvedStyle)
		.ButtonColorAndOpacity(Params.ButtonColorAndOpacity)
		.ContentPadding(Params.ContentPadding)
		.IsEnabled(Params.IsEnabled)
		[
			Params.Content.IsValid()
				? Params.Content.ToSharedRef()
				: StaticCastSharedRef<SWidget>(SNew(SBox))
		];

	if (OutButton)
	{
		*OutButton = Button;
	}

	if (Params.MinWidth > 0.f || Params.WidthOverride > 0.f || Params.HeightOverride > 0.f)
	{
		TSharedRef<SBox> SizedButton = SNew(SBox)[Button];
		if (Params.MinWidth > 0.f)
		{
			SizedButton->SetMinDesiredWidth(Params.MinWidth);
		}
		if (Params.WidthOverride > 0.f)
		{
			SizedButton->SetWidthOverride(Params.WidthOverride);
		}
		if (Params.HeightOverride > 0.f)
		{
			SizedButton->SetHeightOverride(Params.HeightOverride);
		}
		return SizedButton;
	}

	return Button;
}

// ---------------------------------------------------------------------------
// Convenience overload — thin wrapper around the params version.
// ---------------------------------------------------------------------------
TSharedRef<SWidget> FT66Style::MakeButton(
	const FText& Label,
	FOnClicked OnClicked,
	ET66ButtonType Type,
	float MinWidth)
{
	FT66ButtonParams P(Label, MoveTemp(OnClicked), Type);
	P.MinWidth = MinWidth;
	return MakeButton(P);
}

// ---------------------------------------------------------------------------
// HasPanelTextures — public query so callers can check texture availability.
// ---------------------------------------------------------------------------
bool FT66Style::HasPanelTextures()
{
	return GPanelTexturesAvailable();
}

// ---------------------------------------------------------------------------
// Full params-based MakePanel — every panel in the game funnels through here.
// ---------------------------------------------------------------------------
TSharedRef<SWidget> FT66Style::MakePanel(
	const TSharedRef<SWidget>& Content,
	const FT66PanelParams& Params,
	TSharedPtr<SBorder>* OutBorder)
{
	// 1. Resolve brush name from panel type.
	FName BrushName;
	switch (Params.Type)
	{
	case ET66PanelType::Bg:     BrushName = "T66.Brush.Bg";     break;
	case ET66PanelType::Panel2: BrushName = "T66.Brush.Panel2"; break;
	default:                    BrushName = "T66.Brush.Panel";   break;
	}

	const ET66ButtonBorderVisual ResolvedBorderVisual = ResolvePanelBorderVisual(Params);
	const ET66ButtonBackgroundVisual ResolvedBackgroundVisual = ResolvePanelBackgroundVisual(Params);
	const TSharedPtr<FT66ButtonFillBrushSet> FillBrushSet = FT66ButtonVisuals::CreateFillBrushSet(ResolvedBackgroundVisual);
	const bool bHasCustomFill = FillBrushSet.IsValid() && FillBrushSet->IsValid();
	const TSharedPtr<FT66ButtonBorderBrushSet> BorderBrushSet = FT66ButtonVisuals::CreateBorderBrushSet(ResolvedBorderVisual);
	const float BorderThickness = (BorderBrushSet.IsValid() && BorderBrushSet->IsValid())
		? ResolvePanelDecorativeBorderThickness(Params, BorderBrushSet->Thickness)
		: 0.f;
	const bool bHasDecorativeBorder = BorderThickness > 0.f;

	// 2. Background color: when textures are loaded, use white tint to pass through
	//    the baked-in look. Color overrides only apply to the fallback path.
	TAttribute<FSlateColor> BgColor;
	if (bHasCustomFill)
	{
		BgColor = Params.bHasColorOverride
			? Params.ColorOverride
			: TAttribute<FSlateColor>(FSlateColor(FLinearColor::White));
	}
	else if (HasPanelTextures())
	{
		BgColor = TAttribute<FSlateColor>(FSlateColor(FLinearColor::White));
	}
	else if (Params.bHasColorOverride)
	{
		BgColor = Params.ColorOverride;
	}
	else
	{
		// Fallback: match the procedural brush tint to the token color.
		FLinearColor DefaultColor;
		switch (Params.Type)
		{
		case ET66PanelType::Bg:     DefaultColor = Tokens::Bg;     break;
		case ET66PanelType::Panel2: DefaultColor = Tokens::Panel2; break;
		default:                    DefaultColor = Tokens::Panel;   break;
		}
		BgColor = TAttribute<FSlateColor>(FSlateColor(DefaultColor));
	}

	const TAttribute<const FSlateBrush*> BackgroundBrushAttr = TAttribute<const FSlateBrush*>::CreateLambda([FillBrushSet, bHasCustomFill, BrushName]() -> const FSlateBrush*
	{
		if (bHasCustomFill && FillBrushSet.IsValid() && FillBrushSet->Normal.IsValid())
		{
			return FillBrushSet->Normal.Get();
		}

		return FT66Style::Get().GetBrush(BrushName);
	});

	const TAttribute<const FSlateBrush*> HorizontalBorderBrushAttr = TAttribute<const FSlateBrush*>::CreateLambda([BorderBrushSet]() -> const FSlateBrush*
	{
		if (!BorderBrushSet.IsValid())
		{
			return nullptr;
		}

		return BorderBrushSet->GetHorizontalBrush(ET66ButtonBorderState::Normal);
	});

	const TAttribute<const FSlateBrush*> VerticalBorderBrushAttr = TAttribute<const FSlateBrush*>::CreateLambda([BorderBrushSet]() -> const FSlateBrush*
	{
		if (!BorderBrushSet.IsValid())
		{
			return nullptr;
		}

		return BorderBrushSet->GetVerticalBrush(ET66ButtonBorderState::Normal);
	});

	const TSharedRef<SWidget> DecorativeBorderWidget =
		bHasDecorativeBorder
		? StaticCastSharedRef<SWidget>(
			SNew(SOverlay)
			.Visibility(EVisibility::HitTestInvisible)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SNew(SBox)
				.HeightOverride(BorderThickness)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(HorizontalBorderBrushAttr)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Bottom)
			[
				SNew(SBox)
				.HeightOverride(BorderThickness)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(HorizontalBorderBrushAttr)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(BorderThickness)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(VerticalBorderBrushAttr)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(BorderThickness)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(VerticalBorderBrushAttr)
				]
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SBox)
			.Visibility(EVisibility::Collapsed));

	const TSharedRef<SWidget> PanelChild =
		bHasDecorativeBorder
		? StaticCastSharedRef<SWidget>(
			SNew(SOverlay)
			+ SOverlay::Slot()
			.Padding(Params.Padding)
			[
				Content
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				DecorativeBorderWidget
			])
		: Content;

	const bool bUseFlatPanelBody =
		ResolvedBorderVisual == ET66ButtonBorderVisual::None &&
		ResolvedBackgroundVisual == ET66ButtonBackgroundVisual::None &&
		!bHasCustomFill &&
		!bHasDecorativeBorder;

	if (IsDotaTheme())
	{
		if (bUseFlatPanelBody)
		{
			FLinearColor FlatFill = Params.bHasColorOverride
				? Params.ColorOverride.Get().GetSpecifiedColor()
				: (Params.Type == ET66PanelType::Panel2 ? FT66Style::PanelInner() : FT66Style::PanelOuter());

			if (Params.Type == ET66PanelType::Bg)
			{
				FlatFill = Params.bHasColorOverride
					? Params.ColorOverride.Get().GetSpecifiedColor()
					: FT66Style::Background();
			}

			TSharedRef<SBorder> FlatBorder = SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FlatFill)
				.Padding(Params.Padding)
				.Visibility(Params.Visibility)
				[
					Content
				];

			if (OutBorder)
			{
				*OutBorder = FlatBorder;
			}
			return FlatBorder;
		}

		const FLinearColor FillColorValue = Params.bHasColorOverride
			? Params.ColorOverride.Get().GetSpecifiedColor()
			: (Params.Type == ET66PanelType::Panel2 ? FT66Style::PanelInner() : FT66Style::Panel());

		if (Params.Type == ET66PanelType::Bg)
		{
			const TAttribute<FSlateColor> BackgroundColor = Params.bHasColorOverride
				? Params.ColorOverride
				: TAttribute<FSlateColor>(FSlateColor(FT66Style::Background()));

			TSharedRef<SBorder> BackgroundBorder = SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BackgroundColor)
				.Padding(Params.Padding)
				.Visibility(Params.Visibility)
				[
					Content
				];

			if (OutBorder)
			{
				*OutBorder = BackgroundBorder;
			}
			return BackgroundBorder;
		}

		TSharedPtr<SBorder> FillBorder;
		TSharedRef<SBorder> RootBorder = SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::PanelOuter())
			.Padding(1.f)
			.Visibility(Params.Visibility)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Params.Type == ET66PanelType::Panel2 ? FT66Style::Border() : MixColor(FT66Style::Border(), FT66Style::Stroke(), 0.18f))
				.Padding(1.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(FillBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FillColorValue)
						.Padding(Params.Padding)
						[
							Content
						]
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Top)
					.Padding(1.f, 1.f, 1.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(2.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(WithAlpha(MixColor(FillColorValue, FT66Style::Text(), 0.16f), 0.16f))
						]
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Bottom)
					.Padding(1.f, 0.f, 1.f, 1.f)
					[
						SNew(SBox)
						.HeightOverride(2.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.28f))
						]
					]
				]
			];

		if (OutBorder)
		{
			*OutBorder = FillBorder.IsValid() ? FillBorder : RootBorder;
		}
		return RootBorder;
	}

	// 3. Build the SBorder.
	TSharedRef<SBorder> Border = SNew(SBorder)
		.BorderImage(BackgroundBrushAttr)
		.BorderBackgroundColor(BgColor)
		.Padding(bHasDecorativeBorder ? FMargin(0.f) : Params.Padding)
		.Visibility(Params.Visibility)
		[
			PanelChild
		];

	if (OutBorder)
	{
		*OutBorder = Border;
	}

	return Border;
}

// ---------------------------------------------------------------------------
// Convenience overload — panel type + padding only.
// ---------------------------------------------------------------------------
TSharedRef<SWidget> FT66Style::MakePanel(
	const TSharedRef<SWidget>& Content,
	ET66PanelType Type,
	FMargin Padding)
{
	return MakePanel(Content, FT66PanelParams(Type).SetPadding(Padding));
}

// ---------------------------------------------------------------------------
// GetDropdownComboButtonStyle — combo style for dropdowns.
// ---------------------------------------------------------------------------
const FComboButtonStyle& FT66Style::GetDropdownComboButtonStyle()
{
	return Get().GetWidgetStyle<FComboButtonStyle>("T66.Dropdown.ComboButtonStyle");
}

// ---------------------------------------------------------------------------
// MakeDropdown — dark-styled SComboButton.
// ---------------------------------------------------------------------------
TSharedRef<SWidget> FT66Style::MakeDropdown(const FT66DropdownParams& Params)
{
	const FComboButtonStyle& ComboStyle = GetDropdownComboButtonStyle();
	TSharedRef<SComboButton> Combo = SNew(SComboButton)
		.ComboButtonStyle(&ComboStyle)
		.OnGetMenuContent_Lambda([OnGet = Params.OnGetMenuContent]() { return OnGet(); })
		.ContentPadding(Params.Padding)
		.ButtonContent()
		[
			Params.Content
		];

	// Height 0 = no override: let the combo button size to its content so dropdowns aren't clipped.
	return SNew(SBox)
		.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
		.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
		.Visibility(Params.Visibility)
		[
			Combo
		];
}

TSharedRef<SWidget> FT66Style::MakeDropdownOptionButton(
	const FText& Label,
	FOnClicked OnClicked,
	const bool bSelected,
	const float MinWidth,
	const float Height,
	const int32 FontSize,
	const FMargin& Padding)
{
	const FLinearColor OptionTextColor = bSelected
		? FLinearColor(1.f, 0.88f, 0.42f, 1.f)
		: FLinearColor(0.96f, 0.93f, 0.84f, 1.f);

	return T66OverlayChromeStyle::MakeButton(
		T66OverlayChromeStyle::MakeButtonParams(
			Label,
			MoveTemp(OnClicked),
			ET66OverlayChromeButtonFamily::DropdownOption)
		.SetMinWidth(MinWidth)
		.SetMinHeight(Height)
		.SetFontSize(FontSize)
		.SetPadding(Padding)
		.SetSelected(TAttribute<bool>(bSelected))
		.SetContent(
			SNew(STextBlock)
			.Text(Label)
			.Font(FT66Style::Tokens::FontBold(FontSize))
			.ColorAndOpacity(FSlateColor(OptionTextColor))
			.Justification(ETextJustify::Center)));
}

TSharedRef<SWidget> FT66Style::MakeScreenSurface(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return MakeLayeredSurface(
		Content,
		FT66Style::PanelOuter(),
		FT66Style::Border(),
		FT66Style::PanelInner(),
		FT66Style::Stroke() * FLinearColor(1.f, 1.f, 1.f, 0.55f),
		Padding);
}

TSharedRef<SWidget> FT66Style::MakeViewportFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return MakeLayeredSurface(
		Content,
		FT66Style::PanelOuter(),
		FT66Style::Border(),
		FT66Style::Background(),
		FT66Style::Stroke() * FLinearColor(1.f, 1.f, 1.f, 0.70f),
		Padding);
}

TSharedRef<SWidget> FT66Style::MakeViewportCutoutFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return MakeLayeredSurface(
		Content,
		FT66Style::PanelOuter(),
		FT66Style::Border(),
		FLinearColor(0.f, 0.f, 0.f, 0.f),
		FT66Style::Stroke() * FLinearColor(1.f, 1.f, 1.f, 0.70f),
		Padding);
}

TSharedRef<SWidget> FT66Style::MakeSlotFrame(const TSharedRef<SWidget>& Content, const TAttribute<FSlateColor>& AccentColor, const FMargin& Padding)
{
	return SNew(SBorder)
		.BorderImage(GetWhiteBrush())
		.BorderBackgroundColor(FT66Style::SlotOuter())
		.Padding(1.f)
		[
			SNew(SBorder)
			.BorderImage(GetWhiteBrush())
			.BorderBackgroundColor(AccentColor)
			.Padding(1.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(GetWhiteBrush())
					.BorderBackgroundColor(FT66Style::SlotFill())
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
						.Padding(Padding)
						[
							Content
						]
					]
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Top)
				.Padding(1.f, 1.f, 1.f, 0.f)
				[
					SNew(SBox)
					.HeightOverride(2.f)
					[
						SNew(SBorder)
						.BorderImage(GetWhiteBrush())
						.BorderBackgroundColor(FLinearColor(0.95f, 0.96f, 1.0f, 0.10f))
					]
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Bottom)
				.Padding(1.f, 0.f, 1.f, 1.f)
				[
					SNew(SBox)
					.HeightOverride(2.f)
					[
						SNew(SBorder)
						.BorderImage(GetWhiteBrush())
						.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
					]
				]
			]
		];
}

TSharedRef<SWidget> FT66Style::MakeSlotFrame(const TSharedRef<SWidget>& Content, const FLinearColor& AccentColor, const FMargin& Padding)
{
	return MakeSlotFrame(Content, TAttribute<FSlateColor>(FSlateColor(AccentColor)), Padding);
}

TSharedRef<SWidget> FT66Style::MakeHudPanel(const TSharedRef<SWidget>& Content, const FText& Title, const FMargin& Padding)
{
	TSharedRef<SWidget> PanelContent =
		Title.IsEmpty()
		? Content
		: StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Title)
				.Font(FT66Style::MakeFont(TEXT("Bold"), 14))
				.ColorAndOpacity(FLinearColor(0.86f, 0.68f, 0.34f, 1.f))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 4.f)
			[
				MakeDivider()
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				Content
			]);

	return MakeLayeredSurface(
		PanelContent,
		FLinearColor(0.018f, 0.014f, 0.012f, 0.98f),
		FLinearColor(0.58f, 0.42f, 0.22f, 0.92f),
		FLinearColor(0.030f, 0.026f, 0.022f, 0.96f),
		FLinearColor(0.95f, 0.72f, 0.34f, 0.18f),
		Padding);
}

TSharedRef<SWidget> FT66Style::MakeHudPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return MakeHudPanel(Content, FText::GetEmpty(), Padding);
}

TSharedRef<SWidget> FT66Style::MakeDivider(float Height)
{
	return SNew(SBox)
		.HeightOverride(Height)
		[
			SNew(SBorder)
			.BorderImage(GetWhiteBrush())
			.BorderBackgroundColor(FLinearColor(0.70f, 0.52f, 0.26f, 0.55f))
		];
}

TSharedRef<SWidget> FT66Style::MakeMinimapFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	if (const FSlateBrush* FrameBrush = ResolveMinimapFrameBrush())
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.004f, 0.005f, 0.008f, 0.98f))
				.Padding(Padding)
				[
					Content
				]
			]
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FrameBrush)
				.ColorAndOpacity(FLinearColor::White)
				.Visibility(EVisibility::HitTestInvisible)
			];
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			MakeLayeredSurface(
				Content,
				FLinearColor(0.018f, 0.014f, 0.012f, 0.99f),
				FLinearColor(0.63f, 0.48f, 0.27f, 0.96f),
				FLinearColor(0.012f, 0.014f, 0.016f, 0.98f),
				FLinearColor(0.95f, 0.78f, 0.38f, 0.22f),
				Padding)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(5.f)
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.86f, 0.70f, 0.38f, 1.f))
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(5.f)
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.86f, 0.70f, 0.38f, 1.f))
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(5.f)
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.86f, 0.70f, 0.38f, 1.f))
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(5.f)
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.86f, 0.70f, 0.38f, 1.f))
			]
		];
}

TSharedRef<SWidget> FT66Style::MakeResponsiveRoot(
	const TSharedRef<SWidget>& Content,
	const FVector2D& ReferenceResolution,
	bool bAllowUpscale)
{
	const TAttribute<float> Scale = TAttribute<float>::CreateLambda([ReferenceResolution, bAllowUpscale]() -> float
	{
		return FT66Style::GetViewportResponsiveScale(ReferenceResolution, bAllowUpscale);
	});

	return SNew(SDPIScaler)
		.DPIScale(Scale)
		[
			Content
		];
}

float FT66Style::GetViewportResponsiveScale(const FVector2D& ReferenceResolution, bool bAllowUpscale)
{
	return ComputeResponsiveScale(ReferenceResolution, bAllowUpscale);
}

FVector2D FT66Style::GetViewportSize()
{
	return GetViewportSizeOrFallback();
}

FVector2D FT66Style::GetViewportLogicalSize()
{
	const float GlobalScale = GetGlobalUIScale();
	if (GlobalScale <= KINDA_SMALL_NUMBER)
	{
		return GetViewportSize();
	}

	return GetViewportSize() / GlobalScale;
}

float FT66Style::GetEngineDPIScale()
{
	return GetConfiguredDPIScale(GetViewportSize());
}

float FT66Style::GetPlayerUIScale()
{
	return GetPlayerUIScaleOrDefault();
}

float FT66Style::GetGlobalUIScale()
{
	return GetEngineDPIScale() * GetPlayerUIScale();
}

FVector2D FT66Style::GetSafeFrameSize(float AspectRatio)
{
	const FVector2D LogicalViewport = GetViewportLogicalSize();
	const float SafeAspect = FMath::Max(AspectRatio, 0.1f);
	const float Width = FMath::Min(LogicalViewport.X, LogicalViewport.Y * SafeAspect);
	const float Height = FMath::Min(LogicalViewport.Y, LogicalViewport.X / SafeAspect);
	return FVector2D(FMath::Max(1.f, Width), FMath::Max(1.f, Height));
}

FMargin FT66Style::GetSafeFrameInsets(float AspectRatio)
{
	const FVector2D LogicalViewport = GetViewportLogicalSize();
	const FVector2D SafeFrame = GetSafeFrameSize(AspectRatio);
	const float HorizontalInset = FMath::Max(0.f, (LogicalViewport.X - SafeFrame.X) * 0.5f);
	const float VerticalInset = FMath::Max(0.f, (LogicalViewport.Y - SafeFrame.Y) * 0.5f);
	return FMargin(HorizontalInset, VerticalInset, HorizontalInset, VerticalInset);
}

FMargin FT66Style::GetSafePadding(const FMargin& Padding, float AspectRatio)
{
	const FMargin SafeInsets = GetSafeFrameInsets(AspectRatio);
	return FMargin(
		SafeInsets.Left + Padding.Left,
		SafeInsets.Top + Padding.Top,
		SafeInsets.Right + Padding.Right,
		SafeInsets.Bottom + Padding.Bottom);
}

TSharedRef<SWidget> FT66Style::MakeSafeFrame(
	const TSharedRef<SWidget>& Content,
	const FMargin& Padding,
	float MaxWidth,
	float MaxHeight,
	float AspectRatio)
{
	const TAttribute<FOptionalSize> WidthAttr = TAttribute<FOptionalSize>::CreateLambda([Padding, MaxWidth, AspectRatio]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize(AspectRatio);
		float Width = FMath::Max(1.f, SafeFrame.X - (Padding.Left + Padding.Right));
		if (MaxWidth > 0.f)
		{
			Width = FMath::Min(Width, MaxWidth);
		}
		return FOptionalSize(Width);
	});

	const TAttribute<FOptionalSize> HeightAttr = TAttribute<FOptionalSize>::CreateLambda([Padding, MaxHeight, AspectRatio]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize(AspectRatio);
		float Height = FMath::Max(1.f, SafeFrame.Y - (Padding.Top + Padding.Bottom));
		if (MaxHeight > 0.f)
		{
			Height = FMath::Min(Height, MaxHeight);
		}
		return FOptionalSize(Height);
	});

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(WidthAttr)
			.HeightOverride(HeightAttr)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(Padding)
				[
					Content
				]
			]
		];
}

// ---------------------------------------------------------------------------
// DeferRebuild — safe, next-tick widget rebuild for any UUserWidget.
// ---------------------------------------------------------------------------
void FT66Style::DeferRebuild(UUserWidget* Widget, int32 ZOrder)
{
	if (!Widget) return;
	if (!IsInGameThread())
	{
		TWeakObjectPtr<UUserWidget> WeakWidget(Widget);
		AsyncTask(ENamedThreads::GameThread, [WeakWidget, ZOrder]()
		{
			if (UUserWidget* SafeWidget = WeakWidget.Get())
			{
				FT66Style::DeferRebuild(SafeWidget, ZOrder);
			}
		});
		return;
	}
	UWorld* World = Widget->GetWorld();
	if (!World || World->bIsTearingDown || GExitPurge || IsGarbageCollecting()) return;

	TWeakObjectPtr<UUserWidget> Weak(Widget);
	for (auto It = GDeferredRebuildWidgets.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
	if (GDeferredRebuildWidgets.Contains(Weak))
	{
		return;
	}

	GDeferredRebuildWidgets.Add(Weak);
	TWeakObjectPtr<UWorld> WeakWorld(World);
	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateLambda([Weak, WeakWorld, ZOrder]()
		{
			GDeferredRebuildWidgets.Remove(Weak);

			UWorld* LocalWorld = WeakWorld.Get();
			UUserWidget* W = Weak.Get();
			if (!W || !LocalWorld || LocalWorld->bIsTearingDown || GExitPurge || IsGarbageCollecting()) return;

			const bool bInViewport = W->IsInViewport();
			if (bInViewport) W->RemoveFromParent();
			if (LocalWorld->bIsTearingDown || GExitPurge || IsGarbageCollecting()) return;

			W->ReleaseSlateResources(true);
			W->TakeWidget();
			if (LocalWorld->bIsTearingDown || GExitPurge || IsGarbageCollecting()) return;

			if (bInViewport) W->AddToViewport(ZOrder);
		}));
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
