// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendUIRootWidget.h"

#include "Engine/World.h"
#include "Slate/SRetainerWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

void UT66FrontendUIRootWidget::SetMainScreen(UUserWidget* Widget)
{
	SetLayerWidget(MainScreenWidget, MainScreenBox, Widget);
}

void UT66FrontendUIRootWidget::ClearMainScreen()
{
	SetLayerWidget(MainScreenWidget, MainScreenBox, nullptr);
}

void UT66FrontendUIRootWidget::SetTopBar(UUserWidget* Widget)
{
	SetLayerWidget(TopBarWidget, TopBarBox, Widget);
}

void UT66FrontendUIRootWidget::ClearTopBar()
{
	SetLayerWidget(TopBarWidget, TopBarBox, nullptr);
}

void UT66FrontendUIRootWidget::SetModal(UUserWidget* Widget)
{
	SetLayerWidget(ModalWidget, ModalBox, Widget);
}

void UT66FrontendUIRootWidget::ClearModal()
{
	SetLayerWidget(ModalWidget, ModalBox, nullptr);
}

void UT66FrontendUIRootWidget::SetLoading(UUserWidget* Widget)
{
	SetLayerWidget(LoadingWidget, LoadingBox, Widget);
}

void UT66FrontendUIRootWidget::ClearLoading()
{
	SetLayerWidget(LoadingWidget, LoadingBox, nullptr);
}

void UT66FrontendUIRootWidget::SetPopup(UUserWidget* Widget)
{
	SetLayerWidget(PopupWidget, PopupBox, Widget);
}

void UT66FrontendUIRootWidget::ClearPopup()
{
	SetLayerWidget(PopupWidget, PopupBox, nullptr);
}

bool UT66FrontendUIRootWidget::IsTopBarVisible() const
{
	return TopBarWidget != nullptr && TopBarWidget->GetCachedWidget().IsValid();
}

bool UT66FrontendUIRootWidget::IsPopupVisible() const
{
	return PopupWidget != nullptr && PopupWidget->GetCachedWidget().IsValid();
}

void UT66FrontendUIRootWidget::RefreshLayerWidget(UUserWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	TSharedPtr<SBox> TargetLayerBox;
	if (Widget == MainScreenWidget)
	{
		TargetLayerBox = MainScreenBox;
	}
	else if (Widget == TopBarWidget)
	{
		TargetLayerBox = TopBarBox;
	}
	else if (Widget == ModalWidget)
	{
		TargetLayerBox = ModalBox;
	}
	else if (Widget == LoadingWidget)
	{
		TargetLayerBox = LoadingBox;
	}
	else if (Widget == PopupWidget)
	{
		TargetLayerBox = PopupBox;
	}

	if (!TargetLayerBox.IsValid())
	{
		return;
	}

	// Detach the stale Slate subtree before rebuilding the widget. Without this
	// explicit detach/reattach, state changes can be committed on the UWidget
	// while the frontend retainer continues presenting the previous Slate tree.
	TargetLayerBox->SetContent(SNullWidget::NullWidget);
	TargetLayerBox->Invalidate(EInvalidateWidgetReason::Layout);

	Widget->ReleaseSlateResources(true);
	Widget->InvalidateLayoutAndVolatility();

	if (Widget == MainScreenWidget)
	{
		ReapplyLayerWidget(MainScreenWidget, MainScreenBox);
	}
	else if (Widget == TopBarWidget)
	{
		ReapplyLayerWidget(TopBarWidget, TopBarBox);
	}
	else if (Widget == ModalWidget)
	{
		ReapplyLayerWidget(ModalWidget, ModalBox);
	}
	else if (Widget == LoadingWidget)
	{
		ReapplyLayerWidget(LoadingWidget, LoadingBox);
	}
	else if (Widget == PopupWidget)
	{
		ReapplyLayerWidget(PopupWidget, PopupBox);
	}

	TargetLayerBox->Invalidate(EInvalidateWidgetReason::Layout);
	if (RetainerWidget.IsValid())
	{
		RetainerWidget->Invalidate(EInvalidateWidgetReason::Paint);
		RetainerWidget->RequestRender();
	}
}

void UT66FrontendUIRootWidget::RequestFrontendPaintRefresh()
{
	if (MainScreenBox.IsValid())
	{
		MainScreenBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (TopBarBox.IsValid())
	{
		TopBarBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (ModalBox.IsValid())
	{
		ModalBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (LoadingBox.IsValid())
	{
		LoadingBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (PopupBox.IsValid())
	{
		PopupBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (RetainerWidget.IsValid())
	{
		RetainerWidget->Invalidate(EInvalidateWidgetReason::Paint);
		RetainerWidget->RequestRender();
	}
}

TSharedRef<SWidget> UT66FrontendUIRootWidget::RebuildWidget()
{
	TSharedRef<SWidget> LayerStack =
		SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(MainScreenBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(TopBarBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(ModalBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(LoadingBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(PopupBox, SBox)
			.Visibility(EVisibility::Collapsed)
		];

	SAssignNew(RetainerWidget, SRetainerWidget)
		.Visibility(EVisibility::SelfHitTestInvisible)
		.RenderOnPhase(true)
		.RenderOnInvalidation(true)
		.Phase(0)
		.PhaseCount(1)
		.StatId(FName(TEXT("T66FrontendUIRoot")))
		[
			LayerStack
		];

	if (UWorld* World = GetWorld())
	{
		RetainerWidget->SetWorld(World);
	}

	ReapplyAllLayerWidgets();
	return RetainerWidget.ToSharedRef();
}

void UT66FrontendUIRootWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	RetainerWidget.Reset();
	MainScreenBox.Reset();
	TopBarBox.Reset();
	ModalBox.Reset();
	LoadingBox.Reset();
	PopupBox.Reset();
}

void UT66FrontendUIRootWidget::SetLayerWidget(TObjectPtr<UUserWidget>& StoredWidget, const TSharedPtr<SBox>& LayerBox, UUserWidget* Widget)
{
	StoredWidget = Widget;
	ReapplyLayerWidget(StoredWidget, LayerBox);
	if (RetainerWidget.IsValid())
	{
		RetainerWidget->RequestRender();
	}
}

void UT66FrontendUIRootWidget::ReapplyLayerWidget(const TObjectPtr<UUserWidget>& StoredWidget, const TSharedPtr<SBox>& LayerBox)
{
	if (!LayerBox.IsValid())
	{
		return;
	}

	if (StoredWidget)
	{
		LayerBox->SetContent(StoredWidget->TakeWidget());
		LayerBox->SetVisibility(EVisibility::SelfHitTestInvisible);
	}
	else
	{
		LayerBox->SetContent(SNullWidget::NullWidget);
		LayerBox->SetVisibility(EVisibility::Collapsed);
	}
}

void UT66FrontendUIRootWidget::ReapplyAllLayerWidgets()
{
	ReapplyLayerWidget(MainScreenWidget, MainScreenBox);
	ReapplyLayerWidget(TopBarWidget, TopBarBox);
	ReapplyLayerWidget(ModalWidget, ModalBox);
	ReapplyLayerWidget(LoadingWidget, LoadingBox);
	ReapplyLayerWidget(PopupWidget, PopupBox);
}
