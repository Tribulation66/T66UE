// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66CombatHitZoneComponent.h"
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
#include "UI/T66CowardicePromptWidget.h"
#include "UI/T66IdolAltarOverlayWidget.h"
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
#include "Core/T66AudioSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66BuffSubsystem.h"
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
		// Keep the attack reticle slightly above center so it lands on enemies in the forward playfield
		// without drifting so high that manual lock-on feels disconnected from the visible HUD reticle.
		return FMath::Clamp(-0.12f * static_cast<float>(ViewportHeight), -160.f, -96.f);
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

	static bool T66UsesMouseTriggeredUltimate(const FHeroData& HeroData)
	{
		return HeroData.UltimateType == ET66UltimateType::SpearStorm;
	}

	static FName T66GetUltimateAudioEventID(const ET66UltimateType UltimateType)
	{
		if (UltimateType == ET66UltimateType::None)
		{
			return NAME_None;
		}

		const UEnum* Enum = StaticEnum<ET66UltimateType>();
		const FString UltimateName = Enum
			? Enum->GetNameStringByValue(static_cast<int64>(UltimateType))
			: FString();
		return UltimateName.IsEmpty()
			? NAME_None
			: FName(*FString::Printf(TEXT("Hero.Ultimate.%s"), *UltimateName));
	}

	static bool T66PlayUltimateAudio(AT66PlayerController* PlayerController, const ET66UltimateType UltimateType, AActor* Hero)
	{
		if (!PlayerController)
		{
			return false;
		}

		const FVector Location = Hero
			? Hero->GetActorLocation()
			: (PlayerController->GetPawn() ? PlayerController->GetPawn()->GetActorLocation() : FVector::ZeroVector);

		const FName UltimateEventID = T66GetUltimateAudioEventID(UltimateType);
		if (!UltimateEventID.IsNone()
			&& UT66AudioSubsystem::PlayEventFromWorldContext(PlayerController, UltimateEventID, Location, Hero))
		{
			return true;
		}

		return UT66AudioSubsystem::PlayEventFromWorldContext(PlayerController, FName(TEXT("Combat.Ultimate.Cast")), Location, Hero);
	}

	static bool T66ProjectTargetHandleToScreen(
		AT66PlayerController* PlayerController,
		const FT66CombatTargetHandle& TargetHandle,
		FVector2D& OutScreenPosition)
	{
		if (!PlayerController || !TargetHandle.Actor.IsValid())
		{
			return false;
		}

		const FVector AimPoint = TargetHandle.AimPoint.IsNearlyZero()
			? TargetHandle.Actor->GetActorLocation()
			: TargetHandle.AimPoint;
		return PlayerController->ProjectWorldLocationToScreen(AimPoint, OutScreenPosition, true);
	}

	static void T66TryUpdateBestFallbackTarget(
		AT66PlayerController* PlayerController,
		const FT66CombatTargetHandle& CandidateHandle,
		const FVector2D& AimScreenPosition,
		const float MaxScreenDistanceSq,
		AActor*& InOutBestActor,
		UT66CombatHitZoneComponent*& InOutBestZoneComponent,
		ET66HitZoneType& InOutBestZoneType,
		float& InOutBestScoreSq)
	{
		if (!CandidateHandle.Actor.IsValid())
		{
			return;
		}

		FVector2D CandidateScreenPosition = FVector2D::ZeroVector;
		if (!T66ProjectTargetHandleToScreen(PlayerController, CandidateHandle, CandidateScreenPosition))
		{
			return;
		}

		const float ScreenDistanceSq = FVector2D::DistSquared(CandidateScreenPosition, AimScreenPosition);
		if (ScreenDistanceSq > MaxScreenDistanceSq || ScreenDistanceSq >= InOutBestScoreSq)
		{
			return;
		}

		InOutBestActor = CandidateHandle.Actor.Get();
		InOutBestZoneComponent = Cast<UT66CombatHitZoneComponent>(CandidateHandle.HitComponent.Get());
		InOutBestZoneType = (CandidateHandle.HitZoneType == ET66HitZoneType::None)
			? ET66HitZoneType::Body
			: CandidateHandle.HitZoneType;
		InOutBestScoreSq = ScreenDistanceSq;
	}

	static bool T66TryFindFallbackAttackLockTarget(
		AT66PlayerController* PlayerController,
		const FVector2D& AimScreenPosition,
		AActor*& OutActor,
		UT66CombatHitZoneComponent*& OutZoneComponent,
		ET66HitZoneType& OutZoneType)
	{
		OutActor = nullptr;
		OutZoneComponent = nullptr;
		OutZoneType = ET66HitZoneType::None;

		if (!PlayerController)
		{
			return false;
		}

		UWorld* World = PlayerController->GetWorld();
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		if (!Registry)
		{
			return false;
		}

		const float MaxScreenDistanceSq = FMath::Square(110.f);
		float BestScoreSq = TNumericLimits<float>::Max();

		static const ET66HitZoneType EnemyZonePreferences[] =
		{
			ET66HitZoneType::Head,
			ET66HitZoneType::Body,
		};

		for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
		{
			AT66EnemyBase* Enemy = WeakEnemy.Get();
			if (!Enemy || Enemy->CurrentHP <= 0)
			{
				continue;
			}

			for (const ET66HitZoneType CandidateZone : EnemyZonePreferences)
			{
				T66TryUpdateBestFallbackTarget(
					PlayerController,
					Enemy->ResolveCombatTargetHandle(nullptr, CandidateZone),
					AimScreenPosition,
					MaxScreenDistanceSq,
					OutActor,
					OutZoneComponent,
					OutZoneType,
					BestScoreSq);
			}
		}

		static const ET66HitZoneType BossZonePreferences[] =
		{
			ET66HitZoneType::Head,
			ET66HitZoneType::WeakPoint,
			ET66HitZoneType::Core,
			ET66HitZoneType::LeftArm,
			ET66HitZoneType::RightArm,
			ET66HitZoneType::LeftLeg,
			ET66HitZoneType::RightLeg,
			ET66HitZoneType::Body,
		};

		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			AT66BossBase* Boss = WeakBoss.Get();
			if (!Boss || !Boss->IsAwakened() || !Boss->IsAlive())
			{
				continue;
			}

			for (const ET66HitZoneType CandidateZone : BossZonePreferences)
			{
				T66TryUpdateBestFallbackTarget(
					PlayerController,
					Boss->ResolveCombatTargetHandle(nullptr, CandidateZone),
					AimScreenPosition,
					MaxScreenDistanceSq,
					OutActor,
					OutZoneComponent,
					OutZoneType,
					BestScoreSq);
			}
		}

		return OutActor != nullptr;
	}

	static bool T66BuildArthurUltimatePath(
		AT66PlayerController* PlayerController,
		AT66HeroBase* Hero,
		const float MaxRange,
		FVector& OutStart,
		FVector& OutEnd)
	{
		if (!PlayerController || !Hero || MaxRange <= KINDA_SMALL_NUMBER)
		{
			return false;
		}

		UWorld* World = PlayerController->GetWorld();
		if (!World)
		{
			return false;
		}

		FVector2D CrosshairScreenPosition = FVector2D::ZeroVector;
		if (!PlayerController->GetAttackLockScreenPosition(CrosshairScreenPosition))
		{
			return false;
		}

		FVector AimWorldOrigin = FVector::ZeroVector;
		FVector AimWorldDirection = FVector::ForwardVector;
		if (!PlayerController->DeprojectScreenPositionToWorld(CrosshairScreenPosition.X, CrosshairScreenPosition.Y, AimWorldOrigin, AimWorldDirection)
			|| AimWorldDirection.IsNearlyZero())
		{
			AimWorldOrigin = PlayerController->PlayerCameraManager
				? PlayerController->PlayerCameraManager->GetCameraLocation()
				: Hero->GetActorLocation();
			AimWorldDirection = PlayerController->PlayerCameraManager
				? PlayerController->PlayerCameraManager->GetCameraRotation().Vector()
				: Hero->GetActorForwardVector();
		}

		AimWorldDirection = AimWorldDirection.GetSafeNormal();
		if (AimWorldDirection.IsNearlyZero())
		{
			AimWorldDirection = Hero->GetActorForwardVector().GetSafeNormal();
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(ArthurUltimateAimTrace), false);
		Params.AddIgnoredActor(Hero);

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

		const FVector CameraMaxEnd = AimWorldOrigin + (AimWorldDirection * MaxRange);
		FHitResult CameraHit;
		const FVector AimTarget = World->LineTraceSingleByObjectType(CameraHit, AimWorldOrigin, CameraMaxEnd, ObjectParams, Params)
			? CameraHit.ImpactPoint
			: CameraMaxEnd;

		const FVector BaseStart = Hero->GetActorLocation() + (Hero->GetActorUpVector() * 80.f);
		FVector SwordDirection = (AimTarget - BaseStart).GetSafeNormal();
		if (SwordDirection.IsNearlyZero())
		{
			SwordDirection = AimWorldDirection;
		}
		if (SwordDirection.IsNearlyZero())
		{
			SwordDirection = Hero->GetActorForwardVector().GetSafeNormal();
		}

		OutStart = BaseStart + (SwordDirection * 90.f);
		const FVector CandidateEnd = OutStart + (SwordDirection * MaxRange);

		FHitResult SwordHit;
		OutEnd = World->LineTraceSingleByObjectType(SwordHit, OutStart, CandidateEnd, ObjectParams, Params)
			? SwordHit.ImpactPoint
			: CandidateEnd;

		if (FVector::DistSquared(OutStart, OutEnd) <= KINDA_SMALL_NUMBER)
		{
			OutEnd = OutStart + (SwordDirection * MaxRange);
		}

		return true;
	}
}

