// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66UIManager.h"
#include "UI/T66FrontendTopBarWidget.h"
#include "UI/T66ScreenBase.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66PlayerController.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66UIManager, Log, All);

UT66UIManager::UT66UIManager()
{
	CurrentScreenType = ET66ScreenType::None;
	CurrentScreen = nullptr;
	CurrentModal = nullptr;
	FrontendTopBar = nullptr;
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

void UT66UIManager::ShowScreen(ET66ScreenType ScreenType)
{
	if (ScreenType == ET66ScreenType::None)
	{
		HideAllUI();
		return;
	}

	// Close any active modal first
	if (CurrentModal)
	{
		CloseModal();
	}

	ET66ScreenType OldScreenType = CurrentScreenType;

	// Deactivate current screen
	if (CurrentScreen)
	{
		CurrentScreen->OnScreenDeactivated();
		CurrentScreen->RemoveFromParent();
	}

	// Add current screen to history before switching (if valid)
	if (CurrentScreenType != ET66ScreenType::None)
	{
		NavigationHistory.Add(CurrentScreenType);

		// Trim history if too long
		while (NavigationHistory.Num() > MaxHistoryDepth)
		{
			NavigationHistory.RemoveAt(0);
		}
	}

	// Create/get the new screen
	UT66ScreenBase* NewScreen = CreateScreen(ScreenType);
	if (NewScreen)
	{
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

		OnScreenChanged.Broadcast(OldScreenType, ScreenType);
	}
}

void UT66UIManager::ShowModal(ET66ScreenType ModalType)
{
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
	}
}

ET66ScreenType UT66UIManager::GetCurrentModalType() const
{
	return CurrentModal ? CurrentModal->ScreenType : ET66ScreenType::None;
}

void UT66UIManager::CloseModal()
{
	if (CurrentModal)
	{
		CurrentModal->OnScreenDeactivated();
		CurrentModal->RemoveFromParent();
		CurrentModal = nullptr;

		// Refresh the underlying screen in case state changed while modal was open
		if (CurrentScreen)
		{
			CurrentScreen->RefreshScreen();
		}

		UpdateFrontendTopBar();
	}
}

void UT66UIManager::GoBack()
{
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

		// Deactivate current screen
		ET66ScreenType OldScreenType = CurrentScreenType;
		if (CurrentScreen)
		{
			CurrentScreen->OnScreenDeactivated();
			CurrentScreen->RemoveFromParent();
		}

		// Show previous screen without adding to history
		UT66ScreenBase* NewScreen = CreateScreen(PreviousScreen);
		if (NewScreen)
		{
			CurrentScreen = NewScreen;
			CurrentScreenType = PreviousScreen;
			CurrentScreen->bIsModal = false;
			CurrentScreen->AddToViewport(0);
			CurrentScreen->OnScreenActivated();
			UpdateFrontendTopBar();

			OnScreenChanged.Broadcast(OldScreenType, PreviousScreen);
		}
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
