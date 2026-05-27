// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Animation/AnimSequence.h"
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
#include "UI/T66WeaponAltarOverlayWidget.h"
#include "UI/T66CollectorOverlayWidget.h"
#include "UI/T66LoadingScreenWidget.h"
#include "UI/T66ArcadePopupWidget.h"
#include "UI/T66ArcadeSelectionWidget.h"
#include "UI/T66CrateOverlayWidget.h"
#include "UI/T66GoldMinerArcadeWidget.h"
#include "UI/T66BladeSweepArcadeWidget.h"
#include "UI/T66TopwarArcadeWidget.h"
#include "UI/T66WhackAMoleArcadeWidget.h"
#include "UI/WidgetGames/T66WidgetGameHostContext.h"
#include "UI/WidgetGames/T66WidgetGameResult.h"
#include "UI/WidgetGames/T66WidgetGameRegistry.h"
#include "Gameplay/T66FountainInteractable.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66LootWheelInteractable.h"
#include "Gameplay/T66ArcadeInteractableBase.h"
#include "Gameplay/T66ArcadeMachineInteractable.h"
#include "Gameplay/T66CasinoInteractable.h"
#include "Gameplay/T66VendorInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "Gameplay/T66TutorialGate.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66DeprecatedFeatureSettings.h"
#include "Core/T66GameInstance.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66WeaponManagerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66WeaponAltar.h"
#include "TimerManager.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Gameplay/T66UniqueDebuffProjectile.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"
#include "Gameplay/Enemies/Projectiles/T66SpitProjectile.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66BossGroundAOE.h"
#include "Gameplay/T66Hero1AxeAOEVFXLabActor.h"
#include "Gameplay/T66HeroPlagueCloud.h"
#include "Gameplay/Traps/T66FloorFlameTrap.h"
#include "Gameplay/Traps/T66FloorSpikePatchTrap.h"
#include "Gameplay/Traps/T66TrapArrowProjectile.h"
#include "Gameplay/Traps/T66TrapPressurePlate.h"
#include "Data/T66DataTypes.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/GameViewportClient.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
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

#if WITH_EDITOR
#include "Animation/AnimData/IAnimationDataModel.h"
#endif

namespace
{
	const FName T66GameplayAutomationTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	const FName T66GameplayAutomationTraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
	const FName T66GameplayAutomationCameraWallVisualTag(TEXT("T66_CameraOccludingWallVisual"));
	const FName T66GameplayAutomationTowerCeilingTag(TEXT("T66_Tower_Ceiling"));

	static TAutoConsoleVariable<float> CVarT66AutoCaptureHeroHPOverride(
		TEXT("T66.AutoCapture.HeroHPOverride"),
		0.0f,
		TEXT("Automation-only measurement hook. When >0 and an autocapture performance mode is active, starts the hero at this HP. Default 0 has no effect."),
#if UE_BUILD_SHIPPING
		ECVF_ReadOnly
#else
		ECVF_Default
#endif
	);

	template <typename TInteractableType>
	TInteractableType* T66FindRegisteredWorldInteractable(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66WorldInteractableBase>& WeakInteractable : Registry->GetWorldInteractables())
			{
				if (TInteractableType* Interactable = Cast<TInteractableType>(WeakInteractable.Get()))
				{
					return Interactable;
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

	FString T66DescribeMovementQAAnimation(const TCHAR* Label, UAnimationAsset* Animation)
	{
		if (!Animation)
		{
			return FString::Printf(TEXT("%s=None"), Label ? Label : TEXT("Anim"));
		}

		const float PlayLength = Animation->GetPlayLength();
		UAnimSequence* Sequence = Cast<UAnimSequence>(Animation);
		if (!Sequence)
		{
			return FString::Printf(
				TEXT("%s=%s class=%s playLength=%.3f rootMotion=UnsupportedNonSequence"),
				Label ? Label : TEXT("Anim"),
				*GetNameSafe(Animation),
				*GetNameSafe(Animation->GetClass()),
				PlayLength);
		}

		const FTransform RootMotion = Sequence->ExtractRootMotionFromRange(0.0, static_cast<double>(PlayLength), FAnimExtractContext());
		const FVector RootMotionTranslation = RootMotion.GetTranslation();
		FString RootTrackSummary(TEXT("rootTrack=Unavailable"));
#if WITH_EDITOR
		if (const IAnimationDataModel* DataModel = Sequence->GetDataModel())
		{
			FName RootTrackName(NAME_None);
			TArray<FName> TrackNames;
			DataModel->GetBoneTrackNames(TrackNames);
			if (TrackNames.Contains(FName(TEXT("root"))))
			{
				RootTrackName = FName(TEXT("root"));
			}
			else if (TrackNames.Num() > 0)
			{
				RootTrackName = TrackNames[0];
			}

			if (!RootTrackName.IsNone() && DataModel->IsValidBoneTrackName(RootTrackName))
			{
				const int32 LastKeyIndex = FMath::Max(0, DataModel->GetNumberOfKeys() - 1);
				const FTransform FirstRoot = DataModel->GetBoneTrackTransform(RootTrackName, FFrameNumber(0));
				const FTransform LastRoot = DataModel->GetBoneTrackTransform(RootTrackName, FFrameNumber(LastKeyIndex));
				const FVector RootTrackDelta = LastRoot.GetTranslation() - FirstRoot.GetTranslation();
				RootTrackSummary = FString::Printf(
					TEXT("rootTrack=%s rootTrackDelta=%s rootTrackDelta2D=%.3f keys=%d frames=%d"),
					*RootTrackName.ToString(),
					*RootTrackDelta.ToCompactString(),
					RootTrackDelta.Size2D(),
					DataModel->GetNumberOfKeys(),
					DataModel->GetNumberOfFrames());
			}
		}
#endif

		return FString::Printf(
			TEXT("%s=%s class=%s playLength=%.3f bEnableRootMotion=%d hasRootMotion=%d extractedRootMotion=%s extractedRootMotion2D=%.3f %s"),
			Label ? Label : TEXT("Anim"),
			*GetNameSafe(Animation),
			*GetNameSafe(Animation->GetClass()),
			PlayLength,
			Sequence->bEnableRootMotion ? 1 : 0,
			Sequence->HasRootMotion() ? 1 : 0,
			*RootMotionTranslation.ToCompactString(),
			RootMotionTranslation.Size2D(),
			*RootTrackSummary);
	}

	FName T66GetHeroMovementQAVisualID()
	{
		FString VisualIDString;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQAVisualID="), VisualIDString)
			&& !VisualIDString.TrimStartAndEnd().IsEmpty())
		{
			return FName(*VisualIDString.TrimStartAndEnd());
		}
		if (FParse::Value(FCommandLine::Get(), TEXT("T66HeroVisualOverride="), VisualIDString)
			&& !VisualIDString.TrimStartAndEnd().IsEmpty())
		{
			return FName(*VisualIDString.TrimStartAndEnd());
		}
		return FName(TEXT("Hero_1_Chad"));
	}

	FName T66GetHeroMovementQACompanionID()
	{
		FString CompanionIDString(TEXT("Companion_01"));
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQACompanionID="), CompanionIDString);
		CompanionIDString = CompanionIDString.TrimStartAndEnd();
		return CompanionIDString.Equals(TEXT("None"), ESearchCase::IgnoreCase) || CompanionIDString.IsEmpty()
			? NAME_None
			: FName(*CompanionIDString);
	}
}


void AT66PlayerController::SetupGameplayHUD()
{
	if (!IsGameplayLevel() || !IsLocalController()) return;
	if (GameplayHUDWidget)
	{
		if (!GameplayHUDWidget->IsInViewport())
		{
			GameplayHUDWidget->AddToViewport(0);
		}
		GameplayHUDWidget->MarkHUDDirty();
		QueueGameplayAutomationScreenshotIfRequested();
		return;
	}

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

	FString RequestedScreenshotSequenceDir;
	const bool bScreenshotSequenceRequested = FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshotSequenceDir="), RequestedScreenshotSequenceDir);

	FString RequestedWidgetDumpSpec;
	const bool bWidgetDumpRequested = FParse::Value(FCommandLine::Get(), TEXT("T66AutoDumpWidget="), RequestedWidgetDumpSpec);
	if (!bScreenshotRequested && !bScreenshotSequenceRequested && !bWidgetDumpRequested)
	{
		return;
	}

	GameplayAutomationScreenshotPath.Reset();
	GameplayAutomationScreenshotSequenceDir.Reset();
	GameplayAutomationScreenshotSequencePrefix = TEXT("frame");
	GameplayAutomationScreenshotSequenceCount = 0;
	GameplayAutomationScreenshotSequenceIndex = 0;
	GameplayAutomationScreenshotSequenceIntervalSeconds = 0.1f;
	GameplayAutomationWidgetDumpTarget.Reset();
	GameplayAutomationWidgetDumpPath.Reset();
	GameplayAutomationScreenshotDelaySeconds = 4.0f;
	GameplayAutomationWidgetDumpDelaySeconds = 4.0f;
	GameplayAutomationPostCaptureScreenshotDelaySeconds = 0.5f;
	if (bScreenshotRequested)
	{
		GameplayAutomationScreenshotPath = FPaths::ConvertRelativePathToFull(RequestedScreenshotPath);
		FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshotDelay="), GameplayAutomationScreenshotDelaySeconds);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoPostCaptureScreenshotDelay="),
			GameplayAutomationPostCaptureScreenshotDelaySeconds);
	}
	if (bScreenshotSequenceRequested)
	{
		GameplayAutomationScreenshotSequenceDir = FPaths::ConvertRelativePathToFull(RequestedScreenshotSequenceDir);
		FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshotDelay="), GameplayAutomationScreenshotDelaySeconds);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoPostCaptureScreenshotDelay="),
			GameplayAutomationPostCaptureScreenshotDelaySeconds);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoScreenshotSequenceCount="),
			GameplayAutomationScreenshotSequenceCount);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoScreenshotSequenceInterval="),
			GameplayAutomationScreenshotSequenceIntervalSeconds);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoScreenshotSequencePrefix="),
			GameplayAutomationScreenshotSequencePrefix);
		GameplayAutomationScreenshotSequenceCount = FMath::Clamp(GameplayAutomationScreenshotSequenceCount, 1, 240);
		GameplayAutomationScreenshotSequenceIntervalSeconds = FMath::Clamp(GameplayAutomationScreenshotSequenceIntervalSeconds, 0.016f, 2.0f);
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
			(bScreenshotRequested || bScreenshotSequenceRequested) ? GameplayAutomationScreenshotDelaySeconds : GameplayAutomationWidgetDumpDelaySeconds,
			bWidgetDumpRequested ? GameplayAutomationWidgetDumpDelaySeconds : GameplayAutomationScreenshotDelaySeconds)),
		false);
}

void AT66PlayerController::HandleGameplayAutomationPrepare()
{
	if (GameplayAutomationScreenshotPath.IsEmpty()
		&& GameplayAutomationScreenshotSequenceDir.IsEmpty()
		&& GameplayAutomationWidgetDumpPath.IsEmpty())
	{
		return;
	}

	ApplyGameplayAutomationCaptureMode();

	if (GetWorld())
	{
		const float PostCaptureScreenshotDelaySeconds = FMath::Max(0.1f, GameplayAutomationPostCaptureScreenshotDelaySeconds);
		if (!GameplayAutomationScreenshotSequenceDir.IsEmpty())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationScreenshotTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationScreenshotTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationSequenceScreenshot,
				PostCaptureScreenshotDelaySeconds,
				false);
		}
		else if (!GameplayAutomationScreenshotPath.IsEmpty())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationScreenshotTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationScreenshotTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationScreenshot,
				PostCaptureScreenshotDelaySeconds,
				false);
		}

		if (!GameplayAutomationWidgetDumpPath.IsEmpty())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationWidgetDumpTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationWidgetDumpTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationWidgetDump,
				PostCaptureScreenshotDelaySeconds + 0.25f,
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

	if (Mode == TEXT("lootcrate") || Mode == TEXT("lootcrateui"))
	{
		if (!GameplayHUDWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootcrate failed: missing GameplayHUDWidget."));
			return;
		}

		GameplayHUDWidget->SetFullMapOpen(false);
		GameplayHUDWidget->RefreshHUD();
		StartCrateOpenHUD(ET66Rarity::Yellow);
		UE_LOG(LogTemp, Display, TEXT("[LootUICapture] lootcrate triggered via StartCrateOpenHUD rarity=Yellow."));
		return;
	}

	if (Mode == TEXT("lootchest") || Mode == TEXT("lootchestui"))
	{
		if (!GameplayHUDWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootchest failed: missing GameplayHUDWidget."));
			return;
		}

		GameplayHUDWidget->SetFullMapOpen(false);
		GameplayHUDWidget->RefreshHUD();
		const bool bStarted = StartChestRewardHUD(ET66Rarity::Yellow, 188);
		if (bStarted)
		{
			UE_LOG(LogTemp, Display, TEXT("[LootUICapture] lootchest triggered via StartChestRewardHUD rarity=Yellow gold=188."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootchest failed via StartChestRewardHUD rarity=Yellow gold=188."));
		}
		return;
	}

	if (Mode == TEXT("lootwheel") || Mode == TEXT("lootwheelui"))
	{
		if (!GameplayHUDWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootwheel failed: missing GameplayHUDWidget."));
			return;
		}

		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootwheel failed: missing world."));
			return;
		}

		const APawn* ControlledPawn = GetPawn();
		const FVector BaseLocation = ControlledPawn ? ControlledPawn->GetActorLocation() : FVector::ZeroVector;
		const FVector Forward = ControlledPawn ? ControlledPawn->GetActorForwardVector() : FVector::ForwardVector;
		const FVector LootWheelSpawnLocation = BaseLocation + Forward * 260.f + FVector(0.f, 0.f, 30.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66LootWheelInteractable* LootWheel = World->SpawnActor<AT66LootWheelInteractable>(
			AT66LootWheelInteractable::StaticClass(),
			LootWheelSpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!LootWheel)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootwheel failed: could not spawn interactable."));
			return;
		}

		LootWheel->SetShowcaseReusable(true);
		LootWheel->SetRarity(ET66Rarity::Yellow);
		GameplayHUDWidget->SetFullMapOpen(false);
		GameplayHUDWidget->RefreshHUD();
		const bool bStarted = LootWheel->Interact(this);
		UE_LOG(LogTemp, Display, TEXT("[LootUICapture] lootwheel triggered via spawned interactable real path started=%d."), bStarted ? 1 : 0);
		return;
	}

	if (Mode == TEXT("lootbag") || Mode == TEXT("lootbagui"))
	{
		if (!GameplayHUDWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootbag failed: missing GameplayHUDWidget."));
			return;
		}

		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		static const FName LootBagCaptureItemID(TEXT("Item_AoeDamage"));
		static constexpr ET66ItemRarity LootBagCaptureItemRarity = ET66ItemRarity::Yellow;
		FItemData ItemData;
		if (!T66GI || !T66GI->GetItemData(LootBagCaptureItemID, ItemData))
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootbag failed: missing item data for %s."), *LootBagCaptureItemID.ToString());
			return;
		}
		if (!RunState)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootbag failed: missing RunState."));
			return;
		}

		RunState->AddItemWithRarity(LootBagCaptureItemID, LootBagCaptureItemRarity);
		GameplayHUDWidget->SetFullMapOpen(false);
		GameplayHUDWidget->RefreshHUD();
		ShowLootBagItemRevealHUD(LootBagCaptureItemID, LootBagCaptureItemRarity);
		UE_LOG(LogTemp, Display, TEXT("[LootUICapture] lootbag triggered via RunState AddItemWithRarity + ShowLootBagItemRevealHUD item=%s rarity=Yellow."), *LootBagCaptureItemID.ToString());
		return;
	}

