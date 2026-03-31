// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "HUD/ZibraVDBHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "ZibraVDBRuntime/Private/Render/ZibraVDBMaterialRendering.h"

void AZibraVDBHUD::DrawHUD()
{
	Super::DrawHUD();

	// Calculate the FPS
	const float FPS = 1.f / GetWorld()->GetDeltaSeconds();

	FColor TextColor = FColor::White;

	FVector2D ScreenPosition(100, 100);

	DrawText(FString::Printf(TEXT("FPS: %.2f"), FPS), TextColor, ScreenPosition.X, ScreenPosition.Y);

	for (const FZibraVDBMaterialRendering::FZibraVDBGPUProfilerCounters& ZibraVDBGPUProfilerCounters :
		FZibraVDBMaterialRendering::GPUProfilerCounters)
	{
		ScreenPosition.Y += 50;
		DrawText(
			FString::Printf(TEXT("%s:"), *ZibraVDBGPUProfilerCounters.ZibraVDBName), TextColor, ScreenPosition.X, ScreenPosition.Y);
		ScreenPosition.Y += 25;
		DrawText(FString::Printf(TEXT("    - Decompression: %.2f ms"), ZibraVDBGPUProfilerCounters.DecompressionTime / 1000.f),
			TextColor, ScreenPosition.X, ScreenPosition.Y);
		ScreenPosition.Y += 25;
		DrawText(FString::Printf(TEXT("    - Render: %.2f ms"), ZibraVDBGPUProfilerCounters.RenderTime / 1000.f), TextColor,
			ScreenPosition.X, ScreenPosition.Y);
		ScreenPosition.Y += 25;
		DrawText(FString::Printf(TEXT("    - Total frame: %.2f ms"),
					 (ZibraVDBGPUProfilerCounters.DecompressionTime + ZibraVDBGPUProfilerCounters.RenderTime) / 1000.f),
			TextColor, ScreenPosition.X, ScreenPosition.Y);
	}
}
