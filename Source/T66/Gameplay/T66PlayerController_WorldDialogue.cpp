// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/T66UIManager.h"
#include "UI/T66ScreenBase.h"
#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66CompanionGridScreen.h"
#include "UI/Screens/T66SaveSlotsScreen.h"
#include "UI/Screens/T66AchievementsScreen.h"
#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/Screens/T66PlayerSummaryPickerScreen.h"
#include "UI/Screens/T66PowerUpScreen.h"
#include "UI/Screens/T66AccountStatusScreen.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/T66LabOverlayWidget.h"
#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66CowardicePromptWidget.h"
#include "UI/T66LoadPreviewOverlayWidget.h"
#include "UI/T66IdolAltarOverlayWidget.h"
#include "UI/T66VendorOverlayWidget.h"
#include "UI/T66CollectorOverlayWidget.h"
#include "UI/T66CrateOverlayWidget.h"
#include "Gameplay/T66FountainOfLifeInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66CasinoInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "Gameplay/T66StageCatchUpGate.h"
#include "Gameplay/T66StageCatchUpGoldInteractable.h"
#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Gameplay/T66TutorialPortal.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "Camera/CameraComponent.h"

#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66ItemPickup.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66BossGroundAOE.h"
#include "Gameplay/T66HeroPlagueCloud.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Styling/CoreStyle.h"


void AT66PlayerController::OpenWorldDialogueVendor(AT66VendorNPC* Vendor)
{
	if (!IsGameplayLevel()) return;
	if (!GameplayHUDWidget) return;
	if (!Vendor) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const TArray<FText> Options =
	{
		Loc ? Loc->GetText_IWantToShop() : NSLOCTEXT("T66.Vendor", "IWantToShop", "I want to shop"),
		Loc ? Loc->GetText_TeleportMeToYourBrother() : NSLOCTEXT("T66.Gambler", "TeleportMeToYourBrother", "Teleport me to your brother"),
		Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "Back"),
	};

	WorldDialogueKind = ET66WorldDialogueKind::Vendor;
	WorldDialogueTargetNPC = Vendor;
	ActiveVendorNPC = Vendor;
	WorldDialogueTargetCompanion.Reset();
	WorldDialogueSelectedIndex = 0;
	WorldDialogueNumOptions = 3;
	bWorldDialogueOpen = true;
	LastWorldDialogueNavTimeSeconds = -1000.f;

	GameplayHUDWidget->ShowWorldDialogue(Options, WorldDialogueSelectedIndex);
	UpdateWorldDialoguePosition();
	GetWorldTimerManager().SetTimer(WorldDialoguePositionTimerHandle, this, &AT66PlayerController::UpdateWorldDialoguePosition, 0.1f, true);
}


void AT66PlayerController::OpenWorldDialogueGambler(AT66GamblerNPC* Gambler)
{
	if (!IsGameplayLevel()) return;
	if (!GameplayHUDWidget) return;
	if (!Gambler) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const TArray<FText> Options =
	{
		Loc ? Loc->GetText_LetMeGamble() : NSLOCTEXT("T66.Gambler", "LetMeGamble", "Let me gamble"),
		Loc ? Loc->GetText_TeleportMeToYourBrother() : NSLOCTEXT("T66.Gambler", "TeleportMeToYourBrother", "Teleport me to your brother"),
		Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "Back"),
	};

	WorldDialogueKind = ET66WorldDialogueKind::Gambler;
	WorldDialogueTargetNPC = Gambler;
	WorldDialogueTargetCompanion.Reset();
	WorldDialogueSelectedIndex = 0;
	WorldDialogueNumOptions = 3;
	bWorldDialogueOpen = true;
	LastWorldDialogueNavTimeSeconds = -1000.f;

	GameplayHUDWidget->ShowWorldDialogue(Options, WorldDialogueSelectedIndex);
	UpdateWorldDialoguePosition();
	GetWorldTimerManager().SetTimer(WorldDialoguePositionTimerHandle, this, &AT66PlayerController::UpdateWorldDialoguePosition, 0.1f, true);
}


void AT66PlayerController::OpenWorldDialogueCompanion(AT66RecruitableCompanion* Companion)
{
	if (!IsGameplayLevel()) return;
	if (!GameplayHUDWidget) return;
	if (!Companion) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const TArray<FText> Options =
	{
		Loc ? Loc->GetText_FollowMe() : NSLOCTEXT("T66.CompanionRecruit", "FollowMe", "Follow me"),
		Loc ? Loc->GetText_Leave() : NSLOCTEXT("T66.CompanionRecruit", "Leave", "Leave"),
	};

	WorldDialogueKind = ET66WorldDialogueKind::Companion;
	WorldDialogueTargetNPC.Reset();
	WorldDialogueTargetCompanion = Companion;
	WorldDialogueSelectedIndex = 0;
	WorldDialogueNumOptions = 2;
	bWorldDialogueOpen = true;
	LastWorldDialogueNavTimeSeconds = -1000.f;

	GameplayHUDWidget->ShowWorldDialogue(Options, WorldDialogueSelectedIndex);
	UpdateWorldDialoguePosition();
	GetWorldTimerManager().SetTimer(WorldDialoguePositionTimerHandle, this, &AT66PlayerController::UpdateWorldDialoguePosition, 0.1f, true);
}