#if !UE_BUILD_SHIPPING
	if (Mode == TEXT("mobcombatsmoke") || Mode == TEXT("lightweightactorb4"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		if (!World || !HeroPawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobCombatSmoke] Failed: missing world or hero pawn."));
			return;
		}

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f));
		SetControlRotation(FRotator(-28.f, 0.f, 0.f));
		if (HeroPawn->CameraBoom)
		{
			HeroPawn->CameraBoom->TargetArmLength = 620.f;
			HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 125.f));
			HeroPawn->CameraBoom->bDoCollisionTest = false;
		}

		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (GameMode && GameMode->IsUsingTowerMainMapLayout() && GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
			for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
			{
				if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
				{
					TargetFloor = &Floor;
					break;
				}
				if (!TargetFloor && Floor.bGameplayFloor)
				{
					TargetFloor = &Floor;
				}
			}

			if (TargetFloor)
			{
				float PawnHalfHeight = 100.0f;
				if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
				{
					PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
				}

				FVector TargetLocation = TargetFloor->ArrivalPoint;
				if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
				{
					TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
				}
				if (TargetLocation.IsNearlyZero())
				{
					TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
				}
				TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

				HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
				HeroPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::TeleportPhysics);
				SetControlRotation(FRotator(-28.f, 0.f, 0.f));
				GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
				GameMode->SetEnemyDirectorSpawningPaused(true);
				if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
				{
					RunState->SetStageTimerActive(true);
				}
				UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Entered gameplay floor=%d location=%s with director paused."),
					TargetFloor->FloorNumber,
					*HeroPawn->GetActorLocation().ToCompactString());
			}
		}

		if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			TArray<TWeakObjectPtr<AT66MobBase>> ExistingMobs = Manager->GetActiveMobs();
			for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ExistingMobs)
			{
				if (AT66MobBase* Mob = WeakMob.Get())
				{
					if (Mob->MobID.ToString().StartsWith(TEXT("TestMob_")))
					{
						Mob->Destroy();
					}
				}
			}
		}

		TWeakObjectPtr<AT66HeroBase> WeakHero(HeroPawn);
		auto SpawnSmokeMob = [World, WeakHero](const TCHAR* MobIdText, const float HP, const FVector& LocalOffset) -> AT66MobBase*
		{
			AT66HeroBase* Hero = WeakHero.Get();
			if (!World || !Hero)
			{
				return nullptr;
			}

			const FVector Forward = Hero->GetActorForwardVector().GetSafeNormal2D();
			const FVector Right = Hero->GetActorRightVector().GetSafeNormal2D();
			FVector SpawnLocation = Hero->GetActorLocation()
				+ Forward * LocalOffset.X
				+ Right * LocalOffset.Y;
			SpawnLocation.Z += LocalOffset.Z;

			const FTransform SpawnTransform(Hero->GetActorRotation(), SpawnLocation);
			AT66MobBase* Mob = World->SpawnActorDeferred<AT66MobBase>(AT66MobBase::StaticClass(), SpawnTransform);
			if (!Mob)
			{
				return nullptr;
			}

			Mob->MobID = FName(MobIdText);
			Mob->EnemyFamily = ET66EnemyFamily::Melee;
			Mob->MaxHP = HP;
			Mob->CurrentHP = HP;
			Mob->TouchDamageCooldownSeconds = 0.f;
			Mob->LifecycleState = ET66MobLifecycleState::Active;
			Mob->FinishSpawning(SpawnTransform);
			UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Spawned %s hp=%.1f location=%s"),
				MobIdText,
				HP,
				*SpawnLocation.ToCompactString());
			return Mob;
		};

		TWeakObjectPtr<AT66MobBase> KillMob = SpawnSmokeMob(TEXT("TestMob_B4Kill"), 45.f, FVector(240.f, 0.f, 0.f));

		FTimerHandle HeroHitMobHandle;
		World->GetTimerManager().SetTimer(
			HeroHitMobHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakHero, KillMob]()
			{
				AT66HeroBase* Hero = WeakHero.Get();
				AT66MobBase* Mob = KillMob.Get();
				if (!Hero || !Mob || !Hero->CombatComponent)
				{
					return;
				}

				const FVector Start = Hero->GetActorLocation() + FVector(0.f, 0.f, 64.f);
				FVector Direction = Mob->GetActorLocation() - Start;
				Direction.Z = 0.f;
				if (!Direction.Normalize())
				{
					Direction = Hero->GetActorForwardVector().GetSafeNormal2D();
				}
				const FVector End = Start + Direction * 420.f;
				Hero->CombatComponent->PerformScopedPiercingShot(Start, End);
				UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Fired hero combat component scoped shot through %s."), *GetNameSafe(Mob));
			}),
			1.0f,
			false);

		FTimerHandle SpawnStunnedMobHandle;
		World->GetTimerManager().SetTimer(
			SpawnStunnedMobHandle,
			FTimerDelegate::CreateWeakLambda(this, [SpawnSmokeMob]()
			{
				if (AT66MobBase* StunnedMob = SpawnSmokeMob(TEXT("TestMob_B4StunnedTouch"), 300.f, FVector(48.f, 0.f, 0.f)))
				{
					StunnedMob->ApplyStun(3.f);
					UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Applied stun to %s for touch-damage parity check."), *GetNameSafe(StunnedMob));
				}
			}),
			3.5f,
			false);

		FTimerHandle CleanupMobsHandle;
		World->GetTimerManager().SetTimer(
			CleanupMobsHandle,
			FTimerDelegate::CreateWeakLambda(this, [World]()
			{
				if (UT66MobManagerSubsystem* Manager = World ? World->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
				{
					TArray<TWeakObjectPtr<AT66MobBase>> Mobs = Manager->GetActiveMobs();
					for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Mobs)
					{
						if (AT66MobBase* Mob = WeakMob.Get())
						{
							if (Mob->MobID.ToString().StartsWith(TEXT("TestMob_")))
							{
								Mob->Destroy();
							}
						}
					}
				}
			}),
			8.5f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Lightweight Actor B.4 smoke sequence armed."));
		return;
	}

	if (Mode == TEXT("mobdatabindingsmoke") || Mode == TEXT("lightweightactorb5"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		if (!World || !HeroPawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobDataBindingSmoke] Failed: missing world or hero pawn."));
			return;
		}

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f));
		SetControlRotation(FRotator(-30.f, 0.f, 0.f));
		if (HeroPawn->CameraBoom)
		{
			HeroPawn->CameraBoom->TargetArmLength = 760.f;
			HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 155.f));
			HeroPawn->CameraBoom->bDoCollisionTest = false;
		}

		AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode());
		T66TowerMapTerrain::FLayout TowerLayout;
		if (GameMode && GameMode->IsUsingTowerMainMapLayout() && GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
			for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
			{
				if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
				{
					TargetFloor = &Floor;
					break;
				}
				if (!TargetFloor && Floor.bGameplayFloor)
				{
					TargetFloor = &Floor;
				}
			}

			if (TargetFloor)
			{
				float PawnHalfHeight = 100.f;
				if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
				{
					PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
				}

				FVector TargetLocation = TargetFloor->ArrivalPoint;
				if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
				{
					TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
				}
				if (TargetLocation.IsNearlyZero())
				{
					TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
				}
				TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.f;

				HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
				HeroPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::TeleportPhysics);
				SetControlRotation(FRotator(-30.f, 0.f, 0.f));
				GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
				GameMode->SetEnemyDirectorSpawningPaused(true);
				if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
				{
					RunState->SetStageTimerActive(true);
				}
				UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Entered gameplay floor=%d location=%s with director paused."),
					TargetFloor->FloorNumber,
					*HeroPawn->GetActorLocation().ToCompactString());
			}
		}

		const FName TestMobTag(TEXT("T66_TestMob"));
		if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			TArray<TWeakObjectPtr<AT66MobBase>> ExistingMobs = Manager->GetActiveMobs();
			for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ExistingMobs)
			{
				if (AT66MobBase* Mob = WeakMob.Get())
				{
					if (Mob->ActorHasTag(TestMobTag) || Mob->MobID.ToString().StartsWith(TEXT("TestMob_")))
					{
						Mob->Destroy();
					}
				}
			}
		}

		int32 StageNum = 1;
		float DifficultyScalar = 1.f;
		float FinaleScalar = 1.f;
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			StageNum = FMath::Max(1, RunState->GetCurrentStage());
			DifficultyScalar = RunState->GetDifficultyScalar();
			FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
		}

		TWeakObjectPtr<AT66HeroBase> WeakHero(HeroPawn);
		auto SpawnConfiguredSmokeMob = [World, WeakHero, StageNum, DifficultyScalar, FinaleScalar, TestMobTag](const FName MobID, const FVector& LocalOffset) -> AT66MobBase*
		{
			AT66HeroBase* Hero = WeakHero.Get();
			if (!World || !Hero)
			{
				return nullptr;
			}

			const FVector Forward = Hero->GetActorForwardVector().GetSafeNormal2D();
			const FVector Right = Hero->GetActorRightVector().GetSafeNormal2D();
			FVector SpawnLocation = Hero->GetActorLocation()
				+ Forward * LocalOffset.X
				+ Right * LocalOffset.Y;
			SpawnLocation.Z += LocalOffset.Z;

			const FTransform SpawnTransform(Hero->GetActorRotation(), SpawnLocation);
			AT66MobBase* Mob = World->SpawnActorDeferred<AT66MobBase>(AT66MobBase::StaticClass(), SpawnTransform);
			if (!Mob)
			{
				return nullptr;
			}

			Mob->Tags.AddUnique(TestMobTag);
			Mob->MobID = MobID;
			Mob->CharacterVisualID = MobID;
			Mob->LifecycleState = ET66MobLifecycleState::Active;
			Mob->FinishSpawning(SpawnTransform);
			Mob->ConfigureAsMob(MobID, ET66EnemyFamily::Melee, NAME_None, StageNum, DifficultyScalar, 1.f, FinaleScalar, false);
			Mob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 30.f);
			UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Spawned configured MobID=%s hp=%.1f speed=%.1f touch=%d location=%s"),
				*MobID.ToString(),
				Mob->MaxHP,
				Mob->ChaseSpeed,
				Mob->TouchDamageHearts,
				*SpawnLocation.ToCompactString());
			return Mob;
		};

		TWeakObjectPtr<AT66MobBase> SlimeHitMob = SpawnConfiguredSmokeMob(FName(TEXT("Slime")), FVector(240.f, -260.f, 0.f));
		TWeakObjectPtr<AT66MobBase> SlimeTouchMob = SpawnConfiguredSmokeMob(FName(TEXT("Slime")), FVector(52.f, 260.f, 0.f));

		const FName Roster[] =
		{
			FName(TEXT("Slime")),
			FName(TEXT("CaveBat")),
			FName(TEXT("BoneWalker")),
			FName(TEXT("RatPack")),
			FName(TEXT("TombSpider")),
			FName(TEXT("HexSlinger")),
			FName(TEXT("StoneSentinel")),
			FName(TEXT("MimicLure")),
			FName(TEXT("BoneConjurer")),
			FName(TEXT("CryptWraith"))
		};
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(Roster); ++Index)
		{
			const float OffsetY = -900.f + static_cast<float>(Index * 200);
			if (AT66MobBase* RosterMob = SpawnConfiguredSmokeMob(Roster[Index], FVector(520.f, OffsetY, 0.f)))
			{
				RosterMob->TouchDamageCooldownSeconds = 999.f;
			}
		}

		FTimerHandle HeroHitMobHandle;
		World->GetTimerManager().SetTimer(
			HeroHitMobHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakHero, SlimeHitMob]()
			{
				AT66HeroBase* Hero = WeakHero.Get();
				AT66MobBase* Mob = SlimeHitMob.Get();
				if (!Hero || !Mob || !Hero->CombatComponent)
				{
					return;
				}

				const FVector Start = Hero->GetActorLocation() + FVector(0.f, 0.f, 64.f);
				FVector Direction = Mob->GetActorLocation() - Start;
				Direction.Z = 0.f;
				if (!Direction.Normalize())
				{
					Direction = Hero->GetActorForwardVector().GetSafeNormal2D();
				}
				Hero->CombatComponent->PerformScopedPiercingShot(Start, Start + Direction * 420.f);
				UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Fired hero combat component scoped shot through configured %s."), *GetNameSafe(Mob));
			}),
			1.0f,
			false);

		FTimerHandle CleanupTouchMobHandle;
		World->GetTimerManager().SetTimer(
			CleanupTouchMobHandle,
			FTimerDelegate::CreateWeakLambda(this, [SlimeTouchMob]()
			{
				if (AT66MobBase* Mob = SlimeTouchMob.Get())
				{
					Mob->Destroy();
					UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Destroyed touch-damage Slime after verification window."));
				}
			}),
			1.25f,
			false);

		FTimerHandle CleanupMobsHandle;
		World->GetTimerManager().SetTimer(
			CleanupMobsHandle,
			FTimerDelegate::CreateWeakLambda(this, [World, TestMobTag]()
			{
				if (UT66MobManagerSubsystem* Manager = World ? World->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
				{
					TArray<TWeakObjectPtr<AT66MobBase>> Mobs = Manager->GetActiveMobs();
					for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Mobs)
					{
						if (AT66MobBase* Mob = WeakMob.Get())
						{
							if (Mob->ActorHasTag(TestMobTag))
							{
								Mob->Destroy();
							}
						}
					}
				}
			}),
			12.5f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Lightweight Actor B.5 data-binding smoke sequence armed."));
		return;
	}

	if (Mode == TEXT("mobdirectorroutingsmoke") || Mode == TEXT("lightweightactorb6"))
	{
		if (IConsoleVariable* UseLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.UseLightweight")))
		{
			UseLightweightCVar->Set(1, ECVF_SetByConsole);
			UE_LOG(LogTemp, Display, TEXT("[MobDirectorRoutingSmoke] T66.Mob.UseLightweight set to 1 before director routing."));
		}

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobDirectorRoutingSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bGameplayFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobDirectorRoutingSmoke] Failed: no gameplay floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-35.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		FTimerHandle DirectorSmokeCombatHandle;
		World->GetTimerManager().SetTimer(
			DirectorSmokeCombatHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				AT66HeroBase* SmokeHero = PC ? Cast<AT66HeroBase>(PC->GetPawn()) : nullptr;
				UT66MobManagerSubsystem* Manager = SmokeWorld ? SmokeWorld->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
				if (!SmokeWorld || !SmokeHero || !Manager)
				{
					UE_LOG(LogTemp, Warning, TEXT("[MobDirectorRoutingSmoke] Combat step skipped: missing world, hero, or mob manager."));
					return;
				}

				AT66MobBase* TargetMob = nullptr;
				for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Manager->GetActiveMobs())
				{
					AT66MobBase* Mob = WeakMob.Get();
					if (Mob && Mob->IsAliveAndActive() && !Mob->MobID.ToString().StartsWith(TEXT("TestMob_")))
					{
						TargetMob = Mob;
						break;
					}
				}

				UE_LOG(LogTemp, Display, TEXT("[MobDirectorRoutingSmoke] Active lightweight mobs=%d target=%s."),
					Manager->GetActiveMobs().Num(),
					*GetNameSafe(TargetMob));

				if (!TargetMob)
				{
					return;
				}

				SmokeHero->SetActorLocation(TargetMob->GetActorLocation() + FVector(36.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
				if (SmokeHero->CombatComponent)
				{
					for (int32 ShotIndex = 0; ShotIndex < 8 && TargetMob->IsAliveAndActive(); ++ShotIndex)
					{
						const FVector Start = SmokeHero->GetActorLocation() + FVector(0.f, 0.f, 64.f);
						FVector Direction = TargetMob->GetActorLocation() - Start;
						Direction.Z = 0.f;
						if (!Direction.Normalize())
						{
							Direction = SmokeHero->GetActorForwardVector().GetSafeNormal2D();
						}
						SmokeHero->CombatComponent->PerformScopedPiercingShot(Start, Start + Direction * 500.f);
					}
				}
				if (TargetMob->IsAliveAndActive())
				{
					TargetMob->TakeDamageFromHeroHitZone(
						FMath::CeilToInt(TargetMob->GetCurrentHP()),
						TargetMob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body),
						FName(TEXT("B6DirectorRoutingSmoke")),
						NAME_None);
				}
			}),
			8.0f,
			false);

		FTimerHandle DirectorSmokeToggleOffHandle;
		World->GetTimerManager().SetTimer(
			DirectorSmokeToggleOffHandle,
			FTimerDelegate::CreateLambda([]()
			{
				if (IConsoleVariable* UseLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.UseLightweight")))
				{
					UseLightweightCVar->Set(0, ECVF_SetByConsole);
					UE_LOG(LogTemp, Display, TEXT("[MobDirectorRoutingSmoke] T66.Mob.UseLightweight set to 0 after routing sample."));
				}
			}),
			18.0f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[MobDirectorRoutingSmoke] Lightweight Actor B.6 director-routing smoke sequence armed on floor=%d location=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString());
		return;
	}

	if (Mode == TEXT("rushsmoke") || Mode == TEXT("lightweightactorb8"))
	{
		if (IConsoleVariable* UseLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.UseLightweight")))
		{
			UseLightweightCVar->Set(1, ECVF_SetByConsole);
			UE_LOG(LogTemp, Display, TEXT("[RushSmoke] T66.Mob.UseLightweight set to 1 before Rush smoke."));
		}

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[RushSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bGameplayFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[RushSmoke] Failed: no gameplay floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-28.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		const FVector RushSpawnLocation = TargetLocation + FVector(900.0f, 0.0f, 0.0f);
		const FTransform RushSpawnTransform(FRotator::ZeroRotator, RushSpawnLocation);
		AT66MobBase* RushMob = World->SpawnActorDeferred<AT66MobBase>(
			AT66MobBase::StaticClass(),
			RushSpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (RushMob)
		{
			RushMob->Tags.AddUnique(FName(TEXT("T66_TestMob")));
			RushMob->MobID = FName(TEXT("RatPack"));
			RushMob->CharacterVisualID = FName(TEXT("RatPack"));
			RushMob->LifecycleState = ET66MobLifecycleState::Active;
			UGameplayStatics::FinishSpawningActor(RushMob, RushSpawnTransform);

			int32 StageNum = 1;
			float DifficultyScalar = 1.f;
			float EnemyProgressionScalar = 1.f;
			float FinaleScalar = 1.f;
			if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
			{
				StageNum = FMath::Max(1, RunState->GetCurrentStage());
				DifficultyScalar = RunState->GetDifficultyScalar();
				FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
			}
			RushMob->ConfigureAsMob(FName(TEXT("RatPack")), ET66EnemyFamily::Rush, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
			RushMob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 8.f);
		}

		TWeakObjectPtr<AT66MobBase> WeakRushMob(RushMob);
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		const float LogTimes[] = { 0.35f, 0.95f, 1.35f, 1.85f, 2.35f, 3.0f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle RushLogHandle;
			const float LogTime = LogTimes[Index];
			World->GetTimerManager().SetTimer(
				RushLogHandle,
				FTimerDelegate::CreateLambda([WeakRushMob, LogTime]()
				{
					const AT66MobBase* Mob = WeakRushMob.Get();
					if (!Mob)
					{
						UE_LOG(LogTemp, Warning, TEXT("[RushSmoke] t=%.2fs test Rush mob missing."), LogTime);
						return;
					}

					UE_LOG(LogTemp, Display, TEXT("[RushSmoke] t=%.2fs MobID=%s loc=%s velocity=%s rushing=%d rushRemaining=%.2f rushCooldown=%.2f"),
						LogTime,
						Mob->MobID.IsNone() ? TEXT("unset") : *Mob->MobID.ToString(),
						*Mob->GetActorLocation().ToCompactString(),
						*Mob->StoredVelocity.ToCompactString(),
						Mob->bIsRushing ? 1 : 0,
						Mob->RushSecondsRemaining,
						Mob->RushCooldownRemaining);
				}),
				LogTime,
				false);
		}

		FTimerHandle StunHandle;
		World->GetTimerManager().SetTimer(
			StunHandle,
			FTimerDelegate::CreateLambda([WeakRushMob]()
			{
				if (AT66MobBase* Mob = WeakRushMob.Get())
				{
					Mob->ApplyStun(3.0f);
					UE_LOG(LogTemp, Display, TEXT("[RushSmoke] Applied 3.0s stun to Rush mob; rush timers should pause while status blocks chase."));
				}
			}),
			3.25f,
			false);

		FTimerHandle DirectorCountHandle;
		World->GetTimerManager().SetTimer(
			DirectorCountHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
				const AT66EnemyDirector* Director = SmokeGameMode ? SmokeGameMode->GetEnemyDirectorForDiagnostics() : nullptr;
				UE_LOG(LogTemp, Display, TEXT("[RushSmoke] directorCounts rich=%d lightweight=%d lightweightMelee=%d lightweightRush=%d lightweightRanged=%d."),
					Director ? Director->GetAliveRichEnemyCount() : -1,
					Director ? Director->GetAliveLightweightMobCount() : -1,
					Director ? Director->GetAliveLightweightMeleeMobCount() : -1,
					Director ? Director->GetAliveLightweightRushMobCount() : -1,
					Director ? Director->GetAliveLightweightRangedMobCount() : -1);
			}),
			8.0f,
			false);

		FTimerHandle RushSmokeExitHandle;
		World->GetTimerManager().SetTimer(
			RushSmokeExitHandle,
			FTimerDelegate::CreateLambda([]()
			{
				UE_LOG(LogTemp, Display, TEXT("[RushSmoke] Completed B.8 Rush smoke automation."));
				FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("T66RushSmokeComplete"));
			}),
			10.5f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[RushSmoke] Lightweight Actor B.8 Rush smoke sequence armed on floor=%d hero=%s rushMob=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString(),
			*GetNameSafe(RushMob));
		return;
	}

	if (Mode == TEXT("flyingsmoke") || Mode == TEXT("lightweightactorb9"))
	{
		if (IConsoleVariable* UseLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.UseLightweight")))
		{
			UseLightweightCVar->Set(1, ECVF_SetByConsole);
			UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] T66.Mob.UseLightweight set to 1 before Flying smoke."));
		}

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[FlyingSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bGameplayFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[FlyingSmoke] Failed: no gameplay floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-28.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		const FVector FlyingSpawnLocation = TargetLocation + FVector(780.0f, 0.0f, 0.0f);
		const FTransform FlyingSpawnTransform(FRotator::ZeroRotator, FlyingSpawnLocation);
		AT66MobBase* FlyingMob = World->SpawnActorDeferred<AT66MobBase>(
			AT66MobBase::StaticClass(),
			FlyingSpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (FlyingMob)
		{
			FlyingMob->Tags.AddUnique(FName(TEXT("T66_TestMob")));
			FlyingMob->MobID = FName(TEXT("CaveBat"));
			FlyingMob->CharacterVisualID = FName(TEXT("CaveBat"));
			FlyingMob->LifecycleState = ET66MobLifecycleState::Active;
			UGameplayStatics::FinishSpawningActor(FlyingMob, FlyingSpawnTransform);

			int32 StageNum = 1;
			float DifficultyScalar = 1.f;
			float EnemyProgressionScalar = 1.f;
			float FinaleScalar = 1.f;
			if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
			{
				StageNum = FMath::Max(1, RunState->GetCurrentStage());
				DifficultyScalar = RunState->GetDifficultyScalar();
				FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
			}
			FlyingMob->ConfigureAsMob(FName(TEXT("CaveBat")), ET66EnemyFamily::Flying, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
			FlyingMob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 8.f);
		}

		TWeakObjectPtr<AT66MobBase> WeakFlyingMob(FlyingMob);
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		const float LogTimes[] = { 0.35f, 0.95f, 1.45f, 2.05f, 2.65f, 3.25f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle FlyingLogHandle;
			const float LogTime = LogTimes[Index];
			World->GetTimerManager().SetTimer(
				FlyingLogHandle,
				FTimerDelegate::CreateLambda([WeakFlyingMob, LogTime]()
				{
					const AT66MobBase* Mob = WeakFlyingMob.Get();
					if (!Mob)
					{
						UE_LOG(LogTemp, Warning, TEXT("[FlyingSmoke] t=%.2fs test Flying mob missing."), LogTime);
						return;
					}

					UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] t=%.2fs MobID=%s loc=%s velocity=%s hoverAnchorZ=%.1f hoverBobTime=%.2f status=%s"),
						LogTime,
						Mob->MobID.IsNone() ? TEXT("unset") : *Mob->MobID.ToString(),
						*Mob->GetActorLocation().ToCompactString(),
						*Mob->StoredVelocity.ToCompactString(),
						Mob->HoverAnchorZ,
						Mob->HoverBobTime,
						Mob->FreezeSecondsRemaining > 0.f || Mob->StunSecondsRemaining > 0.f || Mob->RootSecondsRemaining > 0.f ? TEXT("Blocked") : TEXT("Active"));
				}),
				LogTime,
				false);
		}

		FTimerHandle StunHandle;
		World->GetTimerManager().SetTimer(
			StunHandle,
			FTimerDelegate::CreateLambda([WeakFlyingMob]()
			{
				if (AT66MobBase* Mob = WeakFlyingMob.Get())
				{
					Mob->ApplyStun(3.0f);
					UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] Applied 3.0s stun to Flying mob; hover and chase should pause while status blocks movement."));
				}
			}),
			3.5f,
			false);

		FTimerHandle DamageHandle;
		World->GetTimerManager().SetTimer(
			DamageHandle,
			FTimerDelegate::CreateLambda([WeakFlyingMob]()
			{
				if (AT66MobBase* Mob = WeakFlyingMob.Get())
				{
					const FT66CombatTargetHandle TargetHandle = Mob->ResolveCombatTargetHandle();
					Mob->TakeDamageFromHeroHitZone(9999, TargetHandle, FName(TEXT("FlyingSmoke")), FName(TEXT("AutomationKill")));
					UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] Applied automation lethal damage to Flying mob; death path should release it."));
				}
			}),
			7.0f,
			false);

		FTimerHandle DirectorCountHandle;
		World->GetTimerManager().SetTimer(
			DirectorCountHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
				const AT66EnemyDirector* Director = SmokeGameMode ? SmokeGameMode->GetEnemyDirectorForDiagnostics() : nullptr;
				UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] directorCounts rich=%d lightweight=%d lightweightMelee=%d lightweightRush=%d lightweightFlying=%d lightweightRanged=%d."),
					Director ? Director->GetAliveRichEnemyCount() : -1,
					Director ? Director->GetAliveLightweightMobCount() : -1,
					Director ? Director->GetAliveLightweightMeleeMobCount() : -1,
					Director ? Director->GetAliveLightweightRushMobCount() : -1,
					Director ? Director->GetAliveLightweightFlyingMobCount() : -1,
					Director ? Director->GetAliveLightweightRangedMobCount() : -1);
			}),
			8.5f,
			false);

		FTimerHandle FlyingSmokeExitHandle;
		World->GetTimerManager().SetTimer(
			FlyingSmokeExitHandle,
			FTimerDelegate::CreateLambda([]()
			{
				UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] Completed B.9 Flying smoke automation."));
				FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("T66FlyingSmokeComplete"));
			}),
			11.0f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] Lightweight Actor B.9 Flying smoke sequence armed on floor=%d hero=%s flyingMob=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString(),
			*GetNameSafe(FlyingMob));
		return;
	}

	if (Mode == TEXT("rangedsmoke") || Mode == TEXT("lightweightactorb10"))
	{
		if (IConsoleVariable* UseLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.UseLightweight")))
		{
			UseLightweightCVar->Set(1, ECVF_SetByConsole);
			UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] T66.Mob.UseLightweight set to 1 before Ranged smoke."));
		}
		if (IConsoleVariable* RouteRangedCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.Diagnostics.RouteRangedLightweight")))
		{
			RouteRangedCVar->Set(1, ECVF_SetByConsole);
			UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] T66.Mob.Diagnostics.RouteRangedLightweight set to 1 before Ranged smoke."));
		}

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bGameplayFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] Failed: no gameplay floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-28.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		float InitialHeroHP = -1.f;
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
			InitialHeroHP = RunState->GetCurrentHP();
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		const FVector RangedSpawnLocation = TargetLocation + FVector(1000.0f, 0.0f, 0.0f);
		const FTransform RangedSpawnTransform(FRotator::ZeroRotator, RangedSpawnLocation);
		AT66MobBase* RangedMob = World->SpawnActorDeferred<AT66MobBase>(
			AT66MobBase::StaticClass(),
			RangedSpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (RangedMob)
		{
			RangedMob->Tags.AddUnique(FName(TEXT("T66_TestMob")));
			RangedMob->MobID = FName(TEXT("HexSlinger"));
			RangedMob->CharacterVisualID = FName(TEXT("HexSlinger"));
			RangedMob->LifecycleState = ET66MobLifecycleState::Active;
			UGameplayStatics::FinishSpawningActor(RangedMob, RangedSpawnTransform);

			int32 StageNum = 1;
			float DifficultyScalar = 1.f;
			float EnemyProgressionScalar = 1.f;
			float FinaleScalar = 1.f;
			if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
			{
				StageNum = FMath::Max(1, RunState->GetCurrentStage());
				DifficultyScalar = RunState->GetDifficultyScalar();
				FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
			}
			RangedMob->ConfigureAsMob(FName(TEXT("HexSlinger")), ET66EnemyFamily::Ranged, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
			RangedMob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 8.f);
		}

		TWeakObjectPtr<AT66MobBase> WeakRangedMob(RangedMob);
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		const float LogTimes[] = { 0.35f, 0.85f, 1.35f, 2.15f, 3.0f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle RangedLogHandle;
			const float LogTime = LogTimes[Index];
			World->GetTimerManager().SetTimer(
				RangedLogHandle,
				FTimerDelegate::CreateLambda([WeakRangedMob, LogTime]()
				{
					const AT66MobBase* Mob = WeakRangedMob.Get();
					if (!Mob)
					{
						UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] t=%.2fs test Ranged mob missing."), LogTime);
						return;
					}

					UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] t=%.2fs MobID=%s loc=%s velocity=%s fireCooldown=%.2f minRange=%.1f maxRange=%.1f projectileHeight=%.1f"),
						LogTime,
						Mob->MobID.IsNone() ? TEXT("unset") : *Mob->MobID.ToString(),
						*Mob->GetActorLocation().ToCompactString(),
						*Mob->StoredVelocity.ToCompactString(),
						Mob->FireCooldownRemaining,
						Mob->DesiredMinRange,
						Mob->DesiredMaxRange,
						Mob->ProjectileSpawnHeight);
				}),
				LogTime,
				false);
		}

		FTimerHandle ProjectileAssertionHandle;
		World->GetTimerManager().SetTimer(
			ProjectileAssertionHandle,
			FTimerDelegate::CreateLambda([WeakThis, WeakRangedMob]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				const AT66MobBase* Mob = WeakRangedMob.Get();
				if (!SmokeWorld || !Mob)
				{
					UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] ProjectileTravelAssertion skipped: missing world or ranged mob."));
					return;
				}

				const FVector SpawnReference = Mob->GetActorLocation() + FVector(0.f, 0.f, Mob->ProjectileSpawnHeight);
				int32 OwnedProjectileCount = 0;
				float MaxTravelDistance = 0.f;
				for (TActorIterator<AT66EnemyProjectileBase> It(SmokeWorld); It; ++It)
				{
					const AT66EnemyProjectileBase* Projectile = *It;
					if (!Projectile || Projectile->GetOwner() != Mob)
					{
						continue;
					}

					++OwnedProjectileCount;
					MaxTravelDistance = FMath::Max(MaxTravelDistance, FVector::Dist(Projectile->GetActorLocation(), SpawnReference));
				}

				UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] ProjectileTravelAssertion spawnHeight=%.1f ownedProjectiles=%d maxTravel=%.1f result=%s"),
					Mob->ProjectileSpawnHeight,
					OwnedProjectileCount,
					MaxTravelDistance,
					(OwnedProjectileCount > 0 && MaxTravelDistance > 80.f) ? TEXT("PASS") : TEXT("CHECK"));
			}),
			0.85f,
			false);

		FTimerHandle HeroDamageHandle;
		World->GetTimerManager().SetTimer(
			HeroDamageHandle,
			FTimerDelegate::CreateLambda([WeakThis, InitialHeroHP]()
			{
				const AT66PlayerController* PC = WeakThis.Get();
				UGameInstance* GameInstance = PC ? PC->GetGameInstance() : nullptr;
				const UT66RunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
				const float CurrentHP = RunState ? RunState->GetCurrentHP() : -1.f;
				UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] HeroDamageAssertion initialHP=%.1f currentHP=%.1f result=%s"),
					InitialHeroHP,
					CurrentHP,
					(CurrentHP >= 0.f && InitialHeroHP >= 0.f && CurrentHP < InitialHeroHP) ? TEXT("PASS") : TEXT("CHECK"));
			}),
			1.45f,
			false);

		FTimerHandle DamageHandle;
		World->GetTimerManager().SetTimer(
			DamageHandle,
			FTimerDelegate::CreateLambda([WeakRangedMob]()
			{
				if (AT66MobBase* Mob = WeakRangedMob.Get())
				{
					const FT66CombatTargetHandle TargetHandle = Mob->ResolveCombatTargetHandle();
					Mob->TakeDamageFromHeroHitZone(9999, TargetHandle, FName(TEXT("RangedSmoke")), FName(TEXT("AutomationKill")));
					UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] Applied automation lethal damage to Ranged mob; death path should release it."));
				}
			}),
			3.5f,
			false);

		FTimerHandle DirectorCountHandle;
		World->GetTimerManager().SetTimer(
			DirectorCountHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
				const AT66EnemyDirector* Director = SmokeGameMode ? SmokeGameMode->GetEnemyDirectorForDiagnostics() : nullptr;
				UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] directorCounts rich=%d lightweight=%d lightweightMelee=%d lightweightRush=%d lightweightFlying=%d lightweightRanged=%d."),
					Director ? Director->GetAliveRichEnemyCount() : -1,
					Director ? Director->GetAliveLightweightMobCount() : -1,
					Director ? Director->GetAliveLightweightMeleeMobCount() : -1,
					Director ? Director->GetAliveLightweightRushMobCount() : -1,
					Director ? Director->GetAliveLightweightFlyingMobCount() : -1,
					Director ? Director->GetAliveLightweightRangedMobCount() : -1);
			}),
			4.0f,
			false);

		FTimerHandle RangedSmokeExitHandle;
		World->GetTimerManager().SetTimer(
			RangedSmokeExitHandle,
			FTimerDelegate::CreateLambda([]()
			{
				UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] Completed B.10 Ranged smoke automation."));
				FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("T66RangedSmokeComplete"));
			}),
			5.5f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] Lightweight Actor B.10 Ranged smoke sequence armed on floor=%d hero=%s rangedMob=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString(),
			*GetNameSafe(RangedMob));
		return;
	}

	if (Mode == TEXT("mobpoolhudsmoke") || Mode == TEXT("lightweightactorb7"))
	{
		if (IConsoleVariable* UseLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.UseLightweight")))
		{
			UseLightweightCVar->Set(1, ECVF_SetByConsole);
			UE_LOG(LogTemp, Display, TEXT("[MobPoolHudSmoke] T66.Mob.UseLightweight set to 1 before pool/HUD smoke."));
		}

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobPoolHudSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bGameplayFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobPoolHudSmoke] Failed: no gameplay floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-28.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}

		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		auto LogPoolHudState = [WeakThis](const TCHAR* Label)
		{
			AT66PlayerController* PC = WeakThis.Get();
			UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
			UT66MobManagerSubsystem* Manager = SmokeWorld ? SmokeWorld->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
			UT66ActorRegistrySubsystem* Registry = SmokeWorld ? SmokeWorld->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
			AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
			AT66EnemyDirector* Director = SmokeGameMode ? SmokeGameMode->GetEnemyDirectorForDiagnostics() : nullptr;
			if (!Manager || !Registry)
			{
				UE_LOG(LogTemp, Warning, TEXT("[MobPoolHudSmoke] %s missing manager or registry."), Label);
				return;
			}

			UE_LOG(LogTemp, Display, TEXT("[MobPoolHudSmoke] %s combined=%d registryLightweight=%d directorRich=%d directorLightweight=%d activeMobs=%d inactiveMobs=%d reuse=%d releases=%d inactivePeak=%d"),
				Label,
				Registry->GetCombinedLiveEnemyCount(),
				Registry->GetLiveMobCount(),
				Director ? Director->GetAliveRichEnemyCount() : -1,
				Director ? Director->GetAliveLightweightMobCount() : -1,
				Manager->GetActiveMobs().Num(),
				Manager->GetInactiveMobCount(),
				Manager->GetPoolReuseAcquireCount(),
				Manager->GetPoolReleaseCount(),
				Manager->GetPeakInactiveMobCount());

			if (PC->GameplayHUDWidget)
			{
				PC->GameplayHUDWidget->RefreshHUD();
			}
		};

		auto KillSomeLightweightMobs = [WeakThis](const int32 MaxKills)
		{
			AT66PlayerController* PC = WeakThis.Get();
			UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
			UT66MobManagerSubsystem* Manager = SmokeWorld ? SmokeWorld->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
			if (!Manager)
			{
				return;
			}

			TArray<TWeakObjectPtr<AT66MobBase>> Mobs = Manager->GetActiveMobs();
			int32 KilledCount = 0;
			for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Mobs)
			{
				AT66MobBase* Mob = WeakMob.Get();
				if (!Mob || !Mob->IsAliveAndActive())
				{
					continue;
				}

				Mob->TakeDamageFromHeroHitZone(
					FMath::CeilToInt(Mob->GetCurrentHP()),
					Mob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body),
					FName(TEXT("B7PoolHudSmoke")),
					NAME_None);
				++KilledCount;
				if (KilledCount >= MaxKills)
				{
					break;
				}
			}

			UE_LOG(LogTemp, Display, TEXT("[MobPoolHudSmoke] Force-killed %d lightweight mob(s) to exercise pool release/reuse."), KilledCount);
		};

		const float KillTimes[] = { 14.0f, 28.0f, 42.0f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(KillTimes); ++Index)
		{
			FTimerHandle KillHandle;
			World->GetTimerManager().SetTimer(
				KillHandle,
				FTimerDelegate::CreateLambda([KillSomeLightweightMobs]()
				{
					KillSomeLightweightMobs(10);
				}),
				KillTimes[Index],
				false);
		}

		const float LogTimes[] = { 10.0f, 25.0f, 40.0f, 55.0f, 65.0f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle LogHandle;
			FString Label = FString::Printf(TEXT("t%.0fs"), LogTimes[Index]);
			World->GetTimerManager().SetTimer(
				LogHandle,
				FTimerDelegate::CreateLambda([LogPoolHudState, Label]()
				{
					LogPoolHudState(*Label);
				}),
				LogTimes[Index],
				false);
		}

		FTimerHandle ToggleOffHandle;
		World->GetTimerManager().SetTimer(
			ToggleOffHandle,
			FTimerDelegate::CreateLambda([]()
			{
				if (IConsoleVariable* UseLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.UseLightweight")))
				{
					UseLightweightCVar->Set(0, ECVF_SetByConsole);
					UE_LOG(LogTemp, Display, TEXT("[MobPoolHudSmoke] T66.Mob.UseLightweight set to 0; subsequent spawns use rich path."));
				}
			}),
			50.0f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[MobPoolHudSmoke] Lightweight Actor B.7 pool/HUD smoke sequence armed on floor=%d location=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString());
		return;
	}

	if (Mode == TEXT("enemywaveperf") || Mode == TEXT("mainboardenemywave") || Mode == TEXT("phaseaperf"))
	{
		int32 LightweightMobOverride = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66MobUseLightweight="), LightweightMobOverride))
		{
			if (IConsoleVariable* UseLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.UseLightweight")))
			{
				UseLightweightCVar->Set(LightweightMobOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Mob.UseLightweight set to %d from command line."), LightweightMobOverride != 0 ? 1 : 0);
			}
		}
		int32 RouteRushLightweightOverride = 1;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66MobRouteRushLightweight="), RouteRushLightweightOverride))
		{
			if (IConsoleVariable* RouteRushLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.Diagnostics.RouteRushLightweight")))
			{
				RouteRushLightweightCVar->Set(RouteRushLightweightOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Mob.Diagnostics.RouteRushLightweight set to %d from command line."), RouteRushLightweightOverride != 0 ? 1 : 0);
			}
		}
		int32 RouteFlyingLightweightOverride = 1;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66MobRouteFlyingLightweight="), RouteFlyingLightweightOverride))
		{
			if (IConsoleVariable* RouteFlyingLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.Diagnostics.RouteFlyingLightweight")))
			{
				RouteFlyingLightweightCVar->Set(RouteFlyingLightweightOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Mob.Diagnostics.RouteFlyingLightweight set to %d from command line."), RouteFlyingLightweightOverride != 0 ? 1 : 0);
			}
		}
		int32 RouteRangedLightweightOverride = 1;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66MobRouteRangedLightweight="), RouteRangedLightweightOverride))
		{
			if (IConsoleVariable* RouteRangedLightweightCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.Diagnostics.RouteRangedLightweight")))
			{
				RouteRangedLightweightCVar->Set(RouteRangedLightweightOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Mob.Diagnostics.RouteRangedLightweight set to %d from command line."), RouteRangedLightweightOverride != 0 ? 1 : 0);
			}
		}
		int32 UseTouchDamageOverlapOverride = 1;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66MobUseTouchDamageOverlap="), UseTouchDamageOverlapOverride))
		{
			if (IConsoleVariable* UseTouchDamageOverlapCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.Diagnostics.UseTouchDamageOverlap")))
			{
				UseTouchDamageOverlapCVar->Set(UseTouchDamageOverlapOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Mob.Diagnostics.UseTouchDamageOverlap set to %d from command line."), UseTouchDamageOverlapOverride != 0 ? 1 : 0);
			}
		}
		int32 ManagerTickProfileOverride = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66MobManagerTickProfileEnabled="), ManagerTickProfileOverride))
		{
			if (IConsoleVariable* ManagerTickProfileCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.ManagerTickProfileEnabled")))
			{
				ManagerTickProfileCVar->Set(ManagerTickProfileOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Mob.ManagerTickProfileEnabled set to %d from command line."), ManagerTickProfileOverride != 0 ? 1 : 0);
			}
		}
		int32 RangedDiagnosticLoggingOverride = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66RangedDiagnosticLogging="), RangedDiagnosticLoggingOverride))
		{
			if (IConsoleVariable* RangedDiagnosticLoggingCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Ranged.DiagnosticLogging")))
			{
				RangedDiagnosticLoggingCVar->Set(RangedDiagnosticLoggingOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Ranged.DiagnosticLogging set to %d from command line."), RangedDiagnosticLoggingOverride != 0 ? 1 : 0);
			}
		}
		float AutoCaptureHeroHPOverride = 0.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66AutoCaptureHeroHPOverride="), AutoCaptureHeroHPOverride))
		{
			CVarT66AutoCaptureHeroHPOverride.AsVariable()->Set(AutoCaptureHeroHPOverride, ECVF_SetByCommandline);
			UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.AutoCapture.HeroHPOverride requested %.1f from command line."), AutoCaptureHeroHPOverride);
		}
		auto ReadIntCVar = [](const TCHAR* CVarName, const int32 FallbackValue)
		{
			if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarName))
			{
				return CVar->GetInt();
			}
			return FallbackValue;
		};
		auto ReadFloatCVar = [](const TCHAR* CVarName, const float FallbackValue)
		{
			if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarName))
			{
				return CVar->GetFloat();
			}
			return FallbackValue;
		};
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[PerfAutomation] MobRoutingFlags UseLightweight=%d RouteRush=%d RouteFlying=%d RouteRanged=%d UseTouchDamageOverlap=%d ManagerTickProfile=%d RangedDiagnosticLogging=%d HeroHPOverride=%.1f"),
			ReadIntCVar(TEXT("T66.Mob.UseLightweight"), 0),
			ReadIntCVar(TEXT("T66.Mob.Diagnostics.RouteRushLightweight"), 1),
			ReadIntCVar(TEXT("T66.Mob.Diagnostics.RouteFlyingLightweight"), 1),
			ReadIntCVar(TEXT("T66.Mob.Diagnostics.RouteRangedLightweight"), 1),
			ReadIntCVar(TEXT("T66.Mob.Diagnostics.UseTouchDamageOverlap"), 1),
			ReadIntCVar(TEXT("T66.Mob.ManagerTickProfileEnabled"), 0),
			ReadIntCVar(TEXT("T66.Ranged.DiagnosticLogging"), 0),
			ReadFloatCVar(TEXT("T66.AutoCapture.HeroHPOverride"), 0.0f));
		if (UWorld* ResetWorld = GetWorld())
		{
			if (UT66MobManagerSubsystem* MobManager = ResetWorld->GetSubsystem<UT66MobManagerSubsystem>())
			{
				MobManager->ResetRangedPressureDiagnostics(TEXT("EnemyWavePerfStart"));
			}
		}

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[PerfAutomation] Enemy wave perf mode failed: missing world, pawn, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bGameplayFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PerfAutomation] Enemy wave perf mode failed: no gameplay floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
		{
			if (const UCapsuleComponent* Capsule = CharacterPawn->GetCapsuleComponent())
			{
				PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			}
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
		{
			if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}
		}
		ControlledPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		ControlledPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-35.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(ControlledPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
			const float RequestedHeroHPOverride = CVarT66AutoCaptureHeroHPOverride.GetValueOnGameThread();
			if (RequestedHeroHPOverride > 0.0f)
			{
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[PerfAutomation] AutoCaptureHeroHPOverrideSnapshot Phase=Before RequestedHP=%.1f MaxHP=%.1f CurrentHP=%.1f CurrentHearts=%d MaxHearts=%d Gold=%d Debt=%d StageTimerActive=%d"),
					RequestedHeroHPOverride,
					RunState->GetMaxHP(),
					RunState->GetCurrentHP(),
					RunState->GetCurrentHearts(),
					RunState->GetMaxHearts(),
					RunState->GetCurrentGold(),
					RunState->GetCurrentDebt(),
					RunState->GetStageTimerActive() ? 1 : 0);
				const float AppliedHeroHPOverride = RunState->ApplyAutomationHeroHPOverride(RequestedHeroHPOverride, *Mode);
				UE_LOG(
					LogTemp,
					Display,
					TEXT("AutoCapture hero HP override applied: starting HP = %.1f"),
					AppliedHeroHPOverride);
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[PerfAutomation] AutoCaptureHeroHPOverride AppliedHP=%.1f RequestedHP=%.1f MaxHP=%.1f CurrentHP=%.1f CurrentHearts=%d MaxHearts=%d"),
					AppliedHeroHPOverride,
					RequestedHeroHPOverride,
					RunState->GetMaxHP(),
					RunState->GetCurrentHP(),
					RunState->GetCurrentHearts(),
					RunState->GetMaxHearts());
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[PerfAutomation] AutoCaptureHeroHPOverrideSnapshot Phase=After AppliedHP=%.1f RequestedHP=%.1f MaxHP=%.1f CurrentHP=%.1f CurrentHearts=%d MaxHearts=%d Gold=%d Debt=%d StageTimerActive=%d"),
					AppliedHeroHPOverride,
					RequestedHeroHPOverride,
					RunState->GetMaxHP(),
					RunState->GetCurrentHP(),
					RunState->GetCurrentHearts(),
					RunState->GetMaxHearts(),
					RunState->GetCurrentGold(),
					RunState->GetCurrentDebt(),
					RunState->GetStageTimerActive() ? 1 : 0);
			}
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[PerfAutomation] Enemy wave perf mode floor=%d location=%s."),
			TargetFloor->FloorNumber,
			*ControlledPawn->GetActorLocation().ToCompactString());
		return;
	}
#endif

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

	if (Mode == TEXT("camerawallocclusion") || Mode == TEXT("wallocclusion") || Mode == TEXT("wallfade"))
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

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		UPrimitiveComponent* WallComponent = nullptr;
		if (World && HeroPawn)
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* CandidateActor = *It;
				if (!CandidateActor
					|| !CandidateActor->ActorHasTag(T66GameplayAutomationTerrainVisualTag)
					|| !CandidateActor->ActorHasTag(T66GameplayAutomationTraversalBarrierTag)
					|| !CandidateActor->ActorHasTag(T66GameplayAutomationCameraWallVisualTag)
					|| CandidateActor->ActorHasTag(T66GameplayAutomationTowerCeilingTag))
				{
					continue;
				}

				TArray<UPrimitiveComponent*> Components;
				CandidateActor->GetComponents<UPrimitiveComponent>(Components);
				for (UPrimitiveComponent* Component : Components)
				{
					if (!Component || !Component->IsRegistered() || !Component->IsVisible())
					{
						continue;
					}

					const FVector Extents = Component->Bounds.BoxExtent;
					const float ThinExtent = FMath::Min(Extents.X, Extents.Y);
					const float LongExtent = FMath::Max(Extents.X, Extents.Y);
					if ((CandidateActor->ActorHasTag(T66GameplayAutomationCameraWallVisualTag)
							|| Component->ComponentHasTag(T66GameplayAutomationCameraWallVisualTag))
						&& Extents.Z >= 300.f
						&& ThinExtent >= 20.f
						&& ThinExtent <= 650.f
						&& LongExtent >= 120.f
						&& LongExtent <= 9000.f)
					{
						WallComponent = Component;
						break;
					}
				}

				if (WallComponent)
				{
					break;
				}
			}
		}

		if (WallComponent && HeroPawn)
		{
			if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}

			const FBoxSphereBounds Bounds = WallComponent->Bounds;
			const FVector Extents = Bounds.BoxExtent;
			const bool bWallRunsAlongX = Extents.X >= Extents.Y;
			const float ThinExtent = bWallRunsAlongX ? Extents.Y : Extents.X;
			const FVector WallNormal = bWallRunsAlongX ? FVector(0.f, 1.f, 0.f) : FVector(1.f, 0.f, 0.f);
			const float CapsuleHalfHeight = HeroPawn->GetCapsuleComponent()
				? HeroPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
				: 100.f;
			FVector HeroLocation = Bounds.Origin - WallNormal * (ThinExtent + 170.f);
			HeroLocation.Z = Bounds.Origin.Z - Extents.Z + CapsuleHalfHeight + 12.f;
			HeroPawn->SetActorLocation(HeroLocation, false, nullptr, ETeleportType::TeleportPhysics);
			HeroPawn->SetActorRotation((-WallNormal).Rotation(), ETeleportType::TeleportPhysics);

			if (HeroPawn->CameraBoom)
			{
				HeroPawn->CameraBoom->TargetArmLength = ThinExtent + 700.f;
				HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 140.f));
				HeroPawn->CameraBoom->bDoCollisionTest = false;
			}

			SetControlRotation((-WallNormal + FVector(0.f, 0.f, -0.18f)).Rotation());
			UpdateGameplayCameraWallOcclusion(0.1f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Gameplay automation wall occlusion capture could not find a tagged tower wall visual."));
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

#if !UE_BUILD_SHIPPING
	if (Mode == TEXT("combatdamagelog") || Mode == TEXT("damagelog"))
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

		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->ApplyDamage(20, this, FName(TEXT("DebugDamage")), this);
		}
		return;
	}

	if (Mode == TEXT("hero1axeaoe") || Mode == TEXT("vfxlabhero1axeaoe") || Mode == TEXT("hero1axeaoehitbox"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		bGameplayAutomationCameraZoomLocked = FParse::Param(FCommandLine::Get(), TEXT("T66GameplayAutoLockCameraZoom"));

		if (UWorld* World = GetWorld())
		{
			const bool bHitboxProofMode = Mode == TEXT("hero1axeaoehitbox");
			static const FName Hero1AxeTargetTag(TEXT("T66Automation_Hero1AxeAOETarget"));
			if (AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode()))
			{
				GameMode->SetEnemyDirectorSpawningPaused(true);
			}
			int32 ClearedEnemyCount = 0;
			for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
			{
				It->Destroy();
				++ClearedEnemyCount;
			}
			UE_LOG(LogTemp, Display, TEXT("Gameplay automation: cleared %d existing enemies for Hero1AxeAOE preview."), ClearedEnemyCount);

			if (!bHitboxProofMode)
			{
				for (TActorIterator<AT66Hero1AxeAOEVFXLabActor> It(World); It; ++It)
				{
					UE_LOG(LogTemp, Display, TEXT("Gameplay automation: Hero1AxeAOE lab actor already present: %s"), *It->GetName());
					return;
				}
			}

			APawn* ControlledPawn = GetPawn();
			AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
			if (HeroPawn)
			{
				if (FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOECenterPlayer")))
				{
					if (AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode()))
					{
						T66TowerMapTerrain::FLayout TowerLayout;
						if (GameMode->IsUsingTowerMainMapLayout() && GameMode->GetTowerMainMapLayout(TowerLayout))
						{
							const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
							for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
							{
								if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
								{
									TargetFloor = &Floor;
									break;
								}
								if (!TargetFloor && Floor.bGameplayFloor)
								{
									TargetFloor = &Floor;
								}
							}

							if (TargetFloor)
							{
								float PawnHalfHeight = 100.0f;
								if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
								{
									PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
								}

								FVector TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f);
								if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
								{
									Movement->StopMovementImmediately();
								}
								HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
								HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
								SetControlRotation(FRotator(-30.0f, 0.0f, 0.0f));
								UE_LOG(LogTemp, Display, TEXT("Gameplay automation: centered Hero1AxeAOE player at %s"), *HeroPawn->GetActorLocation().ToCompactString());
							}
						}
					}
				}

				float RequestedCameraArmLength = 0.0f;
				if (FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoCameraArmLength="), RequestedCameraArmLength)
					&& HeroPawn->CameraBoom)
				{
					DesiredGameplayCameraArmLength = FMath::Max(100.0f, RequestedCameraArmLength);
					HeroPawn->CameraBoom->TargetArmLength = DesiredGameplayCameraArmLength;
				}
			}

			if (bHitboxProofMode)
			{
				if (!HeroPawn || !HeroPawn->CombatComponent)
				{
					UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEHitboxProof] Failed: missing hero or combat component."));
					return;
				}

				static const FName HitboxProofTag(TEXT("T66Automation_Hero1AxeAOEHitboxProof"));
				for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
				{
					if (It->Tags.Contains(HitboxProofTag) || It->Tags.Contains(Hero1AxeTargetTag))
					{
						It->Destroy();
					}
				}
				for (TActorIterator<AT66Hero1AxeAOEVFXLabActor> It(World); It; ++It)
				{
					It->Destroy();
				}

				const FName HeroID = HeroPawn->HeroID.IsNone() ? FName(TEXT("Hero_1")) : HeroPawn->HeroID;
				const FName AoeWeaponID = UT66WeaponManagerSubsystem::MakeWeaponID(HeroID, ET66WeaponRarity::Black, ET66AttackCategory::AOE);
				bool bSelectedAoeWeapon = false;
				if (UT66WeaponManagerSubsystem* WeaponManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr)
				{
					WeaponManager->BuildWeaponOffers(HeroID, ET66WeaponRarity::Black);
					bSelectedAoeWeapon = WeaponManager->SelectWeapon(AoeWeaponID);
				}
				UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] EquippedAoeWeapon=%s Success=%d HeroID=%s"),
					*AoeWeaponID.ToString(),
					bSelectedAoeWeapon ? 1 : 0,
					*HeroID.ToString());

				if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->DisableMovement();
					Movement->SetMovementMode(MOVE_None);
				}

				FVector Forward = HeroPawn->GetActorForwardVector().GetSafeNormal2D();
				if (Forward.IsNearlyZero())
				{
					Forward = FVector::ForwardVector;
				}
				FVector Right = HeroPawn->GetActorRightVector().GetSafeNormal2D();
				if (Right.IsNearlyZero())
				{
					Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal2D();
				}

				const FVector HeroLocation = HeroPawn->GetActorLocation();
				const FVector PrimaryLocation = HeroLocation + Forward * 360.0f;
				struct FHitboxProofTargetSpec
				{
					const TCHAR* Label = TEXT("");
					FVector Offset = FVector::ZeroVector;
					bool bExpectedHit = false;
				};
				const FHitboxProofTargetSpec TargetSpecs[] =
				{
					{ TEXT("Primary"), FVector::ZeroVector, true },
					{ TEXT("InsideForward"), Forward * 150.0f, true },
					{ TEXT("InsideSide"), Forward * 85.0f + Right * 150.0f, true },
					{ TEXT("OutsideBehind"), -Forward * 180.0f, false },
					{ TEXT("OutsideBackSide"), -Forward * 90.0f + Right * 180.0f, false },
				};

				FString TargetMobIDString(TEXT("Slime"));
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetMob="), TargetMobIDString);
				const FName TargetMobID(*TargetMobIDString);

				TArray<TWeakObjectPtr<AT66EnemyBase>> ProofTargets;
				TArray<FString> ProofLabels;
				TArray<bool> ProofExpectedHits;
				AT66EnemyBase* PrimaryTarget = nullptr;
				for (const FHitboxProofTargetSpec& TargetSpec : TargetSpecs)
				{
					FActorSpawnParameters TargetSpawnParameters;
					TargetSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					const FVector TargetLocation = PrimaryLocation + TargetSpec.Offset;
					AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
						AT66EnemyBase::StaticClass(),
						TargetLocation,
						(HeroLocation - TargetLocation).Rotation(),
						TargetSpawnParameters);
					if (!Enemy)
					{
						UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEHitboxProof] Failed to spawn target label=%s"), TargetSpec.Label);
						continue;
					}

					Enemy->Tags.AddUnique(HitboxProofTag);
					Enemy->Tags.AddUnique(Hero1AxeTargetTag);
					Enemy->SetActorEnableCollision(true);
					Enemy->ConfigureAsMob(TargetMobID);
					Enemy->MaxHP = 20000;
					Enemy->CurrentHP = 20000;
					Enemy->TouchDamageHearts = 0;
					Enemy->PointValue = 0;
					Enemy->XPValue = 0;
					Enemy->bDropsLoot = false;
					Enemy->OwningDirector = nullptr;
					if (UCharacterMovementComponent* EnemyMovement = Enemy->GetCharacterMovement())
					{
						EnemyMovement->StopMovementImmediately();
						EnemyMovement->GravityScale = 0.0f;
						EnemyMovement->DisableMovement();
						EnemyMovement->SetMovementMode(MOVE_None);
					}
					Enemy->ForceMobVertexAnimationClipForAutomation(FName(TEXT("HitReact")), 30.0f);
					Enemy->SetActorTickEnabled(false);

					if (FCString::Strcmp(TargetSpec.Label, TEXT("Primary")) == 0)
					{
						PrimaryTarget = Enemy;
					}

					ProofTargets.Add(Enemy);
					ProofLabels.Add(FString(TargetSpec.Label));
					ProofExpectedHits.Add(TargetSpec.bExpectedHit);
					UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] Spawned Target=%s ExpectedHit=%d Location=%s"),
						TargetSpec.Label,
						TargetSpec.bExpectedHit ? 1 : 0,
						*Enemy->GetActorLocation().ToCompactString());
				}

				if (!PrimaryTarget)
				{
					UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEHitboxProof] Failed: no primary target."));
					return;
				}

				HeroPawn->CombatComponent->SetAutoAttackSuppressed(true);
				HeroPawn->CombatComponent->SetLockedTarget(PrimaryTarget);

				float FireDelaySeconds = 5.4f;
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEHitboxFireDelay="), FireDelaySeconds);
				FireDelaySeconds = FMath::Clamp(FireDelaySeconds, 0.2f, 30.0f);
				float VFXLeadSeconds = 0.12f;
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEHitboxVFXLeadSeconds="), VFXLeadSeconds);
				VFXLeadSeconds = FMath::Clamp(VFXLeadSeconds, 0.0f, FMath::Max(0.0f, FireDelaySeconds - 0.05f));
				const float VFXSpawnDelaySeconds = FMath::Max(0.01f, FireDelaySeconds - VFXLeadSeconds);
				TWeakObjectPtr<AT66HeroBase> WeakHero(HeroPawn);
				TWeakObjectPtr<AT66EnemyBase> WeakPrimaryTarget(PrimaryTarget);
				const float ScheduledFireDelaySeconds = FireDelaySeconds;
				const float ScheduledVFXLeadSeconds = VFXLeadSeconds;
				FTimerHandle HitboxProofVFXHandle;
				World->GetTimerManager().SetTimer(
					HitboxProofVFXHandle,
					FTimerDelegate::CreateWeakLambda(this, [this, WeakHero, WeakPrimaryTarget, ScheduledFireDelaySeconds, ScheduledVFXLeadSeconds]()
					{
						AT66HeroBase* CapturedHero = WeakHero.Get();
						AT66EnemyBase* CapturedPrimary = WeakPrimaryTarget.Get();
						UWorld* CapturedWorld = GetWorld();
						if (!CapturedHero || !CapturedPrimary || !CapturedWorld)
						{
							UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEHitboxProof] VFX spawn skipped: missing hero, primary target, or world."));
							return;
						}

						FVector VFXForward = (CapturedPrimary->GetActorLocation() - CapturedHero->GetActorLocation()).GetSafeNormal2D();
						if (VFXForward.IsNearlyZero())
						{
							VFXForward = CapturedHero->GetActorForwardVector().GetSafeNormal2D();
						}
						if (VFXForward.IsNearlyZero())
						{
							VFXForward = FVector::ForwardVector;
						}

						FActorSpawnParameters VFXSpawnParameters;
						VFXSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						VFXSpawnParameters.Owner = CapturedHero;
						const FVector VFXLocation = CapturedPrimary->GetActorLocation() - VFXForward * 28.0f + FVector(0.0f, 0.0f, -82.0f);
						const FRotator VFXRotation = VFXForward.Rotation();
						AT66Hero1AxeAOEVFXLabActor* LabVFXActor = CapturedWorld->SpawnActor<AT66Hero1AxeAOEVFXLabActor>(
							AT66Hero1AxeAOEVFXLabActor::StaticClass(),
							VFXLocation,
							VFXRotation,
							VFXSpawnParameters);
						UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] VFXSpawned=%d WorldTime=%.3f FireDelay=%.2f VFXLead=%.2f Location=%s Rotation=%s"),
							LabVFXActor ? 1 : 0,
							CapturedWorld->GetTimeSeconds(),
							ScheduledFireDelaySeconds,
							ScheduledVFXLeadSeconds,
							*VFXLocation.ToCompactString(),
							*VFXRotation.ToCompactString());
					}),
					VFXSpawnDelaySeconds,
					false);
				FTimerHandle HitboxProofFireHandle;
				World->GetTimerManager().SetTimer(
					HitboxProofFireHandle,
					FTimerDelegate::CreateWeakLambda(this, [this, WeakHero, WeakPrimaryTarget, ProofTargets, ProofLabels, ProofExpectedHits, ScheduledFireDelaySeconds, ScheduledVFXLeadSeconds]()
					{
						AT66HeroBase* CapturedHero = WeakHero.Get();
						AT66EnemyBase* CapturedPrimary = WeakPrimaryTarget.Get();
						if (!CapturedHero || !CapturedPrimary || !CapturedHero->CombatComponent)
						{
							UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEHitboxProof] Fire skipped: missing hero, primary target, or combat component."));
							return;
						}
						if (UWorld* FireWorld = GetWorld())
						{
							UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] Fire WorldTime=%.3f FireDelay=%.2f VFXLead=%.2f Primary=%s"),
								FireWorld->GetTimeSeconds(),
								ScheduledFireDelaySeconds,
								ScheduledVFXLeadSeconds,
								*GetNameSafe(CapturedPrimary));
						}

						TArray<int32> HPBefore;
						HPBefore.Reserve(ProofTargets.Num());
						for (const TWeakObjectPtr<AT66EnemyBase>& WeakTarget : ProofTargets)
						{
							HPBefore.Add(WeakTarget.IsValid() ? WeakTarget.Get()->CurrentHP : INDEX_NONE);
						}

						CapturedHero->CombatComponent->SetLockedTarget(CapturedPrimary);
						CapturedHero->CombatComponent->SetAutoAttackSuppressed(false);
						CapturedHero->CombatComponent->PerformAutomationAutoAttackNow();
						CapturedHero->CombatComponent->SetAutoAttackSuppressed(true);

						FTimerHandle HitboxProofLogHandle;
						if (UWorld* CapturedWorld = GetWorld())
						{
							CapturedWorld->GetTimerManager().SetTimer(
								HitboxProofLogHandle,
								FTimerDelegate::CreateWeakLambda(this, [this, ProofTargets, ProofLabels, ProofExpectedHits, HPBefore]()
								{
									UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance()
										? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>()
										: nullptr;
									for (int32 Index = 0; Index < ProofTargets.Num(); ++Index)
									{
										AT66EnemyBase* Target = ProofTargets[Index].Get();
										const int32 Before = HPBefore.IsValidIndex(Index) ? HPBefore[Index] : INDEX_NONE;
										const int32 After = Target ? Target->CurrentHP : INDEX_NONE;
										const bool bActualHit = Before != INDEX_NONE && After != INDEX_NONE && After < Before;
										const bool bExpectedHit = ProofExpectedHits.IsValidIndex(Index) ? ProofExpectedHits[Index] : false;
										const FString Label = ProofLabels.IsValidIndex(Index) ? ProofLabels[Index] : FString::Printf(TEXT("Target%d"), Index);
										if (FloatingText && Target && bActualHit)
										{
											const int32 DamageDelta = Before - After;
											FloatingText->ShowDamageNumber(Target, DamageDelta, UT66FloatingCombatTextSubsystem::EventType_Crit);
											if (UWorld* DebugWorld = Target->GetWorld())
											{
												DrawDebugString(
													DebugWorld,
													Target->GetActorLocation() + FVector(0.0f, 0.0f, 185.0f),
													FString::Printf(TEXT("%d"), DamageDelta),
													nullptr,
													FColor::Yellow,
													2.0f,
													true,
													3.4f);
											}
											if (GEngine)
											{
												GEngine->AddOnScreenDebugMessage(
													-1,
													2.0f,
													FColor::Orange,
													FString::Printf(TEXT("%s -%d"), *Label, DamageDelta));
											}
											UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] DamageNumber Target=%s Amount=%d"),
												*Label,
												DamageDelta);
										}
										UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] Target=%s ExpectedHit=%d ActualHit=%d HPBefore=%d HPAfter=%d Result=%s"),
											*Label,
											bExpectedHit ? 1 : 0,
											bActualHit ? 1 : 0,
											Before,
											After,
											(bActualHit == bExpectedHit) ? TEXT("PASS") : TEXT("FAIL"));
									}
								}),
								0.2f,
								false);
						}
					}),
					FireDelaySeconds,
					false);

				UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] Armed FireDelay=%.2f VFXLead=%.2f VFXSpawnDelay=%.2f Targets=%d Primary=%s"),
					FireDelaySeconds,
					VFXLeadSeconds,
					VFXSpawnDelaySeconds,
					ProofTargets.Num(),
					*GetNameSafe(PrimaryTarget));
				return;
			}

			const FVector SpawnOrigin = ControlledPawn ? ControlledPawn->GetActorLocation() : FVector::ZeroVector;
			FVector LabSpawnOffset(0.0f, -460.0f, -82.0f);
			FString LabSpawnOffsetString;
			if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabOffset="), LabSpawnOffsetString))
			{
				FVector ParsedOffset;
				if (ParsedOffset.InitFromString(LabSpawnOffsetString))
				{
					LabSpawnOffset = ParsedOffset;
				}
			}
			FVector LabSpawnLocation = SpawnOrigin + LabSpawnOffset;
			float LabForwardOffset = 0.0f;
			float LabRightOffset = 0.0f;
			float LabVerticalOffset = 0.0f;
			const bool bHasLocalOffsetComponents =
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabForwardOffset="), LabForwardOffset) |
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabRightOffset="), LabRightOffset) |
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabVerticalOffset="), LabVerticalOffset);
			if (ControlledPawn && bHasLocalOffsetComponents)
			{
				LabSpawnLocation = SpawnOrigin
					+ ControlledPawn->GetActorForwardVector() * LabForwardOffset
					+ ControlledPawn->GetActorRightVector() * LabRightOffset
					+ FVector::UpVector * LabVerticalOffset;
			}
			else if (ControlledPawn && FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabOffsetLocal")))
			{
				LabSpawnLocation = SpawnOrigin
					+ ControlledPawn->GetActorForwardVector() * LabSpawnOffset.X
					+ ControlledPawn->GetActorRightVector() * LabSpawnOffset.Y
					+ FVector::UpVector * LabSpawnOffset.Z;
			}
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Name = TEXT("T66Hero1AxeAOEVFXLab_AOE");
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AT66Hero1AxeAOEVFXLabActor* LabActor = World->SpawnActor<AT66Hero1AxeAOEVFXLabActor>(
				AT66Hero1AxeAOEVFXLabActor::StaticClass(),
				LabSpawnLocation,
				FRotator::ZeroRotator,
				SpawnParameters))
			{
				UE_LOG(LogTemp, Display, TEXT("Gameplay automation: spawned Hero1AxeAOE lab actor at %s"), *LabActor->GetActorLocation().ToCompactString());

				if (FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOESpawnTargets")))
				{
					for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
					{
						if (It->Tags.Contains(Hero1AxeTargetTag))
						{
							It->Destroy();
						}
					}

					int32 TargetCount = 3;
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetCount="), TargetCount);
					TargetCount = FMath::Clamp(TargetCount, 1, 6);

					float TargetSpacing = 145.0f;
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetSpacing="), TargetSpacing);
					TargetSpacing = FMath::Clamp(TargetSpacing, 80.0f, 260.0f);

					float TargetForwardOffset = 210.0f;
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetForwardOffset="), TargetForwardOffset);
					TargetForwardOffset = FMath::Clamp(TargetForwardOffset, 80.0f, 420.0f);

					FString TargetMobIDString(TEXT("Slime"));
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetMob="), TargetMobIDString);
					const FName TargetMobID(*TargetMobIDString);

					FActorSpawnParameters TargetSpawnParameters;
					TargetSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					const float CenterIndex = (static_cast<float>(TargetCount) - 1.0f) * 0.5f;
					for (int32 TargetIndex = 0; TargetIndex < TargetCount; ++TargetIndex)
					{
						const float LateralOffset = (static_cast<float>(TargetIndex) - CenterIndex) * TargetSpacing;
						const FVector TargetLocation = LabActor->GetActorLocation() + FVector(TargetForwardOffset, LateralOffset, 82.0f);
						FRotator TargetRotation = (SpawnOrigin - TargetLocation).Rotation();
						TargetRotation.Pitch = 0.0f;
						TargetRotation.Roll = 0.0f;
						TargetSpawnParameters.Name = FName(*FString::Printf(TEXT("T66Hero1AxeAOETarget_%02d"), TargetIndex + 1));

						AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
							AT66EnemyBase::StaticClass(),
							TargetLocation,
							TargetRotation,
							TargetSpawnParameters);
						if (!Enemy)
						{
							UE_LOG(LogTemp, Error, TEXT("Gameplay automation: failed to spawn Hero1AxeAOE target %d"), TargetIndex + 1);
							continue;
						}

						Enemy->Tags.AddUnique(Hero1AxeTargetTag);
						Enemy->SetActorEnableCollision(false);
						Enemy->MaxHP = 20000;
						Enemy->CurrentHP = 20000;
						Enemy->TouchDamageHearts = 0;
						Enemy->PointValue = 0;
						Enemy->XPValue = 0;
						Enemy->bDropsLoot = false;
						Enemy->OwningDirector = nullptr;
						Enemy->ConfigureAsMob(TargetMobID);
						if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
						{
							Movement->StopMovementImmediately();
							Movement->GravityScale = 0.0f;
							Movement->DisableMovement();
							Movement->SetMovementMode(MOVE_None);
						}
						Enemy->ForceMobVertexAnimationClipForAutomation(FName(TEXT("HitReact")), 30.0f);
						Enemy->SetActorTickEnabled(false);
						UE_LOG(LogTemp, Display, TEXT("Gameplay automation: spawned Hero1AxeAOE target %s at %s"),
							*TargetMobID.ToString(),
							*TargetLocation.ToCompactString());
					}
				}
			}
		}
		return;
	}

	if (Mode == TEXT("temporaryprojectiles") || Mode == TEXT("projectilevisuals") || Mode == TEXT("projectileqa"))
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
			const FRotator CameraRotation = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : GetControlRotation();
			const FVector CameraLocation = PlayerCameraManager
				? PlayerCameraManager->GetCameraLocation()
				: (GetPawn() ? GetPawn()->GetActorLocation() + FVector(-520.f, 0.f, 260.f) : FVector(-520.f, 0.f, 260.f));
			const FVector Forward = CameraRotation.Vector().GetSafeNormal();
			const FVector Right = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);
			const FVector Up = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Z);
			const FVector Origin = CameraLocation + Forward * 650.f + Up * 120.f;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AT66HeroProjectile* HeroProjectile = World->SpawnActor<AT66HeroProjectile>(
				AT66HeroProjectile::StaticClass(),
				Origin - Right * 240.f,
				Forward.Rotation(),
				SpawnParams))
			{
				HeroProjectile->SetVisualOnly(true);
				HeroProjectile->Damage = 0;
				HeroProjectile->ConfigureTemporaryProjectileVisual(
					FT66TemporaryProjectileSystem::ProfileHeroPierce(),
					FT66TemporaryProjectileSystem::HeroProjectileColor(),
					1.0f,
					FT66TemporaryProjectileSystem::ProfileIdolOverlay(),
					FT66TemporaryProjectileSystem::HeroProjectileColor(),
					0.85f);
				HeroProjectile->SetProjectileSpeed(0.f);
				HeroProjectile->SetTargetLocation(HeroProjectile->GetActorLocation() + Forward * 900.f);
			}

			const FTransform EnemyProjectileTransform(Forward.Rotation(), Origin - Right * 80.f);
			if (AT66SpitProjectile* EnemyProjectile = World->SpawnActorDeferred<AT66SpitProjectile>(
				AT66SpitProjectile::StaticClass(),
				EnemyProjectileTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				EnemyProjectile->SetVisualOnly(true);
				EnemyProjectile->FinishSpawning(EnemyProjectileTransform);
			}

			if (AT66TrapArrowProjectile* TrapProjectile = World->SpawnActor<AT66TrapArrowProjectile>(
				AT66TrapArrowProjectile::StaticClass(),
				FTransform(Forward.Rotation(), Origin + Right * 80.f),
				SpawnParams))
			{
				TrapProjectile->InitializeProjectile(
					Forward,
					12,
					0.f,
					FT66TemporaryProjectileSystem::HostileProjectileColor(),
					FT66TemporaryProjectileSystem::HostileProjectileColor());
			}

			const FTransform DebuffProjectileTransform(Forward.Rotation(), Origin + Right * 240.f);
			if (AT66UniqueDebuffProjectile* DebuffProjectile = World->SpawnActorDeferred<AT66UniqueDebuffProjectile>(
				AT66UniqueDebuffProjectile::StaticClass(),
				DebuffProjectileTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				DebuffProjectile->EffectType = ET66HeroStatusEffectType::Curse;
				DebuffProjectile->SetVisualOnly(true);
				DebuffProjectile->FinishSpawning(DebuffProjectileTransform);
			}
		}
		return;
	}

	if (Mode == TEXT("trapprojectilehitbox") || Mode == TEXT("trapprojectiledebug"))
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
			const FVector Forward = GetControlRotation().Vector().GetSafeNormal();
			const FVector ProjectilePreviewLocation = GetPawn()
				? GetPawn()->GetActorLocation() + Forward * 560.f + FVector(0.f, 0.f, 110.f)
				: FVector(560.f, 0.f, 160.f);
			const FTransform SpawnTransform(Forward.Rotation(), ProjectilePreviewLocation);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66TrapArrowProjectile* Projectile = World->SpawnActor<AT66TrapArrowProjectile>(
				AT66TrapArrowProjectile::StaticClass(),
				SpawnTransform,
				SpawnParams))
			{
				Projectile->InitializeProjectile(
					Forward,
					12,
					0.f,
					FT66TemporaryProjectileSystem::HostileProjectileColor(),
					FT66TemporaryProjectileSystem::HostileProjectileColor());
			}
		}
		return;
	}

	if (Mode == TEXT("trapcontainers") || Mode == TEXT("trapdebugcontainers") || Mode == TEXT("alltrapcontainers"))
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
			const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
			const FVector Forward = YawRotation.Vector().GetSafeNormal();
			const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			FVector PreviewOrigin = GetPawn()
				? GetPawn()->GetActorLocation() + Forward * 920.f
				: FVector(920.f, 0.f, 0.f);
			PreviewOrigin.Z -= 80.f;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			const FTransform FlameTransform(YawRotation, PreviewOrigin - Right * 360.f);
			if (AT66FloorFlameTrap* FlameTrap = World->SpawnActorDeferred<AT66FloorFlameTrap>(
				AT66FloorFlameTrap::StaticClass(),
				FlameTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				FlameTrap->SetActivationMode(ET66TrapActivationMode::Timed);
				FlameTrap->Radius = 190.f;
				FlameTrap->DamageHP = 10;
				FlameTrap->InitialCycleDelaySeconds = 0.05f;
				FlameTrap->WarningDurationSeconds = 0.1f;
				FlameTrap->ActiveDurationSeconds = 8.f;
				FlameTrap->FinishSpawning(FlameTransform);
			}

			const FTransform SpikeTransform(YawRotation, PreviewOrigin + Right * 360.f);
			if (AT66FloorSpikePatchTrap* SpikeTrap = World->SpawnActorDeferred<AT66FloorSpikePatchTrap>(
				AT66FloorSpikePatchTrap::StaticClass(),
				SpikeTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				SpikeTrap->SetActivationMode(ET66TrapActivationMode::Timed);
				SpikeTrap->Radius = 205.f;
				SpikeTrap->DamageHP = 11;
				SpikeTrap->InitialCycleDelaySeconds = 0.05f;
				SpikeTrap->WarningDurationSeconds = 0.1f;
				SpikeTrap->RiseDurationSeconds = 0.1f;
				SpikeTrap->RaisedDurationSeconds = 8.f;
				SpikeTrap->FinishSpawning(SpikeTransform);
			}

			World->SpawnActor<AT66TrapPressurePlate>(
				AT66TrapPressurePlate::StaticClass(),
				PreviewOrigin + Forward * 180.f,
				YawRotation,
				SpawnParams);

			const FVector ProjectilePreviewLocation = PreviewOrigin + Right * 40.f - Forward * 260.f + FVector(0.f, 0.f, 170.f);
			const FTransform ProjectileTransform(Forward.Rotation(), ProjectilePreviewLocation);
			if (AT66TrapArrowProjectile* Projectile = World->SpawnActor<AT66TrapArrowProjectile>(
				AT66TrapArrowProjectile::StaticClass(),
				ProjectileTransform,
				SpawnParams))
			{
				Projectile->InitializeProjectile(
					Forward,
					12,
					0.f,
					FT66TemporaryProjectileSystem::HostileProjectileColor(),
					FT66TemporaryProjectileSystem::HostileProjectileColor());
			}
		}
		return;
	}
