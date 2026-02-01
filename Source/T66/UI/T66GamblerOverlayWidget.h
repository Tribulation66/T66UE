// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66GamblerOverlayWidget.generated.h"

class STextBlock;
class SWidgetSwitcher;
class SButton;
template<typename NumericType> class SSpinBox;
class SBox;
class SBorder;
class SImage;

/** Full-screen, non-pausing Gambler UI (dialogue -> casino -> minigames). */
UCLASS(Blueprintable)
class T66_API UT66GamblerOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	void CloseOverlay();

	/** Skip dialogue and open casino page (used by in-world dialogue). */
	void OpenCasinoPage();

	void SetWinGoldAmount(int32 InAmount);

private:
	enum class EGamblerPage : uint8
	{
		Dialogue = 0,
		Casino = 1,
		CoinFlip = 2,
		RockPaperScissors = 3,
		FindTheBall = 4,
	};

	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	TSharedPtr<SWidgetSwitcher> CasinoSwitcher;
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> DebtText;
	TSharedPtr<STextBlock> StatusText;

	// Anger indicator (0..1) - circle color + optional text
	TSharedPtr<SImage> AngerCircleImage;
	TSharedPtr<STextBlock> AngerText;

	// Inventory strip (shared with vendor)
	static constexpr int32 InventorySlotCount = 5;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TArray<TSharedPtr<SButton>> InventorySlotButtons;
	TArray<TSharedPtr<STextBlock>> InventorySlotTexts;
	int32 SelectedInventoryIndex = -1;

	// Sell panel
	TSharedPtr<SBox> SellPanelContainer;
	TSharedPtr<STextBlock> SellItemNameText;
	TSharedPtr<STextBlock> SellItemDescText;
	TSharedPtr<STextBlock> SellItemPriceText;
	TSharedPtr<SButton> SellItemButton;

	// Cheat prompt overlay
	TSharedPtr<SBox> CheatPromptContainer;
	bool bCheatPromptVisible = false;

	// Banking controls (casino page)
	TSharedPtr<SSpinBox<int32>> GambleAmountSpin;
	TSharedPtr<SSpinBox<int32>> BorrowAmountSpin;
	TSharedPtr<SSpinBox<int32>> PaybackAmountSpin;

	int32 GambleAmount = 10;
	int32 BorrowAmount = 0;
	int32 PaybackAmount = 0;

	// Coin flip page
	TSharedPtr<STextBlock> CoinFlipResultText;
	TSharedPtr<SButton> CoinFlipHeadsButton;
	TSharedPtr<SButton> CoinFlipTailsButton;

	// RPS page
	TSharedPtr<STextBlock> RpsResultText;
	TSharedPtr<SButton> RpsRockButton;
	TSharedPtr<SButton> RpsPaperButton;
	TSharedPtr<SButton> RpsScissorsButton;

	// Find-the-ball page
	TSharedPtr<STextBlock> BallResultText;
	TSharedPtr<SButton> BallCup1Button;
	TSharedPtr<SButton> BallCup2Button;
	TSharedPtr<SButton> BallCup3Button;

	int32 WinGoldAmount = 10; // used as a default gamble amount suggestion from DT
	bool bInputLocked = false;

	enum class ERevealType : uint8
	{
		None = 0,
		CoinFlip,
		RockPaperScissors,
		FindTheBall,
	};

	ERevealType PendingRevealType = ERevealType::None;
	bool bPendingWin = false;
	bool bPendingDraw = false;
	bool bPendingCheat = false;
	bool bPendingCoinFlipChoseHeads = true;
	bool bPendingCoinFlipResultHeads = false;
	int32 PendingRpsPlayerChoice = 0;
	int32 PendingRpsOppChoice = 0;
	int32 PendingFindBallChosenCup = 0;
	int32 PendingFindBallCorrectCup = 0;

	FTimerHandle RevealTimerHandle;
	void RevealPendingOutcome();

	FReply OnBack();
	FReply OnDialogueGamble();
	FReply OnDialogueTeleport();

	FReply OnSelectInventorySlot(int32 InventoryIndex);
	FReply OnSellSelectedClicked();

	FReply OnGambleMax();
	FReply OnBorrowClicked();
	FReply OnPaybackMax();
	FReply OnPaybackClicked();

	FReply OnOpenCoinFlip();
	FReply OnOpenRps();
	FReply OnOpenFindBall();

	FReply OnCoinFlipHeads();
	FReply OnCoinFlipTails();
	FReply OnRpsRock();
	FReply OnRpsPaper();
	FReply OnRpsScissors();
	FReply OnBallCup1();
	FReply OnBallCup2();
	FReply OnBallCup3();

	FReply OnCheatYes();
	FReply OnCheatNo();

	void SetPage(EGamblerPage Page);
	void RefreshTopBar();
	void RefreshInventory();
	void RefreshSellPanel();
	bool IsBossActive() const;
	bool TryPayToPlay();
	void AwardWin();
	void SetStatus(const FText& Msg, const FLinearColor& Color = FLinearColor::White);

	void ShowCheatPrompt();
	void HideCheatPrompt();
	void TriggerGamblerBossIfAngry();

	void ResolveCoinFlip(bool bChoseHeads);
	void ResolveRps(int32 PlayerChoice /*0=Rock,1=Paper,2=Scissors*/);
	void ResolveFindBall(int32 ChosenCup /*0..2*/);

	void TeleportToVendor();
};

