// Copyright Tribulation 66. All Rights Reserved.

#include "T66UISetupSubsystem.h"
#include "T66Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/World.h"
#include "WidgetBlueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "UI/T66UITypes.h"
#include "UI/T66ScreenBase.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66GameMode.h"
#include "Core/T66GameInstance.h"
#include "HAL/IConsoleManager.h"

// Console command for running setup
static FAutoConsoleCommand T66SetupCommand(
	TEXT("T66Setup"),
	TEXT("Runs the T66 full auto-setup (configures Blueprints, levels, etc.)"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (GEditor)
		{
			if (UT66UISetupSubsystem* Subsystem = GEditor->GetEditorSubsystem<UT66UISetupSubsystem>())
			{
				Subsystem->RunFullSetup();
			}
			else
			{
				UE_LOG(LogT66Editor, Error, TEXT("T66UISetupSubsystem not available"));
			}
		}
	})
);

void UT66UISetupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogT66Editor, Log, TEXT("T66UISetupSubsystem initialized"));
}

void UT66UISetupSubsystem::Deinitialize()
{
	UE_LOG(LogT66Editor, Log, TEXT("T66UISetupSubsystem deinitialized"));
	Super::Deinitialize();
}

void UT66UISetupSubsystem::RunFullSetup()
{
	UE_LOG(LogT66Editor, Log, TEXT("=== T66 Full Setup Starting ==="));
	
	bool bAllSuccess = true;

	// Configure GameInstance
	if (ConfigureGameInstance())
	{
		UE_LOG(LogT66Editor, Log, TEXT("[OK] GameInstance configured"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("[FAIL] GameInstance configuration failed"));
		bAllSuccess = false;
	}

	// Configure PlayerController
	if (ConfigurePlayerController())
	{
		UE_LOG(LogT66Editor, Log, TEXT("[OK] PlayerController configured"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("[FAIL] PlayerController configuration failed"));
		bAllSuccess = false;
	}

	// Configure Frontend GameMode
	if (ConfigureFrontendGameMode())
	{
		UE_LOG(LogT66Editor, Log, TEXT("[OK] FrontendGameMode configured"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("[FAIL] FrontendGameMode configuration failed"));
		bAllSuccess = false;
	}

	// Configure Gameplay GameMode
	if (ConfigureGameplayGameMode())
	{
		UE_LOG(LogT66Editor, Log, TEXT("[OK] GameplayGameMode configured"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("[FAIL] GameplayGameMode configuration failed"));
		bAllSuccess = false;
	}

	// Configure FrontendLevel
	if (ConfigureFrontendLevel())
	{
		UE_LOG(LogT66Editor, Log, TEXT("[OK] FrontendLevel configured"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("[FAIL] FrontendLevel configuration failed"));
		bAllSuccess = false;
	}

	// Configure GameplayLevel
	if (ConfigureGameplayLevel())
	{
		UE_LOG(LogT66Editor, Log, TEXT("[OK] GameplayLevel configured"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("[FAIL] GameplayLevel configuration failed"));
		bAllSuccess = false;
	}

	if (bAllSuccess)
	{
		UE_LOG(LogT66Editor, Log, TEXT("=== T66 Full Setup Complete - All Success ==="));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("=== T66 Full Setup Complete - Some failures (see above) ==="));
	}

	PrintSetupStatus();
}

bool UT66UISetupSubsystem::ConfigureFrontendLevel()
{
	// Load the FrontendLevel world
	const FString LevelPath = TEXT("/Game/Maps/FrontendLevel");
	UWorld* World = LoadObject<UWorld>(nullptr, *LevelPath);
	
	if (!World)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load FrontendLevel at %s"), *LevelPath);
		return false;
	}

	// Load the FrontendGameMode class
	const FString GameModePath = TEXT("/Game/Blueprints/GameModes/BP_FrontendGameMode.BP_FrontendGameMode_C");
	UClass* GameModeClass = LoadClass<AGameModeBase>(nullptr, *GameModePath);
	
	if (!GameModeClass)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load BP_FrontendGameMode at %s"), *GameModePath);
		return false;
	}

	// Set the GameMode override in World Settings
	if (World->GetWorldSettings())
	{
		World->GetWorldSettings()->DefaultGameMode = GameModeClass;
		World->GetWorldSettings()->MarkPackageDirty();
		
		// Save the level
		UPackage* Package = World->GetOutermost();
		if (Package)
		{
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Standalone;
			FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetMapPackageExtension());
			UPackage::SavePackage(Package, World, *PackageFileName, SaveArgs);
			UE_LOG(LogT66Editor, Log, TEXT("Saved FrontendLevel with GameMode override"));
		}
		return true;
	}

	return false;
}

bool UT66UISetupSubsystem::ConfigureGameplayLevel()
{
	const FString LevelPath = TEXT("/Game/Maps/GameplayLevel");
	UWorld* World = LoadObject<UWorld>(nullptr, *LevelPath);
	
	if (!World)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load GameplayLevel at %s"), *LevelPath);
		return false;
	}

	const FString GameModePath = TEXT("/Game/Blueprints/GameModes/BP_GameplayGameMode.BP_GameplayGameMode_C");
	UClass* GameModeClass = LoadClass<AGameModeBase>(nullptr, *GameModePath);
	
	if (!GameModeClass)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load BP_GameplayGameMode at %s"), *GameModePath);
		return false;
	}

	if (World->GetWorldSettings())
	{
		World->GetWorldSettings()->DefaultGameMode = GameModeClass;
		World->GetWorldSettings()->MarkPackageDirty();
		
		UPackage* Package = World->GetOutermost();
		if (Package)
		{
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Standalone;
			FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetMapPackageExtension());
			UPackage::SavePackage(Package, World, *PackageFileName, SaveArgs);
			UE_LOG(LogT66Editor, Log, TEXT("Saved GameplayLevel with GameMode override"));
		}
		return true;
	}

	return false;
}

