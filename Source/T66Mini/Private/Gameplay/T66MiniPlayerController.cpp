// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniPlayerController.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniLeaderboardSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniGameState.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "UI/T66MiniPauseMenuWidget.h"
#include "UI/T66UITypes.h"

namespace
{
	const FName MiniPauseAction(TEXT("MiniPause"));
	const FName MiniInteractAction(TEXT("MiniInteract"));
	const FName MiniUltimateAction(TEXT("MiniUltimate"));

	bool IsKeyboardMouseKey(const FKey& Key)
	{
		return Key.IsValid() && !Key.IsGamepadKey() && !Key.IsTouch();
	}

	void EnsureMiniActionHasDefaultBinding(APlayerController* PlayerController, const FName ActionName, const FKey DesiredKey)
	{
		UInputSettings* Settings = UInputSettings::GetInputSettings();
		if (!Settings || !DesiredKey.IsValid())
		{
			return;
		}

		for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
		{
			if (Mapping.ActionName == ActionName && IsKeyboardMouseKey(Mapping.Key))
			{
				return;
			}
		}

		Settings->AddActionMapping(FInputActionKeyMapping(ActionName, DesiredKey));
		Settings->SaveKeyMappings();
		Settings->SaveConfig();

		if (PlayerController && PlayerController->PlayerInput)
		{
			PlayerController->PlayerInput->ForceRebuildingKeyMaps(true);
		}
	}
}

AT66MiniPlayerController::AT66MiniPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AT66MiniPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ConfigureGameplayInputMode();
}

void AT66MiniPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateLootCratePresentation();
	if (IsPaused())
	{
		return;
	}

	UpdateMouseFollowTarget();
}

void AT66MiniPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	EnsureMiniActionHasDefaultBinding(this, MiniPauseAction, EKeys::Escape);
	EnsureMiniActionHasDefaultBinding(this, MiniInteractAction, EKeys::LeftMouseButton);
	EnsureMiniActionHasDefaultBinding(this, MiniUltimateAction, EKeys::RightMouseButton);

	FInputActionBinding& PauseBinding = InputComponent->BindAction(MiniPauseAction, IE_Pressed, this, &AT66MiniPlayerController::HandlePausePressed);
	PauseBinding.bExecuteWhenPaused = true;

	InputComponent->BindAction(MiniInteractAction, IE_Pressed, this, &AT66MiniPlayerController::HandleInteractPressed);
	InputComponent->BindAction(MiniUltimateAction, IE_Pressed, this, &AT66MiniPlayerController::HandleUltimatePressed);
}

void AT66MiniPlayerController::ResumeFromPauseMenu()
{
	ClosePauseMenu();
	if (!IsOnlinePartyMode() && IsPaused())
	{
		SetPause(false);
	}
	ConfigureGameplayInputMode();
}

void AT66MiniPlayerController::SaveAndQuitToMiniMenuFromPause()
{
	if (IsOnlinePartyMode())
	{
		ClosePauseMenu();
		if (HasAuthority())
		{
			if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
			{
				MiniGameMode->RequestPartySaveAndReturnToFrontend(TEXT("Mini party run saved. Returning the whole party to the mini menu."));
			}
		}
		else
		{
			ServerRequestPartySaveAndReturnToFrontend();
		}
		ConfigureGameplayInputMode();
		return;
	}

	ClosePauseMenu();
	if (IsPaused())
	{
		SetPause(false);
	}

	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		MiniGameMode->SaveRunProgressNow(true);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66MiniRunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>())
		{
			RunState->ResetActiveRun();
		}

		if (UT66MiniFrontendStateSubsystem* FrontendState = GameInstance->GetSubsystem<UT66MiniFrontendStateSubsystem>())
		{
			FrontendState->ResetRunSetup();
		}

		if (UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GameInstance))
		{
			T66GameInstance->PendingFrontendScreen = ET66ScreenType::MiniMainMenu;
			UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
		}
	}
}

void AT66MiniPlayerController::ConfigureGameplayInputMode()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	CurrentMouseCursor = EMouseCursor::Crosshairs;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void AT66MiniPlayerController::ConfigurePauseMenuInputMode()
{
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	CurrentMouseCursor = EMouseCursor::Default;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AT66MiniPlayerController::HandlePausePressed()
{
	if (bLootCratePresentationActive)
	{
		return;
	}

	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		if (PauseMenuWidget->HandlePauseBackAction())
		{
			return;
		}

		ResumeFromPauseMenu();
		return;
	}

	OpenPauseMenu();
}

