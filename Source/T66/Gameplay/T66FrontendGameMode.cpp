// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "EngineUtils.h"

AT66FrontendGameMode::AT66FrontendGameMode()
{
	// No default pawn in frontend
	DefaultPawnClass = nullptr;
}

void AT66FrontendGameMode::BeginPlay()
{
	Super::BeginPlay();

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
			World->SpawnActor<AT66HeroPreviewStage>(
				AT66HeroPreviewStage::StaticClass(),
				FVector(0.f, 0.f, 200.f),
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
				FVector(200.f, 0.f, 200.f),
				FRotator(0.f, 0.f, 0.f),
				SpawnParams
			);
			UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode: Spawned CompanionPreviewStage for 3D companion preview"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode BeginPlay - Menu level initialized"));
}
