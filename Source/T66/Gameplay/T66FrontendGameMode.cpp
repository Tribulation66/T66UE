// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "EngineUtils.h"

AT66FrontendGameMode::AT66FrontendGameMode()
{
	// No default pawn in frontend
	DefaultPawnClass = nullptr;
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
				RunState->ResetForNewRun();
			}
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				T66GI->bIsStageTransition = false;
				T66GI->bForceColiseumMode = false;
			}
		}
	}

	// Spawn hero preview stage if not already in level (for 3D hero preview in Hero Selection)
	UWorld* World = GetWorld();
	if (World)
	{
		bool bHasPreviewStage = false;
		for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
		{
			bHasPreviewStage = true;
			break;
		}
		if (!bHasPreviewStage)
		{
			FActorSpawnParameters SpawnParams;
			// Keep preview stages far from the menu camera so the player never sees them in the world.
			const FVector PreviewOrigin(1000000.f, 0.f, 200.f);
			World->SpawnActor<AT66HeroPreviewStage>(
				AT66HeroPreviewStage::StaticClass(),
				PreviewOrigin,
				FRotator(0.f, 0.f, 0.f),
				SpawnParams
			);
			UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode: Spawned HeroPreviewStage for 3D hero preview"));
		}

		bool bHasCompanionPreview = false;
		for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It) { bHasCompanionPreview = true; break; }
		if (!bHasCompanionPreview)
		{
			FActorSpawnParameters SpawnParams;
			World->SpawnActor<AT66CompanionPreviewStage>(
				AT66CompanionPreviewStage::StaticClass(),
				FVector(1000000.f, 1000.f, 200.f),
				FRotator(0.f, 0.f, 0.f),
				SpawnParams
			);
			UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode: Spawned CompanionPreviewStage for 3D companion preview"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode BeginPlay - Menu level initialized"));
}
