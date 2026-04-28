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
#include "UI/Screens/T66PartyInviteModal.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66PlayerController, Log, All);
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
#include "UI/T66LoadingScreenWidget.h"
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
#include "Core/T66BackendSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66SessionSubsystem.h"
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
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Gameplay/T66WorldVisualSetup.h"
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
#include "Camera/PlayerCameraManager.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
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
	constexpr int32 T66PlayerControllerGameplayViewTargetRetryBudget = 20;
	constexpr float T66PlayerControllerGameplayViewTargetRetryDelaySeconds = 0.05f;
	constexpr int32 T66PlayerControllerClientGameplayWorldSetupRetryBudget = 60;
	constexpr float T66PlayerControllerClientGameplayWorldSetupRetryDelaySeconds = 0.10f;
	static TAutoConsoleVariable<float> CVarT66GameplayCameraPitchMin(
		TEXT("T66.Camera.GameplayPitchMin"),
		-34.0f,
		TEXT("Minimum gameplay camera pitch for the fixed-distance third-person camera."),
		ECVF_Default);
	static TAutoConsoleVariable<float> CVarT66GameplayCameraPitchMax(
		TEXT("T66.Camera.GameplayPitchMax"),
		-8.0f,
		TEXT("Maximum gameplay camera pitch for the fixed-distance third-person camera. Keep this below zero to prevent the camera from rotating under the floor."),
		ECVF_Default);
	static TAutoConsoleVariable<int32> CVarT66CameraHideHeroOccluders(
		TEXT("T66.Camera.HideHeroOccluders"),
		1,
		TEXT("0 disables runtime wall cutaways, 1 hides tower wall visual actors between the camera and hero."),
		ECVF_Default);
	static TAutoConsoleVariable<float> CVarT66CameraHeroOccluderTraceRadius(
		TEXT("T66.Camera.HeroOccluderTraceRadius"),
		44.0f,
		TEXT("Radius used when finding tower wall visual actors between the fixed camera and hero."),
		ECVF_Default);
	static TAutoConsoleVariable<int32> CVarT66CameraConstrainAgainstTowerWalls(
		TEXT("T66.Camera.ConstrainAgainstTowerWalls"),
		1,
		TEXT("0 disables runtime side-wall camera retraction, 1 shortens the fixed camera boom before it crosses tower wall geometry."),
		ECVF_Default);
	static TAutoConsoleVariable<float> CVarT66CameraWallProbeRadius(
		TEXT("T66.Camera.WallProbeRadius"),
		120.0f,
		TEXT("Sphere radius used by the fixed camera side-wall spring. Larger values retract earlier and prevent seeing through wall edges."),
		ECVF_Default);
	static TAutoConsoleVariable<float> CVarT66CameraWallProbePadding(
		TEXT("T66.Camera.WallProbePadding"),
		90.0f,
		TEXT("Distance kept between the fixed camera and the first tower wall hit."),
		ECVF_Default);
	static TAutoConsoleVariable<int32> CVarT66CameraWallProbeSamples(
		TEXT("T66.Camera.WallProbeSamples"),
		14,
		TEXT("Number of overlap samples along the fixed camera arm used to catch side walls even when another static mesh blocks the sweep first."),
		ECVF_Default);
	static TAutoConsoleVariable<float> CVarT66CameraWallReturnSpeed(
		TEXT("T66.Camera.WallReturnSpeed"),
		14.0f,
		TEXT("Speed used when the fixed camera expands back to its desired zoom after clearing a wall."),
		ECVF_Default);
	const FName T66PlayerControllerMainMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	const FName T66PlayerControllerTraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
	const FName T66PlayerControllerTowerCeilingTag(TEXT("T66_Tower_Ceiling"));

	static bool T66WeakActorListContains(const TArray<TWeakObjectPtr<AActor>>& Actors, const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		for (const TWeakObjectPtr<AActor>& WeakActor : Actors)
		{
			if (WeakActor.Get() == Actor)
			{
				return true;
			}
		}

		return false;
	}

	static bool T66IsGameplayCameraWallActor(const AActor* Actor)
	{
		return Actor
			&& Actor->ActorHasTag(T66PlayerControllerMainMapTerrainVisualTag)
			&& Actor->ActorHasTag(T66PlayerControllerTraversalBarrierTag)
			&& !Actor->ActorHasTag(T66PlayerControllerTowerCeilingTag);
	}
}


