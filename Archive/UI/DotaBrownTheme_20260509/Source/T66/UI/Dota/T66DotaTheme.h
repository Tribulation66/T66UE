// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

class T66_API FT66DotaTheme
{
public:
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

	static FSlateFontInfo MakeFont(const TCHAR* Weight, int32 Size);
};