#endif

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
	if (Mode == TEXT("slimekingbossqa") || Mode == TEXT("slimekingtelegraphqa") || Mode == TEXT("bosstelegraphqa"))
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

		APawn* ControlledPawn = GetPawn();
		const FVector PlayerReviewLocation = ControlledPawn ? ControlledPawn->GetActorLocation() : T66GameplayLayout::GetStartAreaCenter(120.f);
		const FVector BossReviewLocation = PlayerReviewLocation + FVector(760.f, 0.f, 0.f);
		if (ControlledPawn)
		{
			ControlledPawn->SetActorEnableCollision(false);
			ControlledPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f));
			SetControlRotation(FRotator(-8.f, 0.f, 0.f));

			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
				}
			}
			if (AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn))
			{
				if (HeroPawn->CameraBoom)
				{
					HeroPawn->CameraBoom->TargetArmLength = 720.f;
					HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 125.f));
					HeroPawn->CameraBoom->bDoCollisionTest = false;
				}
			}
		}

		if (UWorld* World = GetWorld())
		{
			UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
			FBossData BossData;
			if (!T66GI || !T66GI->GetBossData(FName(TEXT("Dungeon_SewerSlimeKing")), BossData))
			{
				BossData.BossID = FName(TEXT("Dungeon_SewerSlimeKing"));
				BossData.MaxHP = 1250;
				BossData.AwakenDistance = 1200.f;
				BossData.MoveSpeed = 0.f;
				BossData.FireIntervalSeconds = 4.0f;
				BossData.ProjectileSpeed = 760.f;
				BossData.ProjectileDamageHearts = 1;
				BossData.BossPartProfile = ET66BossPartProfile::Juggernaut;
				BossData.PlaceholderColor = FLinearColor(0.20f, 0.92f, 0.08f, 1.f);
			}

			BossData.MoveSpeed = 0.f;
			BossData.FireIntervalSeconds = 999.f;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66SlimeKingBossTelegraphQA"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
				AT66BossBase::StaticClass(),
				BossReviewLocation,
				FRotator(0.f, 180.f, 0.f),
				SpawnParams);
			if (Boss)
			{
				Boss->Tags.AddUnique(FName(TEXT("T66Automation_SlimeKingBossQA")));
				Boss->InitializeBoss(BossData);
				Boss->SetActorRotation(FRotator(0.f, 180.f, 0.f));
				if (UCharacterMovementComponent* BossMovement = Boss->GetCharacterMovement())
				{
					BossMovement->StopMovementImmediately();
					BossMovement->DisableMovement();
				}
				FT66VisualUtil::SnapToGround(Boss, World);

				static const FName AttackSequence[] =
				{
					FName(TEXT("LeftLobe")),
					FName(TEXT("RightLobe")),
					FName(TEXT("LeftBase")),
					FName(TEXT("RightBase")),
					FName(TEXT("MouthCore"))
				};

				for (int32 Index = 0; Index < UE_ARRAY_COUNT(AttackSequence); ++Index)
				{
					TWeakObjectPtr<AT66BossBase> WeakBoss(Boss);
					const FName AttackPartID = AttackSequence[Index];
					FTimerHandle TimerHandle;
					World->GetTimerManager().SetTimer(
						TimerHandle,
						FTimerDelegate::CreateLambda([WeakBoss, AttackPartID]()
						{
							if (WeakBoss.IsValid())
							{
								WeakBoss->ForceSewerSlimeKingAttackForAutomation(AttackPartID);
							}
						}),
						0.6f + static_cast<float>(Index) * 2.45f,
						false);
				}
			}
		}
		return;
	}

	if (Mode == TEXT("heromovementqa") || Mode == TEXT("herolocomotionqa") || Mode == TEXT("companionmovementqa"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->SetVisibility(ESlateVisibility::Hidden);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[HeroMovementQA] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bGameplayFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[HeroMovementQA] Failed: no gameplay floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector StartLocation = TargetFloor->ArrivalPoint;
		if (StartLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			StartLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (StartLocation.IsNearlyZero())
		{
			StartLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		StartLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		const FVector MoveDirection = FVector(1.0f, 0.0f, 0.0f);
		const FVector RightDirection = FVector(0.0f, 1.0f, 0.0f);
		float CameraDistance = 520.0f;
		float CameraSideOffset = 520.0f;
		float CameraHeight = 240.0f;
		float MoveDurationSeconds = 6.8f;
		float MoveStartDelaySeconds = 0.25f;
		float JumpTimeSeconds = 2.45f;
		float RollTimeSeconds = 4.95f;
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQACameraDistance="), CameraDistance);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQACameraSideOffset="), CameraSideOffset);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQACameraHeight="), CameraHeight);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQAMoveDuration="), MoveDurationSeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQAStartDelay="), MoveStartDelaySeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQAJumpTime="), JumpTimeSeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQARollTime="), RollTimeSeconds);
		CameraDistance = FMath::Clamp(CameraDistance, 250.0f, 2200.0f);
		CameraSideOffset = FMath::Clamp(CameraSideOffset, -1800.0f, 1800.0f);
		CameraHeight = FMath::Clamp(CameraHeight, 120.0f, 900.0f);
		MoveDurationSeconds = FMath::Clamp(MoveDurationSeconds, 1.0f, 12.0f);
		MoveStartDelaySeconds = FMath::Clamp(MoveStartDelaySeconds, 0.0f, 3.0f);
		JumpTimeSeconds = FMath::Clamp(JumpTimeSeconds, 0.0f, MoveDurationSeconds);
		RollTimeSeconds = FMath::Clamp(RollTimeSeconds, 0.0f, MoveDurationSeconds);

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(MOVE_Walking);
		}
		HeroPawn->SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));

		GameMode->SetEnemyDirectorSpawningPaused(true);
		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(true);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(false);
		}

		for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AT66MobBase> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AT66BossBase> It(World); It; ++It)
		{
			It->Destroy();
		}

		AT66CompanionBase* QACompanion = nullptr;
		const FName CompanionID = T66GetHeroMovementQACompanionID();
		if (!CompanionID.IsNone())
		{
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
			{
				FCompanionData CompanionData;
				if (T66GI->GetCompanionData(CompanionID, CompanionData))
				{
					FActorSpawnParameters CompanionSpawnParams;
					CompanionSpawnParams.Owner = HeroPawn;
					CompanionSpawnParams.Instigator = HeroPawn;
					CompanionSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					QACompanion = World->SpawnActor<AT66CompanionBase>(
						AT66CompanionBase::StaticClass(),
						StartLocation + FVector(-155.0f, 115.0f, 0.0f),
						FRotator::ZeroRotator,
						CompanionSpawnParams);
					if (QACompanion)
					{
						QACompanion->Tags.AddUnique(FName(TEXT("T66Automation_HeroMovementQACompanion")));
						QACompanion->InitializeCompanion(CompanionData, FName(TEXT("Default")));
						QACompanion->SetPreviewMode(false);
						FT66VisualUtil::SnapToGround(QACompanion, World);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[HeroMovementQA] Companion data missing for %s."), *CompanionID.ToString());
				}
			}
		}

		const FVector CameraLocation = HeroPawn->GetActorLocation() - MoveDirection * CameraDistance + RightDirection * CameraSideOffset + FVector(0.0f, 0.0f, CameraHeight);
		const FVector CameraLookTarget = HeroPawn->GetActorLocation() + FVector(0.0f, 0.0f, 95.0f);
		FRotator CameraRotation = (CameraLookTarget - CameraLocation).Rotation();
		FActorSpawnParameters CameraSpawnParams;
		CameraSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACameraActor* PreviewCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraLocation, CameraRotation, CameraSpawnParams);
		if (PreviewCamera)
		{
			SetViewTarget(PreviewCamera);
		}
		else
		{
			SetControlRotation(CameraRotation);
		}

		const FName HeroVisualID = T66GetHeroMovementQAVisualID();
		if (UT66CharacterVisualSubsystem* Visuals = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66CharacterVisualSubsystem>() : nullptr)
		{
			UAnimationAsset* HeroWalk = nullptr;
			UAnimationAsset* HeroJump = nullptr;
			UAnimationAsset* HeroIdle = nullptr;
			UAnimationAsset* HeroRoll = nullptr;
			Visuals->GetMovementAnimsForVisual(HeroVisualID, HeroWalk, HeroJump, HeroIdle, HeroRoll);
			UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] HeroVisualID=%s %s"), *HeroVisualID.ToString(), *T66DescribeMovementQAAnimation(TEXT("HeroWalk"), HeroWalk));

			if (!CompanionID.IsNone())
			{
				const FName CompanionVisualID = UT66CharacterVisualSubsystem::GetCompanionVisualID(CompanionID, FName(TEXT("Default")));
				UAnimationAsset* CompanionWalk = nullptr;
				UAnimationAsset* CompanionJump = nullptr;
				UAnimationAsset* CompanionIdle = nullptr;
				UAnimationAsset* CompanionRoll = nullptr;
				Visuals->GetMovementAnimsForVisual(CompanionVisualID, CompanionWalk, CompanionJump, CompanionIdle, CompanionRoll);
				UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] CompanionVisualID=%s %s"), *CompanionVisualID.ToString(), *T66DescribeMovementQAAnimation(TEXT("CompanionWalk"), CompanionWalk));
			}
		}

		TWeakObjectPtr<AT66HeroBase> WeakHero(HeroPawn);
		TWeakObjectPtr<AT66CompanionBase> WeakCompanion(QACompanion);
		TWeakObjectPtr<ACameraActor> WeakPreviewCamera(PreviewCamera);
		const double QAStartWorldTime = World->GetTimeSeconds() + MoveStartDelaySeconds;

		TSharedPtr<FTimerHandle> MoveTimerHandle = MakeShared<FTimerHandle>();
		World->GetTimerManager().SetTimer(
			*MoveTimerHandle,
			FTimerDelegate::CreateLambda([World, WeakHero, MoveTimerHandle, MoveDirection, QAStartWorldTime, MoveDurationSeconds]()
			{
				AT66HeroBase* Hero = WeakHero.Get();
				if (!World || !Hero)
				{
					if (World && MoveTimerHandle.IsValid())
					{
						World->GetTimerManager().ClearTimer(*MoveTimerHandle);
					}
					return;
				}

				const double Elapsed = World->GetTimeSeconds() - QAStartWorldTime;
				if (Elapsed > MoveDurationSeconds)
				{
					if (MoveTimerHandle.IsValid())
					{
						World->GetTimerManager().ClearTimer(*MoveTimerHandle);
					}
					return;
				}

				if (Elapsed >= 0.0)
				{
					Hero->AddMovementInput(MoveDirection, 1.0f, false);
				}
			}),
			0.016f,
			true,
			MoveStartDelaySeconds);

		FTimerHandle JumpTimerHandle;
		World->GetTimerManager().SetTimer(
			JumpTimerHandle,
			FTimerDelegate::CreateLambda([WeakHero]()
			{
				if (AT66HeroBase* Hero = WeakHero.Get())
				{
					Hero->Jump();
				}
			}),
			MoveStartDelaySeconds + JumpTimeSeconds,
			false);

		FTimerHandle RollTimerHandle;
		World->GetTimerManager().SetTimer(
			RollTimerHandle,
			FTimerDelegate::CreateLambda([WeakHero]()
			{
				if (AT66HeroBase* Hero = WeakHero.Get())
				{
					Hero->RollForward();
				}
			}),
			MoveStartDelaySeconds + RollTimeSeconds,
			false);

		if (PreviewCamera)
		{
			FTimerHandle CameraFollowHandle;
			World->GetTimerManager().SetTimer(
				CameraFollowHandle,
				FTimerDelegate::CreateLambda([WeakHero, WeakPreviewCamera, MoveDirection, RightDirection, CameraDistance, CameraSideOffset, CameraHeight]()
				{
					AT66HeroBase* Hero = WeakHero.Get();
					ACameraActor* Camera = WeakPreviewCamera.Get();
					if (!Hero || !Camera)
					{
						return;
					}
					const FVector HeroLocation = Hero->GetActorLocation();
					const FVector FollowCameraLocation = HeroLocation - MoveDirection * CameraDistance + RightDirection * CameraSideOffset + FVector(0.0f, 0.0f, CameraHeight);
					const FVector FollowLookTarget = HeroLocation + FVector(0.0f, 0.0f, 95.0f);
					Camera->SetActorLocation(FollowCameraLocation);
					Camera->SetActorRotation((FollowLookTarget - FollowCameraLocation).Rotation());
				}),
				0.033f,
				true);
		}

		const TSharedRef<FVector> LastHeroLogLocation = MakeShared<FVector>(HeroPawn->GetActorLocation());
		const TSharedRef<FVector> LastCompanionLogLocation = MakeShared<FVector>(QACompanion ? QACompanion->GetActorLocation() : FVector::ZeroVector);
		const TSharedRef<double> LastLogWorldTime = MakeShared<double>(World->GetTimeSeconds());
		for (int32 SampleIndex = 0; SampleIndex <= 14; ++SampleIndex)
		{
			FTimerHandle LogTimerHandle;
			const float SampleTime = MoveStartDelaySeconds + static_cast<float>(SampleIndex) * 0.5f;
			World->GetTimerManager().SetTimer(
				LogTimerHandle,
				FTimerDelegate::CreateLambda([WeakHero, WeakCompanion, LastHeroLogLocation, LastCompanionLogLocation, LastLogWorldTime, SampleTime]()
				{
					AT66HeroBase* Hero = WeakHero.Get();
					if (!Hero)
					{
						UE_LOG(LogTemp, Warning, TEXT("[HeroMovementQA] t=%.2fs hero missing."), SampleTime);
						return;
					}

					UCharacterMovementComponent* Movement = Hero->GetCharacterMovement();
					USkeletalMeshComponent* HeroMesh = Hero->GetMesh();
					const double Now = Hero->GetWorld() ? Hero->GetWorld()->GetTimeSeconds() : *LastLogWorldTime;
					const float DeltaSeconds = FMath::Max(0.001f, static_cast<float>(Now - *LastLogWorldTime));
					const FVector HeroLocation = Hero->GetActorLocation();
					const float HeroDeltaSpeed = FVector::Dist2D(HeroLocation, *LastHeroLogLocation) / DeltaSeconds;
					const float HeroVelocitySpeed = Hero->GetVelocity().Size2D();
					const float HeroMaxWalkSpeed = Movement ? Movement->MaxWalkSpeed : -1.0f;
					const FRotator HeroMeshRotation = HeroMesh ? HeroMesh->GetRelativeRotation() : FRotator::ZeroRotator;
					const float HeroPlayRate = HeroMesh ? HeroMesh->GetPlayRate() : -1.0f;

					float CompanionDeltaSpeed = -1.0f;
					FVector CompanionLocation = FVector::ZeroVector;
					FRotator CompanionMeshRotation = FRotator::ZeroRotator;
					float CompanionPlayRate = -1.0f;
					if (AT66CompanionBase* Companion = WeakCompanion.Get())
					{
						CompanionLocation = Companion->GetActorLocation();
						CompanionDeltaSpeed = FVector::Dist2D(CompanionLocation, *LastCompanionLogLocation) / DeltaSeconds;
						if (USkeletalMeshComponent* CompanionMesh = Companion->SkeletalMesh)
						{
							CompanionMeshRotation = CompanionMesh->GetRelativeRotation();
							CompanionPlayRate = CompanionMesh->GetPlayRate();
						}
						*LastCompanionLogLocation = CompanionLocation;
					}

					UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] t=%.2fs heroLoc=%s heroVelocity=%s heroVelocitySpeed=%.1f heroDeltaSpeed=%.1f maxWalkSpeed=%.1f actorYaw=%.1f meshRelYaw=%.1f meshPlayRate=%.3f companionLoc=%s companionDeltaSpeed=%.1f companionMeshRelYaw=%.1f companionPlayRate=%.3f"),
						SampleTime,
						*HeroLocation.ToCompactString(),
						*Hero->GetVelocity().ToCompactString(),
						HeroVelocitySpeed,
						HeroDeltaSpeed,
						HeroMaxWalkSpeed,
						Hero->GetActorRotation().Yaw,
						HeroMeshRotation.Yaw,
						HeroPlayRate,
						*CompanionLocation.ToCompactString(),
						CompanionDeltaSpeed,
						CompanionMeshRotation.Yaw,
						CompanionPlayRate);

					*LastHeroLogLocation = HeroLocation;
					*LastLogWorldTime = Now;
				}),
				SampleTime,
				false);
		}

		UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] Armed hero=%s visual=%s companion=%s floor=%d start=%s cameraDistance=%.1f cameraSideOffset=%.1f cameraHeight=%.1f moveDuration=%.2f jumpAt=%.2f rollAt=%.2f."),
			*HeroPawn->HeroID.ToString(),
			*T66GetHeroMovementQAVisualID().ToString(),
			*CompanionID.ToString(),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString(),
			CameraDistance,
			CameraSideOffset,
			CameraHeight,
			MoveDurationSeconds,
			JumpTimeSeconds,
			RollTimeSeconds);
		return;
	}

	if (Mode == TEXT("enemyanimpreview") || Mode == TEXT("enemyanimationpreview") || Mode == TEXT("mobanimpreview"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		FString EnemyIDString(TEXT("BoneWalker"));
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewEnemyID="), EnemyIDString);
		EnemyIDString = EnemyIDString.TrimStartAndEnd();
		if (EnemyIDString.IsEmpty())
		{
			EnemyIDString = TEXT("BoneWalker");
		}

		float StartDistance = 1650.0f;
		float StopDistance = 140.0f;
		float CameraDistance = 1120.0f;
		float CameraSideOffset = 420.0f;
		float CameraHeight = 255.0f;
		float TargetForwardOffset = 0.0f;
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewStartDistance="), StartDistance);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewStopDistance="), StopDistance);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewCameraDistance="), CameraDistance);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewCameraSideOffset="), CameraSideOffset);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewCameraHeight="), CameraHeight);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewTargetForwardOffset="), TargetForwardOffset);
		StartDistance = FMath::Clamp(StartDistance, 350.0f, 8000.0f);
		StopDistance = FMath::Clamp(StopDistance, 70.0f, 450.0f);
		CameraDistance = FMath::Clamp(CameraDistance, 350.0f, 2500.0f);
		CameraSideOffset = FMath::Clamp(CameraSideOffset, -1600.0f, 1600.0f);
		CameraHeight = FMath::Clamp(CameraHeight, 80.0f, 900.0f);
		TargetForwardOffset = FMath::Clamp(TargetForwardOffset, -4000.0f, 4000.0f);

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyAnimPreview] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bGameplayFloor && Floor.FloorNumber == TowerLayout.FirstGameplayFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bGameplayFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyAnimPreview] Failed: no gameplay floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;
		TargetLocation.X += TargetForwardOffset;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		HeroPawn->SetActorEnableCollision(false);
		HeroPawn->SetActorHiddenInGame(true);

		GameMode->SetEnemyDirectorSpawningPaused(true);
		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(true);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(false);
		}

		for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AT66MobBase> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AT66BossBase> It(World); It; ++It)
		{
			It->Destroy();
		}

		const FVector MoveDirection = FVector(1.0f, 0.0f, 0.0f);
		const FVector RightDirection = FVector(0.0f, 1.0f, 0.0f);
		FVector PreviewSpawnLocation = TargetLocation - MoveDirection * StartDistance;
		PreviewSpawnLocation.Z = TargetFloor->SurfaceZ + 90.0f;
		FRotator SpawnRotation = (TargetLocation - PreviewSpawnLocation).Rotation();
		SpawnRotation.Pitch = 0.0f;
		SpawnRotation.Roll = 0.0f;

		const FTransform SpawnTransform(SpawnRotation, PreviewSpawnLocation);
		AT66MobBase* PreviewMob = World->SpawnActorDeferred<AT66MobBase>(
			AT66MobBase::StaticClass(),
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (!PreviewMob)
		{
			UE_LOG(LogTemp, Error, TEXT("[EnemyAnimPreview] Failed to spawn AT66MobBase for EnemyID=%s."), *EnemyIDString);
			return;
		}

		const FName EnemyID(*EnemyIDString);
		PreviewMob->Tags.AddUnique(FName(TEXT("T66Automation_EnemyAnimPreview")));
		PreviewMob->MobID = EnemyID;
		PreviewMob->CharacterVisualID = EnemyID;
		PreviewMob->LifecycleState = ET66MobLifecycleState::Active;
		UGameplayStatics::FinishSpawningActor(PreviewMob, SpawnTransform);

		int32 StageNum = 1;
		float DifficultyScalar = 1.0f;
		float EnemyProgressionScalar = 1.0f;
		float FinaleScalar = 1.0f;
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			StageNum = FMath::Max(1, RunState->GetCurrentStage());
			DifficultyScalar = RunState->GetDifficultyScalar();
			FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
		}
		PreviewMob->ConfigureAsMob(EnemyID, ET66EnemyFamily::Melee, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
		PreviewMob->TouchDamageHearts = 0;
		PreviewMob->MaxHP = 200000.0f;
		PreviewMob->CurrentHP = PreviewMob->MaxHP;
		FT66VisualUtil::SnapToGround(PreviewMob, World);

		const FVector CameraLocation = PreviewMob->GetActorLocation() + MoveDirection * CameraDistance + RightDirection * CameraSideOffset + FVector(0.0f, 0.0f, CameraHeight);
		const FVector CameraLookTarget = PreviewMob->GetActorLocation() + FVector(0.0f, 0.0f, 72.0f);
		FRotator CameraRotation = (CameraLookTarget - CameraLocation).Rotation();
		FActorSpawnParameters CameraSpawnParams;
		CameraSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACameraActor* PreviewCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraLocation, CameraRotation, CameraSpawnParams);
		if (PreviewCamera)
		{
			SetViewTarget(PreviewCamera);
		}
		else
		{
			SetControlRotation(CameraRotation);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		TWeakObjectPtr<AT66MobBase> WeakPreviewMob(PreviewMob);
		FTimerHandle PreviewIsolationCleanupHandle;
		World->GetTimerManager().SetTimer(
			PreviewIsolationCleanupHandle,
			FTimerDelegate::CreateLambda([World, WeakPreviewMob]()
			{
				for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
				{
					It->Destroy();
				}
				for (TActorIterator<AT66BossBase> It(World); It; ++It)
				{
					It->Destroy();
				}
				AT66MobBase* PreviewMobToKeep = WeakPreviewMob.Get();
				for (TActorIterator<AT66MobBase> It(World); It; ++It)
				{
					AT66MobBase* Mob = *It;
					if (Mob && Mob != PreviewMobToKeep)
					{
						Mob->Destroy();
					}
				}
			}),
			0.10f,
			true);

		TWeakObjectPtr<ACameraActor> WeakPreviewCamera(PreviewCamera);
		if (PreviewCamera)
		{
			FTimerHandle PreviewCameraFollowHandle;
			World->GetTimerManager().SetTimer(
				PreviewCameraFollowHandle,
				FTimerDelegate::CreateLambda([WeakPreviewMob, WeakPreviewCamera, MoveDirection, RightDirection, CameraDistance, CameraSideOffset, CameraHeight]()
				{
					AT66MobBase* Mob = WeakPreviewMob.Get();
					ACameraActor* Camera = WeakPreviewCamera.Get();
					if (!Mob || !Camera)
					{
						return;
					}

					const FVector MobLocation = Mob->GetActorLocation();
					const FVector FollowCameraLocation = MobLocation + MoveDirection * CameraDistance + RightDirection * CameraSideOffset + FVector(0.0f, 0.0f, CameraHeight);
					const FVector FollowLookTarget = MobLocation + FVector(0.0f, 0.0f, 72.0f);
					Camera->SetActorLocation(FollowCameraLocation);
					Camera->SetActorRotation((FollowLookTarget - FollowCameraLocation).Rotation());
				}),
				0.033f,
				true);
		}

		const float LogTimes[] = { 0.20f, 0.70f, 1.20f, 1.70f, 2.20f, 2.70f, 3.20f, 3.70f, 4.20f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle PreviewLogHandle;
			const float LogTime = LogTimes[Index];
			World->GetTimerManager().SetTimer(
				PreviewLogHandle,
				FTimerDelegate::CreateLambda([WeakPreviewMob, EnemyID, LogTime, TargetLocation]()
				{
					const AT66MobBase* Mob = WeakPreviewMob.Get();
					if (!Mob)
					{
						UE_LOG(LogTemp, Warning, TEXT("[EnemyAnimPreview] t=%.2fs EnemyID=%s mob missing."), LogTime, *EnemyID.ToString());
						return;
					}

					const float DistanceToTarget = FVector::Dist2D(Mob->GetActorLocation(), TargetLocation);
					UE_LOG(LogTemp, Display, TEXT("[EnemyAnimPreview] t=%.2fs EnemyID=%s loc=%s velocity=%s speed=%.1f distance=%.1f family=%d"),
						LogTime,
						*EnemyID.ToString(),
						*Mob->GetActorLocation().ToCompactString(),
						*Mob->StoredVelocity.ToCompactString(),
						Mob->StoredVelocity.Size2D(),
						DistanceToTarget,
						static_cast<int32>(Mob->GetEnemyFamily()));
				}),
				LogTime,
				false);
		}

		UE_LOG(LogTemp, Display, TEXT("[EnemyAnimPreview] Armed EnemyID=%s mob=%s floor=%d spawn=%s target=%s startDistance=%.1f stopDistance=%.1f camera=%s chaseSpeed=%.1f managerDriven=1."),
			*EnemyID.ToString(),
			*GetNameSafe(PreviewMob),
			TargetFloor->FloorNumber,
			*PreviewMob->GetActorLocation().ToCompactString(),
			*TargetLocation.ToCompactString(),
			StartDistance,
			StopDistance,
			*CameraLocation.ToCompactString(),
			PreviewMob->ChaseSpeed);
		return;
	}

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

	if (Mode == TEXT("weapon") || Mode == TEXT("weaponaltar") || Mode == TEXT("weaponaltaroverlay"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		ET66WeaponRarity AutomationRarity = ET66WeaponRarity::Black;
		FString RarityString;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66WeaponAltarRarity="), RarityString))
		{
			const FString NormalizedRarity = RarityString.TrimStartAndEnd().ToLower();
			if (NormalizedRarity == TEXT("red"))
			{
				AutomationRarity = ET66WeaponRarity::Red;
			}
			else if (NormalizedRarity == TEXT("yellow"))
			{
				AutomationRarity = ET66WeaponRarity::Yellow;
			}
			else if (NormalizedRarity == TEXT("white"))
			{
				AutomationRarity = ET66WeaponRarity::White;
			}
		}

		AT66WeaponAltar* AutomationAltar = nullptr;
		if (UWorld* World = GetWorld())
		{
			const FVector AutomationAltarLocation = GetPawn() ? GetPawn()->GetActorLocation() + FVector(300.f, 0.f, 0.f) : FVector::ZeroVector;
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AutomationAltar = World->SpawnActor<AT66WeaponAltar>(AT66WeaponAltar::StaticClass(), AutomationAltarLocation, FRotator::ZeroRotator, SpawnParams);
			if (AutomationAltar)
			{
				AutomationAltar->SetActorHiddenInGame(true);
				AutomationAltar->SetActorEnableCollision(false);
				AutomationAltar->RemainingSelections = FMath::Max(1, AutomationAltar->RemainingSelections);
				AutomationAltar->WeaponOfferRarity = AutomationRarity;
			}
		}

		UT66WeaponAltarOverlayWidget* W = CreateWidget<UT66WeaponAltarOverlayWidget>(this, ResolveWeaponAltarOverlayClass());
		if (W)
		{
			W->SetSourceAltar(AutomationAltar);
			WeaponAltarOverlayWidget = W;
			W->AddToViewport(150);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("vendorinteractable") || Mode == TEXT("vendoronly") || Mode == TEXT("vendor"))
	{
		if (UWorld* World = GetWorld())
		{
			if (AT66VendorInteractable* VendorInteractable = T66FindRegisteredWorldInteractable<AT66VendorInteractable>(World))
			{
				OpenVendorInteractable(VendorInteractable);
				return;
			}
		}

		OpenCasinoVendorTab();
		return;
	}

	if (Mode == TEXT("casinoshop") || Mode == TEXT("shop") || Mode == TEXT("casinotabshop"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToVendorTab();
		return;
	}

	if (Mode == TEXT("casinogambling") || Mode == TEXT("gambling") || Mode == TEXT("casinotabgambling"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToGamblerTab();
		return;
	}

	if (Mode == TEXT("casinoalchemy") || Mode == TEXT("alchemy") || Mode == TEXT("casinotabalchemy"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToVendorTab();
		return;
	}

	if (Mode == TEXT("collector") || Mode == TEXT("collectoroverlay"))
	{
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()); T66GI && !T66GI->IsCollectorPlayable())
		{
			return;
		}
		OpenCollectorOverlay();
		return;
	}

	if (Mode == TEXT("lab") || Mode == TEXT("laboverlay"))
	{
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()); T66GI && !T66GI->IsRunCategoryPlayable(ET66RunCategory::Lab))
		{
			return;
		}

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

	if (Mode == TEXT("arcadeselector") || Mode == TEXT("arcade") || Mode == TEXT("arcademachine"))
	{
		if (T66DeprecatedFeatures::AreArcadeInteractablesDisabled()
			|| T66DeprecatedFeatures::AreArcadeGamesDisabled())
		{
			return;
		}

		if (AT66ArcadeMachineInteractable* AutomationArcade = GetWorld()->SpawnActor<AT66ArcadeMachineInteractable>(
			AT66ArcadeMachineInteractable::StaticClass(),
			GetPawn() ? GetPawn()->GetActorLocation() + FVector(360.f, 0.f, 0.f) : FVector::ZeroVector,
			FRotator::ZeroRotator))
		{
			AutomationArcade->SetActorHiddenInGame(true);
			AutomationArcade->SetActorEnableCollision(false);
			AutomationArcade->Interact(this);
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

void AT66PlayerController::HandleGameplayAutomationSequenceScreenshot()
{
	if (GameplayAutomationScreenshotSequenceDir.IsEmpty() || GameplayAutomationScreenshotSequenceCount <= 0)
	{
		return;
	}

	IFileManager::Get().MakeDirectory(*GameplayAutomationScreenshotSequenceDir, true);
	const FString FramePath = FPaths::Combine(
		GameplayAutomationScreenshotSequenceDir,
		FString::Printf(
			TEXT("%s_%04d.png"),
			*GameplayAutomationScreenshotSequencePrefix,
			GameplayAutomationScreenshotSequenceIndex));
	FScreenshotRequest::RequestScreenshot(FramePath, true, false, false);

	++GameplayAutomationScreenshotSequenceIndex;
	if (GameplayAutomationScreenshotSequenceIndex < GameplayAutomationScreenshotSequenceCount)
	{
		if (GetWorld())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationScreenshotTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationScreenshotTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationSequenceScreenshot,
				GameplayAutomationScreenshotSequenceIntervalSeconds,
				false);
		}
		return;
	}

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
	if (UWorld* World = GetWorld())
	{
		if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			MobManager->EmitRangedPressureSummary(TEXT("GameplayAutomationQuit"), true);
		}
	}
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
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::Full);
		CasinoOverlayWidget->SetShopAllowsSteal(false);
		if (!CasinoOverlayWidget->IsInViewport())
		{
			CasinoOverlayWidget->AddToViewport(100);
		}

		CasinoOverlayWidget->OpenVendorTab();
		ApplyCasinoOverlayInputMode();
	}
}

