// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"

static void SpawnFrontendLightingIfNeeded(UWorld* World)
{
	if (!World) return;

	ASkyAtmosphere* Atmosphere = nullptr;
	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It) { Atmosphere = *It; break; }
	ADirectionalLight* Sun = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It) { Sun = *It; break; }
	ASkyLight* SkyLight = nullptr;
	for (TActorIterator<ASkyLight> It(World); It; ++It) { SkyLight = *It; break; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!Atmosphere)
	{
		Atmosphere = World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (Atmosphere) UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned SkyAtmosphere for preview (same as gameplay)"));
	}
	if (!Sun)
	{
		Sun = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 1000.f), FRotator(-50.f, -45.f, 0.f), SpawnParams);
		if (Sun)
		{
			if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				LC->SetMobility(EComponentMobility::Movable);
				LC->SetIntensity(4.f);
				LC->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
				LC->bAtmosphereSunLight = true;
				LC->AtmosphereSunLightIndex = 0;
			}
			UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned DirectionalLight for preview (same as gameplay)"));
		}
	}
	if (!SkyLight)
	{
		SkyLight = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector(0.f, 0.f, 500.f), FRotator::ZeroRotator, SpawnParams);
		if (SkyLight)
		{
			if (USkyLightComponent* SC = SkyLight->GetLightComponent())
			{
				SC->SetMobility(EComponentMobility::Movable);
				SC->SetIntensity(0.7f);
				SC->SetLightColor(FLinearColor::White);
				SC->RecaptureSky();
			}
			UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned SkyLight for preview (same as gameplay)"));
		}
	}
	else if (SkyLight && SkyLight->GetLightComponent())
	{
		if (USkyLightComponent* SC = Cast<USkyLightComponent>(SkyLight->GetLightComponent()))
			SC->RecaptureSky();
	}
}

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

	UWorld* World = GetWorld();
	// Same sky and lights as gameplay level so SceneCapture2D shows real engine sky.
	SpawnFrontendLightingIfNeeded(World);

	// Spawn hero preview stage if not already in level (for 3D hero preview in Hero Selection)
	if (World)
	{
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
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

		// Pre-warm preview so the first time the UI opens it's already "styled" (no pop-in).
		if (T66GI)
		{
			for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
			{
				const FName HeroID = !T66GI->SelectedHeroID.IsNone()
					? T66GI->SelectedHeroID
					: (T66GI->GetAllHeroIDs().Num() > 0 ? T66GI->GetAllHeroIDs()[0] : NAME_None);
				if (!HeroID.IsNone())
				{
					FName SkinID = T66GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : T66GI->SelectedHeroSkinID;
				It->SetPreviewHero(HeroID, T66GI->SelectedHeroBodyType, SkinID);
				}
				break;
			}
			for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
			{
				It->SetPreviewCompanion(T66GI->SelectedCompanionID);
				break;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode BeginPlay - Menu level initialized"));
}
