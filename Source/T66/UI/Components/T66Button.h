// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/T66UITypes.h"
#include "Data/T66DataTypes.h"
#include "T66Button.generated.h"

class UButton;
class UTextBlock;

/**
 * Action types for T66 buttons
 */
UENUM(BlueprintType)
enum class ET66ButtonAction : uint8
{
	None UMETA(DisplayName = "None"),
	// Navigation actions
	NavigateTo UMETA(DisplayName = "Navigate To Screen"),
	NavigateBack UMETA(DisplayName = "Navigate Back"),
	ShowModal UMETA(DisplayName = "Show Modal"),
	CloseModal UMETA(DisplayName = "Close Modal"),
	// Game actions
	StartGame UMETA(DisplayName = "Start Game (Enter Tribulation)"),
	QuitGame UMETA(DisplayName = "Quit Game"),
	// Selection actions
	SelectPartySize UMETA(DisplayName = "Select Party Size"),
	SelectHero UMETA(DisplayName = "Select Hero"),
	SelectCompanion UMETA(DisplayName = "Select Companion"),
	ConfirmCompanion UMETA(DisplayName = "Confirm Companion"),
	// Custom (calls Blueprint event)
	Custom UMETA(DisplayName = "Custom Action")
};

/**
 * Reusable button component for T66 UI
 * Supports action routing based on configuration
 */
UCLASS(Blueprintable)
class T66_API UT66Button : public UUserWidget
{
	GENERATED_BODY()

public:
	UT66Button(const FObjectInitializer& ObjectInitializer);

	/** The action this button performs when clicked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config")
	ET66ButtonAction Action = ET66ButtonAction::None;

	/** Target screen (for NavigateTo/ShowModal actions) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config", meta = (EditCondition = "Action == ET66ButtonAction::NavigateTo || Action == ET66ButtonAction::ShowModal"))
	ET66ScreenType TargetScreen = ET66ScreenType::None;

	/** Party size to select (for SelectPartySize action) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config", meta = (EditCondition = "Action == ET66ButtonAction::SelectPartySize"))
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** Hero ID to select (for SelectHero action) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config", meta = (EditCondition = "Action == ET66ButtonAction::SelectHero"))
	FName HeroID;

	/** Companion ID to select (for SelectCompanion action) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config", meta = (EditCondition = "Action == ET66ButtonAction::SelectCompanion"))
	FName CompanionID;

	/** Button display text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config")
	FText ButtonText;

	/** The actual UButton component (bind in Blueprint) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UButton> ButtonWidget;

	/** The text display (bind in Blueprint) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> ButtonTextWidget;

	/** Called when button is clicked - executes the configured action */
	UFUNCTION(BlueprintCallable, Category = "Button")
	void OnButtonClicked();

	/** Event for custom actions - implement in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Button")
	void OnCustomAction();

	/** Set button text at runtime */
	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetButtonText(FText NewText);

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

private:
	/** Execute the configured action */
	void ExecuteAction();

	/** Get the owning screen widget */
	class UT66ScreenBase* GetOwningScreen() const;
};