void AT66MiniPlayerController::HandleInteractPressed()
{
	if (IsPaused() || bLootCratePresentationActive || IsPauseMenuVisible())
	{
		return;
	}

	if (HasAuthority())
	{
		if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
		{
			MiniGameMode->TryInteractNearest(Cast<AT66MiniPlayerPawn>(GetPawn()));
		}
		return;
	}

	ServerTryInteract();
}

void AT66MiniPlayerController::HandleUltimatePressed()
{
	if (IsPaused() || bLootCratePresentationActive || IsPauseMenuVisible())
	{
		return;
	}

	AT66MiniPlayerPawn* MiniPawn = Cast<AT66MiniPlayerPawn>(GetPawn());
	if (!MiniPawn)
	{
		return;
	}

	FVector CursorWorldLocation = MiniPawn->GetActorLocation();
	ProjectCursorToGroundPlane(this, CursorWorldLocation);
	MiniPawn->TryActivateUltimate(CursorWorldLocation);
}

void AT66MiniPlayerController::OpenPauseMenu()
{
	const bool bOnlinePartyMode = IsOnlinePartyMode();
	if (!bOnlinePartyMode)
	{
		if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
		{
			MiniGameMode->SaveRunProgressNow(true);
		}
		SetPause(true);
	}

	ConfigurePauseMenuInputMode();

	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
	}

	PauseMenuWidget = CreateWidget<UT66MiniPauseMenuWidget>(this, UT66MiniPauseMenuWidget::StaticClass());
	if (PauseMenuWidget)
	{
		PauseMenuWidget->AddToViewport(500);
		PauseMenuWidget->SetKeyboardFocus();
	}
}

void AT66MiniPlayerController::ClosePauseMenu()
{
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
	}
}

void AT66MiniPlayerController::UpdateMouseFollowTarget()
{
	if (IsPauseMenuVisible())
	{
		return;
	}

	AT66MiniPlayerPawn* MiniPawn = Cast<AT66MiniPlayerPawn>(GetPawn());
	if (!MiniPawn)
	{
		return;
	}

	FVector CursorWorldLocation = FVector::ZeroVector;
	if (ProjectCursorToGroundPlane(this, CursorWorldLocation))
	{
		MiniPawn->SetDesiredMoveLocation(CursorWorldLocation);
		MiniPawn->SetAimLocation(CursorWorldLocation);
	}
}

bool AT66MiniPlayerController::ProjectCursorToGroundPlane(APlayerController* PlayerController, FVector& OutWorldLocation)
{
	if (!PlayerController)
	{
		return false;
	}

	float MouseX = 0.f;
	float MouseY = 0.f;
	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		return false;
	}

	FVector WorldOrigin = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;
	if (!PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldOrigin, WorldDirection))
	{
		return false;
	}

	const APawn* ControlledPawn = PlayerController->GetPawn();
	const float GroundZ = ControlledPawn ? ControlledPawn->GetActorLocation().Z : 0.f;
	const FPlane GroundPlane(FVector(0.f, 0.f, GroundZ), FVector::UpVector);
	const FVector RayEnd = WorldOrigin + (WorldDirection * 100000.f);
	OutWorldLocation = FMath::LinePlaneIntersection(WorldOrigin, RayEnd, GroundPlane);
	return true;
}

bool AT66MiniPlayerController::StartLootCratePresentation()
{
	if (IsOnlinePartyMode())
	{
		BuildLootCrateStrip();
		if (LootCrateStripItemIDs.Num() == 0 || PendingLootCrateRewardItemID.IsNone())
		{
			return false;
		}

		if (AT66MiniPlayerPawn* MiniPawn = Cast<AT66MiniPlayerPawn>(GetPawn()))
		{
			MiniPawn->AcquireItem(PendingLootCrateRewardItemID);
		}

		LootCrateWinnerIndex = 0;
		PendingLootCrateRewardItemID = NAME_None;
		LootCrateStripItemIDs.Reset();
		return true;
	}

	if (bLootCratePresentationActive)
	{
		return false;
	}

	BuildLootCrateStrip();
	if (LootCrateStripItemIDs.Num() == 0 || PendingLootCrateRewardItemID.IsNone())
	{
		return false;
	}

	ClosePauseMenu();
	bLootCratePresentationActive = true;
	LootCratePresentationStartTime = FPlatformTime::Seconds();
	SetPause(true);
	ConfigurePauseMenuInputMode();
	return true;
}

