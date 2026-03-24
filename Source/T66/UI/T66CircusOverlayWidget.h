// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "Input/Reply.h"
#include "T66CircusOverlayWidget.generated.h"

class SBorder;
class SImage;
class STextBlock;
class SWidgetSwitcher;
class FDragDropEvent;
struct FGeometry;
struct FPointerEvent;
class UT66GamblerOverlayWidget;
class UT66VendorOverlayWidget;
class UT66LocalizationSubsystem;
class UT66RunStateSubsystem;

UCLASS(Blueprintable)
class T66_API UT66CircusOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	void CloseOverlay();
	void OpenGamblingTab();
	void OpenVendorTab();
	void OpenAlchemyTab();

private:
	enum class ECasinoTab : uint8
	{
		Vendor = 0,
		Gambling = 1,
		Alchemy = 2,
	};

	TSharedRef<SWidget> BuildAlchemyPage(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc);
	void SetActiveTab(ECasinoTab NewTab);
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
	void HandleAngerChanged();

	UFUNCTION()
	void HandleBossChanged();

	UPROPERTY()
	TObjectPtr<UT66GamblerOverlayWidget> GamblerTabWidget;

	UPROPERTY()
	TObjectPtr<UT66VendorOverlayWidget> VendorTabWidget;

	TSharedPtr<SWidgetSwitcher> TabSwitcher;

	TSharedPtr<STextBlock> AlchemyNetWorthText;
	TSharedPtr<STextBlock> AlchemyGoldText;
	TSharedPtr<STextBlock> AlchemyDebtText;
	TSharedPtr<STextBlock> AlchemyAngerText;
	TSharedPtr<STextBlock> AlchemyStatusText;
	TSharedPtr<STextBlock> AlchemyTargetText;
	TSharedPtr<STextBlock> AlchemySacrificeText;
	TSharedPtr<STextBlock> AlchemyResultText;
	TSharedPtr<SBorder> AlchemyTargetBorder;
	TSharedPtr<SBorder> AlchemySacrificeBorder;
	TSharedPtr<SImage> AlchemyTargetIconImage;
	TSharedPtr<SImage> AlchemySacrificeIconImage;
	TSharedPtr<FSlateBrush> AlchemyTargetIconBrush;
	TSharedPtr<FSlateBrush> AlchemySacrificeIconBrush;

	TArray<TSharedPtr<SBorder>> AlchemyInventorySlotBorders;
	TArray<TSharedPtr<STextBlock>> AlchemyInventorySlotTexts;
	TArray<TSharedPtr<SImage>> AlchemyInventorySlotImages;
	TArray<TSharedPtr<FSlateBrush>> AlchemyInventorySlotBrushes;

	int32 AlchemyTargetInventoryIndex = INDEX_NONE;
	int32 AlchemySacrificeInventoryIndex = INDEX_NONE;
	ECasinoTab ActiveTab = ECasinoTab::Vendor;
	FText AlchemyStatusMessage;
	FLinearColor AlchemyStatusColor = FLinearColor::White;
};

