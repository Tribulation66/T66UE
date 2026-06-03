// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66CasinoGamblerTabWidget.generated.h"

class STextBlock;
class SWidgetSwitcher;
template<typename NumericType> class SSpinBox;
class UT66CoinFlipGameWidget;
class UT66FindJokerGameWidget;
class UT66GuessCupGameWidget;
class UT66StickPickGameWidget;
enum class ET66AntiCheatGamblerGameType : uint8;
struct FT66WidgetGameHostContext;
struct FT66WidgetGameResult;

/** Full-screen, non-pausing Gambler UI for the live casino game set. */
UCLASS(Blueprintable)
class T66_API UT66CasinoGamblerTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	void CloseOverlay();
	void SetEmbeddedInCasinoShell(bool bEmbedded) { bEmbeddedInCasinoShell = bEmbedded; }
	void SetGamblingOnlyKiosk(bool bEnabled) { bGamblingOnlyKiosk = bEnabled; }
	void OpenCasinoPage();
	void SetWinGoldAmount(int32 InAmount);

private:
	enum class EGamblerPage : uint8
	{
		Dialogue = 0,
		Casino = 1,
		CoinFlip = 2,
		GuessCup = 3,
		StickPick = 4,
		FindJoker = 5,
	};

	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	TSharedPtr<SWidgetSwitcher> CasinoSwitcher;
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SSpinBox<int32>> GambleAmountSpin;

	UPROPERTY()
	TObjectPtr<UT66CoinFlipGameWidget> CoinFlipGameWidget;

	UPROPERTY()
	TObjectPtr<UT66GuessCupGameWidget> GuessCupGameWidget;

	UPROPERTY()
	TObjectPtr<UT66StickPickGameWidget> StickPickGameWidget;

	UPROPERTY()
	TObjectPtr<UT66FindJokerGameWidget> FindJokerGameWidget;

	int32 GambleAmount = 10;
	int32 LockedBetAmount = 0;
	int32 WinGoldAmount = 10;
	bool bInputLocked = false;
	bool bPendingStickTargetShortest = false;
	bool bEmbeddedInCasinoShell = false;
	bool bGamblingOnlyKiosk = false;

	FReply OnBack();
	FReply OnDialogueGamble();
	FReply OnBetClicked();
	FReply OnOpenCoinFlip();
	FReply OnOpenGuessCup();
	FReply OnOpenStickPick();
	FReply OnOpenFindJoker();
	FReply OnGameBackToSelection();

	void SetPage(EGamblerPage Page);
	FT66WidgetGameHostContext BuildChildHostContext();
	void ReturnToGameSelection();
	void RefreshTopBar();
	bool IsCasinoGameAllowed(FName CasinoGameID) const;
	FReply HandleBlockedCasinoGame();
	bool TryPayWithLockedBet(int32& OutBetAmount);
	int32 GetCurrentCasinoWager() const;
	void AwardPayoutGoldAmount(int32 PayoutGold);
	void SetStatus(const FText& Msg, const FLinearColor& Color = FLinearColor::White);
	void HandleCasinoWidgetGameResult(const FT66WidgetGameResult& Result);
	void ReportCasinoResult(FName GameID, bool bSuccessful, int32 PayoutGold);
	void RecordCasinoRound(
		ET66AntiCheatGamblerGameType GameType,
		int32 BetGold,
		int32 PayoutGold,
		bool bWin,
		int32 PlayerChoice,
		int32 OpponentChoice,
		int32 OutcomeValue,
		int32 OutcomeSecondaryValue,
		int32 OutcomePreDrawSeed,
		int32 OutcomeDrawIndex,
		float OutcomeExpectedChance01,
		const FString& ActionSequence);

	void ActivateCoinFlipPage();
	void ActivateGuessCupPage();
	void ActivateStickPickPage();
	void ActivateFindJokerPage();

	void ResolveCoinFlip(bool bChoseHeads);
	void ResolveGuessCup(int32 CupIndex);
	void ResolveStickPick(int32 StickIndex);
	void ResolveFindJoker(int32 CardIndex);
	int32 DrawCasinoIndex(int32 ExclusiveMax, int32& OutPreDrawSeed, int32& OutDrawIndex) const;
	bool TryApplyGamblingLuckRescueReroll(int32 ExclusiveMax, int32 PlayerChoice, int32& InOutOutcomeValue, int32& InOutPreDrawSeed, int32& InOutDrawIndex, FString& InOutActionSequence) const;
};
