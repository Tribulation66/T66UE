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
#include "Materials/Material.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Factories/MaterialFactoryNew.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

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

	// Create placeholder material first (needed for hero visuals)
	if (CreatePlaceholderMaterial())
	{
		UE_LOG(LogT66Editor, Log, TEXT("[OK] PlaceholderMaterial created"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("[SKIP] PlaceholderMaterial (may already exist)"));
	}

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
		{ ET66ScreenType::HeroGrid, TEXT("/Game/Blueprints/UI/WBP_HeroGrid.WBP_HeroGrid_C") },
		{ ET66ScreenType::CompanionGrid, TEXT("/Game/Blueprints/UI/WBP_CompanionGrid.WBP_CompanionGrid_C") },
	};

	for (const auto& Mapping : Mappings)
	{
		// Avoid noisy SkipPackage warnings: only attempt to load if the asset package exists.
		FString PackagePath = Mapping.AssetPath;
		int32 DotIdx = INDEX_NONE;
		if (PackagePath.FindChar(TEXT('.'), DotIdx) && DotIdx > 0)
		{
			PackagePath = PackagePath.Left(DotIdx);
		}

		const bool bHasPackage = FPackageName::DoesPackageExist(PackagePath);
		if (!bHasPackage)
		{
			const bool bOptional =
				(Mapping.Type == ET66ScreenType::Achievements) ||
				(Mapping.Type == ET66ScreenType::LanguageSelect) ||
				(Mapping.Type == ET66ScreenType::HeroGrid) ||
				(Mapping.Type == ET66ScreenType::CompanionGrid);
			UE_LOG(LogT66Editor, Log, TEXT("%s widget asset missing: %s"),
				bOptional ? TEXT("[SKIP] Optional") : TEXT("[WARN]"),
				*PackagePath);
			continue;
		}

		UClass* WidgetClass = LoadClass<UT66ScreenBase>(nullptr, Mapping.AssetPath);
		if (WidgetClass)
		{
			PCCDO->ScreenClasses.Add(Mapping.Type, WidgetClass);
			UE_LOG(LogT66Editor, Log, TEXT("Registered screen %d -> %s"), static_cast<int32>(Mapping.Type), Mapping.AssetPath);
		}
		else
		{
			// Optional Blueprint overrides: C++ screens exist and work without these.
			const bool bOptional =
				(Mapping.Type == ET66ScreenType::Achievements) ||
				(Mapping.Type == ET66ScreenType::LanguageSelect) ||
				(Mapping.Type == ET66ScreenType::HeroGrid) ||
				(Mapping.Type == ET66ScreenType::CompanionGrid);
			UE_LOG(LogT66Editor, Log, TEXT("%s widget class missing: %s"),
				bOptional ? TEXT("[SKIP] Optional") : TEXT("[WARN]"),
				Mapping.AssetPath);
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
	const FString ItemsTablePath = TEXT("/Game/Data/DT_Items.DT_Items");
	const FString BossesTablePath = TEXT("/Game/Data/DT_Bosses.DT_Bosses");
	const FString StagesTablePath = TEXT("/Game/Data/DT_Stages.DT_Stages");
	const FString HouseNPCsTablePath = TEXT("/Game/Data/DT_HouseNPCs.DT_HouseNPCs");
	const FString CharacterVisualsTablePath = TEXT("/Game/Data/DT_CharacterVisuals.DT_CharacterVisuals");

	UDataTable* HeroesTable = LoadObject<UDataTable>(nullptr, *HeroesTablePath);
	UDataTable* CompanionsTable = LoadObject<UDataTable>(nullptr, *CompanionsTablePath);
	UDataTable* ItemsTable = LoadObject<UDataTable>(nullptr, *ItemsTablePath);
	UDataTable* BossesTable = LoadObject<UDataTable>(nullptr, *BossesTablePath);
	UDataTable* StagesTable = LoadObject<UDataTable>(nullptr, *StagesTablePath);
	UDataTable* HouseNPCsTable = LoadObject<UDataTable>(nullptr, *HouseNPCsTablePath);
	UDataTable* CharacterVisualsTable = LoadObject<UDataTable>(nullptr, *CharacterVisualsTablePath);

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

	if (ItemsTable)
	{
		GameInstanceCDO->ItemsDataTable = ItemsTable;
		UE_LOG(LogT66Editor, Log, TEXT("Set ItemsDataTable to DT_Items"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load DT_Items (create via CreateAssets.py then ImportData.py)"));
	}

	if (BossesTable)
	{
		GameInstanceCDO->BossesDataTable = BossesTable;
		UE_LOG(LogT66Editor, Log, TEXT("Set BossesDataTable to DT_Bosses"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load DT_Bosses (create via CreateAssets.py then ImportData.py)"));
	}

	if (StagesTable)
	{
		GameInstanceCDO->StagesDataTable = StagesTable;
		UE_LOG(LogT66Editor, Log, TEXT("Set StagesDataTable to DT_Stages"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load DT_Stages (create via CreateAssets.py then ImportData.py)"));
	}

	if (HouseNPCsTable)
	{
		GameInstanceCDO->HouseNPCsDataTable = HouseNPCsTable;
		UE_LOG(LogT66Editor, Log, TEXT("Set HouseNPCsDataTable to DT_HouseNPCs"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load DT_HouseNPCs (create via CreateAssets.py then ImportData.py)"));
	}

	if (CharacterVisualsTable)
	{
		GameInstanceCDO->CharacterVisualsDataTable = CharacterVisualsTable;
		UE_LOG(LogT66Editor, Log, TEXT("Set CharacterVisualsDataTable to DT_CharacterVisuals"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load DT_CharacterVisuals (create via SetupCharacterVisualsDataTable.py)"));
	}

	return SaveBlueprint(Blueprint);
}

bool UT66UISetupSubsystem::CreatePlaceholderMaterial()
{
	// Check if material already exists
	const FString MaterialPath = TEXT("/Game/Materials/M_PlaceholderColor");
	UMaterial* ExistingMaterial = LoadObject<UMaterial>(nullptr, *(MaterialPath + TEXT(".M_PlaceholderColor")));
	
	if (ExistingMaterial)
	{
		UE_LOG(LogT66Editor, Log, TEXT("PlaceholderMaterial already exists at %s"), *MaterialPath);
		return true;
	}

	// Ensure the Materials folder exists
	const FString MaterialsFolder = TEXT("/Game/Materials");
	
	// Create the material using AssetTools
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UObject* NewAsset = AssetTools.CreateAsset(TEXT("M_PlaceholderColor"), MaterialsFolder, UMaterial::StaticClass(), MaterialFactory);
	
	UMaterial* NewMaterial = Cast<UMaterial>(NewAsset);
	if (!NewMaterial)
	{
		UE_LOG(LogT66Editor, Error, TEXT("Failed to create PlaceholderMaterial"));
		return false;
	}

	// Create a Color vector parameter
	UMaterialExpressionVectorParameter* ColorParam = NewObject<UMaterialExpressionVectorParameter>(NewMaterial);
	ColorParam->ParameterName = FName("Color");
	ColorParam->DefaultValue = FLinearColor::White;
	ColorParam->MaterialExpressionEditorX = -300;
	ColorParam->MaterialExpressionEditorY = 0;
	NewMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(ColorParam);

	// Connect the color parameter to Base Color
	NewMaterial->GetEditorOnlyData()->BaseColor.Expression = ColorParam;
	
	// Create roughness constant (0.5)
	UMaterialExpressionConstant* RoughnessConst = NewObject<UMaterialExpressionConstant>(NewMaterial);
	RoughnessConst->R = 0.5f;
	RoughnessConst->MaterialExpressionEditorX = -300;
	RoughnessConst->MaterialExpressionEditorY = 150;
	NewMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(RoughnessConst);
	NewMaterial->GetEditorOnlyData()->Roughness.Expression = RoughnessConst;

	// Create metallic constant (0.0)
	UMaterialExpressionConstant* MetallicConst = NewObject<UMaterialExpressionConstant>(NewMaterial);
	MetallicConst->R = 0.0f;
	MetallicConst->MaterialExpressionEditorX = -300;
	MetallicConst->MaterialExpressionEditorY = 250;
	NewMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MetallicConst);
	NewMaterial->GetEditorOnlyData()->Metallic.Expression = MetallicConst;

	// Compile and save the material
	NewMaterial->PreEditChange(nullptr);
	NewMaterial->PostEditChange();
	NewMaterial->MarkPackageDirty();

	// Save the package
	UPackage* Package = NewMaterial->GetOutermost();
	if (Package)
	{
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Standalone;
		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		
		if (UPackage::SavePackage(Package, NewMaterial, *PackageFileName, SaveArgs))
		{
			UE_LOG(LogT66Editor, Log, TEXT("Created and saved PlaceholderMaterial at %s"), *MaterialPath);
			return true;
		}
	}

	UE_LOG(LogT66Editor, Warning, TEXT("Failed to save PlaceholderMaterial"));
	return false;
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
