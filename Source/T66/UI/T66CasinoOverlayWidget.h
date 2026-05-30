// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "Input/Reply.h"
#include "T66CasinoOverlayWidget.generated.h"

class SBorder;
class SBox;
class SImage;
class STextBlock;
class SWidgetSwitcher;
class FDragDropEvent;
struct FGeometry;
struct FPointerEvent;
class UT66CasinoGamblerTabWidget;
class UT66CasinoVendorTabWidget;
class UT66LocalizationSubsystem;
class UT66RunStateSubsystem;

UCLASS(Blueprintable)
class T66_API UT66CasinoOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	enum class ECasinoOverlayMode : uint8
	{
		Full = 0,
		GamblerOnly = 1,
		VendorOnly = 2,
	};

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	void CloseOverlay();
	void OpenGamblerTab();
	void OpenVendorTab();
	void OpenAlchemyTab();
	void SetOverlayMode(ECasinoOverlayMode InMode);
	void SetCasinoGamblerWinGoldAmount(int32 InAmount);
	void SetShopAllowsSteal(bool bInAllowsSteal);

private:
	enum class ECasinoTab : uint8
	{
		Shop = 0,
		Gambling = 1,
	};

	TSharedRef<SWidget> BuildAlchemyPage(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc);
	void ReleaseCachedSlateResources();
	void SetActiveTab(ECasinoTab NewTab);
	void RefreshHeaderSummary();
	void RefreshAlchemy();
	void RefreshAlchemyTopBar();
	void RefreshAlchemyInventory();
	void RefreshAlchemyDropTargets();
	void SetAlchemyStatus(const FText& Message, const FLinearColor& Color);
	bool TryAssignAlchemySlot(bool bIsTargetSlot, int32 InventoryIndex);
	FReply HandleAlchemyInventoryDragDetected(const FGeometry& MyGeometry, const FPointerEvent& PointerEvent, int32 InventoryIndex);
	FReply HandleAlchemyDropTarget(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, bool bIsTargetSlot);
	FReply OnAlchemyTransmuteClicked();
	FReply OnClearAlchemyTargetClicked();
	FReply OnClearAlchemySacrificeClicked();

	UT66RunStateSubsystem* GetRunState() const;
	UT66LocalizationSubsystem* GetLocalization() const;

	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandleGoldOrDebtChanged();

	UFUNCTION()
	void HandleBossChanged();

	UFUNCTION()
	void HandleScoreChanged();

	UFUNCTION()
	void HandleStageTimerChanged();

	UPROPERTY()
	TObjectPtr<UT66CasinoGamblerTabWidget> CasinoGamblerTabWidget;

	UPROPERTY()
	TObjectPtr<UT66CasinoVendorTabWidget> VendorTabWidget;

	TSharedPtr<SWidgetSwitcher> TabSwitcher;
	TSharedPtr<STextBlock> HeaderTimerText;
	TSharedPtr<STextBlock> HeaderScoreText;

	TSharedPtr<STextBlock> AlchemyNetWorthText;
	TSharedPtr<STextBlock> AlchemyGoldText;
	TSharedPtr<STextBlock> AlchemyDebtText;
	TSharedPtr<STextBlock> AlchemyStatusText;
	TSharedPtr<STextBlock> AlchemyTargetText;
	TSharedPtr<STextBlock> AlchemyTargetDetailText;
	TSharedPtr<STextBlock> AlchemySacrificeText;
	TSharedPtr<STextBlock> AlchemySacrificeDetailText;
	TSharedPtr<STextBlock> AlchemyEmptyStateText;
	TSharedPtr<STextBlock> AlchemyResultText;
	TSharedPtr<SBox> AlchemyCardsRowContainer;
	TSharedPtr<SWidget> AlchemyUpgradeButton;
	TSharedPtr<SBorder> AlchemyTargetBorder;
	TSharedPtr<SBorder> AlchemySacrificeBorder;
	TSharedPtr<SImage> AlchemyTargetIconImage;
	TSharedPtr<SImage> AlchemySacrificeIconImage;
	TSharedPtr<FSlateBrush> AlchemyTargetIconBrush;
	TSharedPtr<FSlateBrush> AlchemySacrificeIconBrush;

	TArray<TSharedPtr<SBorder>> AlchemyInventorySlotBorders;
	TArray<TSharedPtr<STextBlock>> AlchemyInventorySlotCountTexts;
	TArray<TSharedPtr<STextBlock>> AlchemyInventorySlotTexts;
	TArray<TSharedPtr<SImage>> AlchemyInventorySlotImages;
	TArray<TSharedPtr<FSlateBrush>> AlchemyInventorySlotBrushes;

	int32 AlchemyTargetInventoryIndex = INDEX_NONE;
	int32 AlchemySacrificeInventoryIndex = INDEX_NONE;
	ECasinoTab ActiveTab = ECasinoTab::Shop;
	ECasinoOverlayMode OverlayMode = ECasinoOverlayMode::Full;
	FText AlchemyStatusMessage;
	FLinearColor AlchemyStatusColor = FLinearColor::White;
	int32 PendingCasinoGamblerWinGoldAmount = 10;
	bool bShopAllowsSteal = true;
};