AT66PlayerController::AT66PlayerController()
{
	// Default to showing mouse cursor (will be hidden in gameplay mode)
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	JumpVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1")));
	PixelVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle")));
}

void AT66PlayerController::PrimeGameplayPresentationAssetsAsync()
{
	if (!CachedJumpVFXNiagara)
	{
		CachedJumpVFXNiagara = JumpVFXNiagara.Get();
	}
	if (!CachedPixelVFXNiagara)
	{
		CachedPixelVFXNiagara = PixelVFXNiagara.Get();
	}
	if ((CachedJumpVFXNiagara && CachedPixelVFXNiagara) || GameplayPresentationAssetsLoadHandle.IsValid())
	{
		return;
	}

	TArray<FSoftObjectPath> AssetPaths;
	if (!CachedJumpVFXNiagara && !JumpVFXNiagara.IsNull())
	{
		AssetPaths.AddUnique(JumpVFXNiagara.ToSoftObjectPath());
	}
	if (!CachedPixelVFXNiagara && !PixelVFXNiagara.IsNull())
	{
		AssetPaths.AddUnique(PixelVFXNiagara.ToSoftObjectPath());
	}
	if (AssetPaths.Num() <= 0)
	{
		return;
	}

	GameplayPresentationAssetsLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		AssetPaths,
		FStreamableDelegate::CreateUObject(this, &AT66PlayerController::HandleGameplayPresentationAssetsLoaded));
	if (!GameplayPresentationAssetsLoadHandle.IsValid())
	{
		HandleGameplayPresentationAssetsLoaded();
	}
}

void AT66PlayerController::HandleGameplayPresentationAssetsLoaded()
{
	GameplayPresentationAssetsLoadHandle.Reset();
	if (!CachedJumpVFXNiagara)
	{
		CachedJumpVFXNiagara = JumpVFXNiagara.Get();
	}
	if (!CachedPixelVFXNiagara)
	{
		CachedPixelVFXNiagara = PixelVFXNiagara.Get();
	}
}

void AT66PlayerController::ClampGameplayCameraPitch()
{
	if (!IsGameplayLevel())
	{
		return;
	}

	const float PitchMin = FMath::Min(
		CVarT66GameplayCameraPitchMin.GetValueOnGameThread(),
		CVarT66GameplayCameraPitchMax.GetValueOnGameThread());
	const float PitchMax = FMath::Max(
		CVarT66GameplayCameraPitchMin.GetValueOnGameThread(),
		CVarT66GameplayCameraPitchMax.GetValueOnGameThread());

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = PitchMin;
		PlayerCameraManager->ViewPitchMax = PitchMax;
	}

	FRotator Rotation = GetControlRotation();
	Rotation.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Rotation.Pitch), PitchMin, PitchMax);
	Rotation.Roll = 0.0f;
	SetControlRotation(Rotation);
}


