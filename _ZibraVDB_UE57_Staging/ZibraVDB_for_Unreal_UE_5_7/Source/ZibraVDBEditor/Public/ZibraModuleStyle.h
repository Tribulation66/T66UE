// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"

/**  */
class FZibraModuleStyle
{
public:
	static void Initialize();

	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the Shooter game */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:
	static TSharedPtr<class FSlateStyleSet> CreateStyleInstance();

private:
	static TSharedPtr<class FSlateStyleSet> StyleInstance;
};
