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

namespace
{
	static float T66GetAttackReticleYOffset(const int32 ViewportHeight)
	{
		return FMath::Clamp(-0.18f * static_cast<float>(ViewportHeight), -220.f, -140.f);
	}

	static ET66ItemRarity LootRarityToItemRarity(ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Black:  return ET66ItemRarity::Black;
		case ET66Rarity::Red:    return ET66ItemRarity::Red;
		case ET66Rarity::Yellow: return ET66ItemRarity::Yellow;
		case ET66Rarity::White:  return ET66ItemRarity::White;
		default:                 return ET66ItemRarity::Black;
		}
	}

	static bool T66_IsGamblersTokenItem(const FName ItemID)
	{
		return ItemID == FName(TEXT("Item_GamblersToken"));
	}

	static void ApplyUltimateDamageToRegisteredTargets(UWorld* World, int32 DamageAmount)
	{
		if (!World)
		{
			return;
		}

		const FName UltimateSourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
		UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
		if (!Registry)
		{
			return;
		}

		for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
		{
			if (AT66EnemyBase* Enemy = WeakEnemy.Get())
			{
				Enemy->TakeDamageFromHero(DamageAmount, UltimateSourceID, NAME_None);
			}
		}

		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			if (AT66BossBase* Boss = WeakBoss.Get())
			{
				if (Boss->IsAwakened() && Boss->IsAlive())
				{
					Boss->TakeDamageFromHeroHit(DamageAmount, UltimateSourceID, NAME_None);
				}
			}
		}
	}

	static TArray<FKey> GetKeyboardMouseActionKeys(const FName ActionName)
	{
		TArray<FKey> OutKeys;
		if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
		{
			for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			{
				if (Mapping.ActionName != ActionName) continue;
				if (!Mapping.Key.IsValid()) continue;
				if (!Mapping.Key.IsMouseButton() && (Mapping.Key.IsGamepadKey() || Mapping.Key.IsTouch())) continue;
				OutKeys.AddUnique(Mapping.Key);
			}
		}
		return OutKeys;
	}
}

void AT66PlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	SyncLockedEnemyFromCombat();
}

bool AT66PlayerController::HasAttackLockedEnemy() const
{
	const APawn* ControlledPawn = GetPawn();
	const AT66HeroBase* Hero = Cast<AT66HeroBase>(ControlledPawn);
	const UT66CombatComponent* Combat = Hero ? Hero->CombatComponent : nullptr;
	const AT66EnemyBase* CombatLockedEnemy = Combat ? Cast<AT66EnemyBase>(Combat->GetLockedTarget()) : nullptr;
	return CombatLockedEnemy && CombatLockedEnemy->CurrentHP > 0;
}

bool AT66PlayerController::GetAttackLockScreenPosition(FVector2D& OutScreenPosition) const
{
	int32 SizeX = 0;
	int32 SizeY = 0;
	GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0)
	{
		OutScreenPosition = FVector2D::ZeroVector;
		return false;
	}

	OutScreenPosition = FVector2D(
		static_cast<float>(SizeX) * 0.5f,
		(static_cast<float>(SizeY) * 0.5f) + T66GetAttackReticleYOffset(SizeY));
	return true;
}

void AT66PlayerController::SetLockedEnemy(AT66EnemyBase* NewLockedEnemy, const bool bPropagateToCombatTarget)
{
	if (NewLockedEnemy && NewLockedEnemy->CurrentHP <= 0)
	{
		NewLockedEnemy = nullptr;
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	UT66CombatComponent* Combat = Hero ? Hero->CombatComponent : nullptr;

	if (LockedEnemy.Get() == NewLockedEnemy)
	{
		if (bPropagateToCombatTarget && Combat)
		{
			if (NewLockedEnemy)
			{
				Combat->SetLockedTarget(NewLockedEnemy);
			}
			else
			{
				Combat->ClearLockedTarget();
			}
		}
		return;
	}

	if (LockedEnemy.IsValid())
	{
		LockedEnemy->SetLockedIndicator(false);
	}

	LockedEnemy = NewLockedEnemy;

	if (LockedEnemy.IsValid())
	{
		LockedEnemy->SetLockedIndicator(true);
	}

	if (bPropagateToCombatTarget && Combat)
	{
		if (NewLockedEnemy)
		{
			Combat->SetLockedTarget(NewLockedEnemy);
		}
		else
		{
			Combat->ClearLockedTarget();
		}
	}
}

void AT66PlayerController::SyncLockedEnemyFromCombat()
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	UT66CombatComponent* Combat = Hero ? Hero->CombatComponent : nullptr;
	AT66EnemyBase* CombatLockedEnemy = Combat ? Cast<AT66EnemyBase>(Combat->GetLockedTarget()) : nullptr;
	if (CombatLockedEnemy && CombatLockedEnemy->CurrentHP <= 0)
	{
		CombatLockedEnemy = nullptr;
	}

	SetLockedEnemy(CombatLockedEnemy, false);
}