void AT66PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [SessionSubsystem]()
				{
					SessionSubsystem->SyncLocalLobbyProfile();
				}));
			}
		}
	}

	BindPartyInviteEvents();

	if (IsGameplayLevel())
	{
		if (!IsLocalController())
		{
			UE_LOG(LogT66PlayerController, Verbose, TEXT("PlayerController: Skipping gameplay UI init for non-local controller"));
			return;
		}

		SetupGameplayMode();
		ClampGameplayCameraPitch();
		GameplayViewTargetRetriesRemaining = T66PlayerControllerGameplayViewTargetRetryBudget;
		RefreshGameplayViewTarget(true);
		bClientGameplayWorldSetupComplete = false;
		bReceivedGameplayRunSettingsFromServer = false;
		ClientGameplayWorldSetupRetriesRemaining = T66PlayerControllerClientGameplayWorldSetupRetryBudget;
		EnsureClientGameplayWorldSetup(true);
		SetupGameplayHUD();
		PrimeGameplayPresentationAssetsAsync();
		UWorld* World = GetWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;

		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			RunState->OnPlayerDied.AddDynamic(this, &AT66PlayerController::OnPlayerDied);
			RunState->QuickReviveChanged.AddDynamic(this, &AT66PlayerController::HandleQuickReviveStateChanged);
		}

		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI); T66GI && T66GI->bSaveSlotPreviewMode)
		{
			SetPause(true);
			EnsureGameplayUIManager();
			if (UIManager)
			{
				UIManager->ShowModal(ET66ScreenType::SavePreview);
			}

			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
			bEnableClickEvents = true;
			bEnableMouseOverEvents = true;
			if (GameplayHUDWidget)
			{
				GameplayHUDWidget->SetInteractive(true);
				GameplayHUDWidget->MarkHUDDirty();
			}
		}

		UE_LOG(LogT66PlayerController, Log, TEXT("PlayerController: Gameplay mode initialized"));
	}
	else
	{
		if (!IsLocalController())
		{
			UE_LOG(LogT66PlayerController, Verbose, TEXT("PlayerController: Skipping frontend UI init for non-local controller"));
			return;
		}

		EndHeroOneScopedUlt();

		// Frontend mode: UI input, show cursor, initialize UI
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		if (bAutoShowInitialScreen)
		{
			// Defer UI init by a short delay so the PIE viewport can stabilize its resolution.
			// Without this, the first frame renders at a transient viewport size, causing a brief
			// "zoomed-in" flash before snapping to the correct resolution. The delay also gives
			// GameInstance async texture preloads more time to complete, reducing sync load fallbacks.
			FTimerHandle DeferHandle;
			GetWorld()->GetTimerManager().SetTimer(DeferHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				InitializeUI();
			}), 0.1f, false);
		}
		UE_LOG(LogT66PlayerController, Log, TEXT("PlayerController: Frontend mode initialized (UI deferred)"));
	}
}

void AT66PlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	if (!IsGameplayLevel())
	{
		return;
	}

	ClampGameplayCameraPitch();
	GameplayViewTargetRetriesRemaining = T66PlayerControllerGameplayViewTargetRetryBudget;
	RefreshGameplayViewTarget(true);
}

void AT66PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn || !IsGameplayLevel())
	{
		return;
	}

	FRotator DesiredRotation = InPawn->GetActorRotation();
	DesiredRotation.Roll = 0.f;
	if (DesiredRotation.Pitch > 45.f || DesiredRotation.Pitch < -45.f)
	{
		DesiredRotation.Pitch = 0.f;
	}

	SetControlRotation(DesiredRotation);
	ClientSetRotation(DesiredRotation, true);
	ClampGameplayCameraPitch();

	GameplayViewTargetRetriesRemaining = T66PlayerControllerGameplayViewTargetRetryBudget;
	RefreshGameplayViewTarget(true);
}

void AT66PlayerController::ClientRestart_Implementation(APawn* NewPawn)
{
	Super::ClientRestart_Implementation(NewPawn);

	if (!IsGameplayLevel())
	{
		return;
	}

	ClampGameplayCameraPitch();
	GameplayViewTargetRetriesRemaining = T66PlayerControllerGameplayViewTargetRetryBudget;
	RefreshGameplayViewTarget(true);
}

void AT66PlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (!IsGameplayLevel())
	{
		return;
	}

	ClampGameplayCameraPitch();
	GameplayViewTargetRetriesRemaining = T66PlayerControllerGameplayViewTargetRetryBudget;
	RefreshGameplayViewTarget(true);
}


void AT66PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreHeroCameraOccluders();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathVFXTimerHandle);
		World->GetTimerManager().ClearTimer(GameplayViewTargetRetryTimerHandle);
		World->GetTimerManager().ClearTimer(ClientGameplayWorldSetupRetryTimerHandle);
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->OnPlayerDied.RemoveDynamic(this, &AT66PlayerController::OnPlayerDied);
		RunState->QuickReviveChanged.RemoveDynamic(this, &AT66PlayerController::HandleQuickReviveStateChanged);
	}

	UnbindPartyInviteEvents();

	if (FrontendLaunchPolicyResponseHandle.IsValid())
	{
		if (UGameInstance* CurrentGameInstance = GetGameInstance())
		{
			if (UT66BackendSubsystem* Backend = CurrentGameInstance->GetSubsystem<UT66BackendSubsystem>())
			{
				Backend->OnClientLaunchPolicyResponse().Remove(FrontendLaunchPolicyResponseHandle);
			}
		}

		FrontendLaunchPolicyResponseHandle.Reset();
	}

	GetWorldTimerManager().ClearTimer(FrontendLaunchPolicyTimeoutTimerHandle);
	HideFrontendStartupOverlay();
	bClientGameplayWorldSetupComplete = false;
	bReceivedGameplayRunSettingsFromServer = false;
	GameplayPresentationAssetsLoadHandle.Reset();

	Super::EndPlay(EndPlayReason);
}