void AT66MiniPlayerController::ResolveLootCratePresentationNow(const bool bAwardItem)
{
	if (!bLootCratePresentationActive)
	{
		return;
	}

	if (bAwardItem)
	{
		if (AT66MiniPlayerPawn* MiniPawn = Cast<AT66MiniPlayerPawn>(GetPawn()))
		{
			MiniPawn->AcquireItem(PendingLootCrateRewardItemID);
		}
	}

	bLootCratePresentationActive = false;
	LootCratePresentationStartTime = 0.0;
	LootCrateWinnerIndex = 0;
	PendingLootCrateRewardItemID = NAME_None;
	LootCrateStripItemIDs.Reset();
	SetPause(false);
	ConfigureGameplayInputMode();
}

double AT66MiniPlayerController::GetLootCratePresentationElapsedSeconds() const
{
	if (!bLootCratePresentationActive)
	{
		return 0.0;
	}

	return FMath::Max(0.0, FPlatformTime::Seconds() - LootCratePresentationStartTime);
}

void AT66MiniPlayerController::UpdateLootCratePresentation()
{
	if (!bLootCratePresentationActive)
	{
		return;
	}

	if (GetLootCratePresentationElapsedSeconds() >= LootCratePresentationDurationSeconds)
	{
		ResolveLootCratePresentationNow(true);
	}
}

void AT66MiniPlayerController::BuildLootCrateStrip()
{
	LootCrateStripItemIDs.Reset();
	LootCrateWinnerIndex = 29;
	PendingLootCrateRewardItemID = NAME_None;

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const TArray<FT66MiniItemDefinition>* ItemDefinitions = DataSubsystem ? &DataSubsystem->GetItems() : nullptr;
	if (!ItemDefinitions || ItemDefinitions->Num() == 0)
	{
		return;
	}

	const int32 StripItemCount = 34;
	const int32 WinnerItemIndex = FMath::RandRange(0, ItemDefinitions->Num() - 1);
	PendingLootCrateRewardItemID = (*ItemDefinitions)[WinnerItemIndex].ItemID;
	LootCrateStripItemIDs.Reserve(StripItemCount);
	for (int32 Index = 0; Index < StripItemCount; ++Index)
	{
		if (Index == LootCrateWinnerIndex)
		{
			LootCrateStripItemIDs.Add(PendingLootCrateRewardItemID);
			continue;
		}

		const int32 PickedIndex = FMath::RandRange(0, ItemDefinitions->Num() - 1);
		LootCrateStripItemIDs.Add((*ItemDefinitions)[PickedIndex].ItemID);
	}
}

void AT66MiniPlayerController::ServerTryInteract_Implementation()
{
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		MiniGameMode->TryInteractNearest(Cast<AT66MiniPlayerPawn>(GetPawn()));
	}
}

void AT66MiniPlayerController::ServerRequestPartySaveAndReturnToFrontend_Implementation()
{
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		MiniGameMode->RequestPartySaveAndReturnToFrontend(TEXT("Mini party run saved. Returning the whole party to the mini menu."));
	}
}

bool AT66MiniPlayerController::IsOnlinePartyMode() const
{
	const AT66MiniGameState* MiniGameState = GetWorld() ? GetWorld()->GetGameState<AT66MiniGameState>() : nullptr;
	return MiniGameState && MiniGameState->bOnlinePartyMode;
}

bool AT66MiniPlayerController::IsPauseMenuVisible() const
{
	return PauseMenuWidget && PauseMenuWidget->IsInViewport();
}