bool UT66UISetupSubsystem::ConfigureFrontendGameMode()
{
	const FString BlueprintPath = TEXT("/Game/Blueprints/GameModes/BP_FrontendGameMode");
	UBlueprint* Blueprint = LoadBlueprint(BlueprintPath);
	
	if (!Blueprint)
	{
		return false;
	}

	// Get the CDO
	UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
	AGameModeBase* GameModeCDO = Cast<AGameModeBase>(CDO);
	
	if (!GameModeCDO)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to get GameMode CDO"));
		return false;
	}

	// Load PlayerController class
	const FString PlayerControllerPath = TEXT("/Game/Blueprints/Core/BP_T66PlayerController.BP_T66PlayerController_C");
	UClass* PCClass = LoadClass<APlayerController>(nullptr, *PlayerControllerPath);
	
	if (PCClass)
	{
		GameModeCDO->PlayerControllerClass = PCClass;
		UE_LOG(LogT66Editor, Log, TEXT("Set PlayerControllerClass to BP_T66PlayerController"));
	}

	// No default pawn for frontend
	GameModeCDO->DefaultPawnClass = nullptr;

	return SaveBlueprint(Blueprint);
}

bool UT66UISetupSubsystem::ConfigureGameplayGameMode()
{
	const FString BlueprintPath = TEXT("/Game/Blueprints/GameModes/BP_GameplayGameMode");
	UBlueprint* Blueprint = LoadBlueprint(BlueprintPath);
	
	if (!Blueprint)
	{
		return false;
	}

	UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
	AGameModeBase* GameModeCDO = Cast<AGameModeBase>(CDO);
	
	if (!GameModeCDO)
	{
		return false;
	}

	// Load PlayerController class
	const FString PlayerControllerPath = TEXT("/Game/Blueprints/Core/BP_T66PlayerController.BP_T66PlayerController_C");
	UClass* PCClass = LoadClass<APlayerController>(nullptr, *PlayerControllerPath);
	
	if (PCClass)
	{
		GameModeCDO->PlayerControllerClass = PCClass;
	}

	// Load HeroBase class for default pawn
	const FString HeroBasePath = TEXT("/Game/Blueprints/Core/BP_HeroBase.BP_HeroBase_C");
	UClass* HeroClass = LoadClass<APawn>(nullptr, *HeroBasePath);
	
	if (HeroClass)
	{
		GameModeCDO->DefaultPawnClass = HeroClass;
	}

	return SaveBlueprint(Blueprint);
}

