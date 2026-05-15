// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/T66UIManager.h"
#include "UI/T66WidgetDumpTargets.h"
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
#include "UI/Screens/T66SavePreviewScreen.h"
#include "UI/Screens/T66PowerUpScreen.h"
#include "UI/Screens/T66AccountStatusScreen.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/Style/T66Style.h"
#include "UI/T66LabOverlayWidget.h"
#include "UI/T66CasinoOverlayWidget.h"
#include "UI/T66CowardicePromptWidget.h"
#include "UI/T66IdolAltarOverlayWidget.h"
#include "UI/T66CollectorOverlayWidget.h"
#include "UI/T66LoadingScreenWidget.h"
#include "UI/T66ArcadePopupWidget.h"
#include "UI/T66ArcadeSelectionWidget.h"
#include "UI/T66CrateOverlayWidget.h"
#include "UI/T66GoldMinerArcadeWidget.h"
#include "UI/T66QuickArcadeWidget.h"
#include "UI/T66TopwarArcadeWidget.h"
#include "UI/T66WhackAMoleArcadeWidget.h"
#include "Gameplay/T66FountainInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66ArcadeInteractableBase.h"
#include "Gameplay/T66ArcadeMachineInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "Gameplay/T66StageCatchUpGate.h"
#include "Gameplay/T66TutorialPortal.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "Gameplay/T66GameMode.h"
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
#include "HAL/FileManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
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
	template <typename TNpcType>
	TNpcType* T66FindRegisteredNpc(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNpc : Registry->GetNPCs())
			{
				if (TNpcType* Npc = Cast<TNpcType>(WeakNpc.Get()))
				{
					return Npc;
				}
			}
		}

		return nullptr;
	}

	bool T66HasRegisteredGamblerBoss(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
			{
				if (Cast<AT66GamblerBoss>(WeakBoss.Get()) != nullptr)
				{
					return true;
				}
			}
		}

		return false;
	}
}


void AT66PlayerController::SetupGameplayHUD()
{
	if (!IsGameplayLevel() || !IsLocalController()) return;
	UClass* HUDClass = ResolveGameplayHUDClass();
	if (!HUDClass) return;
	GameplayHUDWidget = CreateWidget<UT66GameplayHUDWidget>(this, HUDClass);
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->AddToViewport(0);
		QueueGameplayAutomationScreenshotIfRequested();
	}
	// The Lab: no floating panel; player interacts with The Collector NPC to open full-screen Collector UI.
	// (LabOverlayWidget not created when in Lab.)
}

void AT66PlayerController::ShowInteractionPrompt(AActor* SourceActor, const FText& TargetName)
{
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->ShowInteractionPrompt(SourceActor, TargetName);
	}
}

void AT66PlayerController::HideInteractionPrompt(AActor* SourceActor)
{
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->HideInteractionPrompt(SourceActor);
	}
}

void AT66PlayerController::QueueGameplayAutomationScreenshotIfRequested()
{
	if (!IsGameplayLevel() || !GetWorld())
	{
		return;
	}

	FString RequestedScreenshotPath;
	const bool bScreenshotRequested = FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshot="), RequestedScreenshotPath);

	FString RequestedWidgetDumpSpec;
	const bool bWidgetDumpRequested = FParse::Value(FCommandLine::Get(), TEXT("T66AutoDumpWidget="), RequestedWidgetDumpSpec);
	if (!bScreenshotRequested && !bWidgetDumpRequested)
	{
		return;
	}

	GameplayAutomationScreenshotPath.Reset();
	GameplayAutomationWidgetDumpTarget.Reset();
	GameplayAutomationWidgetDumpPath.Reset();
	GameplayAutomationScreenshotDelaySeconds = 4.0f;
	GameplayAutomationWidgetDumpDelaySeconds = 4.0f;
	if (bScreenshotRequested)
	{
		GameplayAutomationScreenshotPath = FPaths::ConvertRelativePathToFull(RequestedScreenshotPath);
		FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshotDelay="), GameplayAutomationScreenshotDelaySeconds);
	}
	if (bWidgetDumpRequested)
	{
		FString WidgetDumpPath;
		FString ParseError;
		if (!FT66WidgetDumpTargets::ParseAutomationSpec(RequestedWidgetDumpSpec, GameplayAutomationWidgetDumpTarget, WidgetDumpPath, ParseError))
		{
			UE_LOG(LogTemp, Error, TEXT("Gameplay automation: invalid widget dump spec: %s"), *ParseError);
			FPlatformMisc::RequestExitWithStatus(false, 67, TEXT("T66AutoDumpWidgetInvalid"));
			return;
		}

		GameplayAutomationWidgetDumpPath = FPaths::ConvertRelativePathToFull(WidgetDumpPath);
		GameplayAutomationWidgetDumpDelaySeconds = GameplayAutomationScreenshotDelaySeconds;
		FParse::Value(FCommandLine::Get(), TEXT("T66AutoDumpWidgetDelay="), GameplayAutomationWidgetDumpDelaySeconds);
	}

	GameplayAutomationCaptureMode = TEXT("HUD");
	FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoCapture="), GameplayAutomationCaptureMode);
	bGameplayAutomationKeepAliveAfterScreenshot =
		FParse::Param(FCommandLine::Get(), TEXT("T66GameplayKeepAliveAfterScreenshot"))
		|| FParse::Param(FCommandLine::Get(), TEXT("T66KeepAliveAfterScreenshot"));

	int32 AutomationResX = 0;
	int32 AutomationResY = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResX="), AutomationResX)
		&& FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResY="), AutomationResY)
		&& AutomationResX > 0
		&& AutomationResY > 0)
	{
		const TCHAR* WindowModeSuffix = FParse::Param(FCommandLine::Get(), TEXT("T66AutomationWindowed")) ? TEXT("w") : TEXT("");
		ConsoleCommand(FString::Printf(TEXT("r.SetRes %dx%d%s"), AutomationResX, AutomationResY, WindowModeSuffix), true);
	}

	GetWorldTimerManager().ClearTimer(GameplayAutomationPrepareTimerHandle);
	GetWorldTimerManager().SetTimer(
		GameplayAutomationPrepareTimerHandle,
		this,
		&AT66PlayerController::HandleGameplayAutomationPrepare,
		FMath::Max(0.1f, FMath::Min(
			bScreenshotRequested ? GameplayAutomationScreenshotDelaySeconds : GameplayAutomationWidgetDumpDelaySeconds,
			bWidgetDumpRequested ? GameplayAutomationWidgetDumpDelaySeconds : GameplayAutomationScreenshotDelaySeconds)),
		false);
}