void AT66MiniPlayerController::ClientPrepareMiniOnlineRunSummary_Implementation(
	const bool bWasVictory,
	const FString& ResultLabel,
	const int32 WaveReached,
	const float RunSeconds)
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GameInstance);
	const UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const AT66MiniPlayerPawn* MiniPawn = Cast<AT66MiniPlayerPawn>(GetPawn());
	if (!FrontendState || !DataSubsystem)
	{
		if (T66GameInstance)
		{
			T66GameInstance->PendingFrontendScreen = ET66ScreenType::MiniMainMenu;
		}
		return;
	}

	FT66MiniRunSummary Summary;
	Summary.bHasSummary = true;
	Summary.bWasVictory = bWasVictory;
	Summary.HeroID = MiniPawn ? MiniPawn->GetHeroID() : (ActiveRun ? ActiveRun->HeroID : NAME_None);
	Summary.CompanionID = MiniPawn ? MiniPawn->GetSelectedCompanionID() : (ActiveRun ? ActiveRun->CompanionID : NAME_None);
	Summary.DifficultyID = ActiveRun ? ActiveRun->DifficultyID : NAME_None;
	Summary.WaveReached = FMath::Max(1, WaveReached);
	Summary.MaterialsCollected = MiniPawn ? MiniPawn->GetMaterials() : (ActiveRun ? ActiveRun->Materials : 0);
	Summary.OwnedItemCount = MiniPawn ? MiniPawn->GetOwnedItemIDs().Num() : (ActiveRun ? ActiveRun->OwnedItemIDs.Num() : 0);
	Summary.EquippedIdolCount = MiniPawn ? MiniPawn->GetEquippedIdolIDs().Num() : (ActiveRun ? ActiveRun->EquippedIdolIDs.Num() : 0);
	Summary.RunSeconds = RunSeconds;
	Summary.ResultLabel = ResultLabel;
	Summary.LastUpdatedUtc = FDateTime::UtcNow().ToIso8601();

	if (const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem->FindHero(Summary.HeroID))
	{
		Summary.HeroDisplayName = HeroDefinition->DisplayName;
	}
	else
	{
		Summary.HeroDisplayName = Summary.HeroID.ToString();
	}

	if (const FT66MiniCompanionDefinition* CompanionDefinition = DataSubsystem->FindCompanion(Summary.CompanionID))
	{
		Summary.CompanionDisplayName = CompanionDefinition->DisplayName;
	}
	else
	{
		Summary.CompanionDisplayName = Summary.CompanionID.ToString();
	}

	if (const FT66MiniDifficultyDefinition* DifficultyDefinition = DataSubsystem->FindDifficulty(Summary.DifficultyID))
	{
		Summary.DifficultyDisplayName = DifficultyDefinition->DisplayName;
	}
	else
	{
		Summary.DifficultyDisplayName = Summary.DifficultyID.ToString();
	}

	FrontendState->SetLastRunSummary(Summary);
	FrontendState->ExitIntermissionFlow();
	if (SaveSubsystem)
	{
		SaveSubsystem->RecordRunSummary(Summary, DataSubsystem);
	}
	if (UT66MiniLeaderboardSubsystem* MiniLeaderboardSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniLeaderboardSubsystem>() : nullptr)
	{
		const UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
		MiniLeaderboardSubsystem->SubmitScore(
			Summary.DifficultyID,
			PartySubsystem ? PartySubsystem->GetCurrentPartySizeEnum() : ET66PartySize::Solo,
			Summary.MaterialsCollected);
	}
	if (RunState)
	{
		RunState->ResetActiveRun();
	}
	if (T66GameInstance)
	{
		T66GameInstance->PendingFrontendScreen = ET66ScreenType::MiniRunSummary;
	}
}

void AT66MiniPlayerController::ClientHandleMiniStageClear_Implementation(const int32 ChadCouponsAwarded, const FName CompanionID)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (UT66AchievementsSubsystem* Achievements = GameInstance ? GameInstance->GetSubsystem<UT66AchievementsSubsystem>() : nullptr)
	{
		Achievements->AddChadCoupons(ChadCouponsAwarded);
	}

	if (UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr)
	{
		if (UT66MiniDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UT66MiniDataSubsystem>())
		{
			SaveSubsystem->RecordClearedMiniStage(CompanionID, DataSubsystem);
		}
	}
}

void AT66MiniPlayerController::ClientPrepareMiniFrontendTravel_Implementation(const ET66ScreenType PendingScreen, const bool bIntermissionFlow)
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GameInstance);
	if (FrontendState)
	{
		if (bIntermissionFlow)
		{
			FrontendState->ResumeIntermissionFlow();
		}
		else
		{
			FrontendState->ExitIntermissionFlow();
		}
	}

	if (!bIntermissionFlow && RunState)
	{
		RunState->ResetActiveRun();
	}

	if (T66GameInstance)
	{
		T66GameInstance->MiniIntermissionStateRevision = 0;
		T66GameInstance->MiniIntermissionStateJson.Reset();
		T66GameInstance->MiniIntermissionRequestRevision = 0;
		T66GameInstance->MiniIntermissionRequestJson.Reset();
		T66GameInstance->PendingFrontendScreen = PendingScreen;
	}
}
