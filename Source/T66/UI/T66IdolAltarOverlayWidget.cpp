// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66IdolAltarOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Styling/CoreStyle.h"
#include "Input/DragAndDrop.h"

void UT66IdolAltarOverlayWidget::NativeDestruct()
{
	// Safety: this overlay can be removed outside Back/Confirm (level change, pause flow, etc).
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}
	Super::NativeDestruct();
}

class FIdolDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FIdolDragDropOp, FDragDropOperation)

	FName IdolID = NAME_None;

	static TSharedRef<FIdolDragDropOp> New(FName InIdolID)
	{
		TSharedRef<FIdolDragDropOp> Op = MakeShared<FIdolDragDropOp>();
		Op->IdolID = InIdolID;
		Op->Construct();
		return Op;
	}
};

class ST66IdolTile : public SBorder
{
public:
	DECLARE_DELEGATE_OneParam(FOnHoverIdol, FName /*IdolID*/);

	SLATE_BEGIN_ARGS(ST66IdolTile) {}
		SLATE_ARGUMENT(FName, IdolID)
		SLATE_ARGUMENT(bool, bLocked)
		SLATE_ARGUMENT(FText, ToolTipText)
		SLATE_ARGUMENT(FLinearColor, Color)
		SLATE_EVENT(FOnHoverIdol, OnHoverIdol)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		IdolID = InArgs._IdolID;
		bLocked = InArgs._bLocked;
		OnHoverIdol = InArgs._OnHoverIdol;

		SBorder::Construct(
			SBorder::FArguments()
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(InArgs._Color)
			.Padding(2.f)
			.ToolTipText(InArgs._ToolTipText)
		);
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bLocked) return FReply::Handled();
		if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Unhandled();
		return FReply::Handled().DetectDrag(AsShared(), EKeys::LeftMouseButton);
	}

	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bLocked) return FReply::Handled();
		return FReply::Handled().BeginDragDrop(FIdolDragDropOp::New(IdolID));
	}

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		SBorder::OnMouseEnter(MyGeometry, MouseEvent);
		OnHoverIdol.ExecuteIfBound(IdolID);
	}

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		SBorder::OnMouseLeave(MouseEvent);
		OnHoverIdol.ExecuteIfBound(NAME_None);
	}

private:
	FName IdolID = NAME_None;
	bool bLocked = false;
	FOnHoverIdol OnHoverIdol;
};

class ST66IdolDropTarget : public SBorder
{
public:
	DECLARE_DELEGATE_OneParam(FOnIdolDropped, FName /*IdolID*/);

	SLATE_BEGIN_ARGS(ST66IdolDropTarget) {}
		SLATE_ARGUMENT(bool, bLocked)
		SLATE_ARGUMENT(FLinearColor, Color)
		SLATE_ARGUMENT(FText, ToolTipText)
		SLATE_ARGUMENT(FText, CenterText)
		SLATE_EVENT(FOnIdolDropped, OnIdolDropped)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		bLocked = InArgs._bLocked;
		OnIdolDropped = InArgs._OnIdolDropped;

		SBorder::Construct(
			SBorder::FArguments()
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(InArgs._Color)
			.ToolTipText(InArgs._ToolTipText)
			[
				SNew(STextBlock)
				.Text(InArgs._CenterText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.85f, 1.f))
				.Justification(ETextJustify::Center)
			]
		);
	}

	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		if (bLocked) return FReply::Handled();
		TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
		if (Operation.IsValid() && Operation->IsOfType<FIdolDragDropOp>())
		{
			const TSharedPtr<FIdolDragDropOp> IdolOp = StaticCastSharedPtr<FIdolDragDropOp>(Operation);
			OnIdolDropped.ExecuteIfBound(IdolOp->IdolID);
		}
		return FReply::Handled();
	}

private:
	bool bLocked = false;
	FOnIdolDropped OnIdolDropped;
};

static FText GetIdolTooltipText(UT66LocalizationSubsystem* Loc, FName IdolID)
{
	if (Loc)
	{
		return Loc->GetText_IdolTooltip(IdolID);
	}
	return NSLOCTEXT("T66.IdolAltar", "IdolTooltipUnknown", "IDOL\nUnknown.");
}

