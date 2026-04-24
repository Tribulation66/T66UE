// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "Input/DragAndDrop.h"
#include "Styling/CoreStyle.h"
#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66VendorOverlayWidget.h"
#include "UI/Style/T66Style.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define T66_REMOVE_COMMON_OVERLAY_RUNSTATE_DELEGATES(RunStatePtr, WidgetPtr, WidgetClass) \
	do \
	{ \
		(RunStatePtr)->InventoryChanged.RemoveDynamic((WidgetPtr), &WidgetClass::HandleInventoryChanged); \
		(RunStatePtr)->GoldChanged.RemoveDynamic((WidgetPtr), &WidgetClass::HandleGoldOrDebtChanged); \
		(RunStatePtr)->DebtChanged.RemoveDynamic((WidgetPtr), &WidgetClass::HandleGoldOrDebtChanged); \
		(RunStatePtr)->GamblerAngerChanged.RemoveDynamic((WidgetPtr), &WidgetClass::HandleAngerChanged); \
		(RunStatePtr)->BossChanged.RemoveDynamic((WidgetPtr), &WidgetClass::HandleBossChanged); \
	} while (false)

#define T66_RESET_COMMON_OVERLAY_RUNSTATE_DELEGATES(RunStatePtr, WidgetPtr, WidgetClass) \
	do \
	{ \
		T66_REMOVE_COMMON_OVERLAY_RUNSTATE_DELEGATES((RunStatePtr), (WidgetPtr), WidgetClass); \
		(RunStatePtr)->InventoryChanged.AddDynamic((WidgetPtr), &WidgetClass::HandleInventoryChanged); \
		(RunStatePtr)->GoldChanged.AddDynamic((WidgetPtr), &WidgetClass::HandleGoldOrDebtChanged); \
		(RunStatePtr)->DebtChanged.AddDynamic((WidgetPtr), &WidgetClass::HandleGoldOrDebtChanged); \
		(RunStatePtr)->GamblerAngerChanged.AddDynamic((WidgetPtr), &WidgetClass::HandleAngerChanged); \
		(RunStatePtr)->BossChanged.AddDynamic((WidgetPtr), &WidgetClass::HandleBossChanged); \
	} while (false)

namespace T66CasinoOverlayShared
{
	template<typename... TArrays>
	void ResizeArrays(const int32 NewSize, TArrays&... Arrays)
	{
		(Arrays.SetNum(NewSize), ...);
	}

	inline void EnsureBrush(TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}