void AT66PlayerController::HandleGameplayAutomationPrepare()
{
	if (GameplayAutomationScreenshotPath.IsEmpty() && GameplayAutomationWidgetDumpPath.IsEmpty())
	{
		return;
	}

	ApplyGameplayAutomationCaptureMode();

	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(GameplayAutomationScreenshotTimerHandle);
		GetWorldTimerManager().SetTimer(
			GameplayAutomationScreenshotTimerHandle,
			this,
			&AT66PlayerController::HandleGameplayAutomationScreenshot,
			0.5f,
			false);

		if (!GameplayAutomationWidgetDumpPath.IsEmpty())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationWidgetDumpTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationWidgetDumpTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationWidgetDump,
				0.75f,
				false);
		}
	}
}

void AT66PlayerController::ApplyGameplayAutomationCaptureMode()
{
	const FString Mode = GameplayAutomationCaptureMode.TrimStartAndEnd().ToLower();

	if (Mode == TEXT("hudreview") || Mode == TEXT("hudvisualreview") || Mode == TEXT("ingamehudreview"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		FString PickupCardItemIDString;
		const bool bShowPickupCardForReview = FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoPickupCard="), PickupCardItemIDString)
			&& !PickupCardItemIDString.TrimStartAndEnd().IsEmpty();
		const FName PickupCardItemID(*PickupCardItemIDString.TrimStartAndEnd());
		ET66ItemRarity PickupCardRarity = ET66ItemRarity::Yellow;
		FString PickupCardRarityString;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoPickupCardRarity="), PickupCardRarityString))
		{
			const FString NormalizedRarity = PickupCardRarityString.TrimStartAndEnd().ToLower();
			if (NormalizedRarity == TEXT("black"))
			{
				PickupCardRarity = ET66ItemRarity::Black;
			}
			else if (NormalizedRarity == TEXT("red"))
			{
				PickupCardRarity = ET66ItemRarity::Red;
			}
			else if (NormalizedRarity == TEXT("white"))
			{
				PickupCardRarity = ET66ItemRarity::White;
			}
		}
		if (RunState)
		{
			const int32 MissingDifficultySkulls = FMath::Max(0, 4 - RunState->GetDifficultySkulls());
			if (MissingDifficultySkulls > 0)
			{
				RunState->AddDifficultySkulls(MissingDifficultySkulls);
			}

			while (RunState->GetCowardiceGatesTaken() < 4)
			{
				RunState->AddCowardiceGateTaken();
			}

			RunState->AddGold(888);
			RunState->AddScore(1250);
			RunState->SetStageTimerActive(true);

			FRandomStream ReviewItemStream(660427);
			const ET66Rarity ReviewLootRarities[] =
			{
				ET66Rarity::Black,
				ET66Rarity::Red,
				ET66Rarity::Yellow,
				ET66Rarity::White
			};
			int32 RarityIndex = 0;
			int32 ItemAttempts = 0;
			while (T66GI
				&& RunState->GetInventorySlots().Num() < UT66RunStateSubsystem::MaxInventorySlots
				&& ItemAttempts < UT66RunStateSubsystem::MaxInventorySlots * 3)
			{
				++ItemAttempts;
				const ET66Rarity LootRarity = ReviewLootRarities[RarityIndex % UE_ARRAY_COUNT(ReviewLootRarities)];
				const FName ItemID = T66GI->GetRandomItemIDForLootRarityFromStream(LootRarity, ReviewItemStream);
				if (ItemID.IsNone())
				{
					break;
				}

				const ET66ItemRarity ItemRarity = static_cast<ET66ItemRarity>(static_cast<uint8>(LootRarity));
				RunState->AddItemWithRarity(ItemID, ItemRarity);
				++RarityIndex;
			}

			if (bShowPickupCardForReview && !PickupCardItemID.IsNone())
			{
				RunState->AddItemWithRarity(PickupCardItemID, PickupCardRarity);
			}
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			if (!bShowPickupCardForReview)
			{
				GameplayHUDWidget->ShowInteractionPrompt(this, NSLOCTEXT("T66.GameplayHUD", "HudReviewInteractionTarget", "Chest"));
			}
			GameplayHUDWidget->RefreshHUD();
			if (bShowPickupCardForReview && !PickupCardItemID.IsNone())
			{
				GameplayHUDWidget->ShowPickupItemCard(PickupCardItemID, PickupCardRarity);
			}
		}
		return;
	}

	if (Mode == TEXT("heroqa") || Mode == TEXT("herovisualqa") || Mode == TEXT("playableheroqa"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
				}
			}

			ControlledPawn->SetActorRotation(FRotator(0.f, 180.f, 0.f));
			SetControlRotation(FRotator(-8.f, 180.f, 0.f));

			if (AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn))
			{
				if (HeroPawn->CameraBoom)
				{
					HeroPawn->CameraBoom->TargetArmLength = 420.f;
					HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 95.f));
					HeroPawn->CameraBoom->bDoCollisionTest = false;
				}
			}
		}
		return;
	}

	if (Mode == TEXT("inventory") || Mode == TEXT("inspect") || Mode == TEXT("inventoryinspect"))
	{
		SetInventoryInspectOpen(true);
		return;
	}

	if (Mode == TEXT("fullmap") || Mode == TEXT("map"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(true);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("worldprompt") || Mode == TEXT("interactable") || Mode == TEXT("interactionprompt"))
	{
		AT66ChestInteractable* AutomationPromptActor = nullptr;
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66WidgetDump_WorldInteractablePrompt"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AutomationPromptActor = World->SpawnActor<AT66ChestInteractable>(
				AT66ChestInteractable::StaticClass(),
				GetPawn() ? GetPawn()->GetActorLocation() + FVector(260.f, 0.f, 0.f) : FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams);
			if (AutomationPromptActor)
			{
				AutomationPromptActor->SetActorHiddenInGame(true);
				AutomationPromptActor->SetActorEnableCollision(false);
				AutomationPromptActor->SetShowcaseReusable(true);
			}
		}

		if (GameplayHUDWidget)
		{
			AActor* PromptSourceActor = AutomationPromptActor ? static_cast<AActor*>(AutomationPromptActor) : static_cast<AActor*>(this);
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->ShowInteractionPrompt(
				PromptSourceActor,
				NSLOCTEXT("T66.GameplayHUD", "WidgetDumpWorldPromptTarget", "Chest"));
			GameplayHUDWidget->RefreshHUD();
		}
		return;
	}

	if (Mode == TEXT("enemylock") || Mode == TEXT("lockindicator") || Mode == TEXT("enemylockwidget"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66WidgetDump_EnemyLockTarget"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
				AT66EnemyBase::StaticClass(),
				GetPawn() ? GetPawn()->GetActorLocation() + FVector(420.f, 0.f, 0.f) : FVector(420.f, 0.f, 0.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (Enemy)
			{
				Enemy->SetActorEnableCollision(false);
				Enemy->SetLockedIndicator(true);
			}
		}
		return;
	}

	if (Mode == TEXT("floatingcombattext") || Mode == TEXT("combattext") || Mode == TEXT("damagetext"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		AActor* TextTarget = GetPawn();
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66WidgetDump_FloatingCombatTextTarget"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
				AT66EnemyBase::StaticClass(),
				GetPawn() ? GetPawn()->GetActorLocation() + FVector(420.f, 0.f, 0.f) : FVector(420.f, 0.f, 0.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (Enemy)
			{
				Enemy->SetActorEnableCollision(false);
				TextTarget = Enemy;
			}
		}

		if (UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
		{
			FloatingText->ShowDamageNumber(TextTarget, 666, UT66FloatingCombatTextSubsystem::EventType_Crit);
		}
		return;
	}

#if !UE_BUILD_SHIPPING
	if (Mode == TEXT("mobvatqa") || Mode == TEXT("easymobvat") || Mode == TEXT("easymobvatqa"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (UWorld* World = GetWorld())
		{
			const TArray<FName> EasyMobIDs = {
				FName(TEXT("Slime")),
				FName(TEXT("BoneWalker")),
				FName(TEXT("RatPack")),
				FName(TEXT("CaveBat")),
				FName(TEXT("HexSlinger")),
				FName(TEXT("TombSpider")),
				FName(TEXT("StoneSentinel")),
				FName(TEXT("MimicLure")),
				FName(TEXT("BoneConjurer")),
				FName(TEXT("CryptWraith"))
			};
			const TArray<FName> ClipNames = {
				FName(TEXT("Idle")),
				FName(TEXT("Move")),
				FName(TEXT("AttackCue")),
				FName(TEXT("HitReact")),
				FName(TEXT("Death"))
			};

			const FVector Origin = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			for (int32 Index = 0; Index < EasyMobIDs.Num(); ++Index)
			{
				const int32 Row = Index / 5;
				const int32 Col = Index % 5;
				const FVector QAEnemyLocation = Origin + FVector(480.f + static_cast<float>(Row) * 300.f, (static_cast<float>(Col) - 4.f) * 220.f, 0.f);
				FRotator QAEnemyRotation = (Origin - QAEnemyLocation).Rotation();
				QAEnemyRotation.Pitch = 0.f;
				QAEnemyRotation.Roll = 0.f;
				SpawnParams.Name = FName(*FString::Printf(TEXT("T66MobVATQA_%s"), *EasyMobIDs[Index].ToString()));

				AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
					AT66EnemyBase::StaticClass(),
					QAEnemyLocation,
					QAEnemyRotation,
					SpawnParams);
				if (!Enemy)
				{
					UE_LOG(LogTemp, Error, TEXT("Mob VAT QA failed to spawn %s"), *EasyMobIDs[Index].ToString());
					continue;
				}

				Enemy->Tags.AddUnique(FName(TEXT("T66Automation_EasyMobVATQA")));
				Enemy->SetActorEnableCollision(false);
				Enemy->MaxHP = 20000;
				Enemy->CurrentHP = 20000;
				Enemy->TouchDamageHearts = 0;
				Enemy->PointValue = 0;
				Enemy->XPValue = 0;
				Enemy->bDropsLoot = false;
				Enemy->OwningDirector = nullptr;
				Enemy->ConfigureAsMob(EasyMobIDs[Index]);
				if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->DisableMovement();
				}

				const FName ClipName = ClipNames[Index % ClipNames.Num()];
				Enemy->ForceMobVertexAnimationClipForAutomation(ClipName, 30.f);
				UE_LOG(LogTemp, Display, TEXT("Mob VAT QA spawned %s clip=%s location=%s"),
					*EasyMobIDs[Index].ToString(),
					*ClipName.ToString(),
					*QAEnemyLocation.ToCompactString());
			}
		}
		return;
	}
#endif

	if (Mode == TEXT("scopedsniper") || Mode == TEXT("sniperscope") || Mode == TEXT("scopeoverlay"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
		{
			if (UT66CombatComponent* Combat = Hero->FindComponentByClass<UT66CombatComponent>())
			{
				ActivateHeroOneScopedUlt(Hero, Combat);
				SetHeroOneScopeViewEnabled(true);
			}
		}
		return;
	}

	if (Mode == TEXT("cowardiceprompt") || Mode == TEXT("cowardice") || Mode == TEXT("cowardicegate"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		AT66CowardiceGate* AutomationGate = nullptr;
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66WidgetDump_CowardiceGate"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AutomationGate = World->SpawnActor<AT66CowardiceGate>(
				AT66CowardiceGate::StaticClass(),
				GetPawn() ? GetPawn()->GetActorLocation() + FVector(360.f, 0.f, 0.f) : FVector(360.f, 0.f, 0.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (AutomationGate)
			{
				AutomationGate->SetActorHiddenInGame(true);
				AutomationGate->SetActorEnableCollision(false);
			}
		}

		OpenCowardicePrompt(AutomationGate);
		return;
	}

	if (Mode == TEXT("loading") || Mode == TEXT("loadingscreen") || Mode == TEXT("transitionloading"))
	{
		if (UT66LoadingScreenWidget* LoadingOverlay = CreateWidget<UT66LoadingScreenWidget>(this, UT66LoadingScreenWidget::StaticClass()))
		{
			LoadingOverlay->SetLoadingText(NSLOCTEXT("T66.Loading", "WidgetDumpLoadingText", "Loading"));
			LoadingOverlay->AddToViewport(10000);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("idol") || Mode == TEXT("idolaltar"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		AT66IdolAltar* AutomationAltar = nullptr;
		if (UWorld* World = GetWorld())
		{
			const FVector AutomationAltarLocation = GetPawn() ? GetPawn()->GetActorLocation() + FVector(300.f, 0.f, 0.f) : FVector::ZeroVector;
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AutomationAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), AutomationAltarLocation, FRotator::ZeroRotator, SpawnParams);
			if (AutomationAltar)
			{
				AutomationAltar->SetActorHiddenInGame(true);
				AutomationAltar->SetActorEnableCollision(false);
				AutomationAltar->RemainingSelections = FMath::Max(1, AutomationAltar->RemainingSelections);
			}
		}

		UT66IdolAltarOverlayWidget* W = CreateWidget<UT66IdolAltarOverlayWidget>(this, ResolveIdolAltarOverlayClass());
		if (W)
		{
			W->SetSourceAltar(AutomationAltar);
			IdolAltarOverlayWidget = W;
			W->AddToViewport(150);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("casinoshop") || Mode == TEXT("shop") || Mode == TEXT("casinotabshop"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToShopTab();
		return;
	}

	if (Mode == TEXT("casinogambling") || Mode == TEXT("gambling") || Mode == TEXT("casinotabgambling"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToGambling();
		return;
	}

	if (Mode == TEXT("casinoalchemy") || Mode == TEXT("alchemy") || Mode == TEXT("casinotabalchemy"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToShopTab();
		return;
	}

	if (Mode == TEXT("collector") || Mode == TEXT("collectoroverlay"))
	{
		OpenCollectorOverlay();
		return;
	}

	if (Mode == TEXT("lab") || Mode == TEXT("laboverlay"))
	{
		if (!LabOverlayWidget)
		{
			LabOverlayWidget = CreateWidget<UT66LabOverlayWidget>(this, UT66LabOverlayWidget::StaticClass());
		}
		if (LabOverlayWidget && !LabOverlayWidget->IsInViewport())
		{
			LabOverlayWidget->AddToViewport(100);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("crate") || Mode == TEXT("crateoverlay"))
	{
		if (UT66CrateOverlayWidget* CrateOverlay = CreateWidget<UT66CrateOverlayWidget>(this, UT66CrateOverlayWidget::StaticClass()))
		{
			CrateOverlay->SetPresentationHost(GameplayHUDWidget);
			CrateOverlay->AddToViewport(110);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("arcadeselector") || Mode == TEXT("arcade") || Mode == TEXT("arcadecabinet"))
	{
		if (AT66ArcadeMachineInteractable* AutomationArcade = GetWorld()->SpawnActor<AT66ArcadeMachineInteractable>(
			AT66ArcadeMachineInteractable::StaticClass(),
			GetPawn() ? GetPawn()->GetActorLocation() + FVector(360.f, 0.f, 0.f) : FVector::ZeroVector,
			FRotator::ZeroRotator))
		{
			AutomationArcade->SetActorHiddenInGame(true);
			AutomationArcade->SetActorEnableCollision(false);
			OpenArcadePopup(AutomationArcade->GetArcadeData(), AutomationArcade);
		}
		return;
	}

	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->SetFullMapOpen(false);
	}
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}
}

void AT66PlayerController::HandleGameplayAutomationScreenshot()
{
	if (GameplayAutomationScreenshotPath.IsEmpty())
	{
		return;
	}

	if (GameplayHUDWidget)
	{
		FString PickupCardItemIDString;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoPickupCard="), PickupCardItemIDString)
			&& !PickupCardItemIDString.TrimStartAndEnd().IsEmpty())
		{
			ET66ItemRarity PickupCardRarity = ET66ItemRarity::Yellow;
			FString PickupCardRarityString;
			if (FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoPickupCardRarity="), PickupCardRarityString))
			{
				const FString NormalizedRarity = PickupCardRarityString.TrimStartAndEnd().ToLower();
				if (NormalizedRarity == TEXT("black"))
				{
					PickupCardRarity = ET66ItemRarity::Black;
				}
				else if (NormalizedRarity == TEXT("red"))
				{
					PickupCardRarity = ET66ItemRarity::Red;
				}
				else if (NormalizedRarity == TEXT("white"))
				{
					PickupCardRarity = ET66ItemRarity::White;
				}
			}

			GameplayHUDWidget->ShowPickupItemCard(FName(*PickupCardItemIDString.TrimStartAndEnd()), PickupCardRarity);
		}
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(GameplayAutomationScreenshotPath), true);
	FScreenshotRequest::RequestScreenshot(GameplayAutomationScreenshotPath, true, false, false);

	if (!bGameplayAutomationKeepAliveAfterScreenshot && GetWorld())
	{
		GetWorldTimerManager().ClearTimer(GameplayAutomationQuitTimerHandle);
		GetWorldTimerManager().SetTimer(
			GameplayAutomationQuitTimerHandle,
			this,
			&AT66PlayerController::HandleGameplayAutomationQuit,
			1.5f,
			false);
	}
}

void AT66PlayerController::HandleGameplayAutomationWidgetDump()
{
	if (GameplayAutomationWidgetDumpTarget.IsEmpty() || GameplayAutomationWidgetDumpPath.IsEmpty())
	{
		return;
	}

	FString Error;
	const bool bDumped = FT66WidgetDumpTargets::DumpTargetToJson(
		GetWorld(),
		GameplayAutomationWidgetDumpTarget,
		GameplayAutomationWidgetDumpPath,
		Error);

	if (bDumped)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("Gameplay automation: widget target dump wrote Target=%s Path=%s"),
			*GameplayAutomationWidgetDumpTarget,
			*GameplayAutomationWidgetDumpPath);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Gameplay automation: widget target dump failed Target=%s Path=%s Error=%s"),
			*GameplayAutomationWidgetDumpTarget,
			*GameplayAutomationWidgetDumpPath,
			*Error);
	}

	if (GameplayAutomationScreenshotPath.IsEmpty()
		&& !bGameplayAutomationKeepAliveAfterScreenshot
		&& GetWorld())
	{
		GetWorldTimerManager().ClearTimer(GameplayAutomationQuitTimerHandle);
		GetWorldTimerManager().SetTimer(
			GameplayAutomationQuitTimerHandle,
			this,
			&AT66PlayerController::HandleGameplayAutomationQuit,
			1.5f,
			false);
	}
}

void AT66PlayerController::HandleGameplayAutomationQuit()
{
	ConsoleCommand(TEXT("quit"));
}

void AT66PlayerController::RebuildThemeAwareUI()
{
	if (UIManager)
	{
		UIManager->RebuildAllVisibleUI();
	}

	if (GameplayHUDWidget && GameplayHUDWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(GameplayHUDWidget, 0);
	}
	if (CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CasinoOverlayWidget, 100);
	}
	if (CollectorOverlayWidget && CollectorOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CollectorOverlayWidget, 100);
	}
	if (CowardicePromptWidget && CowardicePromptWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CowardicePromptWidget, 200);
	}
	if (IdolAltarOverlayWidget && IdolAltarOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(IdolAltarOverlayWidget, 150);
	}
}


void AT66PlayerController::OpenCasinoOverlay()
{
	if (!IsGameplayLevel()) return;
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CasinoOverlayWidget)
	{
		CasinoOverlayWidget = CreateWidget<UT66CasinoOverlayWidget>(this, ResolveCasinoOverlayClass());
	}

	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->OpenShopTab();
		if (!CasinoOverlayWidget->IsInViewport())
		{
			CasinoOverlayWidget->AddToViewport(100);
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::CloseCasinoOverlay()
{
	if (CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->RemoveFromParent();
	}
	RestoreGameplayInputMode();
}

void AT66PlayerController::SwitchCasinoOverlayToGambling()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->OpenGamblingTab();
	}
}

