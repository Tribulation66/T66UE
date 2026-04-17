// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66UIManager.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "UI/T66FrontendBackButtonWidget.h"
#include "UI/T66FrontendTopBarWidget.h"
#include "UI/T66ScreenBase.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66PlayerController.h"
#include "Blueprint/UserWidget.h"

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
}

UT66UIManager::UT66UIManager()
{
	CurrentScreenType = ET66ScreenType::None;
	CurrentScreen = nullptr;
	CurrentModal = nullptr;
	FrontendTopBar = nullptr;
	FrontendBackButton = nullptr;
}

void UT66UIManager::Initialize(APlayerController* InOwningPlayer)
{
	OwningPlayer = InOwningPlayer;
	NavigationHistory.Empty();
	CurrentScreenType = ET66ScreenType::None;
	if (FrontendTopBar && FrontendTopBar->IsInViewport())
	{
		FrontendTopBar->RemoveFromParent();
	}
	FrontendTopBar = nullptr;
	if (FrontendBackButton && FrontendBackButton->IsInViewport())
	{
		FrontendBackButton->RemoveFromParent();
	}
	FrontendBackButton = nullptr;
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
		UpdateFrontendBackButton();
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
		CurrentScreen->RemoveFromParent();
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
	// Force Hero Selection to rebuild so co-op (Lab + Back to Lobby only) vs solo (difficulty + Enter) layout is correct on first paint. Otherwise cached tree from a previous show can display.
	if (ScreenType == ET66ScreenType::HeroSelection)
	{
		FT66Style::DeferRebuild(CurrentScreen);
	}
	CurrentScreen->AddToViewport(0);
	CurrentScreen->OnScreenActivated();
	UpdateFrontendTopBar();
	UpdateFrontendBackButton();

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
		CurrentModal->AddToViewport(100); // Higher Z-order than main screen
		CurrentModal->OnScreenActivated();
		UpdateFrontendTopBar();
		UpdateFrontendBackButton();
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
		CurrentModal->RemoveFromParent();
		CurrentModal = nullptr;

		// Refresh the underlying screen in case state changed while modal was open
		if (CurrentScreen && ClosingModal && ClosingModal->ShouldRefreshUnderlyingScreenOnModalClose())
		{
			CurrentScreen->RefreshScreen();
		}

		UpdateFrontendTopBar();
		UpdateFrontendBackButton();
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

void UT66UIManager::RebuildAllVisibleUI()
{
	// Always defer rebuilds so theme switches cannot tear down Slate trees mid-input event.
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

	if (FrontendBackButton && FrontendBackButton->IsInViewport())
	{
		FT66Style::DeferRebuild(FrontendBackButton, 40);
	}
}

void UT66UIManager::HideAllUI()
{
	// Close modal if active
	if (CurrentModal)
	{
		CurrentModal->OnScreenDeactivated();
		CurrentModal->RemoveFromParent();
		CurrentModal = nullptr;
	}

	// Hide main screen
	if (CurrentScreen)
	{
		CurrentScreen->OnScreenDeactivated();
		CurrentScreen->RemoveFromParent();
		CurrentScreen = nullptr;
	}

	ET66ScreenType OldScreenType = CurrentScreenType;
	CurrentScreenType = ET66ScreenType::None;
	UpdateFrontendTopBar();
	UpdateFrontendBackButton();

	OnScreenChanged.Broadcast(OldScreenType, ET66ScreenType::None);
}

bool UT66UIManager::IsFrontendTopBarVisible() const
{
	return FrontendTopBar && FrontendTopBar->IsInViewport();
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
	switch (ScreenType)
	{
	case ET66ScreenType::MainMenu:
	case ET66ScreenType::SaveSlots:
	case ET66ScreenType::Settings:
	case ET66ScreenType::LanguageSelect:
	case ET66ScreenType::AccountStatus:
	case ET66ScreenType::PowerUp:
	case ET66ScreenType::Achievements:
	case ET66ScreenType::Unlocks:
		return true;
	default:
		return false;
	}
}

bool UT66UIManager::ShouldShowFrontendBackButton() const
{
	return !CurrentModal
		&& CurrentScreenType != ET66ScreenType::None
		&& CurrentScreenType != ET66ScreenType::MainMenu
		&& ShouldShowFrontendTopBar(CurrentScreenType);
}

void UT66UIManager::UpdateFrontendTopBar()
{
	const bool bShouldShow = ShouldShowFrontendTopBar(CurrentScreenType);
	if (!bShouldShow)
	{
		if (FrontendTopBar && FrontendTopBar->IsInViewport())
		{
			FrontendTopBar->RemoveFromParent();
		}
		return;
	}

	if (!OwningPlayer)
	{
		return;
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
	if (!FrontendTopBar->IsInViewport())
	{
		FrontendTopBar->AddToViewport(50);
	}

	FrontendTopBar->RefreshScreen();
}

void UT66UIManager::UpdateFrontendBackButton()
{
	const bool bShouldShow = ShouldShowFrontendBackButton();
	if (!bShouldShow)
	{
		if (FrontendBackButton && FrontendBackButton->IsInViewport())
		{
			FrontendBackButton->RemoveFromParent();
		}
		return;
	}

	if (!OwningPlayer)
	{
		return;
	}

	if (!FrontendBackButton)
	{
		FrontendBackButton = CreateWidget<UT66FrontendBackButtonWidget>(OwningPlayer, UT66FrontendBackButtonWidget::StaticClass());
		if (!FrontendBackButton)
		{
			return;
		}

		FrontendBackButton->UIManager = this;
	}

	FrontendBackButton->UIManager = this;
	if (!FrontendBackButton->IsInViewport())
	{
		FrontendBackButton->AddToViewport(40);
	}

	FrontendBackButton->RefreshScreen();
}
