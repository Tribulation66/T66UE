// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66UIManager.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ReleaseVariantSubsystem.h"
#include "UI/T66FrontendUIRootWidget.h"
#include "UI/T66FrontendTopBarWidget.h"
#include "UI/T66ScreenBase.h"
#include "UI/Style/T66Style.h"
#include "Core/T66BuffSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66UIManager, Log, All);

namespace
{
	FString T66ScreenTypeToDebugName(const ET66ScreenType ScreenType)
	{
		if (const UEnum* ScreenEnum = StaticEnum<ET66ScreenType>())
		{
			return ScreenEnum->GetNameStringByValue(static_cast<int64>(ScreenType));
		}

		return FString::Printf(TEXT("Screen_%d"), static_cast<int32>(ScreenType));
	}

	void ApplyDirectModalInputMode(APlayerController* PlayerController, UUserWidget* FocusWidget)
	{
		if (!PlayerController || !FocusWidget)
		{
			return;
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;
	}
}

UT66UIManager::UT66UIManager()
{
	CurrentScreenType = ET66ScreenType::None;
	CurrentScreen = nullptr;
	CurrentModal = nullptr;
	FrontendTopBar = nullptr;
	FrontendRoot = nullptr;
}

void UT66UIManager::Initialize(APlayerController* InOwningPlayer)
{
	TearDownFrontendRoot();
	OwningPlayer = InOwningPlayer;
	NavigationHistory.Empty();
	CurrentScreenType = ET66ScreenType::None;
	if (FrontendTopBar && FrontendTopBar->IsInViewport())
	{
		FrontendTopBar->RemoveFromParent();
	}
	FrontendTopBar = nullptr;
	InitializeFrontendRootIfNeeded();
}

void UT66UIManager::RegisterScreenClass(ET66ScreenType ScreenType, TSubclassOf<UT66ScreenBase> WidgetClass)
{
	if (WidgetClass)
	{
		ScreenClasses.Add(ScreenType, WidgetClass);
	}
}

UT66ScreenBase* UT66UIManager::CreateScreen(ET66ScreenType ScreenType)
{
	// Check cache first
	if (TObjectPtr<UT66ScreenBase>* CachedScreen = ScreenCache.Find(ScreenType))
	{
		return CachedScreen->Get();
	}

	// Look up the class for this screen type
	TSubclassOf<UT66ScreenBase>* WidgetClassPtr = ScreenClasses.Find(ScreenType);
	if (!WidgetClassPtr || !*WidgetClassPtr)
	{
		UE_LOG(LogT66UIManager, Warning, TEXT("No widget class registered for screen type %d"), static_cast<int32>(ScreenType));
		return nullptr;
	}

	if (!OwningPlayer)
	{
		UE_LOG(LogT66UIManager, Error, TEXT("Cannot create screen - no owning player set"));
		return nullptr;
	}

	// Create the widget
	UT66ScreenBase* NewScreen = CreateWidget<UT66ScreenBase>(OwningPlayer, *WidgetClassPtr);
	if (NewScreen)
	{
		NewScreen->UIManager = this;
		NewScreen->ScreenType = ScreenType;
		ScreenCache.Add(ScreenType, NewScreen);
	}

	return NewScreen;
}

bool UT66UIManager::SwitchToScreen(ET66ScreenType ScreenType, const bool bAddCurrentToHistory)
{
	if (ScreenType == ET66ScreenType::None)
	{
		HideAllUI();
		return true;
	}

	if (!CanShowScreenForReleaseVariant(ScreenType))
	{
		UE_LOG(LogT66UIManager, Log, TEXT("Blocked screen %s for current release variant"), *T66ScreenTypeToDebugName(ScreenType));
		if (!CurrentScreen && ScreenType != ET66ScreenType::MainMenu)
		{
			return SwitchToScreen(ET66ScreenType::MainMenu, false);
		}
		return false;
	}

	// Close any active modal first so switching screens or refreshing the current
	// screen never duplicates history entries for the same destination.
	if (CurrentModal)
	{
		CloseModal();
	}

	if (CurrentScreen && CurrentScreenType == ScreenType)
	{
		CurrentScreen->RefreshScreen();
		UpdateFrontendTopBar();
		return true;
	}

	ET66ScreenType OldScreenType = CurrentScreenType;
	UT66ScreenBase* NewScreen = CreateScreen(ScreenType);
	if (!NewScreen)
	{
		UE_LOG(LogT66UIManager, Warning, TEXT("Failed to create screen type %d; keeping current screen type %d"),
			static_cast<int32>(ScreenType),
			static_cast<int32>(CurrentScreenType));
		return false;
	}

	// Deactivate current screen
	if (CurrentScreen)
	{
		CurrentScreen->OnScreenDeactivated();
		if (IsFrontendRootActive())
		{
			FrontendRoot->ClearMainScreen();
		}
		else
		{
			CurrentScreen->RemoveFromParent();
		}
	}

	// Add current screen to history before switching (if valid)
	if (bAddCurrentToHistory && CurrentScreenType != ET66ScreenType::None)
	{
		NavigationHistory.Add(CurrentScreenType);

		// Trim history if too long
		while (NavigationHistory.Num() > MaxHistoryDepth)
		{
			NavigationHistory.RemoveAt(0);
		}
	}

	CurrentScreen = NewScreen;
	CurrentScreenType = ScreenType;
	CurrentScreen->bIsModal = false;
	// Force Hero Selection to rebuild so party-ready vs solo layout is correct on first paint. Otherwise cached tree from a previous show can display.
	if (ScreenType == ET66ScreenType::HeroSelection)
	{
		if (IsFrontendRootActive())
		{
			CurrentScreen->ReleaseSlateResources(true);
		}
		else
		{
			FT66Style::DeferRebuild(CurrentScreen);
		}
	}
	if (IsFrontendRootActive())
	{
		FrontendRoot->SetMainScreen(CurrentScreen);
	}
	else
	{
		CurrentScreen->AddToViewport(0);
	}
	CurrentScreen->OnScreenActivated();
	UpdateFrontendTopBar();

	OnScreenChanged.Broadcast(OldScreenType, ScreenType);
	return true;
}

void UT66UIManager::ShowScreen(ET66ScreenType ScreenType)
{
	UWorld* const World = OwningPlayer ? OwningPlayer->GetWorld() : nullptr;
	const TObjectPtr<UT66ScreenBase>* ExistingScreen = ScreenCache.Find(ScreenType);
	const bool bWarmShow = ExistingScreen && ExistingScreen->Get() && ExistingScreen->Get()->HasBuiltSlateUI();
	const FString PerfLabel = FString::Printf(TEXT("UIManager::ShowScreen[%s][%s]"), *T66ScreenTypeToDebugName(ScreenType), bWarmShow ? TEXT("warm") : TEXT("cold"));
	FLagScopedScope LagScope(World, *PerfLabel);

	if (ScreenType == ET66ScreenType::None)
	{
		HideAllUI();
		return;
	}

	if (!CanShowScreenForReleaseVariant(ScreenType))
	{
		UE_LOG(LogT66UIManager, Log, TEXT("ShowScreen ignored gated screen %s"), *T66ScreenTypeToDebugName(ScreenType));
		if (!CurrentScreen && ScreenType != ET66ScreenType::MainMenu)
		{
			SwitchToScreen(ET66ScreenType::MainMenu, false);
		}
		return;
	}

	if (CurrentScreen && CurrentScreenType == ScreenType && !CurrentModal)
	{
		return;
	}

	SwitchToScreen(ScreenType, true);
}

void UT66UIManager::ShowScreenWithoutHistory(ET66ScreenType ScreenType)
{
	UWorld* const World = OwningPlayer ? OwningPlayer->GetWorld() : nullptr;
	const TObjectPtr<UT66ScreenBase>* ExistingScreen = ScreenCache.Find(ScreenType);
	const bool bWarmShow = ExistingScreen && ExistingScreen->Get() && ExistingScreen->Get()->HasBuiltSlateUI();
	const FString PerfLabel = FString::Printf(TEXT("UIManager::ShowScreenWithoutHistory[%s][%s]"), *T66ScreenTypeToDebugName(ScreenType), bWarmShow ? TEXT("warm") : TEXT("cold"));
	FLagScopedScope LagScope(World, *PerfLabel);

	if (!CanShowScreenForReleaseVariant(ScreenType))
	{
		UE_LOG(LogT66UIManager, Log, TEXT("ShowScreenWithoutHistory ignored gated screen %s"), *T66ScreenTypeToDebugName(ScreenType));
		if (!CurrentScreen && ScreenType != ET66ScreenType::MainMenu)
		{
			SwitchToScreen(ET66ScreenType::MainMenu, false);
		}
		return;
	}

	if (CurrentScreen && CurrentScreenType == ScreenType && !CurrentModal)
	{
		return;
	}

	SwitchToScreen(ScreenType, false);
}

void UT66UIManager::ShowModal(ET66ScreenType ModalType)
{
	UWorld* const World = OwningPlayer ? OwningPlayer->GetWorld() : nullptr;
	const TObjectPtr<UT66ScreenBase>* ExistingModal = ScreenCache.Find(ModalType);
	const bool bWarmShow = ExistingModal && ExistingModal->Get() && ExistingModal->Get()->HasBuiltSlateUI();
	const FString PerfLabel = FString::Printf(TEXT("UIManager::ShowModal[%s][%s]"), *T66ScreenTypeToDebugName(ModalType), bWarmShow ? TEXT("warm") : TEXT("cold"));
	FLagScopedScope LagScope(World, *PerfLabel);

	if (ModalType == ET66ScreenType::Challenges || ModalType == ET66ScreenType::DailyDescent)
	{
		ShowScreen(ModalType);
		return;
	}

	if (CurrentModal && CurrentModal->ScreenType == ModalType)
	{
		return;
	}

	// Close any existing modal
	if (CurrentModal)
	{
		CloseModal();
	}

	UT66ScreenBase* NewModal = CreateScreen(ModalType);
	if (NewModal)
	{
		NewModal->bIsModal = true;
		CurrentModal = NewModal;
		if (IsFrontendRootActive())
		{
			FrontendRoot->SetModal(CurrentModal);
		}
		else
		{
			CurrentModal->AddToViewport(100); // Higher Z-order than main screen
		}
		CurrentModal->OnScreenActivated();
		if (!IsFrontendRootActive())
		{
			ApplyDirectModalInputMode(OwningPlayer, CurrentModal);
		}
		UpdateFrontendTopBar();
	}
}

ET66ScreenType UT66UIManager::GetCurrentModalType() const
{
	return CurrentModal ? CurrentModal->ScreenType : ET66ScreenType::None;
}

void UT66UIManager::CloseModal()
{
	const FString PerfLabel = FString::Printf(TEXT("UIManager::CloseModal[%s]"), CurrentModal ? *T66ScreenTypeToDebugName(CurrentModal->ScreenType) : TEXT("None"));
	FLagScopedScope LagScope(OwningPlayer ? OwningPlayer->GetWorld() : nullptr, *PerfLabel);

	if (CurrentModal)
	{
		UT66ScreenBase* const ClosingModal = CurrentModal;
		CurrentModal->OnScreenDeactivated();
		if (IsFrontendRootActive())
		{
			FrontendRoot->ClearModal();
		}
		else
		{
			CurrentModal->RemoveFromParent();
		}
		CurrentModal = nullptr;

		// Refresh the underlying screen in case state changed while modal was open
		if (CurrentScreen && ClosingModal && ClosingModal->ShouldRefreshUnderlyingScreenOnModalClose())
		{
			CurrentScreen->RefreshScreen();
		}

		UpdateFrontendTopBar();
	}
}

void UT66UIManager::GoBack()
{
	FLagScopedScope LagScope(OwningPlayer ? OwningPlayer->GetWorld() : nullptr, TEXT("UIManager::GoBack"));

	// If modal is open, close it first
	if (CurrentModal)
	{
		CloseModal();
		return;
	}

	// Navigate to previous screen in history
	if (NavigationHistory.Num() > 0)
	{
		ET66ScreenType PreviousScreen = NavigationHistory.Pop();
		SwitchToScreen(PreviousScreen, false);
	}
}

bool UT66UIManager::HandleBackAction()
{
	if (CurrentModal)
	{
		return CurrentModal->HandleBackAction();
	}

	return CurrentScreen ? CurrentScreen->HandleBackAction() : false;
}

void UT66UIManager::RebuildAllVisibleUI()
{
	// Always defer rebuilds so theme switches cannot tear down Slate trees mid-input event.
	if (IsFrontendRootActive())
	{
		QueueFrontendRootLayerRefresh(CurrentScreen);
		QueueFrontendRootLayerRefresh(CurrentModal);
		QueueFrontendRootLayerRefresh(FrontendTopBar);
		return;
	}

	if (CurrentScreen && CurrentScreen->IsInViewport())
	{
		FT66Style::DeferRebuild(CurrentScreen, 0);
	}

	if (CurrentModal && CurrentModal->IsInViewport())
	{
		FT66Style::DeferRebuild(CurrentModal, 100);
	}

	if (FrontendTopBar && FrontendTopBar->IsInViewport())
	{
		FT66Style::DeferRebuild(FrontendTopBar, 50);
	}
}

bool UT66UIManager::RequestFrontendRootLayerRefresh(UUserWidget* Widget)
{
	if (!Widget || !IsFrontendRootActive())
	{
		return false;
	}

	QueueFrontendRootLayerRefresh(Widget);
	return true;
}

bool UT66UIManager::RequestFrontendRootPaintRefresh()
{
	if (!IsFrontendRootActive())
	{
		return false;
	}

	FrontendRoot->RequestFrontendPaintRefresh();
	return true;
}

void UT66UIManager::RefreshDirectModalInputMode(UUserWidget* FocusWidget)
{
	if (IsFrontendRootActive())
	{
		return;
	}

	ApplyDirectModalInputMode(OwningPlayer, FocusWidget);
}

void UT66UIManager::HideAllUI()
{
	// Close modal if active
	if (CurrentModal)
	{
		CurrentModal->OnScreenDeactivated();
		if (IsFrontendRootActive())
		{
			FrontendRoot->ClearModal();
		}
		else
		{
			CurrentModal->RemoveFromParent();
		}
		CurrentModal = nullptr;
	}

	// Hide main screen
	if (CurrentScreen)
	{
		CurrentScreen->OnScreenDeactivated();
		if (IsFrontendRootActive())
		{
			FrontendRoot->ClearMainScreen();
		}
		else
		{
			CurrentScreen->RemoveFromParent();
		}
		CurrentScreen = nullptr;
	}

	ET66ScreenType OldScreenType = CurrentScreenType;
	CurrentScreenType = ET66ScreenType::None;
	UpdateFrontendTopBar();
	TearDownFrontendRoot();

	OnScreenChanged.Broadcast(OldScreenType, ET66ScreenType::None);
}

bool UT66UIManager::IsFrontendTopBarVisible() const
{
	if (IsFrontendRootActive())
	{
		return FrontendRoot->IsTopBarVisible();
	}

	return FrontendTopBar
		&& FrontendTopBar->IsInViewport()
		&& FrontendTopBar->GetCachedWidget().IsValid();
}

UT66ScreenBase* UT66UIManager::GetFrontendTopBarScreen() const
{
	return FrontendTopBar;
}

float UT66UIManager::GetFrontendTopBarReservedHeight() const
{
	return IsFrontendTopBarVisible() ? UT66FrontendTopBarWidget::GetReservedHeight() : 0.f;
}

float UT66UIManager::GetFrontendTopBarContentHeight() const
{
	return UT66FrontendTopBarWidget::GetVisibleContentHeight();
}

bool UT66UIManager::ShouldShowFrontendTopBar(ET66ScreenType ScreenType) const
{
	if (ScreenType == ET66ScreenType::PowerUp)
	{
		if (const UWorld* World = OwningPlayer ? OwningPlayer->GetWorld() : nullptr)
		{
			if (const UGameInstance* GI = World->GetGameInstance())
			{
				if (const UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
				{
					if (Buffs->IsHeroSelectionSingleUseBuffEditActive())
					{
						return false;
					}
				}
			}
		}
	}

	switch (ScreenType)
	{
	case ET66ScreenType::MainMenu:
	case ET66ScreenType::Settings:
	case ET66ScreenType::LanguageSelect:
	case ET66ScreenType::AccountStatus:
	case ET66ScreenType::PowerUp:
	case ET66ScreenType::Achievements:
	case ET66ScreenType::DailyDescent:
		return true;
	default:
		return false;
	}
}

void UT66UIManager::UpdateFrontendTopBar()
{
	const bool bShouldShow = ShouldShowFrontendTopBar(CurrentScreenType);
	if (!bShouldShow)
	{
		if (IsFrontendRootActive())
		{
			FrontendRoot->ClearTopBar();
		}
		else if (FrontendTopBar && FrontendTopBar->IsInViewport())
		{
			FrontendTopBar->RemoveFromParent();
		}
		return;
	}

	if (!OwningPlayer)
	{
		return;
	}

	// Recover from a stale widget shell that still exists on the UObject side
	// but no longer has a live Slate tree to draw.
	if (FrontendTopBar && !FrontendTopBar->GetCachedWidget().IsValid())
	{
		if (FrontendTopBar->IsInViewport())
		{
			FrontendTopBar->RemoveFromParent();
		}
		FrontendTopBar = nullptr;
	}

	if (!FrontendTopBar)
	{
		FrontendTopBar = CreateWidget<UT66FrontendTopBarWidget>(OwningPlayer, UT66FrontendTopBarWidget::StaticClass());
		if (!FrontendTopBar)
		{
			return;
		}

		FrontendTopBar->UIManager = this;
	}

	FrontendTopBar->UIManager = this;
	if (IsFrontendRootActive())
	{
		FrontendRoot->SetTopBar(FrontendTopBar);
	}
	else if (!FrontendTopBar->IsInViewport())
	{
		FrontendTopBar->AddToViewport(50);
	}

	FrontendTopBar->SetActiveSection(UT66FrontendTopBarWidget::ResolveFrontendSectionForScreen(CurrentScreenType));
	FrontendTopBar->RefreshScreen();
}

bool UT66UIManager::ShouldUseFrontendRoot() const
{
	const AT66PlayerController* T66Player = Cast<AT66PlayerController>(OwningPlayer);
	return T66Player && T66Player->IsFrontendLevel();
}

bool UT66UIManager::IsFrontendRootActive() const
{
	return FrontendRoot != nullptr && FrontendRoot->IsInViewport();
}

void UT66UIManager::InitializeFrontendRootIfNeeded()
{
	if (!ShouldUseFrontendRoot() || !OwningPlayer)
	{
		return;
	}

	if (!FrontendRoot)
	{
		FrontendRoot = CreateWidget<UT66FrontendUIRootWidget>(OwningPlayer, UT66FrontendUIRootWidget::StaticClass());
		if (!FrontendRoot)
		{
			UE_LOG(LogT66UIManager, Warning, TEXT("Failed to create frontend UI root; falling back to direct viewport UI."));
			return;
		}
	}

	if (!FrontendRoot->IsInViewport())
	{
		FrontendRoot->AddToViewport(0);
	}
}

void UT66UIManager::TearDownFrontendRoot()
{
	if (FrontendRoot)
	{
		FrontendRoot->ClearPopup();
		FrontendRoot->ClearLoading();
		FrontendRoot->ClearModal();
		FrontendRoot->ClearTopBar();
		FrontendRoot->ClearMainScreen();
		if (FrontendRoot->IsInViewport())
		{
			FrontendRoot->RemoveFromParent();
		}
		FrontendRoot = nullptr;
	}
}

void UT66UIManager::QueueFrontendRootLayerRefresh(UUserWidget* Widget)
{
	if (!Widget || !IsFrontendRootActive() || !OwningPlayer)
	{
		return;
	}

	UWorld* World = OwningPlayer->GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return;
	}

	TWeakObjectPtr<UT66FrontendUIRootWidget> WeakRoot(FrontendRoot);
	TWeakObjectPtr<UUserWidget> WeakWidget(Widget);
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakRoot, WeakWidget]()
	{
		UT66FrontendUIRootWidget* Root = WeakRoot.Get();
		UUserWidget* SafeWidget = WeakWidget.Get();
		if (!Root || !SafeWidget)
		{
			return;
		}

		Root->RefreshLayerWidget(SafeWidget);
	}));
}
