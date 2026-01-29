// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/T66UIManager.h"
#include "UI/T66ScreenBase.h"
#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66CompanionGridScreen.h"
#include "UI/Screens/T66SaveSlotsScreen.h"
#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66GameplayHUDWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66ItemPickup.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "EngineUtils.h"

AT66PlayerController::AT66PlayerController()
{
	// Default to showing mouse cursor (will be hidden in gameplay mode)
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AT66PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsGameplayLevel())
	{
		SetupGameplayMode();
		SetupGameplayHUD();
		UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			RunState->OnPlayerDied.AddDynamic(this, &AT66PlayerController::OnPlayerDied);
			OnPlayerDiedHandle.Reset(); // dynamic delegate has no handle; unbind via RemoveDynamic in EndPlay if needed
		}
		UE_LOG(LogTemp, Log, TEXT("PlayerController: Gameplay mode initialized"));
	}
	else
	{
		// Frontend mode: UI input, show cursor, initialize UI
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		if (bAutoShowInitialScreen)
		{
			InitializeUI();
		}
		UE_LOG(LogTemp, Log, TEXT("PlayerController: Frontend mode initialized"));
	}
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
	
	FString MapName = GetWorld()->GetMapName();
	MapName = UWorld::RemovePIEPrefix(MapName);
	
	// Check if this is a gameplay level
	return MapName.Contains(TEXT("Gameplay"));
}

void AT66PlayerController::SetupGameplayMode()
{
	RestoreGameplayInputMode();
	UE_LOG(LogTemp, Log, TEXT("PlayerController: Gameplay input mode set, cursor hidden"));
}

void AT66PlayerController::RestoreGameplayInputMode()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void AT66PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind gameplay inputs (these work when in gameplay mode)
	if (InputComponent)
	{
		// Movement (WASD)
		InputComponent->BindAxis(TEXT("MoveForward"), this, &AT66PlayerController::HandleMoveForward);
		InputComponent->BindAxis(TEXT("MoveRight"), this, &AT66PlayerController::HandleMoveRight);
		
		// Look (Mouse)
		InputComponent->BindAxis(TEXT("LookUp"), this, &AT66PlayerController::HandleLookUp);
		InputComponent->BindAxis(TEXT("Turn"), this, &AT66PlayerController::HandleTurn);
		
		// Zoom (scroll wheel, game world only)
		InputComponent->BindAxis(TEXT("Zoom"), this, &AT66PlayerController::HandleZoom);
		
		// Jump (Space)
		InputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AT66PlayerController::HandleJumpPressed);
		InputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AT66PlayerController::HandleJumpReleased);

		// Escape and P (pause menu in gameplay; P works in PIE without stealing focus)
		InputComponent->BindAction(TEXT("Escape"), IE_Pressed, this, &AT66PlayerController::HandleEscapePressed);
		InputComponent->BindAction(TEXT("Pause"), IE_Pressed, this, &AT66PlayerController::HandleEscapePressed);

		// T = toggle HUD panels (inventory + minimap), F = interact (vendor / pickup)
		InputComponent->BindAction(TEXT("ToggleHUD"), IE_Pressed, this, &AT66PlayerController::HandleToggleHUDPressed);
		InputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AT66PlayerController::HandleInteractPressed);
	}
}

void AT66PlayerController::SetupGameplayHUD()
{
	if (!IsGameplayLevel()) return;
	UClass* HUDClass = UT66GameplayHUDWidget::StaticClass();
	if (!HUDClass) return;
	GameplayHUDWidget = CreateWidget<UT66GameplayHUDWidget>(this, HUDClass);
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->AddToViewport(0);
	}
}

void AT66PlayerController::HandleToggleHUDPressed()
{
	if (!IsGameplayLevel()) return;
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		RunState->ToggleHUDPanels();
	}
}