void AT66PlayerController::RefreshGameplayMouseMappings()
{
	UObject* Outer = GetTransientPackage();
	if (!IA_AttackLock)
	{
		IA_AttackLock = NewObject<UInputAction>(Outer, FName(TEXT("IA_AttackLock_T66")));
		if (IA_AttackLock) IA_AttackLock->ValueType = EInputActionValueType::Boolean;
	}
	if (!IA_AttackUnlock)
	{
		IA_AttackUnlock = NewObject<UInputAction>(Outer, FName(TEXT("IA_AttackUnlock_T66")));
		if (IA_AttackUnlock) IA_AttackUnlock->ValueType = EInputActionValueType::Boolean;
	}
	if (!IA_ToggleMouseLock)
	{
		IA_ToggleMouseLock = NewObject<UInputAction>(Outer, FName(TEXT("IA_ToggleMouseLock_T66")));
		if (IA_ToggleMouseLock) IA_ToggleMouseLock->ValueType = EInputActionValueType::Boolean;
	}

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IMC_GameplayMouse)
			{
				Sub->RemoveMappingContext(IMC_GameplayMouse);
			}

			IMC_GameplayMouse = NewObject<UInputMappingContext>(Outer, FName(TEXT("IMC_GameplayMouse_T66")));
			if (IMC_GameplayMouse)
			{
				auto MapActionKeys = [this](UInputAction* Action, const FName ActionName, const FKey& FallbackKey)
				{
					if (!IMC_GameplayMouse || !Action) return;
					TArray<FKey> Keys = GetKeyboardMouseActionKeys(ActionName);
					if (Keys.Num() == 0)
					{
						Keys.Add(FallbackKey);
					}
					for (const FKey& Key : Keys)
					{
						IMC_GameplayMouse->MapKey(Action, Key);
					}
				};

				MapActionKeys(IA_AttackLock, FName(TEXT("AttackLock")), EKeys::LeftMouseButton);
				MapActionKeys(IA_AttackUnlock, FName(TEXT("AttackUnlock")), EKeys::ThumbMouseButton2);
				MapActionKeys(IA_ToggleMouseLock, FName(TEXT("ToggleMouseLock")), EKeys::RightMouseButton);
				Sub->AddMappingContext(IMC_GameplayMouse, 0);
			}
		}
	}
}


