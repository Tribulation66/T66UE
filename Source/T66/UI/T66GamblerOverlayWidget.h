// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "Styling/SlateBrush.h"
#include "T66GamblerOverlayWidget.generated.h"

class STextBlock;
class SWidgetSwitcher;
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
	virtual void NativeDestruct() override;

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
		BlackJack = 4,
		Lottery = 5,
		Plinko = 6,
		BoxOpening = 7,
	};

	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	TSharedPtr<SWidgetSwitcher> CasinoSwitcher;
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> DebtText;
	TSharedPtr<STextBlock> StatusText;

	// Anger indicator (0..1) - face sprites (Happy/Neutral/Angry)
	TSharedPtr<SImage> AngerCircleImage;
	TSharedPtr<STextBlock> AngerText;
	FSlateBrush AngerFace_Happy;
	FSlateBrush AngerFace_Neutral;
	FSlateBrush AngerFace_Angry;

	// Stats panel (refreshable when inventory changes)
	TSharedPtr<SBox> StatsPanelBox;

	// Inventory strip (shared with vendor)
	static constexpr int32 InventorySlotCount = 5;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TArray<TSharedPtr<SWidget>> InventorySlotButtons;
	TArray<TSharedPtr<STextBlock>> InventorySlotTexts;
	TArray<TSharedPtr<SImage>> InventorySlotIconImages;
	TArray<TSharedPtr<FSlateBrush>> InventorySlotIconBrushes;
	int32 SelectedInventoryIndex = -1;

	// Sell panel
	TSharedPtr<SBox> SellPanelContainer;
	TSharedPtr<STextBlock> SellItemNameText;
	TSharedPtr<STextBlock> SellItemDescText;
	TSharedPtr<STextBlock> SellItemPriceText;
	TSharedPtr<SWidget> SellItemButton;

	// Casino/Buyback tab: 0 = Casino (games), 1 = Buyback
	TSharedPtr<SWidgetSwitcher> CasinoBuybackSwitcher;
	TArray<TSharedPtr<STextBlock>> BuybackNameTexts;
	TArray<TSharedPtr<STextBlock>> BuybackDescTexts;
	TArray<TSharedPtr<STextBlock>> BuybackPriceTexts;
	TArray<TSharedPtr<SBorder>> BuybackTileBorders;
	TArray<TSharedPtr<SBorder>> BuybackIconBorders;
	TArray<TSharedPtr<SImage>> BuybackIconImages;
	TArray<TSharedPtr<FSlateBrush>> BuybackIconBrushes;
	TArray<TSharedPtr<SWidget>> BuybackBuyButtons;

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
	int32 LockedBetAmount = 0;   // amount locked by Bet button; consumed when round starts
	int32 PendingBetAmount = 0;  // amount for current round (after deduct); used for payout/anger

	// Coin flip page
	TSharedPtr<STextBlock> CoinFlipStatusText;  // in-panel status/win (replaces top bar when on this game)
	TSharedPtr<STextBlock> CoinFlipResultText;
	TSharedPtr<SImage> CoinFlipImage;
	FSlateBrush CoinBrush_Heads;
	FSlateBrush CoinBrush_Tails;
	FSlateBrush CoinBrush_Side;
	FTimerHandle CoinSpinTimerHandle;
	float CoinSpinElapsed = 0.f;
	float CoinSpinDuration = 2.0f;
	bool bCoinSpinActive = false;
	bool bCoinSpinResultHeads = false; // final face to land on
	void TickCoinSpin();
	void FinishCoinSpin();

	// RPS page
	TSharedPtr<STextBlock> RpsStatusText;
	TSharedPtr<STextBlock> RpsResultText;
	TSharedPtr<SImage> RpsHumanHandImage;
	TSharedPtr<SImage> RpsDemonHandImage;
	TSharedPtr<SBox> RpsHandsContainer;
	FSlateBrush RpsBrush_HumanRock;
	FSlateBrush RpsBrush_HumanPaper;
	FSlateBrush RpsBrush_HumanScissors;
	FSlateBrush RpsBrush_DemonRock;
	FSlateBrush RpsBrush_DemonPaper;
	FSlateBrush RpsBrush_DemonScissors;

	// Black Jack page
	TSharedPtr<STextBlock> BlackJackStatusText;
	TSharedPtr<STextBlock> BlackJackResultText;
	TSharedPtr<SImage> BlackJackDealerHoleCardBackImage;
	TArray<TSharedPtr<SImage>> BlackJackDealerCardImages;
	TArray<TSharedPtr<SImage>> BlackJackPlayerCardImages;
	TArray<FSlateBrush> BJCardBrushes; // 53 brushes: [0..51] = face cards, [52] = card back
	FSlateBrush BlackJackCardBackBrush;
	void InitBJCardBrushes();
	FSlateBrush* GetBJCardBrush(int32 CardIndex);
	FSlateBrush* GetBJCardBackBrush();

	// Game icons (casino cards)
	FSlateBrush GameIcon_CoinFlip;
	FSlateBrush GameIcon_Rps;
	FSlateBrush GameIcon_BlackJack;
	// More Games (placeholder brushes until textures provided)
	FSlateBrush GameIcon_Lottery;
	FSlateBrush GameIcon_Plinko;
	FSlateBrush GameIcon_BoxOpening;

	TSharedPtr<SWidgetSwitcher> GameSelectionSwitcher;  // 0 = main 3 games, 1 = Lottery/Plinko/Box Opening

	// Lottery: pick 5 from 1-10, then 5 drawn at random
	TSet<int32> LotterySelected;
	TArray<int32> LotteryDrawn;
	int32 LotteryRevealedCount = 0;
	FTimerHandle LotteryDrawTimerHandle;
	TArray<TSharedPtr<SBorder>> LotteryNumberBorders;
	TSharedPtr<STextBlock> LotteryPicksText;
	TSharedPtr<STextBlock> LotteryDrawnText;
	TSharedPtr<STextBlock> LotteryResultText;
	FReply OnLotteryNumberClicked(int32 Num);
	void StartLotteryDraw();
	void TickLotteryRevealNext();

	// Plinko: ball drops through pins, lands in slot (9 slots, middle = 0.25x)
	static constexpr int32 PlinkoSlotCount = 9;
	int32 PlinkoBallSlot = 0;
	int32 PlinkoRow = 0;
	float PlinkoBallPixelX = 0.f;
	float PlinkoBallPixelY = 0.f;
	FTimerHandle PlinkoTimerHandle;
	TSharedPtr<SBox> PlinkoBoardContainer;
	TSharedPtr<STextBlock> PlinkoResultText;
	void StartPlinkoDrop();
	void TickPlinkoDrop();

	// Box Opening: 4 colors (Black 0.25x, Red 1x, Yellow 5x, White 30x)
	static constexpr int32 BoxColorCount = 4;
	int32 BoxOpeningCurrentIndex = 0;
	int32 BoxOpeningResultIndex = 0;
	int32 BoxOpeningSpinSteps = 0;
	int32 BoxOpeningStripOffset = 0;  // which color is at left of visible 5; (offset+2)%4 = middle
	FTimerHandle BoxOpeningTimerHandle;
	TSharedPtr<SBox> BoxOpeningStripContainer;
	TSharedPtr<STextBlock> BoxOpeningResultText;
	void StartBoxOpeningSpin();
	void TickBoxOpeningSpin();
	FLinearColor GetBoxOpeningColor(int32 ColorIndex) const;

	int32 WinGoldAmount = 10; // used as a default gamble amount suggestion from DT
	bool bInputLocked = false;

	enum class ERevealType : uint8
	{
		None = 0,
		CoinFlip,
		RockPaperScissors,
		BlackJack,
	};

	ERevealType PendingRevealType = ERevealType::None;
	bool bPendingWin = false;
	bool bPendingDraw = false;
	bool bPendingCheat = false;
	bool bPendingCoinFlipChoseHeads = true;
	bool bPendingCoinFlipResultHeads = false;
	int32 PendingRpsPlayerChoice = 0;
	int32 PendingRpsOppChoice = 0;

	// Black Jack state (replaces Find-the-Ball)
	enum class EBlackJackPhase : uint8 { WaitingForAction, PlayerBusted, DealerPlaying, RoundOver };
	TArray<int32> BJDeck;           // 0..51 (rank = card%13, suit = card/13)
	TArray<int32> BJDealerHand;
	TArray<TArray<int32>> BJPlayerHands;  // 1 or 2 hands (split)
	int32 BJCurrentHandIndex = 0;
	int32 BJBetAmount = 0;
	bool BJDealerHoleRevealed = false;
	EBlackJackPhase BJPhase = EBlackJackPhase::WaitingForAction;
	int32 BJWinAmount = 0;         // positive=win, negative=loss, 0=push (for reveal display)
	bool bBJPlayerBusted = false;

	FTimerHandle RevealTimerHandle;
	void RevealPendingOutcome();

	// Black Jack: after round ends, animate cards taken then reset for new round
	FTimerHandle BJCardsTakenTimerHandle;
	int32 BJCardsTakenStep = 0;
	void StartBJCardsTakenAnimation();
	void TickBJCardsTaken();
	void ResetBlackJackForNewRound();

	FReply OnBack();
	FReply OnDialogueGamble();
	FReply OnDialogueTeleport();

	FReply OnSelectInventorySlot(int32 InventoryIndex);
	FReply OnSellSelectedClicked();
	FReply OnBuybackSlot(int32 SlotIndex);

	FReply OnBetClicked();
	FReply OnBorrowClicked();
	FReply OnPaybackClicked();

	FReply OnOpenCoinFlip();
	FReply OnOpenRps();
	FReply OnOpenBlackJack();
	FReply OnMoreGamesClicked();
	FReply OnBackToMainGames();
	FReply OnOpenLottery();
	FReply OnOpenPlinko();
	FReply OnOpenBoxOpening();
	FReply OnGameBackToSelection();  // back from game to casino card selection
	FReply OnBJDealClicked();       // lock bet then deal (Black Jack only)

	FReply OnCoinFlipHeads();
	FReply OnCoinFlipTails();
	FReply OnRpsRock();
	FReply OnRpsPaper();
	FReply OnRpsScissors();
	FReply OnBJHit();
	FReply OnBJStand();
	FReply OnBJDouble();
	FReply OnBJSplit();

	FReply OnCheatYes();
	FReply OnCheatNo();

	void SetPage(EGamblerPage Page);
	void RefreshTopBar();
	void RefreshBuyback();
	void RefreshInventory();
	void RefreshSellPanel();
	void RefreshStatsPanel();
	bool IsBossActive() const;
	bool TryPayToPlay();
	bool TryPayWithLockedBet();  // requires LockedBetAmount > 0, deducts and sets PendingBetAmount
	void AwardWin();
	void AwardWin(int32 BetAmount);
	/** Award payout for Lottery/Plinko/Box: AddGold(BetAmount * Multiplier). */
	void AwardPayout(int32 BetAmount, float Multiplier);
	void SetStatus(const FText& Msg, const FLinearColor& Color = FLinearColor::White);

	void ShowCheatPrompt();
	void HideCheatPrompt();
	void TriggerGamblerBossIfAngry();

	void ResolveCoinFlip(bool bChoseHeads);
	void ResolveRps(int32 PlayerChoice /*0=Rock,1=Paper,2=Scissors*/);

	void StartBlackJackRound();
	void RefreshBlackJackCards();
	int32 BJHandValue(const TArray<int32>& Hand) const;
	FText BJCardToText(int32 CardIndex) const;
	void BJDealerPlay();
	void BJEndRound();
	void RevealBlackJackOutcome();
	void HandleBJHit();
	void HandleBJStand();
	void HandleBJDouble();
	void HandleBJSplit();

	void TeleportToVendor();

	UFUNCTION()
	void HandleBuybackChanged();
};

