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
#include "UI/Screens/T66LobbyScreen.h"
#include "UI/Screens/T66LobbyReadyCheckModal.h"
#include "UI/Screens/T66LobbyBackConfirmModal.h"
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
#include "UI/T66CasinoOverlayWidget.h"
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


float AT66PlayerController::GetHeroOneScopedUltRemainingSeconds() const
{
	if (!bHeroOneScopedUltActive || !GetWorld()) return 0.f;
	return FMath::Max(0.f, HeroOneScopedUltEndTime - GetWorld()->GetTimeSeconds());
}


float AT66PlayerController::GetHeroOneScopedShotCooldownRemainingSeconds() const
{
	if (!bHeroOneScopedUltActive || !GetWorld()) return 0.f;
	return FMath::Max(0.f, HeroOneScopedShotCooldownEndTime - GetWorld()->GetTimeSeconds());
}


void AT66PlayerController::ActivateHeroOneScopedUlt(AT66HeroBase* Hero, UT66CombatComponent* Combat)
{
	UWorld* World = GetWorld();
	if (!World || !Hero || !Combat)
	{
		return;
	}

	EndHeroOneScopedUlt();

	bHeroOneScopedUltActive = true;
	bHeroOneScopeViewEnabled = false;
	HeroOneScopedUltEndTime = World->GetTimeSeconds() + HeroOneScopedUltDurationSeconds;
	HeroOneScopedShotCooldownEndTime = World->GetTimeSeconds();

	Combat->SetAutoAttackSuppressed(true);
	Combat->ClearLockedTarget();

	if (LockedEnemy.IsValid())
	{
		LockedEnemy->SetLockedIndicator(false);
		LockedEnemy.Reset();
	}

	RestoreGameplayInputMode();
	World->GetTimerManager().SetTimer(HeroOneScopedUltTimerHandle, this, &AT66PlayerController::EndHeroOneScopedUlt, HeroOneScopedUltDurationSeconds, false);
}


void AT66PlayerController::EndHeroOneScopedUlt()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HeroOneScopedUltTimerHandle);
	}

	if (bHeroOneScopeViewEnabled)
	{
		SetHeroOneScopeViewEnabled(false);
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		if (AT66HeroBase* Hero = Cast<AT66HeroBase>(ControlledPawn))
		{
			if (UT66CombatComponent* Combat = Hero->FindComponentByClass<UT66CombatComponent>())
			{
				Combat->SetAutoAttackSuppressed(false);
			}
		}
	}

	bHeroOneScopedUltActive = false;
	bHeroOneScopeViewEnabled = false;
	HeroOneScopedUltEndTime = -1.f;
	HeroOneScopedShotCooldownEndTime = -1.f;
}


void AT66PlayerController::ToggleHeroOneScopeView()
{
	if (!bHeroOneScopedUltActive)
	{
		return;
	}

	SetHeroOneScopeViewEnabled(!bHeroOneScopeViewEnabled);
}


void AT66PlayerController::SetHeroOneScopeViewEnabled(bool bEnabled)
{
	if (!bHeroOneScopedUltActive && bEnabled)
	{
		return;
	}

	if (bHeroOneScopeViewEnabled == bEnabled)
	{
		return;
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	if (!Hero || !Hero->CameraBoom || !Hero->FollowCamera)
	{
		bHeroOneScopeViewEnabled = false;
		return;
	}

	if (bEnabled)
	{
		RestoreGameplayInputMode();

		SavedScopeCameraBoomLength = Hero->CameraBoom->TargetArmLength;
		SavedScopeCameraBoomLocation = Hero->CameraBoom->GetRelativeLocation();
		SavedScopeCameraFOV = Hero->FollowCamera->FieldOfView;
		bSavedScopeCameraCollisionTest = Hero->CameraBoom->bDoCollisionTest;
		bSavedScopeUseControllerRotationYaw = Hero->bUseControllerRotationYaw;
		bSavedScopePlaceholderVisible = Hero->PlaceholderMesh ? Hero->PlaceholderMesh->IsVisible() : false;
		bSavedScopeMeshVisible = Hero->GetMesh() ? Hero->GetMesh()->IsVisible() : false;

		if (UCharacterMovementComponent* Move = Hero->GetCharacterMovement())
		{
			bSavedScopeOrientRotationToMovement = Move->bOrientRotationToMovement;
			Move->bOrientRotationToMovement = false;
		}

		Hero->bUseControllerRotationYaw = true;
		Hero->CameraBoom->TargetArmLength = 0.f;
		Hero->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 78.f));
		Hero->CameraBoom->bDoCollisionTest = false;
		Hero->FollowCamera->SetFieldOfView(HeroOneScopedCameraFOV);

		if (Hero->PlaceholderMesh)
		{
			Hero->PlaceholderMesh->SetVisibility(false, true);
		}
		if (Hero->GetMesh())
		{
			Hero->GetMesh()->SetVisibility(false, true);
		}
	}
	else
	{
		Hero->CameraBoom->TargetArmLength = SavedScopeCameraBoomLength;
		Hero->CameraBoom->SetRelativeLocation(SavedScopeCameraBoomLocation);
		Hero->CameraBoom->bDoCollisionTest = bSavedScopeCameraCollisionTest;
		Hero->FollowCamera->SetFieldOfView(SavedScopeCameraFOV);
		Hero->bUseControllerRotationYaw = bSavedScopeUseControllerRotationYaw;

		if (UCharacterMovementComponent* Move = Hero->GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = bSavedScopeOrientRotationToMovement;
		}

		if (Hero->PlaceholderMesh)
		{
			Hero->PlaceholderMesh->SetVisibility(bSavedScopePlaceholderVisible, true);
		}
		if (Hero->GetMesh())
		{
			Hero->GetMesh()->SetVisibility(bSavedScopeMeshVisible, true);
		}
	}

	bHeroOneScopeViewEnabled = bEnabled;
}


void AT66PlayerController::TryFireHeroOneScopedUltShot()
{
	if (!bHeroOneScopedUltActive || !bHeroOneScopeViewEnabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	if (!World || !Hero || !Hero->CombatComponent)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now < HeroOneScopedShotCooldownEndTime)
	{
		return;
	}

	const FVector CameraLocation = PlayerCameraManager ? PlayerCameraManager->GetCameraLocation() : Hero->FollowCamera->GetComponentLocation();
	const FRotator CameraRotation = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : Hero->FollowCamera->GetComponentRotation();
	const FVector ShotDirection = CameraRotation.Vector();
	const FVector MaxEnd = CameraLocation + ShotDirection * HeroOneScopedMaxRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(HeroOneScopedUltTrace), false);
	Params.AddIgnoredActor(Hero);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FHitResult BlockingHit;
	FVector FinalEnd = MaxEnd;
	if (World->LineTraceSingleByObjectType(BlockingHit, CameraLocation, MaxEnd, ObjectParams, Params))
	{
		FinalEnd = BlockingHit.ImpactPoint;
	}

	Hero->CombatComponent->PerformScopedPiercingShot(CameraLocation, FinalEnd);
	HeroOneScopedShotCooldownEndTime = Now + HeroOneScopedShotCooldownSeconds;
}
