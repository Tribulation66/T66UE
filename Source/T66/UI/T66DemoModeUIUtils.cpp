// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66DemoModeUIUtils.h"

#include "Core/T66ReleaseVariantSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66FlatStyle.h"

namespace
{
	const UT66ReleaseVariantSubsystem* GetDemoModeSubsystem(const UObject* WorldContextObject)
	{
		const UGameInstance* GI = WorldContextObject ? UGameplayStatics::GetGameInstance(WorldContextObject) : nullptr;
		return GI ? GI->GetSubsystem<UT66ReleaseVariantSubsystem>() : nullptr;
	}
}

bool T66DemoModeUI::IsDemoModeActive(const UObject* WorldContextObject)
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetDemoModeSubsystem(WorldContextObject))
	{
		return ReleaseVariant->IsDemoModeActive();
	}

	return false;
}

FText T66DemoModeUI::GetUnavailableContentText(const UObject* WorldContextObject)
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetDemoModeSubsystem(WorldContextObject))
	{
		return ReleaseVariant->GetUnavailableContentText();
	}

	return NSLOCTEXT("T66.DemoMode", "UnavailableContentFallback", "COMING SOON");
}

TSharedRef<SWidget> T66DemoModeUI::WrapWithComingSoonOverlay(
	const TSharedRef<SWidget>& Content,
	bool bShowOverlay,
	const UObject* WorldContextObject,
	FName Tag)
{
	if (!bShowOverlay)
	{
		return Content;
	}

	return FT66FlatStyle::WrapWithFlatUnavailableOverlay(
		Content,
		true,
		GetUnavailableContentText(WorldContextObject),
		Tag,
		true);
}