void AT66PlayerController::CloseCasinoOverlay()
{
	ActiveCasinoInteractable.Reset();
	ActiveVendorInteractable.Reset();

	if (CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->RemoveFromParent();
	}
	RestoreGameplayInputMode();
}

void AT66PlayerController::SwitchCasinoOverlayToGamblerTab()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::Full);
		CasinoOverlayWidget->OpenGamblerTab();
		ApplyCasinoOverlayInputMode();
	}
}

void AT66PlayerController::SwitchCasinoOverlayToVendorTab()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::Full);
		CasinoOverlayWidget->OpenVendorTab();
		ApplyCasinoOverlayInputMode();
	}
}

void AT66PlayerController::SwitchCasinoOverlayToAlchemy()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::Full);
		CasinoOverlayWidget->OpenVendorTab();
		ApplyCasinoOverlayInputMode();
	}
}

bool AT66PlayerController::IsCasinoOverlayOpen() const
{
	return CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport();
}

void AT66PlayerController::ApplyCasinoOverlayInputMode(const bool bReassertNextTick)
{
	if (!CasinoOverlayWidget || !CasinoOverlayWidget->IsInViewport())
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(CasinoOverlayWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (bReassertNextTick)
	{
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (AT66PlayerController* PC = WeakThis.Get())
				{
					PC->ApplyCasinoOverlayInputMode(false);
				}
			}));
		}
	}
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

	if (AT66VendorInteractable* VendorInteractable = ActiveVendorInteractable.Get())
	{
		SpawnLoc = VendorInteractable->GetActorLocation();
		bHasSpawnLoc = true;
	}

	if (!bHasSpawnLoc)
	{
		if (AT66VendorInteractable* VendorInteractable = T66FindRegisteredWorldInteractable<AT66VendorInteractable>(World))
		{
			SpawnLoc = VendorInteractable->GetActorLocation();
			bHasSpawnLoc = true;
		}
	}

	if (!bHasSpawnLoc)
	{
		if (AT66CasinoInteractable* CasinoInteractable = ActiveCasinoInteractable.Get())
		{
			SpawnLoc = CasinoInteractable->GetActorLocation();
			bHasSpawnLoc = true;
		}
	}

	if (!bHasSpawnLoc)
	{
		if (AT66CasinoInteractable* CasinoInteractable = T66FindRegisteredWorldInteractable<AT66CasinoInteractable>(World))
		{
			SpawnLoc = CasinoInteractable->GetActorLocation();
			bHasSpawnLoc = true;
		}
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
	ActiveCasinoInteractable.Reset();
	ActiveVendorInteractable.Reset();
	RestoreGameplayInputMode();
	return true;
}