void AT66PlayerController::HandleInteractPressed()
{
	if (!IsGameplayLevel()) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const float InteractRadius = 200.f;
	FVector Loc = ControlledPawn->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledPawn);
	World->OverlapMultiByChannel(Overlaps, Loc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(InteractRadius), Params);

	AT66VendorNPC* ClosestVendor = nullptr;
	AT66ItemPickup* ClosestPickup = nullptr;
	float ClosestVendorDistSq = InteractRadius * InteractRadius;
	float ClosestPickupDistSq = InteractRadius * InteractRadius;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* A = R.GetActor();
		if (!A) continue;
		float DistSq = FVector::DistSquared(Loc, A->GetActorLocation());
		if (AT66VendorNPC* V = Cast<AT66VendorNPC>(A))
		{
			if (DistSq < ClosestVendorDistSq) { ClosestVendorDistSq = DistSq; ClosestVendor = V; }
		}
		else if (AT66ItemPickup* P = Cast<AT66ItemPickup>(A))
		{
			if (DistSq < ClosestPickupDistSq) { ClosestPickupDistSq = DistSq; ClosestPickup = P; }
		}
	}

	// Prefer vendor over pickup
	if (ClosestVendor && ClosestVendor->TrySellFirstItem())
	{
		return;
	}
	if (ClosestPickup)
	{
		UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			RunState->AddItem(ClosestPickup->GetItemID());
			ClosestPickup->Destroy();
		}
	}
}

void AT66PlayerController::OnPlayerDied()
{
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

void AT66PlayerController::EnsureGameplayUIManager()
{
	if (UIManager || !IsGameplayLevel()) return;

	UIManager = NewObject<UT66UIManager>(this, UT66UIManager::StaticClass());
	if (!UIManager) return;

	UIManager->Initialize(this);
	UIManager->RegisterScreenClass(ET66ScreenType::PauseMenu, UT66PauseMenuScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::ReportBug, UT66ReportBugScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::Settings, UT66SettingsScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::RunSummary, UT66RunSummaryScreen::StaticClass());
}

void AT66PlayerController::HandleEscapePressed()
{
	if (!IsGameplayLevel()) return;

	// Debounce to avoid key repeat / rapid presses freezing or flickering
	const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
	if (Now - LastPauseToggleTime < PauseToggleDebounceSeconds)
	{
		return;
	}
	LastPauseToggleTime = Now;

	const bool bPaused = IsPaused();
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
	}
	else
	{
		if (UIManager && UIManager->IsModalActive())
		{
			const ET66ScreenType ClosingModal = UIManager->GetCurrentModalType();
			UIManager->CloseModal();
			// Esc as back: Settings or Report Bug -> return to Pause Menu; Pause Menu -> unpause
			if (ClosingModal == ET66ScreenType::Settings || ClosingModal == ET66ScreenType::ReportBug)
			{
				UIManager->ShowModal(ET66ScreenType::PauseMenu);
			}
			else
			{
				SetPause(false);
				RestoreGameplayInputMode();
			}
		}
		else
		{
			SetPause(false);
			RestoreGameplayInputMode();
		}
	}
}

void AT66PlayerController::HandleMoveForward(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;
	
	if (APawn* MyPawn = GetPawn())
	{
		// Get the control rotation (where we're looking)
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f; // Don't include pitch for movement
		ControlRot.Roll = 0.f;
		
		// Get forward vector based on control rotation
		FVector ForwardDir = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X);
		MyPawn->AddMovementInput(ForwardDir, Value);
	}
}

void AT66PlayerController::HandleMoveRight(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;
	
	if (APawn* MyPawn = GetPawn())
	{
		// Get the control rotation (where we're looking)
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll = 0.f;
		
		// Get right vector based on control rotation
		FVector RightDir = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);
		MyPawn->AddMovementInput(RightDir, Value);
	}
}

void AT66PlayerController::HandleLookUp(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;
	
	// Add pitch input (positive = look up, negative = look down)
	AddPitchInput(Value);
}

void AT66PlayerController::HandleTurn(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;
	
	// Add yaw input
	AddYawInput(Value);
}

void AT66PlayerController::HandleJumpPressed()
{
	if (!IsGameplayLevel()) return;
	
	if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
	{
		MyCharacter->Jump();
	}
}

void AT66PlayerController::HandleJumpReleased()
{
	if (!IsGameplayLevel()) return;
	
	if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
	{
		MyCharacter->StopJumping();
	}
}

void AT66PlayerController::HandleZoom(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;
	
	// Adjust camera boom length on the possessed hero (third-person zoom)
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;
	
	AT66HeroBase* Hero = Cast<AT66HeroBase>(MyPawn);
	if (Hero && Hero->CameraBoom)
	{
		const float ZoomSpeed = 25.f;
		const float MinLength = 200.f;
		const float MaxLength = 800.f;
		float NewLength = Hero->CameraBoom->TargetArmLength - Value * ZoomSpeed;
		NewLength = FMath::Clamp(NewLength, MinLength, MaxLength);
		Hero->CameraBoom->TargetArmLength = NewLength;
	}
}

