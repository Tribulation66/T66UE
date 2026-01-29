// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66GameplayHUDWidget.generated.h"

class UT66RunStateSubsystem;
class STextBlock;
class SBorder;
class SBox;

/**
 * In-run HUD: hearts (5 icons), gold, toggleable inventory bar (1x5) and minimap placeholder.
 * T toggles panels; hearts and gold always visible. Event-driven updates via RunState.
 */
UCLASS(Blueprintable)
class T66_API UT66GameplayHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Call when RunState changes (hearts, gold, inventory, panel visibility). */
	UFUNCTION()
	void RefreshHUD();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> BuildSlateUI();

	UT66RunStateSubsystem* GetRunState() const;

	/** Cached Slate widgets for updates (set in BuildSlateUI via SAssignNew) */
	TSharedPtr<STextBlock> GoldText;
	TArray<TSharedPtr<SBorder>> HeartBorders;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TSharedPtr<SBox> InventoryPanelBox;
	TSharedPtr<SBox> MinimapPanelBox;
};
