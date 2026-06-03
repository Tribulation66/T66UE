// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66UIManager.h"

#include "Core/T66ShelvedFeatureGate.h"
#include "Core/T66ReleaseVariantSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

bool UT66UIManager::CanShowScreenForReleaseVariant(const ET66ScreenType ScreenType) const
{
	if (ScreenType == ET66ScreenType::None || ScreenType == ET66ScreenType::MainMenu)
	{
		return true;
	}

	if (!FT66ShelvedFeatureGate::IsScreenAllowed(ScreenType))
	{
		return false;
	}

	const UGameInstance* GameInstance = OwningPlayer ? OwningPlayer->GetGameInstance() : nullptr;
	const UT66ReleaseVariantSubsystem* ReleaseVariant = GameInstance
		? GameInstance->GetSubsystem<UT66ReleaseVariantSubsystem>()
		: nullptr;
	if (!ReleaseVariant || !ReleaseVariant->IsDemoModeActive())
	{
		return true;
	}

	switch (ScreenType)
	{
	case ET66ScreenType::DailyDescent:
		return false;
	default:
		return true;
	}
}