void AT66PlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	SyncLockedCombatTargetFromCombat();
}

bool AT66PlayerController::HasAttackLockedEnemy() const
{
	const APawn* ControlledPawn = GetPawn();
	const AT66HeroBase* Hero = Cast<AT66HeroBase>(ControlledPawn);
	const UT66CombatComponent* Combat = Hero ? Hero->CombatComponent : nullptr;
	AActor* LockedTargetActor = Combat ? Combat->GetLockedTarget() : nullptr;
	if (const AT66EnemyBase* CombatLockedEnemy = Cast<AT66EnemyBase>(LockedTargetActor))
	{
		return CombatLockedEnemy->CurrentHP > 0;
	}
	if (const AT66BossBase* CombatLockedBoss = Cast<AT66BossBase>(LockedTargetActor))
	{
		return CombatLockedBoss->IsAwakened() && CombatLockedBoss->IsAlive();
	}
	return false;
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

void AT66PlayerController::SetLockedCombatTarget(AActor* NewLockedActor, const bool bPropagateToCombatTarget, const ET66HitZoneType InLockedHitZoneType, UT66CombatHitZoneComponent* LockedZoneComponent)
{
	if (AT66EnemyBase* NewLockedEnemy = Cast<AT66EnemyBase>(NewLockedActor))
	{
		if (NewLockedEnemy->CurrentHP <= 0)
		{
			NewLockedActor = nullptr;
		}
	}
	else if (AT66BossBase* NewLockedBoss = Cast<AT66BossBase>(NewLockedActor))
	{
		if (!NewLockedBoss->IsAwakened() || !NewLockedBoss->IsAlive())
		{
			NewLockedActor = nullptr;
		}
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	UT66CombatComponent* Combat = Hero ? Hero->CombatComponent : nullptr;

	const ET66HitZoneType SanitizedHitZoneType = (InLockedHitZoneType == ET66HitZoneType::None) ? ET66HitZoneType::Body : InLockedHitZoneType;
	auto BuildTargetHandle = [SanitizedHitZoneType, LockedZoneComponent](AActor* TargetActor) -> FT66CombatTargetHandle
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(TargetActor))
		{
			return Enemy->ResolveCombatTargetHandle(LockedZoneComponent, SanitizedHitZoneType);
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(TargetActor))
		{
			return Boss->ResolveCombatTargetHandle(LockedZoneComponent, SanitizedHitZoneType);
		}

		FT66CombatTargetHandle Handle;
		Handle.Actor = TargetActor;
		Handle.HitComponent = LockedZoneComponent;
		Handle.HitZoneType = SanitizedHitZoneType;
		Handle.HitZoneName = LockedZoneComponent ? LockedZoneComponent->HitZoneName : NAME_None;
		Handle.AimPoint = TargetActor ? TargetActor->GetActorLocation() : FVector::ZeroVector;
		return Handle;
	};

	if (LockedCombatActor.Get() == NewLockedActor
		&& LockedCombatHitZoneType == SanitizedHitZoneType
		&& LockedCombatZoneComponent.Get() == LockedZoneComponent)
	{
		if (bPropagateToCombatTarget && Combat)
		{
			if (NewLockedActor)
			{
				Combat->SetLockedTarget(BuildTargetHandle(NewLockedActor));
			}
			else
			{
				Combat->ClearLockedTarget();
			}
		}
		return;
	}

	if (AT66EnemyBase* PreviousLockedEnemy = Cast<AT66EnemyBase>(LockedCombatActor.Get()))
	{
		PreviousLockedEnemy->SetLockedIndicator(false);
	}

	LockedCombatActor = NewLockedActor;
	LockedCombatHitZoneType = SanitizedHitZoneType;
	LockedCombatZoneComponent = LockedZoneComponent;

	if (AT66EnemyBase* NextLockedEnemy = Cast<AT66EnemyBase>(LockedCombatActor.Get()))
	{
		NextLockedEnemy->SetLockedIndicator(true);
	}

	if (bPropagateToCombatTarget && Combat)
	{
		if (NewLockedActor)
		{
			Combat->SetLockedTarget(BuildTargetHandle(NewLockedActor));
		}
		else
		{
			Combat->ClearLockedTarget();
		}
	}
}

