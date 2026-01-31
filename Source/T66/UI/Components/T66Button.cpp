// Copyright Tribulation 66. All Rights Reserved.

#include "T66Button.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/T66ScreenBase.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UT66Button::UT66Button(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Action = ET66ButtonAction::None;
	TargetScreen = ET66ScreenType::None;
	PartySize = ET66PartySize::Solo;
}

void UT66Button::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Update text in designer preview
	if (ButtonTextWidget && !ButtonText.IsEmpty())
	{
		ButtonTextWidget->SetText(ButtonText);
	}

	// Apply theme (safe if running in editor preview too)
	if (ButtonTextWidget)
	{
		ButtonTextWidget->SetFont(FT66Style::Tokens::FontButton());
		ButtonTextWidget->SetColorAndOpacity(FSlateColor(FT66Style::Tokens::Text));
	}
	if (ButtonWidget)
	{
		ButtonWidget->SetStyle(FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"));
	}
}

void UT66Button::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button click
	if (ButtonWidget)
	{
		ButtonWidget->OnClicked.AddDynamic(this, &UT66Button::OnButtonClicked);
	}

	// Set initial text
	if (ButtonTextWidget && !ButtonText.IsEmpty())
	{
		ButtonTextWidget->SetText(ButtonText);
	}

	// Ensure theme is applied at runtime (in case BP overrides)
	if (ButtonTextWidget)
	{
		ButtonTextWidget->SetFont(FT66Style::Tokens::FontButton());
		ButtonTextWidget->SetColorAndOpacity(FSlateColor(FT66Style::Tokens::Text));
	}
	if (ButtonWidget)
	{
		ButtonWidget->SetStyle(FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"));
	}
}

void UT66Button::OnButtonClicked()
{
	ExecuteAction();
}

void UT66Button::SetButtonText(FText NewText)
{
	ButtonText = NewText;
	if (ButtonTextWidget)
	{
		ButtonTextWidget->SetText(NewText);
	}
}

UT66ScreenBase* UT66Button::GetOwningScreen() const
{
	// Walk up the widget tree to find the owning screen
	UWidget* Parent = GetParent();
	while (Parent)
	{
		if (UT66ScreenBase* Screen = Cast<UT66ScreenBase>(Parent))
		{
			return Screen;
		}

		// Get parent's parent
		if (UUserWidget* ParentWidget = Cast<UUserWidget>(Parent))
		{
			Parent = ParentWidget->GetParent();
		}
		else
		{
			break;
		}
	}

	// Try getting from outer
	UObject* Outer = GetOuter();
	while (Outer)
	{
		if (UT66ScreenBase* Screen = Cast<UT66ScreenBase>(Outer))
		{
			return Screen;
		}
		Outer = Outer->GetOuter();
	}

	return nullptr;
}

void UT66Button::ExecuteAction()
{
	UT66ScreenBase* OwningScreen = GetOwningScreen();
	UT66GameInstance* GameInstance = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));

	switch (Action)
	{
	case ET66ButtonAction::NavigateTo:
		if (OwningScreen)
		{
			OwningScreen->NavigateTo(TargetScreen);
		}
		break;

	case ET66ButtonAction::NavigateBack:
		if (OwningScreen)
		{
			OwningScreen->NavigateBack();
		}
		break;

	case ET66ButtonAction::ShowModal:
		if (OwningScreen)
		{
			OwningScreen->ShowModal(TargetScreen);
		}
		break;

	case ET66ButtonAction::CloseModal:
		if (OwningScreen)
		{
			OwningScreen->CloseModal();
		}
		break;

	case ET66ButtonAction::StartGame:
		if (OwningScreen && OwningScreen->UIManager)
		{
			OwningScreen->UIManager->HideAllUI();
		}
		UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
		break;

	case ET66ButtonAction::QuitGame:
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
		break;

	case ET66ButtonAction::SelectPartySize:
		if (GameInstance)
		{
			GameInstance->SelectedPartySize = PartySize;
		}
		// Navigate to hero selection after selecting party size
		if (OwningScreen)
		{
			OwningScreen->NavigateTo(ET66ScreenType::HeroSelection);
		}
		break;

	case ET66ButtonAction::SelectHero:
		if (GameInstance && !HeroID.IsNone())
		{
			GameInstance->SelectedHeroID = HeroID;
		}
		break;

	case ET66ButtonAction::SelectCompanion:
		if (GameInstance)
		{
			GameInstance->SelectedCompanionID = CompanionID; // Can be NAME_None for "No Companion"
		}
		break;

	case ET66ButtonAction::ConfirmCompanion:
		// Save companion selection and go back to hero selection
		if (OwningScreen)
		{
			OwningScreen->NavigateBack();
		}
		break;

	case ET66ButtonAction::Custom:
		OnCustomAction();
		break;

	case ET66ButtonAction::None:
	default:
		break;
	}
}