void AT66PlayerController::RestoreHeroCameraOccluders()
{
	for (const TWeakObjectPtr<AActor>& WeakActor : HiddenHeroCameraOccluders)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Actor->SetActorHiddenInGame(false);
		}
	}

	HiddenHeroCameraOccluders.Reset();
}

void AT66PlayerController::UpdateGameplayCameraSideWallSpring(const float DeltaTime)
{
	if (!IsLocalController()
		|| !IsGameplayLevel()
		|| bHeroOneScopeViewEnabled
		|| CVarT66CameraConstrainAgainstTowerWalls.GetValueOnGameThread() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	if (!World || !Hero || !Hero->CameraBoom)
	{
		return;
	}

	if (DesiredGameplayCameraArmLength <= KINDA_SMALL_NUMBER)
	{
		DesiredGameplayCameraArmLength = Hero->CameraBoom->TargetArmLength;
	}
	Hero->CameraBoom->bDoCollisionTest = false;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(T66GameplayCameraSideWallSpring), false);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Hero);
	QueryParams.bFindInitialOverlaps = true;

	const FVector PivotLocation = Hero->CameraBoom->GetComponentLocation();
	const float DesiredArmLength = FMath::Max(0.0f, DesiredGameplayCameraArmLength);
	const float ProbeRadius = FMath::Max(1.0f, CVarT66CameraWallProbeRadius.GetValueOnGameThread());
	const float ProbePadding = FMath::Max(0.0f, CVarT66CameraWallProbePadding.GetValueOnGameThread());
	const int32 ProbeSamples = FMath::Clamp(CVarT66CameraWallProbeSamples.GetValueOnGameThread(), 2, 32);
	const FRotator CameraRotation = GetControlRotation();
	const FVector CameraDirection = (-CameraRotation.Vector()).GetSafeNormal();
	if (CameraDirection.IsNearlyZero())
	{
		return;
	}

	const FVector DesiredCameraLocation = PivotLocation + (CameraDirection * DesiredArmLength);

	auto FindCameraWallHitDistance = [&](float& OutDistance) -> bool
	{
		float ClosestDistance = TNumericLimits<float>::Max();

		TArray<FHitResult> Hits;
		World->SweepMultiByChannel(
			Hits,
			PivotLocation,
			DesiredCameraLocation,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeSphere(ProbeRadius),
			QueryParams);

		for (const FHitResult& Hit : Hits)
		{
			if (!T66IsGameplayCameraWallActor(Hit.GetActor()))
			{
				continue;
			}

			ClosestDistance = FMath::Min(ClosestDistance, FMath::Max(0.0f, Hit.Distance));
		}

		for (int32 SampleIndex = 1; SampleIndex <= ProbeSamples; ++SampleIndex)
		{
			const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(ProbeSamples);
			const FVector SampleLocation = FMath::Lerp(PivotLocation, DesiredCameraLocation, Alpha);
			TArray<FOverlapResult> Overlaps;
			World->OverlapMultiByChannel(
				Overlaps,
				SampleLocation,
				FQuat::Identity,
				ECC_Visibility,
				FCollisionShape::MakeSphere(ProbeRadius),
				QueryParams);

			for (const FOverlapResult& Overlap : Overlaps)
			{
				if (!T66IsGameplayCameraWallActor(Overlap.GetActor()))
				{
					continue;
				}

				ClosestDistance = FMath::Min(ClosestDistance, DesiredArmLength * Alpha);
			}
		}

		if (ClosestDistance == TNumericLimits<float>::Max())
		{
			return false;
		}

		OutDistance = ClosestDistance;
		return true;
	};

	float SafeArmLength = DesiredArmLength;
	float WallHitDistance = 0.0f;
	if (FindCameraWallHitDistance(WallHitDistance))
	{
		SafeArmLength = FMath::Clamp(WallHitDistance - ProbePadding, 0.0f, DesiredArmLength);
	}

	const float CurrentArmLength = FMath::Max(0.0f, Hero->CameraBoom->TargetArmLength);
	if (SafeArmLength < CurrentArmLength)
	{
		Hero->CameraBoom->TargetArmLength = SafeArmLength;
		return;
	}

	const float ReturnSpeed = FMath::Max(0.0f, CVarT66CameraWallReturnSpeed.GetValueOnGameThread());
	const float NewArmLength = ReturnSpeed <= 0.0f
		? SafeArmLength
		: FMath::FInterpTo(CurrentArmLength, SafeArmLength, DeltaTime, ReturnSpeed);
	Hero->CameraBoom->TargetArmLength = FMath::Min(NewArmLength, DesiredArmLength);
	if (FMath::IsNearlyEqual(Hero->CameraBoom->TargetArmLength, DesiredArmLength, 1.0f))
	{
		Hero->CameraBoom->TargetArmLength = DesiredArmLength;
	}
}