void AT66PlayerController::SyncLockedCombatTargetFromCombat()
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	UT66CombatComponent* Combat = Hero ? Hero->CombatComponent : nullptr;
	const FT66CombatTargetHandle CombatTargetHandle = Combat ? Combat->GetLockedTargetHandle() : FT66CombatTargetHandle{};
	AActor* CombatLockedActor = CombatTargetHandle.Actor.Get();
	if (AT66EnemyBase* CombatLockedEnemy = Cast<AT66EnemyBase>(CombatLockedActor))
	{
		if (CombatLockedEnemy->CurrentHP <= 0)
		{
			CombatLockedActor = nullptr;
		}
	}
	else if (AT66BossBase* CombatLockedBoss = Cast<AT66BossBase>(CombatLockedActor))
	{
		if (!CombatLockedBoss->IsAwakened() || !CombatLockedBoss->IsAlive())
		{
			CombatLockedActor = nullptr;
		}
	}

	SetLockedCombatTarget(
		CombatLockedActor,
		false,
		CombatTargetHandle.HitZoneType,
		Cast<UT66CombatHitZoneComponent>(CombatTargetHandle.HitComponent.Get()));
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

bool AT66PlayerController::TryHandleMouseTriggeredUltimate()
{
	if (bHeroOneScopedUltActive || !CanUseCombatMouseInput())
	{
		return false;
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	FHeroData HeroData;
	if (!T66GI || !T66GI->GetSelectedHeroData(HeroData))
	{
		return false;
	}

	ET66UltimateType DesiredUltimateType = HeroData.UltimateType;
	if (const UT66CommunityContentSubsystem* Community = T66GI->GetSubsystem<UT66CommunityContentSubsystem>())
	{
		const ET66UltimateType OverrideType = Community->GetActiveUltimateOverride();
		if (OverrideType != ET66UltimateType::None)
		{
			DesiredUltimateType = OverrideType;
		}
	}

	if (DesiredUltimateType != ET66UltimateType::SpearStorm)
	{
		return false;
	}

	HandleUltimatePressed();
	return true;
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
	ET66UltimateType UltType = (T66GI && T66GI->GetSelectedHeroData(HeroData))
		? HeroData.UltimateType
		: ET66UltimateType::None;
	if (T66GI)
	{
		if (const UT66CommunityContentSubsystem* Community = T66GI->GetSubsystem<UT66CommunityContentSubsystem>())
		{
			const ET66UltimateType OverrideType = Community->GetActiveUltimateOverride();
			if (OverrideType != ET66UltimateType::None)
			{
				UltType = OverrideType;
			}
		}
	}

	const int32 UltDmg = UT66RunStateSubsystem::UltimateDamage;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	UT66CombatComponent* Combat = Hero ? Hero->FindComponentByClass<UT66CombatComponent>() : nullptr;
	T66PlayUltimateAudio(this, UltType, Hero);

	switch (UltType)
	{
	case ET66UltimateType::SpearStorm:
		if (Combat && Hero)
		{
			FVector SwordStart = FVector::ZeroVector;
			FVector SwordEnd = FVector::ZeroVector;
			if (!T66BuildArthurUltimatePath(this, Hero, 2400.f, SwordStart, SwordEnd))
			{
				const FVector FallbackDirection = Hero->GetActorForwardVector().GetSafeNormal();
				SwordStart = Hero->GetActorLocation() + (Hero->GetActorUpVector() * 80.f) + (FallbackDirection * 90.f);
				SwordEnd = SwordStart + (FallbackDirection * 2400.f);
			}

			Combat->PerformUltimateSpearStorm(UltDmg, SwordStart, SwordEnd);
		}
		else if (Combat)
		{
			const AActor* CombatOwner = Combat->GetOwner();
			const FVector FallbackDirection = CombatOwner ? CombatOwner->GetActorForwardVector().GetSafeNormal() : FVector::ForwardVector;
			const FVector FallbackStart = (CombatOwner ? CombatOwner->GetActorLocation() : FVector::ZeroVector) + FVector(0.f, 0.f, 80.f) + (FallbackDirection * 90.f);
			const FVector FallbackEnd = FallbackStart + (FallbackDirection * 2400.f);
			Combat->PerformUltimateSpearStorm(UltDmg, FallbackStart, FallbackEnd);
		}
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
	return IsGameplayLevel()
		&& !bInventoryInspectOpen
		&& !IsArcadePopupOpen()
		&& !(GameplayHUDWidget && GameplayHUDWidget->IsFullMapOpen())
		&& !IsCasinoOverlayOpen()
		&& !(CowardicePromptWidget && CowardicePromptWidget->IsInViewport())
		&& !(IdolAltarOverlayWidget && IdolAltarOverlayWidget->IsInViewport());
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

	AActor* TargetActor = bHit ? Hit.GetActor() : nullptr;
	UT66CombatHitZoneComponent* HitZoneComponent = bHit ? Cast<UT66CombatHitZoneComponent>(Hit.GetComponent()) : nullptr;
	if (!TargetActor && HitZoneComponent)
	{
		TargetActor = HitZoneComponent->GetOwner();
	}

	ET66HitZoneType HitZoneType = HitZoneComponent ? HitZoneComponent->HitZoneType : ET66HitZoneType::Body;
	auto IsLockableCombatTarget = [](AActor* CandidateActor) -> bool
	{
		if (const AT66EnemyBase* CandidateEnemy = Cast<AT66EnemyBase>(CandidateActor))
		{
			return CandidateEnemy->CurrentHP > 0;
		}
		if (const AT66BossBase* CandidateBoss = Cast<AT66BossBase>(CandidateActor))
		{
			return CandidateBoss->IsAwakened() && CandidateBoss->IsAlive();
		}
		return false;
	};

	if (!IsLockableCombatTarget(TargetActor))
	{
		AActor* FallbackTargetActor = nullptr;
		UT66CombatHitZoneComponent* FallbackZoneComponent = nullptr;
		ET66HitZoneType FallbackHitZoneType = ET66HitZoneType::None;
		if (T66TryFindFallbackAttackLockTarget(this, CrosshairScreenPosition, FallbackTargetActor, FallbackZoneComponent, FallbackHitZoneType))
		{
			TargetActor = FallbackTargetActor;
			HitZoneComponent = FallbackZoneComponent;
			HitZoneType = (FallbackHitZoneType == ET66HitZoneType::None) ? ET66HitZoneType::Body : FallbackHitZoneType;
		}
	}

	if (LockedCombatActor.IsValid())
	{
		if (!IsLockableCombatTarget(TargetActor))
		{
			SetLockedCombatTarget(nullptr, true);
			return;
		}

		if (LockedCombatActor.Get() == TargetActor)
		{
			const bool bSameZone = LockedCombatHitZoneType == HitZoneType
				&& LockedCombatZoneComponent.Get() == HitZoneComponent;
			if (bSameZone)
			{
				SetLockedCombatTarget(nullptr, true);
				return;
			}
		}

		SetLockedCombatTarget(TargetActor, true, HitZoneType, HitZoneComponent);
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->NotifyTutorialAttackLockInput();
			}
		}
		return;
	}

	if (!IsLockableCombatTarget(TargetActor))
	{
		// Clicking empty space does nothing when no lock is active.
		return;
	}

	SetLockedCombatTarget(TargetActor, true, HitZoneType, HitZoneComponent);
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

	if (TryHandleMouseTriggeredUltimate())
	{
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
				UT66AudioSubsystem::PlayEventAtActorFromWorldContext(this, FName(TEXT("Pickup.Discard")), Bag);
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
		SetLockedCombatTarget(nullptr, true);
	}
	else
	{
		SetLockedCombatTarget(nullptr, false);
	}
}


