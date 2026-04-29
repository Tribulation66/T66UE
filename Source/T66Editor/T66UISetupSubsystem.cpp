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
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66GameMode.h"
#include "Core/T66GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionSceneTexture.h"
#include "Materials/MaterialExpressionScreenPosition.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionLength.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Factories/MaterialFactoryNew.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Misc/PackageName.h"
#include "Engine/World.h"
#include "EngineUtils.h"

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

static FAutoConsoleCommand T66CreateLabLevelCommand(
	TEXT("T66CreateLabLevel"),
	TEXT("Creates The Lab level at Content/Maps/LabLevel (PlayerStart, lighting, GameMode)."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (GEditor)
		{
			if (UT66UISetupSubsystem* Sub = GEditor->GetEditorSubsystem<UT66UISetupSubsystem>())
			{
				Sub->CreateLabLevel();
			}
		}
	})
);

static FAutoConsoleCommand T66CreateRetroChromaticAberrationMaterialCommand(
	TEXT("T66CreateRetroChromaticAberrationMaterial"),
	TEXT("Creates M_RetroChromaticAberrationPostProcess (full-screen radial chromatic aberration / distortion post-process material)."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (GEditor)
		{
			if (UT66UISetupSubsystem* Sub = GEditor->GetEditorSubsystem<UT66UISetupSubsystem>())
			{
				Sub->CreateRetroChromaticAberrationMaterial();
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

	if (CreateRetroChromaticAberrationMaterial())
	{
		UE_LOG(LogT66Editor, Log, TEXT("[OK] RetroChromaticAberrationMaterial created"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("[SKIP] RetroChromaticAberrationMaterial (may already exist)"));
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

bool UT66UISetupSubsystem::ConfigureDemoMapLevel()
{
	const FString LevelPath = TEXT("/Game/LowPolyNature/Maps/Map_Summer");
	UWorld* World = LoadObject<UWorld>(nullptr, *LevelPath);

	if (!World)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load demo map at %s"), *LevelPath);
		return false;
	}

	const FString GameModePath = TEXT("/Game/Blueprints/GameModes/BP_GameplayGameMode.BP_GameplayGameMode_C");
	UClass* GameModeClass = LoadClass<AGameModeBase>(nullptr, *GameModePath);

	if (!GameModeClass)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load BP_GameplayGameMode at %s"), *GameModePath);
		return false;
	}

	if (!World->GetWorldSettings())
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Demo map has no WorldSettings"));
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = GameModeClass;
	World->GetWorldSettings()->MarkPackageDirty();

	// Add PlayerStart if none exists
	bool bHasPlayerStart = false;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		bHasPlayerStart = true;
		break;
	}
	if (!bHasPlayerStart)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.OverrideLevel = World->PersistentLevel;
		World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator, SpawnParams);
		World->MarkPackageDirty();
		UE_LOG(LogT66Editor, Log, TEXT("Added PlayerStart to demo map at (0, 0, 200)"));
	}

	UPackage* Package = World->GetOutermost();
	if (Package)
	{
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Standalone;
		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetMapPackageExtension());
		if (UPackage::SavePackage(Package, World, *PackageFileName, SaveArgs))
		{
			UE_LOG(LogT66Editor, Log, TEXT("Saved demo map (Map_Summer) with GameMode and PlayerStart"));
			return true;
		}
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to save demo map package"));
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

	// Use the native controller so frontend startup does not depend on Blueprint screen mappings.
	GameModeCDO->PlayerControllerClass = AT66PlayerController::StaticClass();
	UE_LOG(LogT66Editor, Log, TEXT("Set PlayerControllerClass to native AT66PlayerController"));

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

	// Clear existing and set up runtime screen class mappings
	PCCDO->RuntimeScreenClasses.Empty();

	// Define screen mappings
	struct FScreenMapping
	{
		ET66ScreenType Type;
		const TCHAR* AssetPath;
	};

	static const FScreenMapping Mappings[] = {
		{ ET66ScreenType::MainMenu, TEXT("/Game/Blueprints/UI/WBP_MainMenu.WBP_MainMenu_C") },
		{ ET66ScreenType::CompanionSelection, TEXT("/Game/Blueprints/UI/WBP_CompanionSelection.WBP_CompanionSelection_C") },
		{ ET66ScreenType::SaveSlots, TEXT("/Game/Blueprints/UI/WBP_SaveSlots.WBP_SaveSlots_C") },
		{ ET66ScreenType::Settings, TEXT("/Game/Blueprints/UI/WBP_Settings.WBP_Settings_C") },
		{ ET66ScreenType::Achievements, TEXT("/Game/Blueprints/UI/WBP_Achievements.WBP_Achievements_C") },
		{ ET66ScreenType::QuitConfirmation, TEXT("/Game/Blueprints/UI/WBP_QuitConfirmation.WBP_QuitConfirmation_C") },
		{ ET66ScreenType::LanguageSelect, TEXT("/Game/Blueprints/UI/WBP_LanguageSelect.WBP_LanguageSelect_C") },
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
			PCCDO->RuntimeScreenClasses.Add(Mapping.Type, WidgetClass);
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

	// Hero selection is native and changes faster than the legacy Blueprint override.
	PCCDO->RuntimeScreenClasses.Add(ET66ScreenType::HeroSelection, UT66HeroSelectionScreen::StaticClass());
	UE_LOG(LogT66Editor, Log, TEXT("Registered native HeroSelection screen"));

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
	const FString ArcadeInteractablesTablePath = TEXT("/Game/Data/DT_ArcadeInteractables.DT_ArcadeInteractables");

	UDataTable* HeroesTable = LoadObject<UDataTable>(nullptr, *HeroesTablePath);
	UDataTable* CompanionsTable = LoadObject<UDataTable>(nullptr, *CompanionsTablePath);
	UDataTable* ItemsTable = LoadObject<UDataTable>(nullptr, *ItemsTablePath);
	UDataTable* BossesTable = LoadObject<UDataTable>(nullptr, *BossesTablePath);
	UDataTable* StagesTable = LoadObject<UDataTable>(nullptr, *StagesTablePath);
	UDataTable* HouseNPCsTable = LoadObject<UDataTable>(nullptr, *HouseNPCsTablePath);
	UDataTable* CharacterVisualsTable = LoadObject<UDataTable>(nullptr, *CharacterVisualsTablePath);
	UDataTable* ArcadeInteractablesTable = LoadObject<UDataTable>(nullptr, *ArcadeInteractablesTablePath);

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

	if (ArcadeInteractablesTable)
	{
		GameInstanceCDO->ArcadeInteractablesDataTable = ArcadeInteractablesTable;
		UE_LOG(LogT66Editor, Log, TEXT("Set ArcadeInteractablesDataTable to DT_ArcadeInteractables"));
	}
	else
	{
		UE_LOG(LogT66Editor, Warning, TEXT("Failed to load DT_ArcadeInteractables (create via SetupArcadeInteractablesDataTable.py)"));
	}

	return SaveBlueprint(Blueprint);
}

bool UT66UISetupSubsystem::CreateRetroChromaticAberrationMaterial()
{
	const FString MaterialPath = TEXT("/Game/Materials/Retro/M_RetroChromaticAberrationPostProcess");
	UMaterial* ExistingMaterial = LoadObject<UMaterial>(nullptr, *(MaterialPath + TEXT(".M_RetroChromaticAberrationPostProcess")));
	if (ExistingMaterial)
	{
		UE_LOG(LogT66Editor, Log, TEXT("RetroChromaticAberrationMaterial already exists at %s"), *MaterialPath);
		return true;
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UObject* NewAsset = AssetTools.CreateAsset(TEXT("M_RetroChromaticAberrationPostProcess"), TEXT("/Game/Materials/Retro"), UMaterial::StaticClass(), MaterialFactory);
	UMaterial* Mat = Cast<UMaterial>(NewAsset);
	if (!Mat)
	{
		UE_LOG(LogT66Editor, Error, TEXT("Failed to create RetroChromaticAberrationMaterial"));
		return false;
	}

	Mat->MaterialDomain = MD_PostProcess;
	Mat->BlendableLocation = BL_SceneColorAfterTonemapping;

	auto* EdData = Mat->GetEditorOnlyData();
	auto AddExpression = [EdData](UMaterialExpression* Expression, int32 X, int32 Y)
	{
		Expression->MaterialExpressionEditorX = X;
		Expression->MaterialExpressionEditorY = Y;
		EdData->ExpressionCollection.Expressions.Add(Expression);
		return Expression;
	};

	UMaterialExpressionScreenPosition* ScreenPosition = Cast<UMaterialExpressionScreenPosition>(AddExpression(NewObject<UMaterialExpressionScreenPosition>(Mat), -1800, 0));

	UMaterialExpressionComponentMask* ScreenUV = Cast<UMaterialExpressionComponentMask>(AddExpression(NewObject<UMaterialExpressionComponentMask>(Mat), -1560, 0));
	ScreenUV->Input.Expression = ScreenPosition;
	ScreenUV->R = 1;
	ScreenUV->G = 1;

	UMaterialExpressionConstant2Vector* Center = Cast<UMaterialExpressionConstant2Vector>(AddExpression(NewObject<UMaterialExpressionConstant2Vector>(Mat), -1800, 220));
	Center->R = 0.5f;
	Center->G = 0.5f;

	UMaterialExpressionSubtract* OffsetFromCenter = Cast<UMaterialExpressionSubtract>(AddExpression(NewObject<UMaterialExpressionSubtract>(Mat), -1320, 0));
	OffsetFromCenter->A.Expression = ScreenUV;
	OffsetFromCenter->B.Expression = Center;

	UMaterialExpressionDotProduct* RadiusSquared = Cast<UMaterialExpressionDotProduct>(AddExpression(NewObject<UMaterialExpressionDotProduct>(Mat), -1080, -100));
	RadiusSquared->A.Expression = OffsetFromCenter;
	RadiusSquared->B.Expression = OffsetFromCenter;

	UMaterialExpressionScalarParameter* DistortionAmount = Cast<UMaterialExpressionScalarParameter>(AddExpression(NewObject<UMaterialExpressionScalarParameter>(Mat), -1080, 180));
	DistortionAmount->ParameterName = FName(TEXT("DistortionAmount"));
	DistortionAmount->DefaultValue = 0.0f;

	UMaterialExpressionMultiply* DistortionRadius = Cast<UMaterialExpressionMultiply>(AddExpression(NewObject<UMaterialExpressionMultiply>(Mat), -840, 60));
	DistortionRadius->A.Expression = DistortionAmount;
	DistortionRadius->B.Expression = RadiusSquared;

	UMaterialExpressionConstant* One = Cast<UMaterialExpressionConstant>(AddExpression(NewObject<UMaterialExpressionConstant>(Mat), -840, 240));
	One->R = 1.0f;

	UMaterialExpressionAdd* DistortionFactor = Cast<UMaterialExpressionAdd>(AddExpression(NewObject<UMaterialExpressionAdd>(Mat), -600, 120));
	DistortionFactor->A.Expression = One;
	DistortionFactor->B.Expression = DistortionRadius;

	UMaterialExpressionMultiply* DistortedOffset = Cast<UMaterialExpressionMultiply>(AddExpression(NewObject<UMaterialExpressionMultiply>(Mat), -360, 0));
	DistortedOffset->A.Expression = OffsetFromCenter;
	DistortedOffset->B.Expression = DistortionFactor;

	UMaterialExpressionAdd* FinalUV = Cast<UMaterialExpressionAdd>(AddExpression(NewObject<UMaterialExpressionAdd>(Mat), -120, 0));
	FinalUV->A.Expression = DistortedOffset;
	FinalUV->B.Expression = Center;

	UMaterialExpressionSubtract* DistortedDirection = Cast<UMaterialExpressionSubtract>(AddExpression(NewObject<UMaterialExpressionSubtract>(Mat), 120, -80));
	DistortedDirection->A.Expression = FinalUV;
	DistortedDirection->B.Expression = Center;

	UMaterialExpressionNormalize* RadialDirection = Cast<UMaterialExpressionNormalize>(AddExpression(NewObject<UMaterialExpressionNormalize>(Mat), 360, -160));
	RadialDirection->VectorInput.Expression = DistortedDirection;

	UMaterialExpressionLength* DirectionLength = Cast<UMaterialExpressionLength>(AddExpression(NewObject<UMaterialExpressionLength>(Mat), 360, 40));
	DirectionLength->Input.Expression = DistortedDirection;

	UMaterialExpressionScalarParameter* ChromaticStrength = Cast<UMaterialExpressionScalarParameter>(AddExpression(NewObject<UMaterialExpressionScalarParameter>(Mat), 120, 220));
	ChromaticStrength->ParameterName = FName(TEXT("ChromaticStrength"));
	ChromaticStrength->DefaultValue = 0.0f;

	UMaterialExpressionMultiply* WeightedStrength = Cast<UMaterialExpressionMultiply>(AddExpression(NewObject<UMaterialExpressionMultiply>(Mat), 600, 100));
	WeightedStrength->A.Expression = ChromaticStrength;
	WeightedStrength->B.Expression = DirectionLength;

	UMaterialExpressionMultiply* ChromaticOffset = Cast<UMaterialExpressionMultiply>(AddExpression(NewObject<UMaterialExpressionMultiply>(Mat), 840, -20));
	ChromaticOffset->A.Expression = RadialDirection;
	ChromaticOffset->B.Expression = WeightedStrength;

	UMaterialExpressionAdd* RedUV = Cast<UMaterialExpressionAdd>(AddExpression(NewObject<UMaterialExpressionAdd>(Mat), 1080, -140));
	RedUV->A.Expression = FinalUV;
	RedUV->B.Expression = ChromaticOffset;

	UMaterialExpressionSubtract* BlueUV = Cast<UMaterialExpressionSubtract>(AddExpression(NewObject<UMaterialExpressionSubtract>(Mat), 1080, 120));
	BlueUV->A.Expression = FinalUV;
	BlueUV->B.Expression = ChromaticOffset;

	UMaterialExpressionSaturate* SaturatedFinalUV = Cast<UMaterialExpressionSaturate>(AddExpression(NewObject<UMaterialExpressionSaturate>(Mat), 1080, 0));
	SaturatedFinalUV->Input.Expression = FinalUV;

	UMaterialExpressionSaturate* SaturatedRedUV = Cast<UMaterialExpressionSaturate>(AddExpression(NewObject<UMaterialExpressionSaturate>(Mat), 1320, -140));
	SaturatedRedUV->Input.Expression = RedUV;

	UMaterialExpressionSaturate* SaturatedBlueUV = Cast<UMaterialExpressionSaturate>(AddExpression(NewObject<UMaterialExpressionSaturate>(Mat), 1320, 120));
	SaturatedBlueUV->Input.Expression = BlueUV;

	UMaterialExpressionSceneTexture* RedSample = Cast<UMaterialExpressionSceneTexture>(AddExpression(NewObject<UMaterialExpressionSceneTexture>(Mat), 1560, -220));
	RedSample->SceneTextureId = PPI_PostProcessInput0;
	RedSample->bFiltered = true;
	RedSample->Coordinates.Expression = SaturatedRedUV;

	UMaterialExpressionSceneTexture* GreenSample = Cast<UMaterialExpressionSceneTexture>(AddExpression(NewObject<UMaterialExpressionSceneTexture>(Mat), 1560, 0));
	GreenSample->SceneTextureId = PPI_PostProcessInput0;
	GreenSample->bFiltered = true;
	GreenSample->Coordinates.Expression = SaturatedFinalUV;

	UMaterialExpressionSceneTexture* BlueSample = Cast<UMaterialExpressionSceneTexture>(AddExpression(NewObject<UMaterialExpressionSceneTexture>(Mat), 1560, 220));
	BlueSample->SceneTextureId = PPI_PostProcessInput0;
	BlueSample->bFiltered = true;
	BlueSample->Coordinates.Expression = SaturatedBlueUV;

	UMaterialExpressionComponentMask* RedChannel = Cast<UMaterialExpressionComponentMask>(AddExpression(NewObject<UMaterialExpressionComponentMask>(Mat), 1800, -220));
	RedChannel->Input.Expression = RedSample;
	RedChannel->R = 1;

	UMaterialExpressionComponentMask* GreenChannel = Cast<UMaterialExpressionComponentMask>(AddExpression(NewObject<UMaterialExpressionComponentMask>(Mat), 1800, 0));
	GreenChannel->Input.Expression = GreenSample;
	GreenChannel->G = 1;

	UMaterialExpressionComponentMask* BlueChannel = Cast<UMaterialExpressionComponentMask>(AddExpression(NewObject<UMaterialExpressionComponentMask>(Mat), 1800, 220));
	BlueChannel->Input.Expression = BlueSample;
	BlueChannel->B = 1;

	UMaterialExpressionAppendVector* RedGreen = Cast<UMaterialExpressionAppendVector>(AddExpression(NewObject<UMaterialExpressionAppendVector>(Mat), 2040, -120));
	RedGreen->A.Expression = RedChannel;
	RedGreen->B.Expression = GreenChannel;

	UMaterialExpressionAppendVector* FinalColor = Cast<UMaterialExpressionAppendVector>(AddExpression(NewObject<UMaterialExpressionAppendVector>(Mat), 2280, -40));
	FinalColor->A.Expression = RedGreen;
	FinalColor->B.Expression = BlueChannel;

	EdData->EmissiveColor.Expression = FinalColor;

	Mat->PreEditChange(nullptr);
	Mat->PostEditChange();
	Mat->MarkPackageDirty();

	UPackage* Package = Mat->GetOutermost();
	if (Package)
	{
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Standalone;
		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		if (UPackage::SavePackage(Package, Mat, *PackageFileName, SaveArgs))
		{
			UE_LOG(LogT66Editor, Log, TEXT("Created and saved RetroChromaticAberrationMaterial at %s"), *MaterialPath);
			return true;
		}
	}

	UE_LOG(LogT66Editor, Warning, TEXT("Failed to save RetroChromaticAberrationMaterial"));
	return false;
}

bool UT66UISetupSubsystem::CreateLabLevel()
{
	const FString LabLevelPath = TEXT("/Game/Maps/LabLevel");

	// Create a new world in our package (no template duplication = no cross-package BSP references)
	UPackage* NewPackage = CreatePackage(*LabLevelPath);
	if (!NewPackage)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("CreateLabLevel: CreatePackage failed"));
		return false;
	}
	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false, FName("LabLevel"), NewPackage, false);
	if (!World || !World->PersistentLevel)
	{
		UE_LOG(LogT66Editor, Warning, TEXT("CreateLabLevel: CreateWorld failed"));
		return false;
	}
	World->SetFlags(RF_Standalone | RF_Public);
	World->PersistentLevel->SetFlags(RF_Standalone | RF_Public);
	if (World->GetWorldSettings())
	{
		World->GetWorldSettings()->SetFlags(RF_Standalone | RF_Public);
	}

	// GameMode: same as GameplayLevel
	const FString GameModePath = TEXT("/Game/Blueprints/GameModes/BP_GameplayGameMode.BP_GameplayGameMode_C");
	UClass* GameModeClass = LoadClass<AGameModeBase>(nullptr, *GameModePath);
	if (GameModeClass && World->GetWorldSettings())
	{
		World->GetWorldSettings()->DefaultGameMode = GameModeClass;
	}

	// Spawn PlayerStart at origin (Lab spawn uses 0,0,200)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.OverrideLevel = World->PersistentLevel;
	FVector PlayerStartLoc(0.f, 0.f, 200.f);
	World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), PlayerStartLoc, FRotator::ZeroRotator, SpawnParams);

	// Spawn DirectionalLight
	ADirectionalLight* DirLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector::ZeroVector, FRotator(-45.f, 0.f, 0.f), SpawnParams);
	if (DirLight && DirLight->GetLightComponent())
	{
		DirLight->GetLightComponent()->SetIntensity(10.f);
	}

	// Spawn SkyLight
	if (ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
	{
		if (USkyLightComponent* SkyLightComponent = Cast<USkyLightComponent>(SkyLight->GetLightComponent()))
		{
			SkyLightComponent->SetRealTimeCapture(false);
		}
	}

	World->MarkPackageDirty();
	NewPackage->MarkPackageDirty();

	// Save to disk
	FString Filename = FPackageName::LongPackageNameToFilename(LabLevelPath, FPackageName::GetMapPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;
	if (!UPackage::SavePackage(NewPackage, World, *Filename, SaveArgs))
	{
		UE_LOG(LogT66Editor, Warning, TEXT("CreateLabLevel: SavePackage failed for %s"), *LabLevelPath);
		return false;
	}
	UE_LOG(LogT66Editor, Log, TEXT("CreateLabLevel: created and saved LabLevel at %s"), *LabLevelPath);
	return true;
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
				UE_LOG(LogT66Editor, Log, TEXT("PlayerController RuntimeScreenClasses: %d registered"), PCCDO->RuntimeScreenClasses.Num());
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