bool UT66UISetupSubsystem::ConfigurePlayerController()
{
	const FString BlueprintPath = TEXT("/Game/Blueprints/Core/BP_T66PlayerController");
	UBlueprint* Blueprint = LoadBlueprint(BlueprintPath);
	
	if (!Blueprint)
	{
		return false;
	}

	UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
	AT66PlayerController* PCCDO = Cast<AT66PlayerController>(CDO);
	
	if (!PCCDO)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to cast to AT66PlayerController"));
		return false;
	}

	// Clear existing and set up screen class mappings
	PCCDO->ScreenClasses.Empty();

	// Define screen mappings
	struct FScreenMapping
	{
		ET66ScreenType Type;
		const TCHAR* AssetPath;
	};

	static const FScreenMapping Mappings[] = {
		{ ET66ScreenType::MainMenu, TEXT("/Game/Blueprints/UI/WBP_MainMenu.WBP_MainMenu_C") },
		{ ET66ScreenType::PartySizePicker, TEXT("/Game/Blueprints/UI/WBP_PartySizePicker.WBP_PartySizePicker_C") },
		{ ET66ScreenType::HeroSelection, TEXT("/Game/Blueprints/UI/WBP_HeroSelection.WBP_HeroSelection_C") },
		{ ET66ScreenType::CompanionSelection, TEXT("/Game/Blueprints/UI/WBP_CompanionSelection.WBP_CompanionSelection_C") },
		{ ET66ScreenType::SaveSlots, TEXT("/Game/Blueprints/UI/WBP_SaveSlots.WBP_SaveSlots_C") },
		{ ET66ScreenType::Settings, TEXT("/Game/Blueprints/UI/WBP_Settings.WBP_Settings_C") },
		{ ET66ScreenType::Achievements, TEXT("/Game/Blueprints/UI/WBP_Achievements.WBP_Achievements_C") },
		{ ET66ScreenType::QuitConfirmation, TEXT("/Game/Blueprints/UI/WBP_QuitConfirmation.WBP_QuitConfirmation_C") },
		{ ET66ScreenType::LanguageSelect, TEXT("/Game/Blueprints/UI/WBP_LanguageSelect.WBP_LanguageSelect_C") },
	};

	for (const auto& Mapping : Mappings)
	{
		UClass* WidgetClass = LoadClass<UT66ScreenBase>(nullptr, Mapping.AssetPath);
		if (WidgetClass)
		{
			PCCDO->ScreenClasses.Add(Mapping.Type, WidgetClass);
			UE_LOG(LogT66Editor, Log, TEXT("Registered screen %d -> %s"), static_cast<int32>(Mapping.Type), Mapping.AssetPath);
		}
		else
		{
			UE_LOG(LogT66Editor, Warning, TEXT("Failed to load widget class: %s"), Mapping.AssetPath);
		}
	}

	// Set initial screen to MainMenu
	PCCDO->InitialScreen = ET66ScreenType::MainMenu;
	PCCDO->bAutoShowInitialScreen = true;
	PCCDO->bAutoLoadScreenClasses = true;

	return SaveBlueprint(Blueprint);
}