void AT66PlayerController::UpdateHeroCameraOccluders()
{
	if (!IsLocalController()
		|| !IsGameplayLevel()
		|| bHeroOneScopeViewEnabled
		|| CVarT66CameraHideHeroOccluders.GetValueOnGameThread() == 0)
	{
		RestoreHeroCameraOccluders();
		return;
	}

	UWorld* World = GetWorld();
	AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn());
	if (!World || !Hero || !PlayerCameraManager)
	{
		RestoreHeroCameraOccluders();
		return;
	}

	const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	const FVector HeroLocation = Hero->GetActorLocation();
	const FVector TargetPoints[] =
	{
		HeroLocation + FVector(0.0f, 0.0f, 45.0f),
		HeroLocation + FVector(0.0f, 0.0f, 105.0f),
		HeroLocation + FVector(0.0f, 0.0f, 165.0f)
	};

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(T66HeroCameraOccluderTrace), false);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Hero);
	QueryParams.bFindInitialOverlaps = true;

	TArray<AActor*> NewOccluders;
	auto AddOccluderHit = [this, &NewOccluders](const FHitResult& Hit)
	{
		AActor* HitActor = Hit.GetActor();
		if (!T66IsGameplayCameraWallActor(HitActor))
		{
			return;
		}

		if (HitActor->IsHidden() && !T66WeakActorListContains(HiddenHeroCameraOccluders, HitActor))
		{
			return;
		}

		NewOccluders.AddUnique(HitActor);
	};

	const float TraceRadius = FMath::Max(1.0f, CVarT66CameraHeroOccluderTraceRadius.GetValueOnGameThread());
	for (const FVector& TargetPoint : TargetPoints)
	{
		TArray<FHitResult> Hits;
		World->SweepMultiByChannel(
			Hits,
			CameraLocation,
			TargetPoint,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeSphere(TraceRadius),
			QueryParams);

		for (const FHitResult& Hit : Hits)
		{
			AddOccluderHit(Hit);
		}
	}

	for (const TWeakObjectPtr<AActor>& WeakActor : HiddenHeroCameraOccluders)
	{
		AActor* Actor = WeakActor.Get();
		if (Actor && !NewOccluders.Contains(Actor))
		{
			Actor->SetActorHiddenInGame(false);
		}
	}

	TArray<TWeakObjectPtr<AActor>> UpdatedHiddenOccluders;
	for (AActor* Actor : NewOccluders)
	{
		if (!Actor)
		{
			continue;
		}

		if (!T66WeakActorListContains(HiddenHeroCameraOccluders, Actor))
		{
			Actor->SetActorHiddenInGame(true);
		}
		UpdatedHiddenOccluders.Add(Actor);
	}

	HiddenHeroCameraOccluders = MoveTemp(UpdatedHiddenOccluders);
}