void AT66PlayerController::SwitchCasinoOverlayToShopTab()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->OpenShopTab();
	}
}

void AT66PlayerController::SwitchCasinoOverlayToAlchemy()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->OpenShopTab();
	}
}

bool AT66PlayerController::IsCasinoOverlayOpen() const
{
	return CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport();
}

bool AT66PlayerController::TriggerCasinoBossIfAngry()
{
	if (!IsGameplayLevel()) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || RunState->GetCasinoAnger01() < 1.f || RunState->GetBossActive())
	{
		return false;
	}

	if (T66HasRegisteredGamblerBoss(World))
	{
		return false;
	}

	FVector SpawnLoc = FVector::ZeroVector;
	bool bHasSpawnLoc = false;

	if (AT66GamblerNPC* Gambler = T66FindRegisteredNpc<AT66GamblerNPC>(World))
	{
		SpawnLoc = Gambler->GetActorLocation();
		bHasSpawnLoc = true;
		Gambler->Destroy();
	}

	if (!bHasSpawnLoc)
	{
		if (APawn* FallbackPawn = GetPawn())
		{
			SpawnLoc = FallbackPawn->GetActorLocation() + FVector(600.f, 0.f, 0.f);
			bHasSpawnLoc = true;
		}
	}
	if (!bHasSpawnLoc)
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66GamblerBoss>(AT66GamblerBoss::StaticClass(), SpawnLoc + FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator, Params);

	if (CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->RemoveFromParent();
	}
	RestoreGameplayInputMode();
	return true;
}

