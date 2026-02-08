// Copyright Tribulation 66. All Rights Reserved.

#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"

namespace
{
	// Retro pixelation: level 1 = very slight (100%), level 10 = strong (25%).
	// Tune these values in one place to adjust the look.
	static const float GPixelationLevelToPercentage[10] = {
		100.f, 90.f, 80.f, 70.f, 60.f, 50.f, 42.f, 35.f, 29.f, 25.f
	};

	static void ApplyPixelationLevel(int32 Level)
	{
		if (Level < 1 || Level > 10) return;
		const float Percentage = GPixelationLevelToPercentage[Level - 1];

		// Use ECVF_SetByConsole — highest user-settable priority — so the engine's
		// internal ApplySettings (which uses SetByGameSetting) cannot overwrite us.
		// The Scalability system may log "ignored" warnings; those are harmless and
		// simply confirm our value is kept.
		if (IConsoleVariable* CVarScreenPct = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
		{
			CVarScreenPct->Set(Percentage, ECVF_SetByConsole);
		}
		if (IConsoleVariable* CVarSecondary = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SecondaryScreenPercentage.GameViewport")))
		{
			CVarSecondary->Set(Percentage, ECVF_SetByConsole);
		}
	}
}

#define T66_REGISTER_PIXEL_CMD(N) \
	static FAutoConsoleCommand T66Pixel##N##Command( \
		TEXT("Pixel" #N), \
		TEXT("Retro pixelation level " #N " (1=very slight, 10=strong). Console only; not persisted."), \
		FConsoleCommandDelegate::CreateLambda([]() { ApplyPixelationLevel(N); }) \
	);

T66_REGISTER_PIXEL_CMD(1)
T66_REGISTER_PIXEL_CMD(2)
T66_REGISTER_PIXEL_CMD(3)
T66_REGISTER_PIXEL_CMD(4)
T66_REGISTER_PIXEL_CMD(5)
T66_REGISTER_PIXEL_CMD(6)
T66_REGISTER_PIXEL_CMD(7)
T66_REGISTER_PIXEL_CMD(8)
T66_REGISTER_PIXEL_CMD(9)
T66_REGISTER_PIXEL_CMD(10)

#undef T66_REGISTER_PIXEL_CMD