void AT66PlayerController::RefreshGameplayViewTarget(bool bAllowRetry)
{
	if (!IsLocalController() || !IsGameplayLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		if (bAllowRetry && GameplayViewTargetRetriesRemaining > 0)
		{
			--GameplayViewTargetRetriesRemaining;
			World->GetTimerManager().SetTimer(
				GameplayViewTargetRetryTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					RefreshGameplayViewTarget(true);
				}),
				T66PlayerControllerGameplayViewTargetRetryDelaySeconds,
				false);
		}
		return;
	}

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(ControlledPawn))
	{
		Hero->SetPreviewMode(false);
		if (Hero->FollowCamera && !Hero->FollowCamera->IsActive())
		{
			Hero->FollowCamera->SetActive(true);
		}
	}

	World->GetTimerManager().ClearTimer(GameplayViewTargetRetryTimerHandle);
	GameplayViewTargetRetriesRemaining = 0;
	AutoManageActiveCameraTarget(ControlledPawn);

	if (GetViewTarget() != ControlledPawn)
	{
		SetViewTarget(ControlledPawn);
		UE_LOG(LogT66PlayerController, Log, TEXT("PlayerController: restored gameplay view target to %s"), *GetNameSafe(ControlledPawn));
	}

	if (World->GetNetMode() == NM_Client)
	{
		if (bClientGameplayWorldSetupComplete)
		{
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
			{
				T66GI->HidePersistentGameplayTransitionCurtain();
			}
		}
	}
}

bool AT66PlayerController::ApplyHostPartyRunSettingsToGameInstance() const
{
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!T66GI)
	{
		return false;
	}

	UT66SessionSubsystem* SessionSubsystem = T66GI->GetSubsystem<UT66SessionSubsystem>();
	if (!SessionSubsystem)
	{
		return false;
	}

	FT66LobbyPlayerInfo HostLobbyInfo;
	if (!SessionSubsystem->GetHostLobbyProfile(HostLobbyInfo))
	{
		return false;
	}

	if (HostLobbyInfo.RunSeed == 0)
	{
		return false;
	}

	T66GI->RunSeed = HostLobbyInfo.RunSeed;
	T66GI->SelectedDifficulty = HostLobbyInfo.LobbyDifficulty;
	T66GI->CurrentMainMapLayoutVariant = HostLobbyInfo.MainMapLayoutVariant;
	return true;
}

void AT66PlayerController::EnsureClientGameplayWorldSetup(bool bAllowRetry)
{
	if (!IsLocalController() || !IsGameplayLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() != NM_Client)
	{
		return;
	}

	if (bClientGameplayWorldSetupComplete)
	{
		return;
	}

	int32 ExistingTerrainVisualCount = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (AActor* Actor = *It; Actor && Actor->ActorHasTag(T66PlayerControllerMainMapTerrainVisualTag))
		{
			++ExistingTerrainVisualCount;
		}
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const bool bHasRunSettings = T66GI && (bReceivedGameplayRunSettingsFromServer || ApplyHostPartyRunSettingsToGameInstance());
	if (!T66GI || !bHasRunSettings)
	{
		if (bAllowRetry && ClientGameplayWorldSetupRetriesRemaining > 0)
		{
			--ClientGameplayWorldSetupRetriesRemaining;
			World->GetTimerManager().SetTimer(
				ClientGameplayWorldSetupRetryTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					EnsureClientGameplayWorldSetup(true);
				}),
				T66PlayerControllerClientGameplayWorldSetupRetryDelaySeconds,
				false);
		}
		else if (ClientGameplayWorldSetupRetriesRemaining <= 0)
		{
			UE_LOG(LogT66PlayerController, Warning, TEXT("PlayerController: client gameplay setup timed out waiting for host run settings."));
		}
		return;
	}

	FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(World);

	if (T66GI->CurrentMainMapLayoutVariant != ET66MainMapLayoutVariant::Tower)
	{
		World->GetTimerManager().ClearTimer(ClientGameplayWorldSetupRetryTimerHandle);
		ClientGameplayWorldSetupRetriesRemaining = 0;
		bClientGameplayWorldSetupComplete = true;
		RefreshGameplayViewTarget(true);
		return;
	}

	if (ExistingTerrainVisualCount >= 10)
	{
		World->GetTimerManager().ClearTimer(ClientGameplayWorldSetupRetryTimerHandle);
		ClientGameplayWorldSetupRetriesRemaining = 0;
		bClientGameplayWorldSetupComplete = true;
		RefreshGameplayViewTarget(true);
		return;
	}

	if (ExistingTerrainVisualCount > 0)
	{
		TArray<AActor*> TerrainActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (AActor* Actor = *It; Actor && Actor->ActorHasTag(T66PlayerControllerMainMapTerrainVisualTag))
			{
				TerrainActorsToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : TerrainActorsToDestroy)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}
	}

	const FT66MapPreset Preset = T66MainMapTerrain::BuildPresetForDifficulty(T66GI->SelectedDifficulty, T66GI->RunSeed);
	T66TowerMapTerrain::FLayout Layout;
	if (!T66TowerMapTerrain::BuildLayout(Preset, Layout))
	{
		UE_LOG(LogT66PlayerController, Warning, TEXT("PlayerController: failed to build client tower layout for run seed %d"), T66GI->RunSeed);
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	bool bCollisionReady = false;
	if (!T66TowerMapTerrain::Spawn(World, Layout, T66GI->SelectedDifficulty, SpawnParams, bCollisionReady))
	{
		UE_LOG(LogT66PlayerController, Warning, TEXT("PlayerController: failed to spawn client tower terrain for run seed %d"), T66GI->RunSeed);
		return;
	}

	UE_LOG(
		LogT66PlayerController,
		Log,
		TEXT("PlayerController: spawned client-side gameplay tower terrain (seed=%d, difficulty=%d, collisionReady=%d)"),
		T66GI->RunSeed,
		static_cast<int32>(T66GI->SelectedDifficulty),
		bCollisionReady ? 1 : 0);

	World->GetTimerManager().ClearTimer(ClientGameplayWorldSetupRetryTimerHandle);
	ClientGameplayWorldSetupRetriesRemaining = 0;
	bClientGameplayWorldSetupComplete = true;
	RefreshGameplayViewTarget(true);
}