void AT66PlayerController::OpenCasinoVendorTab()
{
	if (!IsGameplayLevel()) return;

	OpenCasinoOverlay();
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::VendorOnly);
		CasinoOverlayWidget->SetShopAllowsSteal(true);
		CasinoOverlayWidget->OpenVendorTab();
		ApplyCasinoOverlayInputMode();
	}
}

bool AT66PlayerController::OpenCasinoGamblerInteractable(AT66CasinoInteractable* SourceInteractable)
{
	if (!IsGameplayLevel() || !SourceInteractable || SourceInteractable->bConsumed || IsCasinoOverlayOpen())
	{
		return false;
	}

	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CasinoOverlayWidget)
	{
		CasinoOverlayWidget = CreateWidget<UT66CasinoOverlayWidget>(this, ResolveCasinoOverlayClass());
	}

	if (!CasinoOverlayWidget)
	{
		return false;
	}

	ActiveCasinoInteractable = SourceInteractable;
	ActiveVendorInteractable.Reset();
	CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::GamblerOnly);
	CasinoOverlayWidget->SetCasinoGamblerWinGoldAmount(SourceInteractable->GetWinGoldAmount());
	if (!CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->AddToViewport(100);
	}

	CasinoOverlayWidget->OpenGamblerTab();
	ApplyCasinoOverlayInputMode();
	return true;
}

