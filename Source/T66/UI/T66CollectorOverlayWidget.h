// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66CollectorOverlayWidget.generated.h"

struct FSlateBrush;

/**
 * Full-screen Lab Collector UI: Items (ADD to inventory), NPCs, Enemies, Interactables (Spawn with random spread).
 * Opened when interacting with The Collector NPC in the Lab.
 */
UCLASS()
class T66_API UT66CollectorOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	void CloseOverlay();

private:
	void OnAddItem(FName ItemID);
	void OnSpawnNPC(FName NPCID);
	void OnSpawnEnemy(FName EnemyID, bool bIsBoss);
	void OnSpawnInteractable(FName InteractableID);
	void OnExitLab();
	void RefreshContent();

	TArray<FName> GetUnlockedItemIDs() const;
	TArray<FName> GetUnlockedEnemyIDs() const;

	/** 0=Items, 1=NPCs, 2=Enemies, 3=Interactables */
	UPROPERTY()
	int32 CollectorTabIndex = 0;

	/** Keeps item icon brushes alive for the lifetime of the widget. */
	TArray<TSharedPtr<FSlateBrush>> ItemIconBrushes;
};