bool UT66UISetupSubsystem::ConfigureGameInstance()
{
	const FString BlueprintPath = TEXT("/Game/Blueprints/Core/BP_T66GameInstance");
	UBlueprint* Blueprint = LoadBlueprint(BlueprintPath);
	
	if (!Blueprint)
	{
		return false;
	}

	UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
	UT66GameInstance* GameInstanceCDO = Cast<UT66GameInstance>(CDO);
	
	if (!GameInstanceCDO)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to cast to UT66GameInstance"));
		return false;
	}

	// Load DataTables
	const FString HeroesTablePath = TEXT("/Game/Data/DT_Heroes.DT_Heroes");
	const FString CompanionsTablePath = TEXT("/Game/Data/DT_Companions.DT_Companions");

	UDataTable* HeroesTable = LoadObject<UDataTable>(nullptr, *HeroesTablePath);
	UDataTable* CompanionsTable = LoadObject<UDataTable>(nullptr, *CompanionsTablePath);

	if (HeroesTable)
	{
		GameInstanceCDO->HeroDataTable = HeroesTable;
		UE_LOG(LogT66Editor, Log, TEXT("Set HeroDataTable to DT_Heroes"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load DT_Heroes"));
	}

	if (CompanionsTable)
	{
		GameInstanceCDO->CompanionDataTable = CompanionsTable;
		UE_LOG(LogT66Editor, Log, TEXT("Set CompanionDataTable to DT_Companions"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load DT_Companions"));
	}

	return SaveBlueprint(Blueprint);
}

void UT66UISetupSubsystem::PrintSetupStatus()
{
	UE_LOG(LogT66Editor, Log, TEXT(""));
	UE_LOG(LogT66Editor, Log, TEXT("=== T66 Setup Status ==="));
	
	// Check FrontendLevel
	{
		const FString LevelPath = TEXT("/Game/Maps/FrontendLevel");
		UWorld* World = LoadObject<UWorld>(nullptr, *LevelPath);
		if (World && World->GetWorldSettings())
		{
			UClass* GameModeClass = World->GetWorldSettings()->DefaultGameMode;
			UE_LOG(LogT66Editor, Log, TEXT("FrontendLevel GameMode: %s"), 
				GameModeClass ? *GameModeClass->GetName() : TEXT("(none)"));
		}
	}

	// Check PlayerController
	{
		const FString PCPath = TEXT("/Game/Blueprints/Core/BP_T66PlayerController.BP_T66PlayerController_C");
		UClass* PCClass = LoadClass<AT66PlayerController>(nullptr, *PCPath);
		if (PCClass)
		{
			AT66PlayerController* PCCDO = Cast<AT66PlayerController>(PCClass->GetDefaultObject());
			if (PCCDO)
			{
				UE_LOG(LogT66Editor, Log, TEXT("PlayerController ScreenClasses: %d registered"), PCCDO->ScreenClasses.Num());
				UE_LOG(LogT66Editor, Log, TEXT("PlayerController InitialScreen: %d"), static_cast<int32>(PCCDO->InitialScreen));
			}
		}
	}

	UE_LOG(LogT66Editor, Log, TEXT("========================"));
}

UBlueprint* UT66UISetupSubsystem::LoadBlueprint(const FString& AssetPath)
{
	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	if (!Blueprint)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load Blueprint: %s"), *AssetPath);
	}
	return Blueprint;
}

bool UT66UISetupSubsystem::SaveBlueprint(UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return false;
	}

	Blueprint->MarkPackageDirty();
	
	UPackage* Package = Blueprint->GetOutermost();
	if (Package)
	{
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Standalone;
		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		
		if (UPackage::SavePackage(Package, Blueprint, *PackageFileName, SaveArgs))
		{
			UE_LOG(LogT66Editor, Log, TEXT("Saved Blueprint: %s"), *Blueprint->GetName());
			return true;
		}
	}
	
	UE_LOG(LogT66Editor, Warning, TEXT("Failed to save Blueprint: %s"), *Blueprint->GetName());
	return false;
}