bool AT66PlayerController::OpenVendorInteractable(AT66VendorInteractable* SourceInteractable)
{
	if (!IsGameplayLevel() || !SourceInteractable || IsCasinoOverlayOpen())
	{
		return false;
	}

	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CasinoOverlayWidget)
	{
		CasinoOverlayWidget = CreateWidget<UT66CasinoOverlayWidget>(this, ResolveCasinoOverlayClass());
	}

	if (!CasinoOverlayWidget)
	{
		return false;
	}

	ActiveVendorInteractable = SourceInteractable;
	ActiveCasinoInteractable.Reset();
	CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::VendorOnly);
	CasinoOverlayWidget->SetShopAllowsSteal(true);
	if (!CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->AddToViewport(100);
	}

	CasinoOverlayWidget->OpenVendorTab();
	ApplyCasinoOverlayInputMode();
	return true;
}

void AT66PlayerController::HandleCasinoInteractableGambleResolved(const FName GameID, const bool bSuccessful, const int32 PayoutGold)
{
	(void)GameID;
	(void)bSuccessful;
	(void)PayoutGold;

	AT66CasinoInteractable* CasinoInteractable = ActiveCasinoInteractable.Get();
	if (!CasinoInteractable)
	{
		return;
	}

	CasinoInteractable->HandleCasinoGambleCompleted();
	ActiveCasinoInteractable.Reset();

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakThis]()
		{
			if (AT66PlayerController* PC = WeakThis.Get())
			{
				PC->CloseCasinoOverlay();
			}
		}));
	}
}