void AT66PlayerController::HandleUltimatePressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!RunState) return;

	if (!RunState->TryActivateUltimate())
		return;

	RunState->NotifyTutorialUltimateUsed();

	FHeroData HeroData;
	const ET66UltimateType UltType = (T66GI && T66GI->GetSelectedHeroData(HeroData))
		? HeroData.UltimateType
		: ET66UltimateType::None;

	const int32 UltDmg = UT66RunStateSubsystem::UltimateDamage;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	UT66CombatComponent* Combat = Hero ? Hero->FindComponentByClass<UT66CombatComponent>() : nullptr;

	switch (UltType)
	{
	case ET66UltimateType::SpearStorm:
		if (Combat)
			Combat->PerformUltimateSpearStorm(UltDmg);
		else
		{
			ApplyUltimateDamageToRegisteredTargets(World, UltDmg);
		}
		break;
	case ET66UltimateType::MeteorStrike:
	{
		if (Hero)
		{
			const FVector HeroLoc = Hero->GetActorLocation();
			const FVector Fwd = Hero->GetActorForwardVector();
			const FVector Right = Hero->GetActorRightVector();
			TArray<FVector> Offsets = {
				Fwd * 400.f,
				Fwd * 300.f + Right * 280.f,
				Fwd * 300.f - Right * 280.f,
				Fwd * 200.f + Right * 350.f,
				Fwd * 200.f - Right * 350.f
			};
			for (const FVector& Off : Offsets)
			{
				const FTransform SpawnTM(HeroLoc + Off);
				AT66BossGroundAOE* AOE = World->SpawnActorDeferred<AT66BossGroundAOE>(AT66BossGroundAOE::StaticClass(), SpawnTM);
				if (AOE)
				{
					AOE->DamageHP = UltDmg;
					AOE->bDamageEnemies = true;
					AOE->Radius = 280.f;
					AOE->WarningDurationSeconds = 1.f;
					AOE->PillarLingerSeconds = 0.6f;
					AOE->FinishSpawning(SpawnTM);
				}
			}
		}
		break;
	}
	case ET66UltimateType::ChainLightning:
		if (Combat)
			Combat->PerformUltimateChainLightning(UltDmg);
		else
		{
			ApplyUltimateDamageToRegisteredTargets(World, UltDmg);
		}
		break;
	case ET66UltimateType::PlagueCloud:
		if (Hero)
		{
			AT66HeroPlagueCloud* Cloud = World->SpawnActor<AT66HeroPlagueCloud>(Hero->GetActorLocation(), FRotator::ZeroRotator);
			if (Cloud)
				Cloud->InitFromUltimate(UltDmg);
		}
		break;
	case ET66UltimateType::PrecisionStrike:
		if (Combat) Combat->PerformUltimatePrecisionStrike(UltDmg);
		break;
	case ET66UltimateType::FanTheHammer:
		if (Combat) Combat->PerformUltimateFanTheHammer(UltDmg);
		break;
	case ET66UltimateType::Deadeye:
		if (Combat) Combat->PerformUltimateDeadeye(UltDmg);
		break;
	case ET66UltimateType::Discharge:
		if (Combat) Combat->PerformUltimateDischarge(UltDmg);
		break;
	case ET66UltimateType::Juiced:
		if (Combat) Combat->PerformUltimateJuiced();
		break;
	case ET66UltimateType::DeathSpiral:
		if (Combat) Combat->PerformUltimateDeathSpiral(UltDmg);
		break;
	case ET66UltimateType::Shockwave:
		if (Combat) Combat->PerformUltimateShockwave(UltDmg);
		break;
	case ET66UltimateType::TidalWave:
		if (Combat) Combat->PerformUltimateTidalWave(UltDmg);
		break;
	case ET66UltimateType::GoldRush:
		if (Combat) Combat->PerformUltimateGoldRush(UltDmg);
		break;
	case ET66UltimateType::MiasmaBomb:
		if (Combat) Combat->PerformUltimateMiasmaBomb(UltDmg);
		break;
	case ET66UltimateType::RabidFrenzy:
		if (Combat) Combat->PerformUltimateRabidFrenzy();
		break;
	case ET66UltimateType::Blizzard:
		if (Combat) Combat->PerformUltimateBlizzard(UltDmg);
		break;
	case ET66UltimateType::ScopedSniper:
		if (Hero && Combat)
		{
			ActivateHeroOneScopedUlt(Hero, Combat);
		}
		break;
	case ET66UltimateType::None:
	default:
	{
		ApplyUltimateDamageToRegisteredTargets(World, UltDmg);
		break;
	}
	}
}


bool AT66PlayerController::CanUseCombatMouseInput() const
{
	// Suppress combat mouse inputs if a modal overlay is active (cursor is visible for UI).
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	return IsGameplayLevel()
		&& !(GameplayHUDWidget && GameplayHUDWidget->IsFullMapOpen())
		&& !(GamblerOverlayWidget && GamblerOverlayWidget->IsInViewport())
		&& !(CowardicePromptWidget && CowardicePromptWidget->IsInViewport())
		&& !(IdolAltarOverlayWidget && IdolAltarOverlayWidget->IsInViewport())
		&& !(VendorOverlayWidget && VendorOverlayWidget->IsInViewport());
}


