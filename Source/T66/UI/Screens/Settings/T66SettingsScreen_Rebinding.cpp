// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

using namespace T66SettingsScreenPrivate;
FReply UT66SettingsScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bWaitingForRebind)
	{
		const FKey K = InKeyEvent.GetKey();
		if (K == EKeys::Escape || K == EKeys::Gamepad_FaceButton_Right)
		{
			bWaitingForRebind = false;
			if (RebindStatusText.IsValid())
			{
				if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
				{
					RebindStatusText->SetText(Loc->GetText_RebindCancelled());
				}
			}
			return FReply::Handled();
		}
		ApplyRebindToInputSettings(K);
		return FReply::Handled();
	}

	// Esc closes settings (Bible: no global cancel button).
	if (InKeyEvent.GetKey() == EKeys::Escape
		|| InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Right
		|| InKeyEvent.GetKey() == EKeys::Gamepad_Special_Left)
	{
		HandleCloseClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UT66SettingsScreen::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bWaitingForRebind)
	{
		ApplyRebindToInputSettings(InMouseEvent.GetEffectingButton());
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FText UT66SettingsScreen::KeyToText(const FKey& K)
{
	return K.IsValid() ? K.GetDisplayName() : NSLOCTEXT("T66.Common", "Dash", "-");
}

bool UT66SettingsScreen::IsKeyboardMouseKey(const FKey& K)
{
	if (!K.IsValid()) return false;
	if (K.IsMouseButton()) return true;
	// Treat any non-gamepad, non-touch key as "keyboard/mouse" for our purposes.
	return !K.IsGamepadKey() && !K.IsTouch();
}

bool UT66SettingsScreen::IsControllerKey(const FKey& K)
{
	return K.IsValid() && K.IsGamepadKey();
}

FKey UT66SettingsScreen::FindActionKey(FName ActionName, const FKey& PreferredOldKey)
{
	if (PreferredOldKey.IsValid()) return PreferredOldKey;
	if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
	{
		const TArray<FInputActionKeyMapping>& M = Settings->GetActionMappings();
		for (const FInputActionKeyMapping& A : M)
		{
			if (A.ActionName == ActionName && IsKeyboardMouseKey(A.Key))
			{
				return A.Key;
			}
		}
	}
	return FKey();
}

FKey UT66SettingsScreen::FindAxisKey(FName AxisName, float Scale, const FKey& PreferredOldKey)
{
	if (PreferredOldKey.IsValid()) return PreferredOldKey;
	if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
	{
		const TArray<FInputAxisKeyMapping>& M = Settings->GetAxisMappings();
		for (const FInputAxisKeyMapping& A : M)
		{
			if (A.AxisName == AxisName && FMath::IsNearlyEqual(A.Scale, Scale) && IsKeyboardMouseKey(A.Key))
			{
				return A.Key;
			}
		}
	}
	return FKey();
}

FReply UT66SettingsScreen::BeginRebindAction(FName ActionName, bool bIsController, int32 SlotIndex, const FKey& OldKey, TSharedPtr<STextBlock> KeyText)
{
	bWaitingForRebind = true;
	Pending = {};
	Pending.bIsAxis = false;
	Pending.Name = ActionName;
	Pending.Scale = 1.f;
	Pending.bIsController = bIsController;
	Pending.SlotIndex = SlotIndex;
	Pending.OldKey = OldKey;
	Pending.KeyText = KeyText;
	if (RebindStatusText.IsValid())
	{
		if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
		{
			RebindStatusText->SetText(FText::Format(Loc->GetText_RebindPressKeyFormat(), FText::FromName(ActionName)));
		}
	}
	return FReply::Handled();
}

FReply UT66SettingsScreen::BeginRebindAxis(FName AxisName, float Scale, bool bIsController, int32 SlotIndex, const FKey& OldKey, TSharedPtr<STextBlock> KeyText)
{
	bWaitingForRebind = true;
	Pending = {};
	Pending.bIsAxis = true;
	Pending.Name = AxisName;
	Pending.Scale = Scale;
	Pending.bIsController = bIsController;
	Pending.SlotIndex = SlotIndex;
	Pending.OldKey = OldKey;
	Pending.KeyText = KeyText;
	if (RebindStatusText.IsValid())
	{
		if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
		{
			RebindStatusText->SetText(FText::Format(Loc->GetText_RebindPressKeyFormat(), FText::FromName(AxisName)));
		}
	}
	return FReply::Handled();
}

void UT66SettingsScreen::ApplyRebindToInputSettings(const FKey& NewKey)
{
	if (!bWaitingForRebind) return;
	bWaitingForRebind = false;
	if (!NewKey.IsValid()) return;

	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (!Settings) return;

	// Enforce device type: KBM bindings only accept KBM keys; Controller bindings only accept gamepad keys.
	if (Pending.bIsController && !IsControllerKey(NewKey))
	{
		return;
	}
	if (!Pending.bIsController && !IsKeyboardMouseKey(NewKey))
	{
		return;
	}

	// Conflict overwrite: remove any existing mapping using NewKey on the same device class.
	FString Overwrote;
	{
		const TArray<FInputActionKeyMapping> ActionMappings = Settings->GetActionMappings();
		for (const FInputActionKeyMapping& M : ActionMappings)
		{
			const bool bSameDevice = Pending.bIsController ? IsControllerKey(M.Key) : IsKeyboardMouseKey(M.Key);
			if (bSameDevice && M.Key == NewKey && (M.ActionName != Pending.Name))
			{
				if (Overwrote.IsEmpty())
				{
					Overwrote = M.ActionName.ToString();
				}
				Settings->RemoveActionMapping(M);
			}
		}
		const TArray<FInputAxisKeyMapping> AxisMappings = Settings->GetAxisMappings();
		for (const FInputAxisKeyMapping& M : AxisMappings)
		{
			const bool bSameDevice = Pending.bIsController ? IsControllerKey(M.Key) : IsKeyboardMouseKey(M.Key);
			if (bSameDevice && M.Key == NewKey && (M.AxisName != Pending.Name || !FMath::IsNearlyEqual(M.Scale, Pending.Scale)))
			{
				if (Overwrote.IsEmpty())
				{
					Overwrote = M.AxisName.ToString();
				}
				Settings->RemoveAxisMapping(M);
			}
		}
	}

	if (Pending.bIsAxis)
	{
		if (Pending.OldKey.IsValid())
		{
			Settings->RemoveAxisMapping(FInputAxisKeyMapping(Pending.Name, Pending.OldKey, Pending.Scale));
		}
		Settings->AddAxisMapping(FInputAxisKeyMapping(Pending.Name, NewKey, Pending.Scale));
	}
	else
	{
		if (Pending.OldKey.IsValid())
		{
			Settings->RemoveActionMapping(FInputActionKeyMapping(Pending.Name, Pending.OldKey));
		}
		Settings->AddActionMapping(FInputActionKeyMapping(Pending.Name, NewKey));
	}

	Settings->SaveKeyMappings();
	Settings->SaveConfig();

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (PC->PlayerInput) PC->PlayerInput->ForceRebuildingKeyMaps(true);
			if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC))
			{
				T66PC->RefreshGameplayMouseMappings();
			}
		}
	}

	if (Pending.KeyText.IsValid())
	{
		Pending.KeyText->SetText(KeyToText(NewKey));
	}
	if (RebindStatusText.IsValid())
	{
		if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
		{
			if (!Overwrote.IsEmpty())
			{
				RebindStatusText->SetText(FText::FromString(FString::Printf(TEXT("Overwrote: %s"), *Overwrote)));
			}
			else
			{
				RebindStatusText->SetText(Loc->GetText_RebindSaved());
			}
		}
	}
}

void UT66SettingsScreen::ClearBindingInInputSettings()
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (!Settings) return;

	if (Pending.bIsAxis)
	{
		if (Pending.OldKey.IsValid())
		{
			Settings->RemoveAxisMapping(FInputAxisKeyMapping(Pending.Name, Pending.OldKey, Pending.Scale));
		}
	}
	else
	{
		if (Pending.OldKey.IsValid())
		{
			Settings->RemoveActionMapping(FInputActionKeyMapping(Pending.Name, Pending.OldKey));
		}
	}

	Settings->SaveKeyMappings();
	Settings->SaveConfig();

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (PC->PlayerInput) PC->PlayerInput->ForceRebuildingKeyMaps(true);
			if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC))
			{
				T66PC->RefreshGameplayMouseMappings();
			}
		}
	}
	if (Pending.KeyText.IsValid())
	{
		Pending.KeyText->SetText(NSLOCTEXT("T66.Common", "Dash", "-"));
	}
}