void AT66PlayerController::OpenCollectorOverlay()
{
	if (!IsGameplayLevel()) return;
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()); T66GI && !T66GI->IsCollectorPlayable())
	{
		return;
	}
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
	if (T66DeprecatedFeatures::AreArcadeGamesDisabled())
	{
		return false;
	}

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

bool AT66PlayerController::OpenArcadePopupFromFrontend(const FT66ArcadeInteractableData& ArcadeData)
{
	if (T66DeprecatedFeatures::AreArcadeGamesDisabled())
	{
		return false;
	}

	if (ArcadeData.ArcadeClass != ET66ArcadeInteractableClass::PopupArcade || IsArcadePopupOpen())
	{
		return false;
	}

	if (!SpawnArcadePopupWidget(ArcadeData, nullptr))
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
	if (T66DeprecatedFeatures::AreArcadeGamesDisabled())
	{
		return false;
	}

	UT66ArcadePopupWidget* NewPopup = nullptr;
	TSubclassOf<UT66ArcadePopupWidget> PopupWidgetClass = ArcadeData.PopupWidgetClass;
	if (!PopupWidgetClass)
	{
		if (ArcadeData.ArcadeGameType == ET66ArcadeGameType::Random)
		{
			PopupWidgetClass = UT66ArcadeSelectionWidget::StaticClass();
		}
		else if (ArcadeData.ArcadeGameType != ET66ArcadeGameType::None)
		{
			const FName ArcadeGameID = T66WidgetGames::Registry::GetArcadeRowID(ArcadeData.ArcadeGameType);
			if (const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindDescriptor(ArcadeGameID))
			{
				if (UClass* ResolvedWidgetClass = T66WidgetGames::Registry::ResolveWidgetClass(*Descriptor).Get())
				{
					if (ResolvedWidgetClass->IsChildOf(UT66ArcadePopupWidget::StaticClass()))
					{
						PopupWidgetClass = ResolvedWidgetClass;
					}
				}
			}

			if (!PopupWidgetClass)
			{
				PopupWidgetClass = UT66BladeSweepArcadeWidget::StaticClass();
			}
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
	FT66WidgetGameHostContext HostContext;
	HostContext.WorldContextObject = this;
	HostContext.OwningPlayer = this;
	HostContext.ResultCallback = [WeakThis = TWeakObjectPtr<AT66PlayerController>(this)](const FT66WidgetGameResult& Result)
	{
		if (AT66PlayerController* This = WeakThis.Get())
		{
			This->HandleArcadeWidgetGameResult(Result);
		}
	};
	ArcadePopupWidget = NewPopup;
	ArcadePopupWidget->AddToViewport(110);

	if (SourceInteractable)
	{
		StartArcadePopupCountdown(NewPopup, HostContext);
	}
	else
	{
		NewPopup->ActivateWidgetGame(HostContext);
	}

	return true;
}

void AT66PlayerController::StartArcadePopupCountdown(
	UT66ArcadePopupWidget* PopupWidget,
	const FT66WidgetGameHostContext& HostContext)
{
	if (!PopupWidget || PopupWidget != ArcadePopupWidget)
	{
		return;
	}

	ClearArcadePopupCountdown();

	PendingArcadeWidgetGameHostContext = HostContext;
	ArcadeCountdownDurationSeconds = 3.0f;
	ArcadeCountdownStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();

	TSharedRef<SWidget> CountdownOverlay =
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.72f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(24.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Arcade", "ArcadeCountdownLabel", "ARCADE STARTING"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				.ColorAndOpacity(FLinearColor(0.25f, 0.92f, 1.f, 1.f))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 12.f, 0.f, 0.f)
			[
				SAssignNew(ArcadeCountdownText, STextBlock)
				.Text(NSLOCTEXT("T66.Arcade", "ArcadeCountdownThree", "3"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 112))
				.ColorAndOpacity(FLinearColor(1.f, 0.78f, 0.22f, 1.f))
				.Justification(ETextJustify::Center)
			]
		];

	ArcadeCountdownOverlayWidget = CountdownOverlay;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(CountdownOverlay, 250);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ArcadeCountdownTimerHandle,
			this,
			&AT66PlayerController::TickArcadePopupCountdown,
			0.10f,
			true);
	}
}