void AT66PlayerController::OpenCasinoShopTab()
{
	if (!IsGameplayLevel()) return;

	OpenCasinoOverlay();
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetShopAllowsSteal(false);
		CasinoOverlayWidget->OpenShopTab();
	}
}


void AT66PlayerController::OpenCollectorOverlay()
{
	if (!IsGameplayLevel()) return;
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CollectorOverlayWidget)
	{
		CollectorOverlayWidget = CreateWidget<UT66CollectorOverlayWidget>(this, ResolveCollectorOverlayClass());
	}

	if (CollectorOverlayWidget && !CollectorOverlayWidget->IsInViewport())
	{
		CollectorOverlayWidget->AddToViewport(100);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

bool AT66PlayerController::OpenArcadePopup(
	const FT66ArcadeInteractableData& ArcadeData,
	AT66ArcadeInteractableBase* SourceInteractable)
{
	if (!IsGameplayLevel()
		|| ArcadeData.ArcadeClass != ET66ArcadeInteractableClass::PopupArcade
		|| IsArcadePopupOpen()
		|| !CanUseCombatMouseInput())
	{
		return false;
	}

	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!SpawnArcadePopupWidget(ArcadeData, SourceInteractable))
	{
		return false;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	return true;
}

bool AT66PlayerController::SpawnArcadePopupWidget(
	const FT66ArcadeInteractableData& ArcadeData,
	AT66ArcadeInteractableBase* SourceInteractable)
{
	UT66ArcadePopupWidget* NewPopup = nullptr;
	TSubclassOf<UT66ArcadePopupWidget> PopupWidgetClass = ArcadeData.PopupWidgetClass;
	if (!PopupWidgetClass)
	{
		switch (ArcadeData.ArcadeGameType)
		{
		case ET66ArcadeGameType::WhackAMole:
			PopupWidgetClass = UT66WhackAMoleArcadeWidget::StaticClass();
			break;

		case ET66ArcadeGameType::Topwar:
			PopupWidgetClass = UT66TopwarArcadeWidget::StaticClass();
			break;

		case ET66ArcadeGameType::GoldMiner:
			PopupWidgetClass = UT66GoldMinerArcadeWidget::StaticClass();
			break;

		case ET66ArcadeGameType::Random:
			PopupWidgetClass = UT66ArcadeSelectionWidget::StaticClass();
			break;

		case ET66ArcadeGameType::RuneSwipe:
		case ET66ArcadeGameType::CartSwitcher:
		case ET66ArcadeGameType::CrystalDash:
		case ET66ArcadeGameType::PotionPour:
		case ET66ArcadeGameType::RelicStack:
		case ET66ArcadeGameType::ShieldParry:
		case ET66ArcadeGameType::MimicMemory:
		case ET66ArcadeGameType::BombSorter:
		case ET66ArcadeGameType::LanternLeap:
		case ET66ArcadeGameType::BladeSweep:
			PopupWidgetClass = UT66QuickArcadeWidget::StaticClass();
			break;

		case ET66ArcadeGameType::None:
		default:
			break;
		}
	}

	if (PopupWidgetClass)
	{
		NewPopup = CreateWidget<UT66ArcadePopupWidget>(this, PopupWidgetClass);
	}

	if (!NewPopup)
	{
		return false;
	}

	NewPopup->InitializeArcadePopup(ArcadeData, SourceInteractable);
	ArcadePopupWidget = NewPopup;
	ArcadePopupWidget->AddToViewport(110);
	return true;
}

void AT66PlayerController::HandleArcadeGameSelected(
	UT66ArcadePopupWidget* SelectorWidget,
	const FT66ArcadeInteractableData& SelectedGameData)
{
	if (!SelectorWidget || SelectorWidget != ArcadePopupWidget)
	{
		return;
	}

	AT66ArcadeInteractableBase* SourceInteractable = SelectorWidget->GetSourceInteractable();
	if (SelectorWidget->IsInViewport())
	{
		SelectorWidget->RemoveFromParent();
	}
	ArcadePopupWidget = nullptr;

	if (!SpawnArcadePopupWidget(SelectedGameData, SourceInteractable))
	{
		if (SourceInteractable)
		{
			SourceInteractable->HandleArcadePopupDismissedWithoutResult();
		}
		RestoreGameplayInputMode();
	}
}

void AT66PlayerController::HandleArcadePopupResult(UT66ArcadePopupWidget* PopupWidget, const bool bSucceeded, const int32 FinalScore)
{
	if (PopupWidget != ArcadePopupWidget)
	{
		return;
	}

	CloseArcadePopup(bSucceeded, FinalScore);
}

void AT66PlayerController::CloseArcadePopup(const bool bSucceeded, const int32 FinalScore)
{
	UT66ArcadePopupWidget* PopupWidget = ArcadePopupWidget;
	if (!PopupWidget)
	{
		return;
	}

	AT66ArcadeInteractableBase* SourceInteractable = PopupWidget->GetSourceInteractable();
	ArcadePopupWidget = nullptr;

	if (PopupWidget->IsInViewport())
	{
		PopupWidget->RemoveFromParent();
	}

	if (SourceInteractable)
	{
		if (PopupWidget->ReportsArcadeResult())
		{
			SourceInteractable->HandleArcadePopupClosed(bSucceeded, FinalScore);
		}
		else
		{
			SourceInteractable->HandleArcadePopupDismissedWithoutResult();
		}
	}

	if (!IsPaused() && !(UIManager && UIManager->IsModalActive()))
	{
		RestoreGameplayInputMode();
	}
}

bool AT66PlayerController::IsArcadePopupOpen() const
{
	return ArcadePopupWidget && ArcadePopupWidget->IsInViewport();
}


void AT66PlayerController::OpenCowardicePrompt(AT66CowardiceGate* Gate)
{
	if (!IsGameplayLevel() || !Gate) return;
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CowardicePromptWidget)
	{
		CowardicePromptWidget = CreateWidget<UT66CowardicePromptWidget>(this, ResolveCowardicePromptClass());
	}

	if (CowardicePromptWidget && !CowardicePromptWidget->IsInViewport())
	{
		CowardicePromptWidget->SetGate(Gate);
		CowardicePromptWidget->AddToViewport(200); // above gambler overlay
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::StartCrateOpenHUD()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->StartCrateOpen();
	}
}

