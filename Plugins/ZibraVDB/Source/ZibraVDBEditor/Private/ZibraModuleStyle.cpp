// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraModuleStyle.h"

#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FZibraModuleStyle::StyleInstance = nullptr;

void FZibraModuleStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = CreateStyleInstance();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FZibraModuleStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FZibraModuleStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ZibraModuleStyle"));
	return StyleSetName;
}

#define SLATE_IMAGE_BRUSH(RelativePath, Size) \
	new FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), FVector2D(Size, Size))

TSharedPtr<FSlateStyleSet> FZibraModuleStyle::CreateStyleInstance()
{
	TSharedPtr<FSlateStyleSet> Style = MakeShared<FSlateStyleSet>("ZibraModuleStyle");
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ZibraVDB")->GetBaseDir() / TEXT("Content/Icons"));

	Style->Set("ClassIcon.ZibraVDBAsset", SLATE_IMAGE_BRUSH("/AssetIcon", 128.0f));
	Style->Set("ClassThumbnail.ZibraVDBAsset", SLATE_IMAGE_BRUSH("/AssetIcon", 128.0f));

	return Style;
}

#undef SLATE_IMAGE_BRUSH

void FZibraModuleStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FZibraModuleStyle::Get()
{
	return *StyleInstance;
}
