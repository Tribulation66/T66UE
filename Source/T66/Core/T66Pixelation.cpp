// Copyright Tribulation 66. All Rights Reserved.

#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Core/T66PixelationSubsystem.h"

namespace
{
	static void ApplyPixelationLevel(int32 Level)
	{
		UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
		if (!World)
		{
			World = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWorld() : nullptr;
		}
		if (!World || !World->GetGameInstance())
		{
			return;
		}
		if (UT66PixelationSubsystem* Sub = World->GetGameInstance()->GetSubsystem<UT66PixelationSubsystem>())
		{
			Sub->SetPixelationLevel(Level);
		}
	}
}

static FAutoConsoleCommand T66Pixel0Command(
	TEXT("Pixel0"),
	TEXT("Turn off retro pixelation (scene + UI at full res)."),
	FConsoleCommandDelegate::CreateLambda([]() { ApplyPixelationLevel(0); })
);

#define T66_REGISTER_PIXEL_CMD(N) \
	static FAutoConsoleCommand T66Pixel##N##Command( \
		TEXT("Pixel" #N), \
		TEXT("Retro pixelation level " #N " (1=least, 10=most; 10 = former slight). Scene only; UI stays crisp. Console only; not persisted."), \
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