void AT66PlayerController::HandleAttackLockPressed()
{
	if (bHeroOneScopedUltActive)
	{
		if (!CanUseCombatMouseInput() || !bHeroOneScopeViewEnabled)
		{
			return;
		}
		TryFireHeroOneScopedUltShot();
		return;
	}

	if (!CanUseCombatMouseInput()) return;

	APawn* P = GetPawn();
	if (!P) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(P);
	if (!Hero || !Hero->CombatComponent) return;

	FVector2D CrosshairScreenPosition = FVector2D::ZeroVector;
	if (!GetAttackLockScreenPosition(CrosshairScreenPosition)) return;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AimLock), false);
	Params.AddIgnoredActor(Hero);
	const bool bHit = GetHitResultAtScreenPosition(CrosshairScreenPosition, ECC_Visibility, Params, Hit);

	AT66EnemyBase* Enemy = bHit ? Cast<AT66EnemyBase>(Hit.GetActor()) : nullptr;
	if (LockedEnemy.IsValid())
	{
		if (!Enemy || Enemy->CurrentHP <= 0)
		{
			SetLockedEnemy(nullptr, true);
			return;
		}

		if (LockedEnemy.Get() == Enemy)
		{
			SetLockedEnemy(nullptr, true);
			return;
		}

		SetLockedEnemy(Enemy, true);
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->NotifyTutorialAttackLockInput();
			}
		}
		return;
	}

	if (!Enemy || Enemy->CurrentHP <= 0)
	{
		// Clicking empty space does nothing when no lock is active.
		return;
	}

	SetLockedEnemy(Enemy, true);
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialAttackLockInput();
		}
	}
}


void AT66PlayerController::HandleAttackUnlockPressed()
{
	if (bHeroOneScopedUltActive)
	{
		if (!CanUseCombatMouseInput()) return;
		ToggleHeroOneScopeView();
		return;
	}

	// If we're standing on a loot bag, RMB discards it (so loot accept/reject works without showing cursor).
	if (AT66LootBagPickup* Bag = NearbyLootBag.Get())
	{
		if (APawn* P = GetPawn())
		{
			const float DistSq = FVector::DistSquared(P->GetActorLocation(), Bag->GetActorLocation());
			if (DistSq <= (250.f * 250.f))
			{
				Bag->ConsumeAndDestroy();
				NearbyLootBag.Reset();
				return;
			}
		}
	}

	APawn* P = GetPawn();
	AT66HeroBase* Hero = Cast<AT66HeroBase>(P);
	if (Hero && Hero->CombatComponent)
	{
		SetLockedEnemy(nullptr, true);
	}
	else
	{
		SetLockedEnemy(nullptr, false);
	}
}


void AT66PlayerController::HandleToggleMouseLockPressed()
{
	if (!IsGameplayLevel()) return;
	if (bHeroOneScopedUltActive) return;
	if (!CanUseCombatMouseInput()) return;

	if (bShowMouseCursor)
	{
		RestoreGameplayInputMode();
	}
	else
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->SetInteractive(bShowMouseCursor);
	}
}


void AT66PlayerController::SetupGameplayEnhancedInputMappings()
{
	UObject* Outer = GetTransientPackage();
	if (!IA_AttackLock)
	{
		IA_AttackLock = NewObject<UInputAction>(Outer, FName(TEXT("IA_AttackLock_T66")));
		if (IA_AttackLock) IA_AttackLock->ValueType = EInputActionValueType::Boolean;
	}
	if (!IA_AttackUnlock)
	{
		IA_AttackUnlock = NewObject<UInputAction>(Outer, FName(TEXT("IA_AttackUnlock_T66")));
		if (IA_AttackUnlock) IA_AttackUnlock->ValueType = EInputActionValueType::Boolean;
	}
	if (!IA_ToggleMouseLock)
	{
		IA_ToggleMouseLock = NewObject<UInputAction>(Outer, FName(TEXT("IA_ToggleMouseLock_T66")));
		if (IA_ToggleMouseLock) IA_ToggleMouseLock->ValueType = EInputActionValueType::Boolean;
	}

	RefreshGameplayMouseMappings();
}


