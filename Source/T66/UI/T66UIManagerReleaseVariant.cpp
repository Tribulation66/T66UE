// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66UIManager.h"

#include "Core/T66ReleaseVariantSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

namespace
{
	bool T66IsDemoGatedMinigameScreenType(const ET66ScreenType ScreenType)
	{
		switch (ScreenType)
		{
		case ET66ScreenType::Minigames:
		case ET66ScreenType::VersusMainMenu:
		case ET66ScreenType::MiniMainMenu:
		case ET66ScreenType::MiniSaveSlots:
		case ET66ScreenType::MiniCharacterSelect:
		case ET66ScreenType::MiniCompanionSelect:
		case ET66ScreenType::MiniDifficultySelect:
		case ET66ScreenType::MiniIdolSelect:
		case ET66ScreenType::MiniShop:
		case ET66ScreenType::MiniRunSummary:
		case ET66ScreenType::MiniBattle:
		case ET66ScreenType::TDMainMenu:
		case ET66ScreenType::TDDifficultySelect:
		case ET66ScreenType::TDBattle:
		case ET66ScreenType::IdleMainMenu:
		case ET66ScreenType::DeckMainMenu:
			return true;
		default:
			return false;
		}
	}
}

bool UT66UIManager::CanShowScreenForReleaseVariant(const ET66ScreenType ScreenType) const
{
	if (ScreenType == ET66ScreenType::None || ScreenType == ET66ScreenType::MainMenu)
	{
		return true;
	}

	const UGameInstance* GameInstance = OwningPlayer ? OwningPlayer->GetGameInstance() : nullptr;
	const UT66ReleaseVariantSubsystem* ReleaseVariant = GameInstance
		? GameInstance->GetSubsystem<UT66ReleaseVariantSubsystem>()
		: nullptr;
	if (!ReleaseVariant || !ReleaseVariant->IsDemoModeActive())
	{
		return true;
	}

	if (T66IsDemoGatedMinigameScreenType(ScreenType))
	{
		return false;
	}

	switch (ScreenType)
	{
	case ET66ScreenType::DailyDescent:
		return false;
	default:
		return true;
	}
}
