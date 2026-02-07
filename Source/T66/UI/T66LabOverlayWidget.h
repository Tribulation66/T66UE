// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66LabOverlayWidget.generated.h"

class UT66GameInstance;
class UT66RunStateSubsystem;
class UT66AchievementsSubsystem;
class AT66GameMode;

/**
 * Lab overlay: single right-side panel (Items | NPC | Mobs | Stage Bosses tabs) between Power and inventory.
 * Toggleable like minimap. Shown only when bIsLabLevel. All strings localized.
 */
UCLASS()
class T66_API UT66LabOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	/** Call to refresh the widget (e.g. after granting item or spawning). */
	void RefreshLabUI();

protected:
	UT66GameInstance* GetT66GameInstance() const;
	AT66GameMode* GetLabGameMode() const;
	UT66RunStateSubsystem* GetRunState() const;
	UT66AchievementsSubsystem* GetAchievements() const;

	void OnGrantItem(FName ItemID);
	void OnResetItems();
	void OnSpawnEnemy(FName EnemyID, int32 TabIndex);
	void OnResetEnemies();
	void OnExitLab();
	void OnToggleLabPanel();

	TArray<FName> GetUnlockedItemIDs() const;
	TArray<FName> GetUnlockedEnemyIDs() const;

	/** Tab index: 0 = Items, 1 = NPC, 2 = Mobs, 3 = Stage Bosses */
	UPROPERTY()
	int32 LabTabIndex = 0;

	/** When true, the Lab tabbed panel is visible; when false, only the toggle button shows. */
	UPROPERTY()
	bool bLabPanelExpanded = true;
};