void AT66PlayerController::StartChestRewardHUD(ET66Rarity Rarity, int32 GoldAmount)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->StartChestReward(Rarity, GoldAmount);
	}
}


void AT66PlayerController::OnPlayerDied()
{
	EndHeroOneScopedUlt();
	CloseArcadePopup(false);

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
			}
		}
	}

	// Spawn death VFX (red pixel burst) at hero location before pausing.
	if (APawn* HeroPawn = GetPawn())
	{
		const FVector DeathLoc = HeroPawn->GetActorLocation();
		if (UT66CombatComponent* Combat = HeroPawn->FindComponentByClass<UT66CombatComponent>())
		{
			Combat->SpawnDeathVFX(DeathLoc);
		}
		HeroPawn->SetActorHiddenInGame(true);
	}

	// Brief delay so the death particles are visible before the game pauses.
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(DeathVFXTimerHandle, [this]()
		{
			SetPause(true);
			EnsureGameplayUIManager();
			if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
			{
				UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
				if (RunState)
				{
					RunState->MarkRunEnded(false);
				}
				if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
				{
					Achieve->NotifyRunCompleted(RunState);
				}
			}
			if (UIManager)
			{
				UIManager->ShowModal(ET66ScreenType::RunSummary);
			}
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}, 0.5f, false);
	}
	else
	{
		SetPause(true);
		EnsureGameplayUIManager();
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->MarkRunEnded(false);
			}
		}
		if (UIManager)
		{
			UIManager->ShowModal(ET66ScreenType::RunSummary);
		}
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::ShowVictoryRunSummary()
{
	EndHeroOneScopedUlt();
	CloseArcadePopup(false);

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
			}
		}

		UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		if (RunState)
		{
			RunState->MarkRunEnded(true);
		}
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->NotifyRunCompleted(RunState);
		}
	}

	SetPause(true);
	EnsureGameplayUIManager();
	if (UIManager)
	{
		UIManager->ShowModal(ET66ScreenType::RunSummary);
	}
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AT66PlayerController::ClientShowVictoryRunSummary_Implementation()
{
	ShowVictoryRunSummary();
}