void AT66PlayerController::AutoLoadScreenClasses()
{
	// Screen type to asset path mapping
	struct FScreenPathMapping
	{
		ET66ScreenType Type;
		const TCHAR* AssetPath;
	};

	static const FScreenPathMapping ScreenPaths[] = {
		{ ET66ScreenType::MainMenu, TEXT("/Game/Blueprints/UI/WBP_MainMenu.WBP_MainMenu_C") },
		{ ET66ScreenType::PartySizePicker, TEXT("/Game/Blueprints/UI/WBP_PartySizePicker.WBP_PartySizePicker_C") },
		{ ET66ScreenType::HeroSelection, TEXT("/Game/Blueprints/UI/WBP_HeroSelection.WBP_HeroSelection_C") },
		{ ET66ScreenType::CompanionSelection, TEXT("/Game/Blueprints/UI/WBP_CompanionSelection.WBP_CompanionSelection_C") },
		{ ET66ScreenType::SaveSlots, TEXT("/Game/Blueprints/UI/WBP_SaveSlots.WBP_SaveSlots_C") },
		{ ET66ScreenType::Settings, TEXT("/Game/Blueprints/UI/WBP_Settings.WBP_Settings_C") },
		{ ET66ScreenType::QuitConfirmation, TEXT("/Game/Blueprints/UI/WBP_QuitConfirmation.WBP_QuitConfirmation_C") },
		{ ET66ScreenType::HeroGrid, TEXT("/Game/Blueprints/UI/WBP_HeroGrid.WBP_HeroGrid_C") },
		{ ET66ScreenType::CompanionGrid, TEXT("/Game/Blueprints/UI/WBP_CompanionGrid.WBP_CompanionGrid_C") },
	};

	for (const auto& Mapping : ScreenPaths)
	{
		// Skip if already registered
		if (ScreenClasses.Contains(Mapping.Type) && ScreenClasses[Mapping.Type] != nullptr)
		{
			continue;
		}

		// Try to load the class
		UClass* LoadedClass = LoadClass<UT66ScreenBase>(nullptr, Mapping.AssetPath);
		if (LoadedClass)
		{
			ScreenClasses.Add(Mapping.Type, LoadedClass);
			UE_LOG(LogTemp, Log, TEXT("Auto-loaded screen class: %s"), Mapping.AssetPath);
		}
		else
		{
			if (Mapping.Type == ET66ScreenType::HeroGrid)
			{
				ScreenClasses.Add(ET66ScreenType::HeroGrid, UT66HeroGridScreen::StaticClass());
				UE_LOG(LogTemp, Log, TEXT("Using C++ HeroGrid screen (WBP not found)"));
			}
			else if (Mapping.Type == ET66ScreenType::CompanionGrid)
			{
				ScreenClasses.Add(ET66ScreenType::CompanionGrid, UT66CompanionGridScreen::StaticClass());
				UE_LOG(LogTemp, Log, TEXT("Using C++ CompanionGrid screen (WBP not found)"));
			}
			else if (Mapping.Type == ET66ScreenType::SaveSlots)
			{
				ScreenClasses.Add(ET66ScreenType::SaveSlots, UT66SaveSlotsScreen::StaticClass());
				UE_LOG(LogTemp, Log, TEXT("Using C++ SaveSlots screen (WBP not found)"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to auto-load screen class: %s"), Mapping.AssetPath);
			}
		}
	}
}

void AT66PlayerController::InitializeUI()
{
	if (bUIInitialized)
	{
		return;
	}

	// Auto-load screen classes if enabled
	if (bAutoLoadScreenClasses)
	{
		AutoLoadScreenClasses();
	}

	// Create the UI Manager
	UIManager = NewObject<UT66UIManager>(this, UT66UIManager::StaticClass());
	if (!UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UI Manager!"));
		return;
	}

	UIManager->Initialize(this);

	// Register all screen classes
	for (const auto& Pair : ScreenClasses)
	{
		if (Pair.Value)
		{
			UIManager->RegisterScreenClass(Pair.Key, Pair.Value);
			UE_LOG(LogTemp, Log, TEXT("Registered screen class for type %d"), static_cast<int32>(Pair.Key));
		}
	}

	bUIInitialized = true;

	// Show the initial screen
	if (InitialScreen != ET66ScreenType::None)
	{
		UIManager->ShowScreen(InitialScreen);
	}
}

void AT66PlayerController::ShowScreen(ET66ScreenType ScreenType)
{
	if (!bUIInitialized)
	{
		InitializeUI();
	}

	if (UIManager)
	{
		UIManager->ShowScreen(ScreenType);
	}
}