void AT66PlayerController::HandleToggleMouseLockPressed()
{
	if (!IsGameplayLevel()) return;
	if (bHeroOneScopedUltActive) return;
	if (TryHandleMouseTriggeredUltimate()) return;
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
	if (IsArcadePopupOpen())
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
		else if (const AT66IdolAltar* IdolAltar = Cast<AT66IdolAltar>(Actor))
		{
			InteractionPrimitive = IdolAltar->InteractTrigger.Get();
		}
		else if (const AT66LootBagPickup* LootBag = Cast<AT66LootBagPickup>(Actor))
		{
			InteractionPrimitive = LootBag->SphereComponent.Get();
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

	auto ComputeActorDistSq = [&Loc](AActor* Actor) -> float
	{
		return Actor ? FVector::DistSquared(Loc, Actor->GetActorLocation()) : FLT_MAX;
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
	auto PlayInteractAudio = [this, HeroPawn](const FName EventID, AActor* SourceActor)
	{
		UT66AudioSubsystem::PlayEventAtActorFromWorldContext(this, EventID, SourceActor ? SourceActor : HeroPawn);
	};

	if (ClosestTractor && ClosestTractor->IsMountedByHero(HeroPawn) && ClosestTractor->Interact(this))
	{
		PlayInteractAudio(FName(TEXT("Interact.Generic")), ClosestTractor);
		return;
	}

	// Recruitable companion should take priority over stage gate so you can recruit right after boss death.
	if (ClosestRecruitableCompanion && ClosestRecruitableCompanion->Interact(this))
	{
		PlayInteractAudio(FName(TEXT("Interact.Generic")), ClosestRecruitableCompanion);
		return;
	}

	// Interact with Stage Gate (F) advances to next stage and reloads level
	if (ClosestStageGate && ClosestStageGate->AdvanceToNextStage())
	{
		PlayInteractAudio(FName(TEXT("Interact.Generic")), ClosestStageGate);
		return;
	}

	// Tutorial portal (teleport within the same map; no load)
	if (ClosestTutorialPortal && ClosestTutorialPortal->Interact(this))
	{
		PlayInteractAudio(FName(TEXT("Interact.Generic")), ClosestTutorialPortal);
		return;
	}

	// Stage Catch Up gate (F) enters the chosen difficulty start stage
	if (ClosestCatchUpGate && ClosestCatchUpGate->EnterChosenStage())
	{
		PlayInteractAudio(FName(TEXT("Interact.Generic")), ClosestCatchUpGate);
		return;
	}

	// Difficulty totem (F) increases difficulty tier
	if (ClosestTotem && ClosestTotem->Interact(this))
	{
		PlayInteractAudio(FName(TEXT("Interact.Totem.Activate")), ClosestTotem);
		return;
	}

	// Cowardice Gate (F) opens yes/no prompt
	if (ClosestCowardiceGate && ClosestCowardiceGate->Interact(this))
	{
		PlayInteractAudio(FName(TEXT("Interact.Generic")), ClosestCowardiceGate);
		return;
	}

	// NPCs (Vendor/Gambler/Saint/Ouroboros)
	if (ClosestNPC && ClosestNPC->Interact(this))
	{
		PlayInteractAudio(FName(TEXT("Interact.Generic")), ClosestNPC);
		return;
	}

	enum class ESecondaryInteractPriority : uint8
	{
		LootBag = 0,
		SpecificWorld = 1,
		GenericWorld = 2,
		IdolAltar = 3,
	};

	struct FSecondaryInteractCandidate
	{
		AActor* Actor = nullptr;
		float CollisionDistSq = FLT_MAX;
		float ActorDistSq = FLT_MAX;
		ESecondaryInteractPriority Priority = ESecondaryInteractPriority::GenericWorld;
	};

	auto ConsiderSecondaryCandidate = [](FSecondaryInteractCandidate& Current, AActor* Candidate, const float CollisionDistSq, const float ActorDistSq, const ESecondaryInteractPriority Priority)
	{
		if (!Candidate)
		{
			return;
		}

		const bool bNoCurrent = (Current.Actor == nullptr);
		const bool bCloserCollision = CollisionDistSq < (Current.CollisionDistSq - 1.f);
		const bool bEqualCollision = FMath::Abs(CollisionDistSq - Current.CollisionDistSq) <= 1.f;
		const bool bCloserActor = ActorDistSq < (Current.ActorDistSq - 1.f);
		const bool bEqualActor = FMath::Abs(ActorDistSq - Current.ActorDistSq) <= 1.f;
		const bool bHigherPriority = static_cast<uint8>(Priority) < static_cast<uint8>(Current.Priority);
		if (bNoCurrent
			|| bCloserCollision
			|| (bEqualCollision && (bCloserActor || (bEqualActor && bHigherPriority))))
		{
			Current.Actor = Candidate;
			Current.CollisionDistSq = CollisionDistSq;
			Current.ActorDistSq = ActorDistSq;
			Current.Priority = Priority;
		}
	};

	FSecondaryInteractCandidate SecondaryInteract;
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestLootBag, ClosestLootBagDistSq, ComputeActorDistSq(ClosestLootBag), ESecondaryInteractPriority::LootBag);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestTractor, ClosestTractorDistSq, ComputeActorDistSq(ClosestTractor), ESecondaryInteractPriority::SpecificWorld);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestFountain, ClosestFountainDistSq, ComputeActorDistSq(ClosestFountain), ESecondaryInteractPriority::SpecificWorld);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestChest, ClosestChestDistSq, ComputeActorDistSq(ClosestChest), ESecondaryInteractPriority::SpecificWorld);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestWheel, ClosestWheelDistSq, ComputeActorDistSq(ClosestWheel), ESecondaryInteractPriority::SpecificWorld);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestCrate, ClosestCrateDistSq, ComputeActorDistSq(ClosestCrate), ESecondaryInteractPriority::SpecificWorld);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestCatchUpGold, ClosestCatchUpGoldDistSq, ComputeActorDistSq(ClosestCatchUpGold), ESecondaryInteractPriority::SpecificWorld);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestCatchUpLoot, ClosestCatchUpLootDistSq, ComputeActorDistSq(ClosestCatchUpLoot), ESecondaryInteractPriority::SpecificWorld);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestWorldInteractable, ClosestWorldInteractableDistSq, ComputeActorDistSq(ClosestWorldInteractable), ESecondaryInteractPriority::GenericWorld);
	ConsiderSecondaryCandidate(SecondaryInteract, ClosestIdolAltar, ClosestIdolAltarDistSq, ComputeActorDistSq(ClosestIdolAltar), ESecondaryInteractPriority::IdolAltar);
	if (AT66LootBagPickup* NearbyBag = NearbyLootBag.Get())
	{
		ConsiderSecondaryCandidate(SecondaryInteract, NearbyBag, ComputeInteractionDistSq(NearbyBag), ComputeActorDistSq(NearbyBag), ESecondaryInteractPriority::LootBag);
	}

	if (AT66LootBagPickup* SelectedLootBag = Cast<AT66LootBagPickup>(SecondaryInteract.Actor))
	{
		UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			const FName PickedItemID = SelectedLootBag->GetItemID();
			const ET66ItemRarity PickedItemRarity = LootRarityToItemRarity(SelectedLootBag->GetLootRarity());
			if (T66_IsGamblersTokenItem(PickedItemID))
			{
				const int32 TokenLevel = SelectedLootBag->HasExplicitLine1RolledValue() ? SelectedLootBag->GetExplicitLine1RolledValue() : 1;
				RunState->ApplyGamblersTokenPickup(TokenLevel);
				PlayInteractAudio(FName(TEXT("Pickup.LootBag")), SelectedLootBag);
				SelectedLootBag->ConsumeAndDestroy();
				ClearNearbyLootBag(SelectedLootBag);
				if (GameplayHUDWidget)
				{
					GameplayHUDWidget->ShowPickupItemCard(PickedItemID, PickedItemRarity);
				}
			}
			else if (RunState->HasInventorySpace())
			{
				RunState->AddItemWithRarity(PickedItemID, PickedItemRarity);
				PlayInteractAudio(FName(TEXT("Pickup.LootBag")), SelectedLootBag);
				SelectedLootBag->ConsumeAndDestroy();
				ClearNearbyLootBag(SelectedLootBag);
				if (GameplayHUDWidget)
				{
					GameplayHUDWidget->ShowPickupItemCard(PickedItemID, PickedItemRarity);
				}
			}
		}
		return;
	}
	if (AT66PilotableTractor* SelectedTractor = Cast<AT66PilotableTractor>(SecondaryInteract.Actor))
	{
		if (SelectedTractor->Interact(this))
		{
			PlayInteractAudio(FName(TEXT("Interact.Generic")), SelectedTractor);
			return;
		}
	}
	if (AT66FountainOfLifeInteractable* SelectedFountain = Cast<AT66FountainOfLifeInteractable>(SecondaryInteract.Actor))
	{
		if (SelectedFountain->Interact(this))
		{
			PlayInteractAudio(FName(TEXT("Interact.Generic")), SelectedFountain);
			return;
		}
	}
	if (AT66ChestInteractable* SelectedChest = Cast<AT66ChestInteractable>(SecondaryInteract.Actor))
	{
		if (SelectedChest->Interact(this))
		{
			PlayInteractAudio(FName(TEXT("Interact.Chest.Open")), SelectedChest);
			return;
		}
	}
	if (AT66WheelSpinInteractable* SelectedWheel = Cast<AT66WheelSpinInteractable>(SecondaryInteract.Actor))
	{
		if (SelectedWheel->Interact(this))
		{
			PlayInteractAudio(FName(TEXT("Interact.Generic")), SelectedWheel);
			return;
		}
	}
	if (AT66CrateInteractable* SelectedCrate = Cast<AT66CrateInteractable>(SecondaryInteract.Actor))
	{
		if (SelectedCrate->Interact(this))
		{
			PlayInteractAudio(FName(TEXT("Interact.Crate.Open")), SelectedCrate);
			return;
		}
	}
	if (AT66StageCatchUpGoldInteractable* SelectedCatchUpGold = Cast<AT66StageCatchUpGoldInteractable>(SecondaryInteract.Actor))
	{
		if (SelectedCatchUpGold->Interact(this))
		{
			PlayInteractAudio(FName(TEXT("Pickup.Gold")), SelectedCatchUpGold);
			return;
		}
	}
	if (AT66StageCatchUpLootInteractable* SelectedCatchUpLoot = Cast<AT66StageCatchUpLootInteractable>(SecondaryInteract.Actor))
	{
		if (SelectedCatchUpLoot->Interact(this))
		{
			PlayInteractAudio(FName(TEXT("Pickup.Item")), SelectedCatchUpLoot);
			return;
		}
	}
	if (AT66IdolAltar* SelectedIdolAltar = Cast<AT66IdolAltar>(SecondaryInteract.Actor))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		UT66IdolAltarOverlayWidget* W = CreateWidget<UT66IdolAltarOverlayWidget>(this, ResolveIdolAltarOverlayClass());
		if (W)
		{
			W->SetSourceAltar(SelectedIdolAltar);
			IdolAltarOverlayWidget = W;
			W->AddToViewport(150); // above gambler overlay
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		PlayInteractAudio(FName(TEXT("Interact.Generic")), SelectedIdolAltar);
		return;
	}
	if (AT66WorldInteractableBase* SelectedWorldInteractable = Cast<AT66WorldInteractableBase>(SecondaryInteract.Actor))
	{
		if (SelectedWorldInteractable->Interact(this))
		{
			PlayInteractAudio(FName(TEXT("Interact.Generic")), SelectedWorldInteractable);
			return;
		}
	}
	// Backward-compat: if any old pickups exist, collect instantly (no popup).
	if (ClosestPickup)
	{
		if (UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			if (RunState->HasInventorySpace())
			{
				RunState->AddItem(ClosestPickup->GetItemID());
				PlayInteractAudio(FName(TEXT("Pickup.Item")), ClosestPickup);
				ClosestPickup->Destroy();
			}
			return;
		}
	}
}