void AT66PlayerController::TickArcadePopupCountdown()
{
	if (!ArcadePopupWidget)
	{
		ClearArcadePopupCountdown();
		return;
	}

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float RemainingSeconds = ArcadeCountdownDurationSeconds - static_cast<float>(NowSeconds - ArcadeCountdownStartTimeSeconds);
	if (RemainingSeconds <= 0.f)
	{
		FinishArcadePopupCountdown();
		return;
	}

	if (ArcadeCountdownText.IsValid())
	{
		const int32 DisplaySeconds = FMath::Clamp(FMath::CeilToInt(RemainingSeconds), 1, 3);
		ArcadeCountdownText->SetText(FText::AsNumber(DisplaySeconds));
	}
}

void AT66PlayerController::FinishArcadePopupCountdown()
{
	UT66ArcadePopupWidget* PopupWidget = ArcadePopupWidget;
	const FT66WidgetGameHostContext HostContext = PendingArcadeWidgetGameHostContext;
	ClearArcadePopupCountdown();

	if (PopupWidget && PopupWidget == ArcadePopupWidget)
	{
		PopupWidget->ActivateWidgetGame(HostContext);
	}
}

void AT66PlayerController::ClearArcadePopupCountdown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ArcadeCountdownTimerHandle);
	}

	if (ArcadeCountdownOverlayWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ArcadeCountdownOverlayWidget.ToSharedRef());
	}

	ArcadeCountdownOverlayWidget.Reset();
	ArcadeCountdownText.Reset();
	PendingArcadeWidgetGameHostContext = FT66WidgetGameHostContext();
}

void AT66PlayerController::HandleArcadeWidgetGameResult(const FT66WidgetGameResult& Result)
{
	LastArcadeWidgetGameResultID = Result.GameID;
	LastArcadeWidgetGameFinalScore = Result.bHasFinalScore ? Result.FinalScore : 0;
	bLastArcadeWidgetGameSuccessful = Result.bSuccessful;
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
		if (IsGameplayLevel())
		{
			RestoreGameplayInputMode();
		}
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
	ClearArcadePopupCountdown();

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

	if (IsGameplayLevel() && !IsPaused() && !(UIManager && UIManager->IsModalActive()))
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

void AT66PlayerController::StartCrateOpenHUD(const ET66Rarity SourceCrateRarity)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->StartCrateOpen(SourceCrateRarity);
	}
}

bool AT66PlayerController::StartChestRewardHUD(
	ET66Rarity Rarity,
	int32 GoldAmount,
	TFunction<void()> OnCommit,
	TFunction<void()> OnFinished)
{
	if (!IsGameplayLevel()) return false;
	if (IsPaused()) return false;
	if (GameplayHUDWidget)
	{
		return GameplayHUDWidget->StartChestReward(Rarity, GoldAmount, MoveTemp(OnCommit), MoveTemp(OnFinished));
	}
	return false;
}

bool AT66PlayerController::StartLootWheelSpinHUD(FT66LootWheelPresentationParams Params)
{
	if (!IsGameplayLevel()) return false;
	if (IsPaused()) return false;
	if (GameplayHUDWidget)
	{
		return GameplayHUDWidget->StartLootWheelSpin(MoveTemp(Params));
	}
	return false;
}

void AT66PlayerController::ShowPickupItemCardHUD(const FName ItemID, const ET66ItemRarity ItemRarity)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->ShowPickupItemCard(ItemID, ItemRarity);
	}
}

void AT66PlayerController::ShowLootBagItemRevealHUD(const FName ItemID, const ET66ItemRarity ItemRarity)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->ShowLootBagItemReveal(ItemID, ItemRarity);
	}
}

void AT66PlayerController::OnPlayerDied()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			MobManager->EmitRangedPressureSummary(TEXT("OnPlayerDied"), true);
		}
	}
#if !UE_BUILD_SHIPPING
	if (GameplayAutomationCaptureMode.TrimStartAndEnd().Equals(TEXT("enemywaveperf"), ESearchCase::IgnoreCase))
	{
		FPlatformMisc::RequestExitWithStatus(false, -1, TEXT("T66EnemyWavePerfHeroDied"));
		return;
	}
#endif
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
				UIManager->ShowModal(ET66ScreenType::GameOver);
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
			UIManager->ShowModal(ET66ScreenType::GameOver);
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


void AT66PlayerController::EnsureGameplayUIManager()
{
	if (!IsGameplayLevel() || !IsLocalController()) return;

	if (!UIManager)
	{
		UIManager = NewObject<UT66UIManager>(this, UT66UIManager::StaticClass());
		if (!UIManager) return;

		UIManager->Initialize(this);
	}

	auto RegisterGameplayScreen = [this](const ET66ScreenType ScreenType)
	{
		if (TSubclassOf<UT66ScreenBase> ScreenClass = ResolveScreenClass(ScreenType))
		{
			UIManager->RegisterScreenClass(ScreenType, ScreenClass);
		}
	};

	RegisterGameplayScreen(ET66ScreenType::PauseMenu);
	RegisterGameplayScreen(ET66ScreenType::Achievements);
	RegisterGameplayScreen(ET66ScreenType::ReportBug);
	RegisterGameplayScreen(ET66ScreenType::Settings);
	RegisterGameplayScreen(ET66ScreenType::RunSummary);
	RegisterGameplayScreen(ET66ScreenType::GameOver);
	RegisterGameplayScreen(ET66ScreenType::PlayerSummaryPicker);
	RegisterGameplayScreen(ET66ScreenType::SavePreview);
	RegisterGameplayScreen(ET66ScreenType::PowerUp);
	RegisterGameplayScreen(ET66ScreenType::AccountStatus);
	RegisterGameplayScreen(ET66ScreenType::PartyInvite);
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

	if (bPaused
		&& UIManager
		&& UIManager->IsModalActive()
		&& UIManager->GetCurrentModalType() == ET66ScreenType::PauseMenu)
	{
		if (UT66ScreenBase* PauseModal = UIManager->GetCurrentModal())
		{
			if (PauseModal->HandleBackAction())
			{
				return;
			}
		}
	}

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
		if (UIManager && UIManager->GetCurrentModal())
		{
			InputMode.SetWidgetToFocus(UIManager->GetCurrentModal()->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
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