void AT66PlayerController::ClientApplyGameplayRunSettings_Implementation(int32 InRunSeed, ET66Difficulty InDifficulty, ET66MainMapLayoutVariant InLayoutVariant)
{
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		T66GI->RunSeed = InRunSeed;
		T66GI->SelectedDifficulty = InDifficulty;
		T66GI->CurrentMainMapLayoutVariant = InLayoutVariant;
	}

	bReceivedGameplayRunSettingsFromServer = true;

	if (IsGameplayLevel())
	{
		bClientGameplayWorldSetupComplete = false;
		ClientGameplayWorldSetupRetriesRemaining = T66PlayerControllerClientGameplayWorldSetupRetryBudget;
		EnsureClientGameplayWorldSetup(true);
	}
}

void AT66PlayerController::PushLobbyProfileToServer(const FT66LobbyPlayerInfo& LobbyInfo)
{
	if (AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>())
	{
		if (HasAuthority())
		{
			SessionPlayerState->ApplyLobbyInfo(LobbyInfo);
		}
		else
		{
			ServerSubmitLobbyProfile(LobbyInfo);
		}
	}
}

void AT66PlayerController::PushPartyRunSummaryToServer(const FString& RequestKey, const FString& RunSummaryJson)
{
	if (RequestKey.IsEmpty() || RunSummaryJson.IsEmpty())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>())
			{
				if (HasAuthority())
				{
					SessionSubsystem->StorePartyRunSummaryForSteamId(RequestKey, SessionPlayerState->GetSteamId(), RunSummaryJson);
				}
				else
				{
					ServerSubmitPartyRunSummary(RequestKey, RunSummaryJson);
				}
			}
		}
	}
}

void AT66PlayerController::ServerSubmitLobbyProfile_Implementation(const FT66LobbyPlayerInfo& LobbyInfo)
{
	if (AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>())
	{
		FT66LobbyPlayerInfo SanitizedInfo = LobbyInfo;
		const FT66LobbyPlayerInfo& ExistingInfo = SessionPlayerState->GetLobbyInfo();
		if (!ExistingInfo.SteamId.IsEmpty())
		{
			SanitizedInfo.SteamId = ExistingInfo.SteamId;
		}
		if (SanitizedInfo.DisplayName.IsEmpty())
		{
			SanitizedInfo.DisplayName = ExistingInfo.DisplayName.IsEmpty() ? SessionPlayerState->GetPlayerName() : ExistingInfo.DisplayName;
		}

		// Remote clients never get to self-assign host authority.
		SanitizedInfo.bPartyHost = false;
		SessionPlayerState->ApplyLobbyInfo(SanitizedInfo);
	}
}