void AT66PlayerController::ShowDifficultyClearSummary()
{
	EndHeroOneScopedUlt();
	CloseArcadePopup(false);

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
			}
		}
	}

	SetPause(true);
	EnsureGameplayUIManager();
	if (UIManager)
	{
		UIManager->ShowModal(ET66ScreenType::RunSummary);
	}
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AT66PlayerController::ClientShowDifficultyClearSummary_Implementation()
{
	ShowDifficultyClearSummary();
}


void AT66PlayerController::HandleQuickReviveStateChanged()
{
	if (!IsGameplayLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const bool bDowned = RunState && RunState->IsInQuickReviveDownedState();

	if (bDowned)
	{
		EndHeroOneScopedUlt();
	}

	if (bDowned == bQuickReviveInputSuppressed)
	{
		return;
	}

	bQuickReviveInputSuppressed = bDowned;
	SetIgnoreMoveInput(bDowned);
	SetIgnoreLookInput(bDowned);

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
	{
		Hero->SetQuickReviveDowned(bDowned);

		if (UT66CombatComponent* Combat = Hero->FindComponentByClass<UT66CombatComponent>())
		{
			if (bDowned)
			{
				Combat->SetAutoAttackSuppressed(true);
				Combat->ClearLockedTarget();
			}
			else if (!bHeroOneScopedUltActive && !Hero->IsVehicleMounted())
			{
				Combat->SetAutoAttackSuppressed(false);
			}
		}
	}

	SetLockedCombatTarget(nullptr, false);
}


void AT66PlayerController::EnsureGameplayUIManager()
{
	if (UIManager || !IsGameplayLevel() || !IsLocalController()) return;

	UIManager = NewObject<UT66UIManager>(this, UT66UIManager::StaticClass());
	if (!UIManager) return;

	UIManager->Initialize(this);
	if (TSubclassOf<UT66ScreenBase> PauseClass = ResolveScreenClass(ET66ScreenType::PauseMenu))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PauseMenu, PauseClass);
	}
	if (TSubclassOf<UT66ScreenBase> AchievementsClass = ResolveScreenClass(ET66ScreenType::Achievements))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Achievements, AchievementsClass);
	}
	if (TSubclassOf<UT66ScreenBase> ReportBugClass = ResolveScreenClass(ET66ScreenType::ReportBug))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::ReportBug, ReportBugClass);
	}
	if (TSubclassOf<UT66ScreenBase> SettingsClass = ResolveScreenClass(ET66ScreenType::Settings))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Settings, SettingsClass);
	}
	if (TSubclassOf<UT66ScreenBase> RunSummaryClass = ResolveScreenClass(ET66ScreenType::RunSummary))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::RunSummary, RunSummaryClass);
	}
	if (TSubclassOf<UT66ScreenBase> SummaryPickerClass = ResolveScreenClass(ET66ScreenType::PlayerSummaryPicker))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PlayerSummaryPicker, SummaryPickerClass);
	}
	if (TSubclassOf<UT66ScreenBase> SavePreviewClass = ResolveScreenClass(ET66ScreenType::SavePreview))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::SavePreview, SavePreviewClass);
	}
	if (TSubclassOf<UT66ScreenBase> ShopClass = ResolveScreenClass(ET66ScreenType::PowerUp))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PowerUp, ShopClass);
	}
	if (TSubclassOf<UT66ScreenBase> AccountStatusClass = ResolveScreenClass(ET66ScreenType::AccountStatus))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::AccountStatus, AccountStatusClass);
	}
	if (TSubclassOf<UT66ScreenBase> PartyInviteClass = ResolveScreenClass(ET66ScreenType::PartyInvite))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PartyInvite, PartyInviteClass);
	}
}


