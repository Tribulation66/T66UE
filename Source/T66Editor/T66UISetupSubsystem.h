// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "T66UISetupSubsystem.generated.h"

class UBlueprint;
class UWidgetBlueprint;

/**
 * Editor Subsystem for T66 UI Setup
 * Provides tools to configure Blueprint defaults, populate Widget Blueprints,
 * and set up levels for proper PIE testing
 */
UCLASS()
class T66EDITOR_API UT66UISetupSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Run the full auto-configuration setup
	 * Call this from Editor Utility Widget or console command
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	void RunFullSetup();

	/**
	 * Configure the FrontendLevel to use BP_FrontendGameMode
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	bool ConfigureFrontendLevel();

	/**
	 * Configure the GameplayLevel to use BP_GameplayGameMode
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	bool ConfigureGameplayLevel();

	/**
	 * Configure the ColiseumLevel to use BP_GameplayGameMode
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	bool ConfigureColiseumLevel();

	/**
	 * Configure BP_FrontendGameMode defaults
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	bool ConfigureFrontendGameMode();

	/**
	 * Configure BP_GameplayGameMode defaults
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	bool ConfigureGameplayGameMode();

	/**
	 * Configure BP_T66PlayerController with screen class mappings
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	bool ConfigurePlayerController();

	/**
	 * Configure BP_T66GameInstance with DataTable references
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	bool ConfigureGameInstance();

	/**
	 * Print current setup status to the log
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	void PrintSetupStatus();

	/**
	 * Create the placeholder color material for hero visuals
	 */
	UFUNCTION(BlueprintCallable, Category = "T66 Setup")
	bool CreatePlaceholderMaterial();

private:
	/** Helper to load and modify a Blueprint */
	UBlueprint* LoadBlueprint(const FString& AssetPath);
	
	/** Helper to save a modified Blueprint */
	bool SaveBlueprint(UBlueprint* Blueprint);

	/** Helper to set a property value on a Blueprint's CDO */
	template<typename T>
	bool SetBlueprintProperty(UBlueprint* Blueprint, FName PropertyName, const T& Value);
};
