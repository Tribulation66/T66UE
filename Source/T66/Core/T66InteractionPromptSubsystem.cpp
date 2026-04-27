// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66InteractionPromptSubsystem.h"

#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66CasinoInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66QuickReviveVendingMachine.h"
#include "Gameplay/T66StageCatchUpGoldInteractable.h"
#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Gameplay/T66TeleportPadInteractable.h"
#include "Gameplay/T66TreeOfLifeInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "GameFramework/InputSettings.h"

namespace
{
	static bool T66_IsKeyboardMouseKey(const FKey& Key)
	{
		if (!Key.IsValid())
		{
			return false;
		}

		if (Key.IsMouseButton())
		{
			return true;
		}

		return !Key.IsGamepadKey() && !Key.IsTouch();
	}

	static FText T66_GetActionKeyText(FName ActionName)
	{
		if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
		{
			for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			{
				if (Mapping.ActionName == ActionName && T66_IsKeyboardMouseKey(Mapping.Key))
				{
					return Mapping.Key.GetDisplayName();
				}
			}

			for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			{
				if (Mapping.ActionName == ActionName)
				{
					return Mapping.Key.GetDisplayName();
				}
			}
		}

		return NSLOCTEXT("T66.InteractionPrompt", "FallbackInteractKey", "F");
	}

	static FText T66_GetActionText(ET66InteractionPromptAction Action)
	{
		switch (Action)
		{
		case ET66InteractionPromptAction::PilotTractor:
			return NSLOCTEXT("T66.InteractionPrompt", "PilotTractor", "pilot tractor");
		case ET66InteractionPromptAction::ExitTractor:
			return NSLOCTEXT("T66.InteractionPrompt", "ExitTractor", "exit tractor");
		case ET66InteractionPromptAction::UseFountain:
			return NSLOCTEXT("T66.InteractionPrompt", "UseFountain", "use fountain");
		case ET66InteractionPromptAction::OpenChest:
			return NSLOCTEXT("T66.InteractionPrompt", "OpenChest", "open chest");
		case ET66InteractionPromptAction::SpinWheel:
			return NSLOCTEXT("T66.InteractionPrompt", "SpinWheel", "spin wheel");
		case ET66InteractionPromptAction::OpenCrate:
			return NSLOCTEXT("T66.InteractionPrompt", "OpenCrate", "open crate");
		case ET66InteractionPromptAction::EnterCasino:
			return NSLOCTEXT("T66.InteractionPrompt", "EnterCasino", "enter casino");
		case ET66InteractionPromptAction::GetQuickRevive:
			return NSLOCTEXT("T66.InteractionPrompt", "GetQuickRevive", "get quick revive");
		case ET66InteractionPromptAction::UseTeleporter:
			return NSLOCTEXT("T66.InteractionPrompt", "UseTeleporter", "use teleporter");
		case ET66InteractionPromptAction::RaiseDifficulty:
			return NSLOCTEXT("T66.InteractionPrompt", "RaiseDifficulty", "raise difficulty");
		case ET66InteractionPromptAction::ClaimGold:
			return NSLOCTEXT("T66.InteractionPrompt", "ClaimGold", "claim gold");
		case ET66InteractionPromptAction::ClaimLoot:
			return NSLOCTEXT("T66.InteractionPrompt", "ClaimLoot", "claim loot");
		default:
			return FText::GetEmpty();
		}
	}
}

ET66InteractionPromptAction UT66InteractionPromptSubsystem::GetPromptActionForActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return ET66InteractionPromptAction::None;
	}

	if (Cast<AT66PilotableTractor>(Actor))
	{
		return ET66InteractionPromptAction::PilotTractor;
	}
	if (Cast<AT66FountainOfLifeInteractable>(Actor))
	{
		return ET66InteractionPromptAction::UseFountain;
	}
	if (Cast<AT66ChestInteractable>(Actor))
	{
		return ET66InteractionPromptAction::OpenChest;
	}
	if (Cast<AT66WheelSpinInteractable>(Actor))
	{
		return ET66InteractionPromptAction::SpinWheel;
	}
	if (Cast<AT66CrateInteractable>(Actor))
	{
		return ET66InteractionPromptAction::OpenCrate;
	}
	if (Cast<AT66CasinoInteractable>(Actor))
	{
		return ET66InteractionPromptAction::EnterCasino;
	}
	if (Cast<AT66QuickReviveVendingMachine>(Actor))
	{
		return ET66InteractionPromptAction::GetQuickRevive;
	}
	if (Cast<AT66TeleportPadInteractable>(Actor))
	{
		return ET66InteractionPromptAction::UseTeleporter;
	}
	if (Cast<AT66DifficultyTotem>(Actor))
	{
		return ET66InteractionPromptAction::RaiseDifficulty;
	}
	if (Cast<AT66StageCatchUpGoldInteractable>(Actor))
	{
		return ET66InteractionPromptAction::ClaimGold;
	}
	if (Cast<AT66StageCatchUpLootInteractable>(Actor))
	{
		return ET66InteractionPromptAction::ClaimLoot;
	}

	return ET66InteractionPromptAction::None;
}