void AT66PlayerController::CloseWorldDialogue()
{
	bWorldDialogueOpen = false;
	WorldDialogueKind = ET66WorldDialogueKind::None;
	WorldDialogueSelectedIndex = 0;
	WorldDialogueNumOptions = 3;
	WorldDialogueTargetNPC.Reset();
	WorldDialogueTargetCompanion.Reset();
	GetWorldTimerManager().ClearTimer(WorldDialoguePositionTimerHandle);

	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->HideWorldDialogue();
	}
}


void AT66PlayerController::NavigateWorldDialogue(int32 Delta)
{
	if (!bWorldDialogueOpen) return;
	const int32 MaxIndex = FMath::Max(0, WorldDialogueNumOptions - 1);
	WorldDialogueSelectedIndex = FMath::Clamp(WorldDialogueSelectedIndex + Delta, 0, MaxIndex);
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->SetWorldDialogueSelection(WorldDialogueSelectedIndex);
	}
}


void AT66PlayerController::ConfirmWorldDialogue()
{
	if (!bWorldDialogueOpen) return;

	// Capture before we close (closing clears state).
	const ET66WorldDialogueKind Kind = WorldDialogueKind;
	const int32 Choice = WorldDialogueSelectedIndex;
	AT66HouseNPCBase* TargetNPC = WorldDialogueTargetNPC.Get();
	AT66RecruitableCompanion* TargetCompanion = WorldDialogueTargetCompanion.Get();
	const int32 GamblerWinGoldAmount = (Kind == ET66WorldDialogueKind::Gambler)
		? (Cast<AT66GamblerNPC>(TargetNPC) ? Cast<AT66GamblerNPC>(TargetNPC)->GetWinGoldAmount() : 0)
		: 0;

	CloseWorldDialogue();
	if (!(Kind == ET66WorldDialogueKind::Vendor && Choice == 0))
	{
		ActiveVendorNPC.Reset();
	}

	if (Kind == ET66WorldDialogueKind::Companion)
	{
		// 0: follow me, 1: leave
		if (Choice != 0)
		{
			return;
		}

		if (!TargetCompanion || TargetCompanion->CompanionID.IsNone())
		{
			return;
		}

		if (AT66GameMode* GM = GetWorld() ? Cast<AT66GameMode>(UGameplayStatics::GetGameMode(this)) : nullptr)
		{
			const FName NewCompanionID = TargetCompanion->CompanionID;
			if (GM->SwapCompanionForPlayer(this, NewCompanionID))
			{
				TargetCompanion->Destroy();
			}
		}
		return;
	}

	// Vendor/Gambler: 0: main action, 1: teleport, 2: back
	if (Choice == 2)
	{
		return;
	}

	if (Choice == 1)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
				{
					// Mirror overlay behavior: no teleport once boss encounter starts.
					if (RunState->GetBossActive())
					{
						return;
					}
				}
			}
		}

		TeleportToNPC((Kind == ET66WorldDialogueKind::Vendor) ? FName(TEXT("Gambler")) : FName(TEXT("Vendor")));
		return;
	}

	// Choice == 0
	if (Kind == ET66WorldDialogueKind::Vendor)
	{
		OpenVendorOverlay();
		if (VendorOverlayWidget)
		{
			// Skip overlay dialogue page; go straight to shop.
			VendorOverlayWidget->OpenShopPage();
		}
	}
	else if (Kind == ET66WorldDialogueKind::Gambler)
	{
		OpenGamblerOverlay(GamblerWinGoldAmount);
		if (GamblerOverlayWidget)
		{
			// Skip overlay dialogue page; go straight to casino.
			GamblerOverlayWidget->OpenCasinoPage();
		}
	}
}


void AT66PlayerController::UpdateWorldDialoguePosition()
{
	if (!bWorldDialogueOpen) return;
	if (!GameplayHUDWidget) return;

	AActor* AnchorActor = nullptr;
	if (WorldDialogueKind == ET66WorldDialogueKind::Companion)
	{
		AnchorActor = WorldDialogueTargetCompanion.Get();
	}
	else
	{
		AnchorActor = WorldDialogueTargetNPC.Get();
	}

	if (!AnchorActor)
	{
		CloseWorldDialogue();
		return;
	}

	FVector2D ScreenPos(0.f, 0.f);
	const FVector WorldAnchor = AnchorActor->GetActorLocation() + FVector(0.f, 0.f, 180.f);
	if (!ProjectWorldLocationToScreen(WorldAnchor, ScreenPos, true))
	{
		return;
	}

	// Nudge to the right of the NPC (like the sketch), slightly upward.
	ScreenPos += FVector2D(140.f, -40.f);

	int32 VX = 0, VY = 0;
	GetViewportSize(VX, VY);
	ScreenPos.X = FMath::Clamp(ScreenPos.X, 12.f, FMath::Max(12.f, static_cast<float>(VX) - 340.f));
	ScreenPos.Y = FMath::Clamp(ScreenPos.Y, 12.f, FMath::Max(12.f, static_cast<float>(VY) - 180.f));

	GameplayHUDWidget->SetWorldDialogueScreenPosition(ScreenPos);
}


void AT66PlayerController::TeleportToNPC(FName NPCID)
{
	UWorld* World = GetWorld();
	if (!World) return;
	APawn* PlayerPawn = GetPawn();
	if (!PlayerPawn) return;

	AT66HouseNPCBase* Target = nullptr;
	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
		{
			if (AT66HouseNPCBase* N = WeakNPC.Get())
			{
				if (N->NPCID == NPCID)
				{
					Target = N;
					break;
				}
			}
		}
	}
	if (!Target) return;

	PlayerPawn->SetActorLocation(Target->GetActorLocation() + FVector(250.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
}
