// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniBattleScreen.generated.h"

class FT66MiniBattleSimulation;
class ST66MiniBattleBoardWidget;

UCLASS(Blueprintable)
class T66MINI_API UT66MiniBattleScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniBattleScreen(const FObjectInitializer& ObjectInitializer);

	void TickWidgetBattle(float DeltaSeconds);
	void HandleBattleTransitionFromBoard();
	void ToggleBattlePause();
	void RequestInteract();
	void RequestUltimateAtBoardPosition(const FVector2D& BoardPosition);
	void SetPointerBoardPosition(const FVector2D& BoardPosition);
	void SetKeyboardMoveInput(const FVector2D& MoveInput);

	const FT66MiniBattleSimulation* GetBattleSimulationForSlate() const { return BattleSimulation; }

protected:
	virtual void BeginDestroy() override;
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual bool HandleBackAction() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	void EnsureBattleSimulation();
	void ReleaseRetainedSlateState();
	FReply HandleResumeClicked();
	FReply HandleSaveAndExitClicked();
	FText GetStatusText() const;
	FText GetWaveText() const;
	FText GetHealthText() const;
	FText GetResourceText() const;
	FText GetCombatText() const;
	EVisibility GetPauseOverlayVisibility() const;

	FT66MiniBattleSimulation* BattleSimulation = nullptr;
	TSharedPtr<ST66MiniBattleBoardWidget> BattleBoardWidget;
};
