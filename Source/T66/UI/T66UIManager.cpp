// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66UIManager.h"
#include "UI/T66ScreenBase.h"
#include "UI/Style/T66Style.h"
#include "UI/Components/T66ThemeToggleWidget.h"
#include "Gameplay/T66PlayerController.h"
#include "Blueprint/UserWidget.h"

UT66UIManager::UT66UIManager()
{
	CurrentScreenType = ET66ScreenType::None;
	CurrentScreen = nullptr;
	CurrentModal = nullptr;
}

void UT66UIManager::Initialize(APlayerController* InOwningPlayer)
{
	OwningPlayer = InOwningPlayer;
	NavigationHistory.Empty();
	CurrentScreenType = ET66ScreenType::None;

	// Create the persistent Dark/Light theme toggle (stays above screens, below modals)
	if (OwningPlayer && !ThemeToggle)
	{
		ThemeToggle = CreateWidget<UT66ThemeToggleWidget>(OwningPlayer);
		if (ThemeToggle)
		{
			ThemeToggle->SetUIManager(this);
			ThemeToggle->AddToViewport(50);
			// Hide in gameplay; theme is changed via Settings → Gameplay tab
			if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(OwningPlayer))
			{
				if (T66PC->IsGameplayLevel() && ThemeToggle->IsInViewport())
				{
					ThemeToggle->RemoveFromParent();
				}
			}
		}
	}
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
		UE_LOG(LogTemp, Warning, TEXT("No widget class registered for screen type %d"), static_cast<int32>(ScreenType));
		return nullptr;
	}

	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot create screen - no owning player set"));
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
		// Force Hero Selection to rebuild so co-op (Lab + Back to Lobby only) vs solo (difficulty + Enter) layout is correct on first paint. Otherwise cached tree from a previous show can display.
		if (ScreenType == ET66ScreenType::HeroSelection)
		{
			FT66Style::DeferRebuild(CurrentScreen);
		}
		CurrentScreen->AddToViewport(0);
		CurrentScreen->OnScreenActivated();

		// Ensure the persistent theme toggle is visible (re-add if it was removed by HideAllUI), but not in gameplay
		if (ThemeToggle && !ThemeToggle->IsInViewport())
		{
			bool bShowToggle = true;
			if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(OwningPlayer))
			{
				bShowToggle = !T66PC->IsGameplayLevel();
			}
			if (bShowToggle)
			{
				ThemeToggle->AddToViewport(50);
			}
		}

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
			CurrentScreen->AddToViewport(0);
			CurrentScreen->OnScreenActivated();

			OnScreenChanged.Broadcast(OldScreenType, PreviousScreen);
		}
	}
}

void UT66UIManager::RebuildAllVisibleUI()
{
	// Rebuild the persistent theme toggle so its brushes point to the current style set.
	if (ThemeToggle && ThemeToggle->IsInViewport())
	{
		ThemeToggle->ForceRebuildSlate();
	}

	// Rebuild the underlying screen (e.g. MainMenu) so it picks up new theme tokens.
	if (CurrentScreen && CurrentScreen->IsInViewport())
	{
		CurrentScreen->ForceRebuildSlate();
		CurrentScreen->OnScreenActivated();
	}

	// Rebuild the modal (e.g. Settings) so it picks up new theme tokens.
	if (CurrentModal && CurrentModal->IsInViewport())
	{
		CurrentModal->ForceRebuildSlate();
		CurrentModal->OnScreenActivated();
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

	// Hide the persistent theme toggle
	if (ThemeToggle && ThemeToggle->IsInViewport())
	{
		ThemeToggle->RemoveFromParent();
	}

	ET66ScreenType OldScreenType = CurrentScreenType;
	CurrentScreenType = ET66ScreenType::None;

	OnScreenChanged.Broadcast(OldScreenType, ET66ScreenType::None);
}