TSharedRef<SWidget> UT66IdolAltarOverlayWidget::RebuildWidget()
{
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	const TArray<FName>& Equipped = RunState ? RunState->GetEquippedIdols() : TArray<FName>();
	int32 FirstEmptySlot = INDEX_NONE;
	for (int32 i = 0; i < Equipped.Num(); ++i)
	{
		if (Equipped[i].IsNone())
		{
			FirstEmptySlot = i;
			break;
		}
	}
	const bool bLocked = (FirstEmptySlot == INDEX_NONE);

	auto MakeIdolTile = [&](FName IdolID) -> TSharedRef<SWidget>
	{
		const FLinearColor C = UT66RunStateSubsystem::GetIdolColor(IdolID);
		return SNew(ST66IdolTile)
			.IdolID(IdolID)
			.bLocked(bLocked)
			.Color(C)
			.ToolTipText(GetIdolTooltipText(Loc, IdolID))
			.OnHoverIdol(ST66IdolTile::FOnHoverIdol::CreateLambda([this, Loc](FName Hovered)
			{
				if (!HoverTooltipText.IsValid()) return;
				if (Hovered.IsNone())
				{
					HoverTooltipText->SetText(Loc ? Loc->GetText_IdolAltarHoverHint() : NSLOCTEXT("T66.IdolAltar", "HoverHint", "Hover an idol to see its effect."));
				}
				else
				{
					HoverTooltipText->SetText(GetIdolTooltipText(Loc, Hovered));
				}
			}));
	};

	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);

	// Outer ring placement:
	// Row0: 4 idols
	// Row1: left + right idols, middle is part of center pad
	// Row2: left + right idols, middle is part of center pad
	// Row3: 4 idols
	const TArray<FName>& Idols = UT66RunStateSubsystem::GetAllIdolIDs();
	int32 idx = 0;

	auto NextIdol = [&]() -> FName
	{
		const FName Out = (idx >= 0 && idx < Idols.Num()) ? Idols[idx] : NAME_None;
		++idx;
		return Out;
	};

	// Top row
	for (int32 c = 0; c < 4; ++c)
	{
		Grid->AddSlot(c, 0)
		.Padding(6.f)
		[
			SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
			[
				MakeIdolTile(NextIdol())
			]
		];
	}

	// Row1 left/right
	Grid->AddSlot(0, 1).Padding(6.f)
	[
		SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
		[
			MakeIdolTile(NextIdol())
		]
	];
	Grid->AddSlot(3, 1).Padding(6.f)
	[
		SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
		[
			MakeIdolTile(NextIdol())
		]
	];

	// Row2 left/right
	Grid->AddSlot(0, 2).Padding(6.f)
	[
		SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
		[
			MakeIdolTile(NextIdol())
		]
	];
	Grid->AddSlot(3, 2).Padding(6.f)
	[
		SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
		[
			MakeIdolTile(NextIdol())
		]
	];

	// Bottom row
	for (int32 c = 0; c < 4; ++c)
	{
		Grid->AddSlot(c, 3)
		.Padding(6.f)
		[
			SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
			[
				MakeIdolTile(NextIdol())
			]
		];
	}

	// Center pad spans 2x2 at (1,1)
	const FLinearColor CenterC = PendingSelectedIdolID.IsNone()
		? FLinearColor(0.12f, 0.12f, 0.14f, 1.f)
		: UT66RunStateSubsystem::GetIdolColor(PendingSelectedIdolID);

	Grid->AddSlot(1, 1)
		.Padding(6.f)
		.ColumnSpan(2)
		.RowSpan(2)
		[
			SNew(SBox)
			.WidthOverride(64.f * 2.f + 12.f)
			.HeightOverride(64.f * 2.f + 12.f)
			[
				SAssignNew(CenterPadBorder, ST66IdolDropTarget)
				.bLocked(bLocked)
				.Color(CenterC)
				.ToolTipText(PendingSelectedIdolID.IsNone()
					? (Loc ? Loc->GetText_IdolAltarDropAnIdolHere() : NSLOCTEXT("T66.IdolAltar", "DropAnIdolHere", "Drop an Idol here."))
					: GetIdolTooltipText(Loc, PendingSelectedIdolID))
				.CenterText(Loc ? Loc->GetText_IdolAltarDropHere() : NSLOCTEXT("T66.IdolAltar", "DropHere", "DROP\nHERE"))
				.OnIdolDropped(ST66IdolDropTarget::FOnIdolDropped::CreateLambda([this](FName IdolID)
				{
					PendingSelectedIdolID = IdolID;
					RefreshCenterPad();
				}))
			]
		];

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 1.f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.f))
				.Padding(26.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_IdolAltarTitle() : NSLOCTEXT("T66.IdolAltar", "Title", "IDOL ALTAR"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(SBox)
						.WidthOverride(360.f)
						.HeightOverride(28.f)
						[
							SAssignNew(StatusText, STextBlock)
							.Text(bLocked
							? (Loc ? Loc->GetText_IdolAltarAlreadySelectedStage1() : NSLOCTEXT("T66.IdolAltar", "AllSlotsFull", "All idol slots are full."))
								: FText::GetEmpty())
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
							.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
							.Justification(ETextJustify::Center)
							.WrapTextAt(360.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(SBox)
						.WidthOverride(360.f)
						.HeightOverride(72.f) // fixed height prevents overlay resizing when text changes
						[
							SAssignNew(HoverTooltipText, STextBlock)
							.Text(Loc ? Loc->GetText_IdolAltarHoverHint() : NSLOCTEXT("T66.IdolAltar", "HoverHint", "Hover an idol to see its effect."))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
							.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.9f, 1.f))
							.Justification(ETextJustify::Center)
							.WrapTextAt(360.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[
						Grid
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 14.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							SNew(SBox).WidthOverride(180.f).HeightOverride(44.f)
							[
								SNew(SButton)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnConfirm))
								.IsEnabled(!bLocked)
								.ButtonColorAndOpacity(FLinearColor(0.25f, 0.55f, 0.25f, 1.f))
								[
									SNew(STextBlock)
									.Text(Loc ? Loc->GetText_Confirm() : NSLOCTEXT("T66.Common", "Confirm", "CONFIRM"))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							SNew(SBox).WidthOverride(180.f).HeightOverride(44.f)
							[
								SNew(SButton)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnBack))
								.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
								[
									SNew(STextBlock)
									.Text(Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK"))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
					]
				]
			]
		];
}

void UT66IdolAltarOverlayWidget::RefreshCenterPad()
{
	if (!CenterPadBorder.IsValid()) return;
	UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	const FLinearColor C = PendingSelectedIdolID.IsNone()
		? FLinearColor(0.12f, 0.12f, 0.14f, 1.f)
		: UT66RunStateSubsystem::GetIdolColor(PendingSelectedIdolID);

	CenterPadBorder->SetBorderBackgroundColor(C);
	CenterPadBorder->SetToolTipText(PendingSelectedIdolID.IsNone()
		? (Loc ? Loc->GetText_IdolAltarDropAnIdolHere() : NSLOCTEXT("T66.IdolAltar", "DropAnIdolHere", "Drop an Idol here."))
		: GetIdolTooltipText(Loc, PendingSelectedIdolID));
}

FReply UT66IdolAltarOverlayWidget::OnConfirm()
{
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return FReply::Handled();
	}

	const TArray<FName>& Equipped = RunState->GetEquippedIdols();
	int32 FirstEmptySlot = INDEX_NONE;
	for (int32 i = 0; i < Equipped.Num(); ++i)
	{
		if (Equipped[i].IsNone())
		{
			FirstEmptySlot = i;
			break;
		}
	}

	if (FirstEmptySlot == INDEX_NONE)
	{
		if (StatusText.IsValid())
		{
			UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
			StatusText->SetText(Loc ? Loc->GetText_IdolAltarAlreadySelectedStage1() : NSLOCTEXT("T66.IdolAltar", "AllSlotsFull", "All idol slots are full."));
		}
		return FReply::Handled();
	}

	if (PendingSelectedIdolID.IsNone())
	{
		if (StatusText.IsValid())
		{
			UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
			StatusText->SetText(Loc ? Loc->GetText_IdolAltarDragFirst() : NSLOCTEXT("T66.IdolAltar", "DragFirst", "Drag an idol into the center slot first."));
		}
		return FReply::Handled();
	}

	RunState->EquipIdolInSlot(FirstEmptySlot, PendingSelectedIdolID);
	if (StatusText.IsValid())
	{
		UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		StatusText->SetText(Loc ? Loc->GetText_IdolAltarEquipped() : NSLOCTEXT("T66.IdolAltar", "Equipped", "Equipped."));
	}

	// Close immediately to restore gameplay input.
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnBack()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
	return FReply::Handled();
}

