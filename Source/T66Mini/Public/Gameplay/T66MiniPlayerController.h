// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/T66UITypes.h"
#include "T66MiniPlayerController.generated.h"

class AT66MiniPlayerPawn;
class UT66MiniPauseMenuWidget;

UCLASS()
class T66MINI_API AT66MiniPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AT66MiniPlayerController();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

	void ResumeFromPauseMenu();
	void SaveAndQuitToMiniMenuFromPause();
	bool StartLootCratePresentation();
	void ResolveLootCratePresentationNow(bool bAwardItem = true);
	bool IsLootCratePresentationActive() const { return bLootCratePresentationActive; }
	double GetLootCratePresentationElapsedSeconds() const;
	double GetLootCratePresentationDurationSeconds() const { return LootCratePresentationDurationSeconds; }
	const TArray<FName>& GetLootCrateStripItemIDs() const { return LootCrateStripItemIDs; }
	int32 GetLootCrateWinnerIndex() const { return LootCrateWinnerIndex; }
	FName GetLootCrateRewardItemID() const { return PendingLootCrateRewardItemID; }

	UFUNCTION(Client, Reliable)
	void ClientPrepareMiniOnlineRunSummary(bool bWasVictory, const FString& ResultLabel, int32 WaveReached, float RunSeconds);

	UFUNCTION(Client, Reliable)
	void ClientHandleMiniStageClear(int32 ChadCouponsAwarded, FName CompanionID);

	UFUNCTION(Client, Reliable)
	void ClientPrepareMiniFrontendTravel(ET66ScreenType PendingScreen, bool bIntermissionFlow);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION(Server, Reliable)
	void ServerTryInteract();

	UFUNCTION(Server, Reliable)
	void ServerRequestPartySaveAndReturnToFrontend();

	void ConfigureGameplayInputMode();
	void ConfigurePauseMenuInputMode();
	void HandleInteractPressed();
	void HandleUltimatePressed();
	void HandlePausePressed();
	void OpenPauseMenu();
	void ClosePauseMenu();
	void UpdateMouseFollowTarget();
	void UpdateLootCratePresentation();
	void BuildLootCrateStrip();
	bool IsOnlinePartyMode() const;
	bool IsPauseMenuVisible() const;
	static bool ProjectCursorToGroundPlane(APlayerController* PlayerController, FVector& OutWorldLocation);

	UPROPERTY()
	TObjectPtr<UT66MiniPauseMenuWidget> PauseMenuWidget;

	bool bLootCratePresentationActive = false;
	double LootCratePresentationStartTime = 0.0;
	double LootCratePresentationDurationSeconds = 2.8;
	int32 LootCrateWinnerIndex = 0;
	FName PendingLootCrateRewardItemID = NAME_None;
	TArray<FName> LootCrateStripItemIDs;
};