void AT66PlayerController::HandleInteractPressed()
{
	if (!IsGameplayLevel()) return;
	if (GameplayHUDWidget && GameplayHUDWidget->TrySkipActivePresentation())
	{
		return;
	}

	// If we're in an in-world NPC dialogue, Interact confirms the selection.
	if (bWorldDialogueOpen)
	{
		ConfirmWorldDialogue();
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const float InteractRadius = 450.f;
	FVector Loc = ControlledPawn->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledPawn);
	World->OverlapMultiByChannel(Overlaps, Loc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(InteractRadius), Params);

	auto ComputeInteractionDistSq = [&Loc](AActor* Actor) -> float
	{
		if (!Actor)
		{
			return FLT_MAX;
		}

		const UPrimitiveComponent* InteractionPrimitive = nullptr;
		if (const AT66WorldInteractableBase* WorldInteractable = Cast<AT66WorldInteractableBase>(Actor))
		{
			InteractionPrimitive = WorldInteractable->TriggerBox.Get();
		}
		else
		{
			InteractionPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
		}

		if (InteractionPrimitive)
		{
			float CollisionDistSq = 0.f;
			FVector ClosestPoint = FVector::ZeroVector;
			if (InteractionPrimitive->GetSquaredDistanceToCollision(Loc, CollisionDistSq, ClosestPoint))
			{
				return CollisionDistSq;
			}
		}

		return FVector::DistSquared(Loc, Actor->GetActorLocation());
	};

	AT66StageGate* ClosestStageGate = nullptr;
	AT66CowardiceGate* ClosestCowardiceGate = nullptr;
	AT66DifficultyTotem* ClosestTotem = nullptr;
	AT66HouseNPCBase* ClosestNPC = nullptr;
	AT66RecruitableCompanion* ClosestRecruitableCompanion = nullptr;
	AT66ItemPickup* ClosestPickup = nullptr;
	AT66LootBagPickup* ClosestLootBag = nullptr;
	AT66TutorialPortal* ClosestTutorialPortal = nullptr;
	AT66IdolAltar* ClosestIdolAltar = nullptr;
	AT66PilotableTractor* ClosestTractor = nullptr;
	AT66FountainOfLifeInteractable* ClosestFountain = nullptr;
	AT66ChestInteractable* ClosestChest = nullptr;
	AT66WheelSpinInteractable* ClosestWheel = nullptr;
	AT66CrateInteractable* ClosestCrate = nullptr;
	AT66StageCatchUpGate* ClosestCatchUpGate = nullptr;
	AT66StageCatchUpGoldInteractable* ClosestCatchUpGold = nullptr;
	AT66StageCatchUpLootInteractable* ClosestCatchUpLoot = nullptr;
	AT66WorldInteractableBase* ClosestWorldInteractable = nullptr;
	float ClosestStageGateDistSq = InteractRadius * InteractRadius;
	float ClosestCowardiceGateDistSq = InteractRadius * InteractRadius;
	float ClosestTotemDistSq = InteractRadius * InteractRadius;
	float ClosestNPCDistSq = InteractRadius * InteractRadius;
	float ClosestRecruitableCompanionDistSq = InteractRadius * InteractRadius;
	float ClosestPickupDistSq = InteractRadius * InteractRadius;
	float ClosestLootBagDistSq = InteractRadius * InteractRadius;
	float ClosestTutorialPortalDistSq = InteractRadius * InteractRadius;
	float ClosestIdolAltarDistSq = InteractRadius * InteractRadius;
	float ClosestTractorDistSq = InteractRadius * InteractRadius;
	float ClosestFountainDistSq = InteractRadius * InteractRadius;
	float ClosestChestDistSq = InteractRadius * InteractRadius;
	float ClosestWheelDistSq = InteractRadius * InteractRadius;
	float ClosestCrateDistSq = InteractRadius * InteractRadius;
	float ClosestCatchUpGateDistSq = InteractRadius * InteractRadius;
	float ClosestCatchUpGoldDistSq = InteractRadius * InteractRadius;
	float ClosestCatchUpLootDistSq = InteractRadius * InteractRadius;
	float ClosestWorldInteractableDistSq = InteractRadius * InteractRadius;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* A = R.GetActor();
		if (!A) continue;
		const float DistSq = ComputeInteractionDistSq(A);
		if (AT66StageGate* G = Cast<AT66StageGate>(A))
		{
			if (DistSq < ClosestStageGateDistSq) { ClosestStageGateDistSq = DistSq; ClosestStageGate = G; }
		}
		else if (AT66CowardiceGate* CG = Cast<AT66CowardiceGate>(A))
		{
			if (DistSq < ClosestCowardiceGateDistSq) { ClosestCowardiceGateDistSq = DistSq; ClosestCowardiceGate = CG; }
		}
		else if (AT66DifficultyTotem* DT = Cast<AT66DifficultyTotem>(A))
		{
			if (DistSq < ClosestTotemDistSq) { ClosestTotemDistSq = DistSq; ClosestTotem = DT; }
		}
		else if (AT66HouseNPCBase* N = Cast<AT66HouseNPCBase>(A))
		{
			if (DistSq < ClosestNPCDistSq) { ClosestNPCDistSq = DistSq; ClosestNPC = N; }
		}
		else if (AT66RecruitableCompanion* RC = Cast<AT66RecruitableCompanion>(A))
		{
			if (DistSq < ClosestRecruitableCompanionDistSq) { ClosestRecruitableCompanionDistSq = DistSq; ClosestRecruitableCompanion = RC; }
		}
		else if (AT66ItemPickup* P = Cast<AT66ItemPickup>(A))
		{
			if (DistSq < ClosestPickupDistSq) { ClosestPickupDistSq = DistSq; ClosestPickup = P; }
		}
		else if (AT66LootBagPickup* Bag = Cast<AT66LootBagPickup>(A))
		{
			if (DistSq < ClosestLootBagDistSq) { ClosestLootBagDistSq = DistSq; ClosestLootBag = Bag; }
		}
		else if (AT66TutorialPortal* Portal = Cast<AT66TutorialPortal>(A))
		{
			if (DistSq < ClosestTutorialPortalDistSq) { ClosestTutorialPortalDistSq = DistSq; ClosestTutorialPortal = Portal; }
		}
		else if (AT66IdolAltar* Altar = Cast<AT66IdolAltar>(A))
		{
			if (DistSq < ClosestIdolAltarDistSq) { ClosestIdolAltarDistSq = DistSq; ClosestIdolAltar = Altar; }
		}
		else if (AT66PilotableTractor* Tractor = Cast<AT66PilotableTractor>(A))
		{
			if (DistSq < ClosestTractorDistSq) { ClosestTractorDistSq = DistSq; ClosestTractor = Tractor; }
		}
		else if (AT66FountainOfLifeInteractable* Fountain = Cast<AT66FountainOfLifeInteractable>(A))
		{
			if (DistSq < ClosestFountainDistSq) { ClosestFountainDistSq = DistSq; ClosestFountain = Fountain; }
		}
		else if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(A))
		{
			if (DistSq < ClosestChestDistSq) { ClosestChestDistSq = DistSq; ClosestChest = Chest; }
		}
		else if (AT66WheelSpinInteractable* W = Cast<AT66WheelSpinInteractable>(A))
		{
			if (DistSq < ClosestWheelDistSq) { ClosestWheelDistSq = DistSq; ClosestWheel = W; }
		}
		else if (AT66CrateInteractable* CR = Cast<AT66CrateInteractable>(A))
		{
			if (DistSq < ClosestCrateDistSq) { ClosestCrateDistSq = DistSq; ClosestCrate = CR; }
		}
		else if (AT66StageCatchUpGate* CUG = Cast<AT66StageCatchUpGate>(A))
		{
			if (DistSq < ClosestCatchUpGateDistSq) { ClosestCatchUpGateDistSq = DistSq; ClosestCatchUpGate = CUG; }
		}
		else if (AT66StageCatchUpGoldInteractable* CUGold = Cast<AT66StageCatchUpGoldInteractable>(A))
		{
			if (DistSq < ClosestCatchUpGoldDistSq) { ClosestCatchUpGoldDistSq = DistSq; ClosestCatchUpGold = CUGold; }
		}
		else if (AT66StageCatchUpLootInteractable* L = Cast<AT66StageCatchUpLootInteractable>(A))
		{
			if (DistSq < ClosestCatchUpLootDistSq) { ClosestCatchUpLootDistSq = DistSq; ClosestCatchUpLoot = L; }
		}
		else if (AT66WorldInteractableBase* WorldInteractable = Cast<AT66WorldInteractableBase>(A))
		{
			if (DistSq < ClosestWorldInteractableDistSq) { ClosestWorldInteractableDistSq = DistSq; ClosestWorldInteractable = WorldInteractable; }
		}
	}

	AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);

	if (ClosestTractor && ClosestTractor->IsMountedByHero(HeroPawn) && ClosestTractor->Interact(this))
	{
		return;
	}

	// Recruitable companion should take priority over stage gate so you can recruit right after boss death.
	if (ClosestRecruitableCompanion && ClosestRecruitableCompanion->Interact(this))
	{
		return;
	}

	// Interact with Stage Gate (F) advances to next stage and reloads level
	if (ClosestStageGate && ClosestStageGate->AdvanceToNextStage())
	{
		return;
	}

	// Tutorial portal (teleport within the same map; no load)
	if (ClosestTutorialPortal && ClosestTutorialPortal->Interact(this))
	{
		return;
	}

	// Stage Catch Up gate (F) enters the chosen difficulty start stage
	if (ClosestCatchUpGate && ClosestCatchUpGate->EnterChosenStage())
	{
		return;
	}

	// Difficulty totem (F) increases difficulty tier
	if (ClosestTotem && ClosestTotem->Interact(this))
	{
		return;
	}

	// Cowardice Gate (F) opens yes/no prompt
	if (ClosestCowardiceGate && ClosestCowardiceGate->Interact(this))
	{
		return;
	}

	// NPCs (Vendor/Gambler/Saint/Ouroboros)
	if (ClosestNPC && ClosestNPC->Interact(this))
	{
		return;
	}
	// Idol Altar (Stage 1)
	if (ClosestIdolAltar)
	{
		UT66IdolAltarOverlayWidget* W = CreateWidget<UT66IdolAltarOverlayWidget>(this, ResolveIdolAltarOverlayClass());
		if (W)
		{
			W->SetSourceAltar(ClosestIdolAltar);
			IdolAltarOverlayWidget = W;
			W->AddToViewport(150); // above gambler overlay
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}
	// World interactables (Tractor/Fountain/Chest/Wheel)
	if (ClosestTractor && ClosestTractor->Interact(this))
	{
		return;
	}
	if (ClosestFountain && ClosestFountain->Interact(this))
	{
		return;
	}
	if (ClosestChest && ClosestChest->Interact(this))
	{
		return;
	}
	if (ClosestWheel && ClosestWheel->Interact(this))
	{
		return;
	}
	if (ClosestCrate && ClosestCrate->Interact(this))
	{
		return;
	}

	// Stage Catch Up interactables (Gold/Loot)
	if (ClosestCatchUpGold && ClosestCatchUpGold->Interact(this))
	{
		return;
	}
	if (ClosestCatchUpLoot && ClosestCatchUpLoot->Interact(this))
	{
		return;
	}
	if (ClosestWorldInteractable && ClosestWorldInteractable->Interact(this))
	{
		return;
	}
	if (ClosestLootBag)
	{
		UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			const FName PickedItemID = ClosestLootBag->GetItemID();
			const ET66ItemRarity PickedItemRarity = LootRarityToItemRarity(ClosestLootBag->GetLootRarity());
			if (T66_IsGamblersTokenItem(PickedItemID))
			{
				const int32 TokenLevel = ClosestLootBag->HasExplicitLine1RolledValue() ? ClosestLootBag->GetExplicitLine1RolledValue() : 1;
				RunState->ApplyGamblersTokenPickup(TokenLevel);
				ClosestLootBag->ConsumeAndDestroy();
				ClearNearbyLootBag(ClosestLootBag);
				if (GameplayHUDWidget)
				{
					GameplayHUDWidget->ShowPickupItemCard(PickedItemID, PickedItemRarity);
				}
			}
			else if (RunState->HasInventorySpace())
			{
				RunState->AddItemWithRarity(PickedItemID, PickedItemRarity);
				ClosestLootBag->ConsumeAndDestroy();
				ClearNearbyLootBag(ClosestLootBag);
				if (GameplayHUDWidget)
				{
					GameplayHUDWidget->ShowPickupItemCard(PickedItemID, PickedItemRarity);
				}
			}
		}
		return;
	}
	// Backward-compat: if any old pickups exist, collect instantly (no popup).
	if (ClosestPickup)
	{
		if (UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			if (RunState->HasInventorySpace())
			{
				RunState->AddItem(ClosestPickup->GetItemID());
				ClosestPickup->Destroy();
			}
			return;
		}
	}
}