		Brush->ImageSize = ImageSize;
	}

	inline void EnsureImageBrush(TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		EnsureBrush(Brush, ImageSize);
		Brush->DrawAs = ESlateBrushDrawType::Image;
	}

	inline void EnsureBrushArray(TArray<TSharedPtr<FSlateBrush>>& Brushes, const int32 NumBrushes, const FVector2D& ImageSize)
	{
		Brushes.SetNum(NumBrushes);
		for (int32 Index = 0; Index < NumBrushes; ++Index)
		{
			EnsureBrush(Brushes[Index], ImageSize);
		}
	}

	inline void EnsureImageBrushArray(TArray<TSharedPtr<FSlateBrush>>& Brushes, const int32 NumBrushes, const FVector2D& ImageSize)
	{
		Brushes.SetNum(NumBrushes);
		for (int32 Index = 0; Index < NumBrushes; ++Index)
		{
			EnsureImageBrush(Brushes[Index], ImageSize);
		}
	}

	template<typename TOwner, typename TConfigureGamblerFn, typename TConfigureVendorFn>
	void EnsureShellTabWidgets(
		TOwner* Owner,
		TObjectPtr<UT66GamblerOverlayWidget>& GamblerTabWidget,
		TObjectPtr<UT66VendorOverlayWidget>& VendorTabWidget,
		TConfigureGamblerFn&& ConfigureGambler,
		TConfigureVendorFn&& ConfigureVendor)
	{
		if (!Owner)
		{
			return;
		}

		if (!GamblerTabWidget)
		{
			GamblerTabWidget = CreateWidget<UT66GamblerOverlayWidget>(Owner->GetOwningPlayer(), UT66GamblerOverlayWidget::StaticClass());
			if (GamblerTabWidget)
			{
				ConfigureGambler(GamblerTabWidget);
			}
		}

		if (!VendorTabWidget)
		{
			VendorTabWidget = CreateWidget<UT66VendorOverlayWidget>(Owner->GetOwningPlayer(), UT66VendorOverlayWidget::StaticClass());
			if (VendorTabWidget)
			{
				ConfigureVendor(VendorTabWidget);
			}
		}
	}

	template<typename TWidgetType>
	void RemoveFromParentAndReset(TObjectPtr<TWidgetType>& Widget)
	{
		if (!Widget)
		{
			return;
		}

		Widget->RemoveFromParent();
		Widget = nullptr;
	}

	inline void CloseOverlay(UUserWidget* Widget)
	{
		if (!Widget)
		{
			return;
		}

		Widget->RemoveFromParent();
		if (AT66PlayerController* PC = Cast<AT66PlayerController>(Widget->GetOwningPlayer()))
		{
			PC->RestoreGameplayInputMode();
		}
	}

	template<typename TActivateTabFn>
	void OpenGamblingTab(UT66GamblerOverlayWidget* GamblerTabWidget, TActivateTabFn&& ActivateTab)
	{
		if (GamblerTabWidget)
		{
			GamblerTabWidget->OpenCasinoPage();
		}

		ActivateTab();
	}

	template<typename TActivateTabFn>
	void OpenVendorTab(UT66VendorOverlayWidget* VendorTabWidget, TActivateTabFn&& ActivateTab)
	{
		if (VendorTabWidget)
		{
			VendorTabWidget->OpenShopPage();
		}

		ActivateTab();
	}

	template<typename TActivateTabFn, typename TRefreshFn>
	void OpenAlchemyTab(TActivateTabFn&& ActivateTab, TRefreshFn&& RefreshAlchemy)
	{
		ActivateTab();
		RefreshAlchemy();
	}

	inline void ApplyAlchemyStatus(const TSharedPtr<STextBlock>& StatusText, const FText& Message, const FLinearColor& Color)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Message);
			StatusText->SetColorAndOpacity(Color);
		}
	}

	template<typename TRefreshTopBarFn, typename TRefreshInventoryFn, typename TRefreshDropTargetsFn>
	void RefreshAlchemy(
		TRefreshTopBarFn&& RefreshTopBar,
		TRefreshInventoryFn&& RefreshInventory,
		TRefreshDropTargetsFn&& RefreshDropTargets,
		const TSharedPtr<STextBlock>& StatusText,
		const FText& StatusMessage,
		const FLinearColor& StatusColor)
	{
		RefreshTopBar();
		RefreshInventory();
		RefreshDropTargets();
		ApplyAlchemyStatus(StatusText, StatusMessage, StatusColor);
	}

	inline void RefreshAlchemyTopBar(
		UT66RunStateSubsystem* RunState,
		UT66LocalizationSubsystem* Loc,
		const TSharedPtr<STextBlock>& NetWorthText,
		const TSharedPtr<STextBlock>& GoldText,
		const TSharedPtr<STextBlock>& DebtText,
		const TSharedPtr<STextBlock>& AngerText)
	{
		if (!RunState)
		{
			return;
		}

		if (NetWorthText.IsValid())
		{
			const int32 NetWorth = RunState->GetNetWorth();
			const FText Fmt = Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}");
			NetWorthText->SetText(FText::Format(Fmt, FText::AsNumber(NetWorth)));
			const FLinearColor NetWorthColor = NetWorth > 0
				? FT66Style::Tokens::Success
				: (NetWorth < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text);
			NetWorthText->SetColorAndOpacity(NetWorthColor);
		}

		if (GoldText.IsValid())
		{
			const FText Fmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
			GoldText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentGold())));
		}
		if (DebtText.IsValid())
		{
			const FText Fmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");
			DebtText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentDebt())));
		}
		if (AngerText.IsValid())
		{
			const int32 Pct = FMath::RoundToInt(RunState->GetCasinoAnger01() * 100.f);
			AngerText->SetText(FText::Format(NSLOCTEXT("T66.Casino", "AngerFormat", "ANGER: {0}%"), FText::AsNumber(Pct)));
		}
	}

	inline FString MakeAlchemyStackKey(const FT66InventorySlot& Slot)
	{
		return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
	}

	class FT66OverlayAlchemyDragDropOp : public FDragDropOperation
	{
	public:
		DRAG_DROP_OPERATOR_TYPE(FT66OverlayAlchemyDragDropOp, FDragDropOperation)

		int32 InventoryIndex = INDEX_NONE;
		FText Label;
		TSharedPtr<FSlateBrush> IconBrush;

		static TSharedRef<FT66OverlayAlchemyDragDropOp> New(
			const int32 InInventoryIndex,
			const FText& InLabel,
			const TSharedPtr<FSlateBrush>& InIconBrush)
		{
			TSharedRef<FT66OverlayAlchemyDragDropOp> Op = MakeShared<FT66OverlayAlchemyDragDropOp>();
			Op->InventoryIndex = InInventoryIndex;
			Op->Label = InLabel;
			Op->IconBrush = InIconBrush.IsValid() ? InIconBrush : MakeShared<FSlateBrush>();
			Op->Construct();
			return Op;
		}

		virtual void Construct() override
		{
			MouseCursor = EMouseCursor::GrabHandClosed;
			Decorator =
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 0.97f))
				.Padding(FMargin(12.f, 10.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(72.f)
						.HeightOverride(72.f)
						[
							SNew(SImage)
							.Image(IconBrush.Get())
							.ColorAndOpacity(FLinearColor::White)
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Label)
						.ColorAndOpacity(FLinearColor::White)
						.Font(FT66Style::Tokens::FontBold(16))
					]
				];

			FDragDropOperation::Construct();
		}

		virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
		{
			return Decorator;
		}

	private:
		TSharedPtr<SWidget> Decorator;
	};

	inline TSharedPtr<FSlateBrush> MakeAlchemyDragIconBrush(
		const TArray<TSharedPtr<FSlateBrush>>& InventorySlotBrushes,
		const int32 InventoryIndex)
	{
		TSharedPtr<FSlateBrush> DragIconBrush = MakeShared<FSlateBrush>();
		DragIconBrush->DrawAs = ESlateBrushDrawType::Image;
		DragIconBrush->ImageSize = FVector2D(72.f, 72.f);
		if (InventorySlotBrushes.IsValidIndex(InventoryIndex) && InventorySlotBrushes[InventoryIndex].IsValid())
		{
			*DragIconBrush = *InventorySlotBrushes[InventoryIndex];
			DragIconBrush->DrawAs = ESlateBrushDrawType::Image;
			DragIconBrush->ImageSize = FVector2D(72.f, 72.f);
		}

		return DragIconBrush;
	}

	template<typename TMakeLabelFn>
	FReply BeginAlchemyInventoryDrag(
		UT66RunStateSubsystem* RunState,
		const int32 InventoryIndex,
		const TArray<TSharedPtr<FSlateBrush>>& InventorySlotBrushes,
		TMakeLabelFn&& MakeLabel)
	{
		if (!RunState)
		{
			return FReply::Unhandled();
		}

		const TArray<FName> Inventory = RunState->GetInventory();
		if (!Inventory.IsValidIndex(InventoryIndex) || Inventory[InventoryIndex].IsNone())
		{
			return FReply::Unhandled();
		}

		const TArray<FT66InventorySlot>& InventorySlots = RunState->GetInventorySlots();
		return FReply::Handled().BeginDragDrop(FT66OverlayAlchemyDragDropOp::New(
			InventoryIndex,
			MakeLabel(Inventory, InventorySlots),
			MakeAlchemyDragIconBrush(InventorySlotBrushes, InventoryIndex)));
	}

	template<typename TTryAssignSlotFn>
	FReply HandleAlchemyDropTarget(const FDragDropEvent& DragDropEvent, TTryAssignSlotFn&& TryAssignSlot)
	{
		const TSharedPtr<FT66OverlayAlchemyDragDropOp> DragOp = DragDropEvent.GetOperationAs<FT66OverlayAlchemyDragDropOp>();
		if (!DragOp.IsValid())
		{
			return FReply::Unhandled();
		}

		return TryAssignSlot(DragOp->InventoryIndex) ? FReply::Handled() : FReply::Unhandled();
	}

	inline UT66RunStateSubsystem* GetRunState(const UUserWidget* Widget)
	{
		UGameInstance* GI = Widget ? Widget->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	}

	inline UT66LocalizationSubsystem* GetLocalization(const UUserWidget* Widget)
	{
		UGameInstance* GI = Widget ? Widget->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	}
}
