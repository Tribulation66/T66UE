// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66WorldVisualSetup.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66FrontendGameMode, Log, All);

AT66FrontendGameMode::AT66FrontendGameMode()
{
	// No default pawn in frontend
	DefaultPawnClass = nullptr;
	PlayerStateClass = AT66SessionPlayerState::StaticClass();
	bUseSeamlessTravel = true;
}

void AT66FrontendGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Returning to frontend should reset run state (fresh menu flow).
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->BeginNewRun();
			}
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				T66GI->bIsStageTransition = false;
			}
		}
	}

	UWorld* World = GetWorld();
	// The frontend map is now an invisible shell, but keep neutral world setup here so
	// travel/settings transitions retain the same harmless initialization path.
	FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(World);

	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &AT66FrontendGameMode::HandleSettingsChanged);
		}
	}
	HandleSettingsChanged();

	UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode BeginPlay - Menu level initialized (UI-only frontend)"));
}

void AT66FrontendGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &AT66FrontendGameMode::HandleSettingsChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AT66FrontendGameMode::HandleSettingsChanged()
{
	UWorld* World = GetWorld();
	// Harmless on the empty frontend shell; retained to avoid exposing latent travel
	// or settings-order dependencies while frontend world rendering is invisible.
	FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(World);
}