void AT66PlayerController::HandleEscapePressed()
{
	if (!IsGameplayLevel())
	{
		if (UIManager)
		{
			UIManager->HandleBackAction();
		}
		return;
	}

#if !UE_BUILD_SHIPPING
	// Highest priority: if dev console is open, Esc should close it and never open PauseMenu.
	if (bDevConsoleOpen)
	{
		CloseDevConsole();

		// Debounce to avoid the same keypress immediately toggling PauseMenu.
		const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
		LastPauseToggleTime = Now;
		return;
	}
#endif

	if (IsPaused()
		&& UIManager
		&& UIManager->IsModalActive()
		&& UIManager->GetCurrentModalType() == ET66ScreenType::SavePreview)
	{
		if (UT66SavePreviewScreen* SavePreviewScreen = Cast<UT66SavePreviewScreen>(UIManager->GetCurrentModal()))
		{
			SavePreviewScreen->OnBackClicked();
		}
		return;
	}

	// Esc as back: close Media Viewer before other UI.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
				SetIgnoreMoveInput(false);
				SetIgnoreLookInput(false);

				const bool bPauseMenuStillOpen = IsPaused()
					&& UIManager
					&& UIManager->IsModalActive()
					&& UIManager->GetCurrentModalType() == ET66ScreenType::PauseMenu;
				if (bPauseMenuStillOpen)
				{
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
				else
				{
					RestoreGameplayInputMode();
				}
				return;
			}
		}
	}

	// Esc as back: close the full map before opening pause menu.
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
		return;
	}

	if (IsArcadePopupOpen())
	{
		CloseArcadePopup(false);
		return;
	}

	if (GameplayHUDWidget && GameplayHUDWidget->IsFullMapOpen())
	{
		GameplayHUDWidget->SetFullMapOpen(false);
		RestoreGameplayInputMode();
		return;
	}

	const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
	const bool bPaused = IsPaused();

	// Closing a sub-modal (Settings / Report Bug / Achievements) returns to Pause menu without debounce so the next Esc can unpause.
	if (bPaused && UIManager && UIManager->IsModalActive())
	{
		const ET66ScreenType ClosingModal = UIManager->GetCurrentModalType();
		if (ClosingModal == ET66ScreenType::Settings
			|| ClosingModal == ET66ScreenType::ReportBug
			|| ClosingModal == ET66ScreenType::Achievements
			|| ClosingModal == ET66ScreenType::PlayerSummaryPicker
			|| ClosingModal == ET66ScreenType::AccountStatus
			|| ClosingModal == ET66ScreenType::RunSummary)
		{
			UIManager->CloseModal();

			if (ClosingModal == ET66ScreenType::RunSummary)
			{
				if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
				{
					if (UT66LeaderboardSubsystem* LB = GI->GetSubsystem<UT66LeaderboardSubsystem>())
					{
						const ET66ScreenType ReturnModal = LB->ConsumePendingReturnModalAfterViewerRunSummary();
						if (ReturnModal != ET66ScreenType::None)
						{
							UIManager->ShowModal(ReturnModal);
							return;
						}
					}
				}
			}

			UIManager->ShowModal(ET66ScreenType::PauseMenu);
			return;
		}
	}

	// Debounce only when actually toggling pause (open or full unpause)
	if (Now - LastPauseToggleTime < PauseToggleDebounceSeconds)
	{
		return;
	}
	LastPauseToggleTime = Now;

	if (!bPaused)
	{
		SetPause(true);
		EnsureGameplayUIManager();
		if (UIManager)
		{
			UIManager->ShowModal(ET66ScreenType::PauseMenu);
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
	else
	{
		if (UIManager && UIManager->IsModalActive())
		{
			UIManager->CloseModal();
			SetPause(false);
			RestoreGameplayInputMode();
		}
		else
		{
			SetPause(false);
			RestoreGameplayInputMode();
		}
	}
}