void AT66PlayerController::ServerSubmitPartyRunSummary_Implementation(const FString& RequestKey, const FString& RunSummaryJson)
{
	if (RequestKey.IsEmpty() || RunSummaryJson.IsEmpty())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>())
			{
				SessionSubsystem->StorePartyRunSummaryForSteamId(RequestKey, SessionPlayerState->GetSteamId(), RunSummaryJson);
			}
		}
	}
}

void AT66PlayerController::ServerRequestPartySaveAndReturnToFrontend_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SaveCurrentRunAndReturnToFrontend();
		}
	}
}

void AT66PlayerController::BindPartyInviteEvents()
{
	UnbindPartyInviteEvents();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			PendingPartyInvitesChangedHandle = Backend->OnPendingPartyInvitesChanged().AddUObject(this, &AT66PlayerController::HandlePendingPartyInvitesChanged);
			Backend->PollPendingPartyInvites(true);
		}
	}
}

void AT66PlayerController::UnbindPartyInviteEvents()
{
	if (!PendingPartyInvitesChangedHandle.IsValid())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnPendingPartyInvitesChanged().Remove(PendingPartyInvitesChangedHandle);
		}
	}

	PendingPartyInvitesChangedHandle.Reset();
}

void AT66PlayerController::HandlePendingPartyInvitesChanged()
{
	RefreshPartyInviteModal();
}

void AT66PlayerController::RefreshPartyInviteModal()
{
	if (!UIManager)
	{
		if (IsGameplayLevel())
		{
			EnsureGameplayUIManager();
		}
		else
		{
			return;
		}
	}

	if (!UIManager)
	{
		return;
	}

	UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		return;
	}

	const bool bHasPendingInvites = Backend->HasPendingPartyInvites();
	const ET66ScreenType CurrentModalType = UIManager->GetCurrentModalType();
	if (!bHasPendingInvites)
	{
		if (CurrentModalType == ET66ScreenType::PartyInvite)
		{
			UIManager->CloseModal();
			if (IsGameplayLevel() && !IsPaused())
			{
				RestoreGameplayInputMode();
			}
		}
		return;
	}

	if (CurrentModalType == ET66ScreenType::PartyInvite)
	{
		if (UT66ScreenBase* CurrentModal = UIManager->GetCurrentModal())
		{
			CurrentModal->ForceRebuildSlate();
		}
		return;
	}

	if (CurrentModalType != ET66ScreenType::None)
	{
		UIManager->CloseModal();
	}

	if (IsGameplayLevel())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}

	UIManager->ShowModal(ET66ScreenType::PartyInvite);
}


bool AT66PlayerController::IsFrontendLevel() const
{
	if (!GetWorld()) return true; // Default to frontend if no world
	
	FString MapName = GetWorld()->GetMapName();
	// Remove any prefix like "UEDPIE_0_" from PIE sessions
	MapName = UWorld::RemovePIEPrefix(MapName);
	
	// Check if this is a frontend level (contains "Frontend" or "Menu")
	return MapName.Contains(TEXT("Frontend")) || MapName.Contains(TEXT("Menu"));
}


bool AT66PlayerController::IsGameplayLevel() const
{
	if (!GetWorld()) return false;
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			if (T66GI->bIsLabLevel) return true;
		}
	}
	FString MapName = GetWorld()->GetMapName();
	MapName = UWorld::RemovePIEPrefix(MapName);
	if (MapName.Contains(TEXT("Gameplay"))) return true;
	return false;
}


void AT66PlayerController::SetupGameplayMode()
{
	const bool bHadScopedState = bHeroOneScopedUltActive || bHeroOneScopeViewEnabled;
	EndHeroOneScopedUlt();
	RestoreGameplayInputMode();
	UE_LOG(LogT66PlayerController, Log, TEXT("PlayerController: Gameplay input mode set, cursor hidden (scoped state reset=%d)"), bHadScopedState ? 1 : 0);
}