FText UT66InteractionPromptSubsystem::GetPromptTargetNameForActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return FText::GetEmpty();
	}

	if (Cast<AT66PilotableTractor>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetTractor", "Tractor");
	}
	if (Cast<AT66FountainOfLifeInteractable>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetFountainOfLife", "Fountain of Life");
	}
	if (Cast<AT66ChestInteractable>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetChest", "Chest");
	}
	if (Cast<AT66WheelSpinInteractable>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetWheelSpin", "Wheel Spin");
	}
	if (Cast<AT66CrateInteractable>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetCrate", "Crate");
	}
	if (Cast<AT66CasinoInteractable>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetCasino", "Casino");
	}
	if (Cast<AT66QuickReviveVendingMachine>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetQuickRevive", "Quick Revive");
	}
	if (Cast<AT66TeleportPadInteractable>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetTeleporter", "Teleporter");
	}
	if (Cast<AT66DifficultyTotem>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetDifficultyTotem", "Difficulty Totem");
	}
	if (Cast<AT66StageCatchUpGoldInteractable>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetGold", "Gold");
	}
	if (Cast<AT66StageCatchUpLootInteractable>(Actor))
	{
		return NSLOCTEXT("T66.InteractionPrompt", "TargetLoot", "Loot");
	}

	return FText::FromString(Actor->GetName());
}

FText UT66InteractionPromptSubsystem::BuildPromptText(ET66InteractionPromptAction Action) const
{
	return BuildPromptTextInternal(Action, 0, false);
}

FText UT66InteractionPromptSubsystem::BuildPromptTextWithSeconds(ET66InteractionPromptAction Action, int32 RemainingSeconds) const
{
	return BuildPromptTextInternal(Action, RemainingSeconds, true);
}

FText UT66InteractionPromptSubsystem::BuildCustomPromptText(const FText& ActionText) const
{
	return BuildCustomPromptTextInternal(ActionText, 0, false);
}

FText UT66InteractionPromptSubsystem::BuildCustomPromptTextWithSeconds(const FText& ActionText, int32 RemainingSeconds) const
{
	return BuildCustomPromptTextInternal(ActionText, RemainingSeconds, true);
}

FText UT66InteractionPromptSubsystem::BuildPromptTextInternal(
	const ET66InteractionPromptAction Action,
	const int32 RemainingSeconds,
	const bool bIncludeSeconds) const
{
	const FText ActionText = T66_GetActionText(Action);
	if (ActionText.IsEmpty())
	{
		return FText::GetEmpty();
	}

	const FText KeyText = T66_GetActionKeyText(FName(TEXT("Interact")));
	if (bIncludeSeconds)
	{
		return FText::Format(
			NSLOCTEXT("T66.InteractionPrompt", "PromptWithCountdown", "Press {0} to {1} ({2}s)"),
			KeyText,
			ActionText,
			FText::AsNumber(FMath::Max(1, RemainingSeconds)));
	}

	return FText::Format(
		NSLOCTEXT("T66.InteractionPrompt", "Prompt", "Press {0} to {1}"),
		KeyText,
		ActionText);
}

FText UT66InteractionPromptSubsystem::BuildCustomPromptTextInternal(
	const FText& ActionText,
	const int32 RemainingSeconds,
	const bool bIncludeSeconds) const
{
	if (ActionText.IsEmpty())
	{
		return FText::GetEmpty();
	}

	const FText KeyText = T66_GetActionKeyText(FName(TEXT("Interact")));
	if (bIncludeSeconds)
	{
		return FText::Format(
			NSLOCTEXT("T66.InteractionPrompt", "CustomPromptWithCountdown", "Press {0} to {1} ({2}s)"),
			KeyText,
			ActionText,
			FText::AsNumber(FMath::Max(1, RemainingSeconds)));
	}

	return FText::Format(
		NSLOCTEXT("T66.InteractionPrompt", "CustomPrompt", "Press {0} to {1}"),
		KeyText,
		ActionText);
}
