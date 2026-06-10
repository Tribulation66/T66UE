// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Gambler/T66GamblerGameStage.h"

#include "Core/T66AudioSubsystem.h"
#include "Rendering/SlateRenderTransform.h"
#include "UI/Style/T66FriendslopStyle.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const TCHAR* CasinoGamesDir = TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/CasinoGames/");
}

namespace T66GamblerStage
{
	const FSlateBrush* SpriteBrush(const TCHAR* FileName, const FVector2D& NativeSize)
	{
		return FT66FriendslopStyle::GetCustomBrush(
			FString(CasinoGamesDir) + FileName,
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			NativeSize,
			FLinearColor(1.f, 1.f, 1.f, 0.f));
	}

	const FSlateBrush* TablePlateBrush()
	{
		return FT66FriendslopStyle::GetCustomBrush(
			FString(CasinoGamesDir) + TEXT("table_stage.png"),
			FMargin(0.06f),
			ESlateBrushDrawType::Box,
			FVector2D(StageWidth, StageHeight),
			FLinearColor(0.05f, 0.045f, 0.055f, 1.f));
	}

	TSharedRef<SWidget> MakeStage(const TSharedRef<SWidget>& Content)
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(StageWidth)
				.HeightOverride(StageHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage).Image(TablePlateBrush())
					]
					+ SOverlay::Slot()
					[
						Content
					]
				]
			];
	}

	TSharedRef<STextBlock> MakeResultBanner(TSharedPtr<STextBlock>& OutText)
	{
		TSharedRef<STextBlock> Banner = SNew(STextBlock)
			.Text(FText::GetEmpty())
			.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(30, true))
			.ColorAndOpacity(WinGold())
			.Justification(ETextJustify::Center)
			.RenderOpacity(0.f);
		OutText = Banner;
		return Banner;
	}

	void PlayUISound(const TCHAR* EventID)
	{
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(EventID));
	}

	void ApplySpriteTransform(
		const TSharedPtr<SWidget>& Widget,
		const FVector2D& Translation,
		const FVector2D& Scale,
		const float AngleDegrees)
	{
		if (!Widget.IsValid())
		{
			return;
		}

		Widget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		const FSlateRenderTransform Transform = ::Concatenate(
			FScale2D(Scale.X, Scale.Y),
			FQuat2D(FMath::DegreesToRadians(AngleDegrees)),
			FVector2D(Translation));
		Widget->SetRenderTransform(Transform);
	}

	FLinearColor WinGold()
	{
		return FLinearColor(0.98f, 0.78f, 0.22f, 1.f);
	}

	FLinearColor LoseRed()
	{
		return FLinearColor(0.92f, 0.22f, 0.18f, 1.f);
	}

	FLinearColor DimTint()
	{
		return FLinearColor(0.42f, 0.40f, 0.45f, 1.f);
	}
}
