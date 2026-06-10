// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SWidget.h"

class SImage;
class STextBlock;

/**
 * Shared visual kit for the live casino gambler games (hellfire era).
 * Sprites are loose PNGs under RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/CasinoGames/
 * loaded through FT66FriendslopStyle::GetCustomBrush (size-exact Image plates; the table plate
 * is the only 9-slice surface).
 */
namespace T66GamblerStage
{
	// Fixed play-stage size (slate units). Sized to fit the gambler tab at 1280x720.
	constexpr float StageWidth = 980.f;
	constexpr float StageHeight = 360.f;

	const FSlateBrush* SpriteBrush(const TCHAR* FileName, const FVector2D& NativeSize);
	const FSlateBrush* TablePlateBrush();

	/** Table plate + caller content in a fixed, centered stage box. */
	TSharedRef<SWidget> MakeStage(const TSharedRef<SWidget>& Content);

	/** Big result banner text block (hidden until a reveal lands). */
	TSharedRef<STextBlock> MakeResultBanner(TSharedPtr<STextBlock>& OutText);

	void PlayUISound(const TCHAR* EventID);

	/** Compose translation/scale/rotation into a render transform around the widget center. */
	void ApplySpriteTransform(
		const TSharedPtr<SWidget>& Widget,
		const FVector2D& Translation,
		const FVector2D& Scale = FVector2D(1.f, 1.f),
		float AngleDegrees = 0.f);

	FLinearColor WinGold();
	FLinearColor LoseRed();
	FLinearColor DimTint();
}
