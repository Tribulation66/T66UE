// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "TimerManager.h"

using namespace T66SettingsScreenPrivate;
TSharedRef<SWidget> UT66SettingsScreen::BuildControlsTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	auto MakeDeviceTabButton = [this, Loc](ET66ControlsDeviceTab Tab) -> TSharedRef<SWidget>
	{
		const FText Text = (Tab == ET66ControlsDeviceTab::Controller)
			? (Loc ? Loc->GetText_Controller() : NSLOCTEXT("T66.Settings.Fallback", "CONTROLLER", "CONTROLLER"))
			: (Loc ? Loc->GetText_KeyboardAndMouse() : NSLOCTEXT("T66.Settings.Fallback", "KEYBOARD & MOUSE", "KEYBOARD & MOUSE"));

		return SNew(SBox).Padding(FMargin(2.f, 0.f))
		[
			MakeSelectableSettingsButton(
				FT66ButtonParams(Text, FOnClicked::CreateLambda([this, Tab]()
				{
					CurrentControlsDeviceTab = Tab;
					if (ControlsDeviceSwitcher.IsValid())
					{
						ControlsDeviceSwitcher->SetActiveWidgetIndex(Tab == ET66ControlsDeviceTab::Controller ? 1 : 0);
					}
					RefreshControlsKeyTexts();
					return FReply::Handled();
				}), ET66ButtonType::Neutral)
				.SetFontSize(20)
				.SetMinWidth(0.f)
				.SetPadding(FMargin(10.f, 5.f)),
				[this, Tab]() { return CurrentControlsDeviceTab == Tab; })
		];
	};

	auto MakeBindingCell = [this, Loc](bool bAxis, FName Name, float Scale, bool bIsController, int32 SlotIndex) -> TSharedRef<SWidget>
	{
		TArray<FKey> Keys;
		if (bAxis)
		{
			FindAxisKeysForDevice(Name, Scale, bIsController, Keys);
		}
		else
		{
			FindActionKeysForDevice(Name, bIsController, Keys);
		}
		const FKey OldKey = Keys.IsValidIndex(SlotIndex) ? Keys[SlotIndex] : FKey();

		TSharedPtr<STextBlock> KeyText;
		FControlRowKey RowKey;
		RowKey.bIsAxis = bAxis;
		RowKey.Name = Name;
		RowKey.Scale = Scale;
		RowKey.bIsController = bIsController;
		RowKey.SlotIndex = SlotIndex;

		const FText RebindText = Loc ? Loc->GetText_Rebind() : NSLOCTEXT("T66.Settings.Fallback", "REBIND", "REBIND");
		const FText ClearText = Loc ? Loc->GetText_Clear() : NSLOCTEXT("T66.Settings.Fallback", "CLEAR", "CLEAR");

		TSharedRef<SHorizontalBox> Row =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.45f).VAlign(VAlign_Center).Padding(4.f, 0.f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(T66SettingsRowFill())
				.Padding(FMargin(8.f, 4.f))
				[
					SAssignNew(KeyText, STextBlock)
					.Text(KeyToText(OldKey))
					.Font(SettingsBoldFont(18))
					.ColorAndOpacity(GetSettingsPageText())
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
			[
				MakeSettingsButton(
					FT66ButtonParams(RebindText, FOnClicked::CreateLambda([this, bAxis, Name, Scale, bIsController, SlotIndex, OldKey, KeyText]()
					{
						return bAxis ? BeginRebindAxis(Name, Scale, bIsController, SlotIndex, OldKey, KeyText)
							: BeginRebindAction(Name, bIsController, SlotIndex, OldKey, KeyText);
					}), ET66ButtonType::Primary)
					.SetMinWidth(88.f)
					.SetPadding(FMargin(10.f, 5.f))
					.SetColor(FT66Style::ButtonPrimary()))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
			[
				MakeSettingsButton(
					FT66ButtonParams(ClearText, FOnClicked::CreateLambda([this, bAxis, Name, Scale, bIsController, SlotIndex, OldKey, KeyText]()
					{
						Pending = {};
						Pending.bIsAxis = bAxis;
						Pending.Name = Name;
						Pending.Scale = Scale;
						Pending.bIsController = bIsController;
						Pending.SlotIndex = SlotIndex;
						Pending.OldKey = OldKey;
						Pending.KeyText = KeyText;
						ClearBindingInInputSettings();
						return FReply::Handled();
					}), ET66ButtonType::Danger)
					.SetMinWidth(88.f)
					.SetPadding(FMargin(10.f, 5.f))
					.SetColor(FT66Style::Tokens::Danger))
			];

		ControlKeyTextMap.Add(RowKey, KeyText);
		return Row;
	};

	auto MakeRow = [this, Loc, MakeBindingCell](const FText& Label, bool bAxis, FName Name, bool bIsController, float Scale = 1.f) -> TSharedRef<SWidget>
	{
		const FText PrimaryText = Loc ? Loc->GetText_Primary() : NSLOCTEXT("T66.Settings.Fallback", "PRIMARY", "PRIMARY");
		const FText SecondaryText = Loc ? Loc->GetText_Secondary() : NSLOCTEXT("T66.Settings.Fallback", "SECONDARY", "SECONDARY");

		return SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(FMargin(12.f, 8.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(20))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
					[
						SNew(STextBlock)
						.Text(PrimaryText)
						.Font(SettingsRegularFont(20))
						.ColorAndOpacity(GetSettingsPageMuted())
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						MakeBindingCell(bAxis, Name, Scale, bIsController, 0)
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
					[
						SNew(STextBlock)
						.Text(SecondaryText)
						.Font(SettingsRegularFont(20))
						.ColorAndOpacity(GetSettingsPageMuted())
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						MakeBindingCell(bAxis, Name, Scale, bIsController, 1)
					]
				]
			];
	};

	struct FControlDefinition
	{
		FText Label;
		bool bIsAxis = false;
		FName Name = NAME_None;
		float Scale = 1.f;
		bool bShowOnKeyboard = true;
		bool bShowOnController = true;
	};

	const TArray<FControlDefinition> ControlDefinitions = {
		{ Loc ? Loc->GetText_ControlMoveForward() : NSLOCTEXT("T66.Settings.Fallback", "Move Forward", "Move Forward"), true, FName(TEXT("MoveForward")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlMoveBack() : NSLOCTEXT("T66.Settings.Fallback", "Move Back", "Move Back"), true, FName(TEXT("MoveForward")), -1.f, true, true },
		{ Loc ? Loc->GetText_ControlMoveLeft() : NSLOCTEXT("T66.Settings.Fallback", "Move Left", "Move Left"), true, FName(TEXT("MoveRight")), -1.f, true, true },
		{ Loc ? Loc->GetText_ControlMoveRight() : NSLOCTEXT("T66.Settings.Fallback", "Move Right", "Move Right"), true, FName(TEXT("MoveRight")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlJump() : NSLOCTEXT("T66.Settings.Fallback", "Jump", "Jump"), false, FName(TEXT("Jump")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlInteract() : NSLOCTEXT("T66.Settings.Fallback", "Interact", "Interact"), false, FName(TEXT("Interact")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlPauseMenuPrimary() : NSLOCTEXT("T66.Settings.Fallback", "Pause Menu (primary)", "Pause Menu (primary)"), false, FName(TEXT("Escape")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlPauseMenuSecondary() : NSLOCTEXT("T66.Settings.Fallback", "Pause Menu (secondary)", "Pause Menu (secondary)"), false, FName(TEXT("Pause")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlToggleHUD() : NSLOCTEXT("T66.Settings.Fallback", "Toggle HUD", "Toggle HUD"), false, FName(TEXT("ToggleHUD")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlToggleTikTok() : NSLOCTEXT("T66.Settings.Fallback", "Toggle TikTok", "Toggle TikTok"), false, FName(TEXT("ToggleTikTok")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlOpenFullMap() : NSLOCTEXT("T66.Settings.Fallback", "Open Full Map", "Open Full Map"), false, FName(TEXT("OpenFullMap")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlInspectInventory() : NSLOCTEXT("T66.Settings.Fallback", "Inspect Inventory", "Inspect Inventory"), false, FName(TEXT("InspectInventory")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlToggleMediaViewer() : NSLOCTEXT("T66.Settings.Fallback", "Toggle Media Viewer", "Toggle Media Viewer"), false, FName(TEXT("ToggleMediaViewer")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlToggleGamerMode() : NSLOCTEXT("T66.Settings.Fallback", "Toggle Gamer Mode (Hitboxes)", "Toggle Gamer Mode (Hitboxes)"), false, FName(TEXT("ToggleGamerMode")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlRestartRun() : NSLOCTEXT("T66.Settings.Fallback", "Restart Run", "Restart Run"), false, FName(TEXT("RestartRun")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlDash() : NSLOCTEXT("T66.Settings.Fallback", "Dash", "Dash"), false, FName(TEXT("Dash")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlUltimate() : NSLOCTEXT("T66.Settings.Fallback", "Ultimate", "Ultimate"), false, FName(TEXT("Ultimate")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlAttackLock() : NSLOCTEXT("T66.Settings.Fallback", "Attack Lock", "Attack Lock"), false, FName(TEXT("AttackLock")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlAttackUnlock() : NSLOCTEXT("T66.Settings.Fallback", "Attack Unlock", "Attack Unlock"), false, FName(TEXT("AttackUnlock")), 1.f, true, true },
		{ Loc ? Loc->GetText_ControlToggleMouseLock() : NSLOCTEXT("T66.Settings.Fallback", "Toggle Mouse Lock", "Toggle Mouse Lock"), false, FName(TEXT("ToggleMouseLock")), 1.f, true, false }
	};

	auto MakeDeviceBindingList = [MakeRow, &ControlDefinitions](bool bIsController) -> TSharedRef<SWidget>
	{
		const TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
		for (const FControlDefinition& ControlDefinition : ControlDefinitions)
		{
			const bool bShowRow = bIsController ? ControlDefinition.bShowOnController : ControlDefinition.bShowOnKeyboard;
			if (!bShowRow)
			{
				continue;
			}

			Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeRow(ControlDefinition.Label, ControlDefinition.bIsAxis, ControlDefinition.Name, bIsController, ControlDefinition.Scale)
			];
		}

		return SNew(SScrollBox)
			.ScrollBarStyle(GetSettingsReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(10.f, 0.f, 2.f, 0.f))
			+ SScrollBox::Slot()
			[
				Box
			];
	};

	ControlKeyTextMap.Empty();

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()[ MakeDeviceTabButton(ET66ControlsDeviceTab::KeyboardMouse) ]
			+ SHorizontalBox::Slot().AutoWidth()[ MakeDeviceTabButton(ET66ControlsDeviceTab::Controller) ]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SAssignNew(RebindStatusText, STextBlock)
			.Text(Loc ? Loc->GetText_RebindInstructions() : NSLOCTEXT("T66.Settings.Fallback", "RebindInstructions", "Click REBIND, then press a key/button (Esc cancels)."))
			.Font(SettingsRegularFont(18))
			.ColorAndOpacity(GetSettingsPageMuted())
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SAssignNew(ControlsDeviceSwitcher, SWidgetSwitcher)
			.WidgetIndex(0)
			+ SWidgetSwitcher::Slot()
			[
				MakeDeviceBindingList(false)
			]
			+ SWidgetSwitcher::Slot()
			[
				MakeDeviceBindingList(true)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 15.f, 0.f, 0.f)
		[
			MakeSettingsButton(
				FT66ButtonParams(Loc ? Loc->GetText_RestoreDefaults() : NSLOCTEXT("T66.Settings.Fallback", "Restore Defaults", "RESTORE DEFAULTS"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleRestoreDefaultsClicked), ET66ButtonType::Danger)
				.SetFontSize(20)
				.SetMinWidth(160.f))
		];
}

FReply UT66SettingsScreen::HandleRestoreDefaultsClicked()
{
	// Device-specific restore (KBM vs Controller) to match the current sub-tab.
	const bool bRestoreController = (CurrentControlsDeviceTab == ET66ControlsDeviceTab::Controller);

	// Confirmation (Bible requires a confirm): arm on first click, execute on second click shortly after.
	if (!bRestoreDefaultsArmed || (bRestoreDefaultsArmedForController != bRestoreController))
	{
		bRestoreDefaultsArmed = true;
		bRestoreDefaultsArmedForController = bRestoreController;
		if (RebindStatusText.IsValid())
		{
			RebindStatusText->SetText(NSLOCTEXT("T66.Settings", "PressRestoreDefaultsAgain", "Press Restore Defaults again to confirm."));
		}
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RestoreDefaultsArmTimerHandle);
			FTimerDelegate D;
			D.BindLambda([this]()
			{
				bRestoreDefaultsArmed = false;
				if (RebindStatusText.IsValid())
				{
					if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
					{
						RebindStatusText->SetText(Loc->GetText_RebindInstructions());
					}
				}
			});
			World->GetTimerManager().SetTimer(RestoreDefaultsArmTimerHandle, D, 2.5f, false);
		}
		return FReply::Handled();
	}

	// Disarm now that we're executing.
	bRestoreDefaultsArmed = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RestoreDefaultsArmTimerHandle);
	}

	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (!Settings) return FReply::Handled();

	// Remove current mappings for this device class.
	{
		const TArray<FInputActionKeyMapping> ActionMappings = Settings->GetActionMappings();
		for (const FInputActionKeyMapping& M : ActionMappings)
		{
			if (bRestoreController ? IsControllerKey(M.Key) : IsKeyboardMouseKey(M.Key))
			{
				Settings->RemoveActionMapping(M);
			}
		}
		const TArray<FInputAxisKeyMapping> AxisMappings = Settings->GetAxisMappings();
		for (const FInputAxisKeyMapping& M : AxisMappings)
		{
			if (bRestoreController ? IsControllerKey(M.Key) : IsKeyboardMouseKey(M.Key))
			{
				Settings->RemoveAxisMapping(M);
			}
		}
	}

	// Re-add defaults from the class default object.
	if (const UInputSettings* DefaultIS = GetDefault<UInputSettings>())
	{
		const TArray<FInputActionKeyMapping>& DefaultActions = DefaultIS->GetActionMappings();
		for (const FInputActionKeyMapping& M : DefaultActions)
		{
			if (bRestoreController ? IsControllerKey(M.Key) : IsKeyboardMouseKey(M.Key))
			{
				Settings->AddActionMapping(M);
			}
		}
		const TArray<FInputAxisKeyMapping>& DefaultAxes = DefaultIS->GetAxisMappings();
		for (const FInputAxisKeyMapping& M : DefaultAxes)
		{
			if (bRestoreController ? IsControllerKey(M.Key) : IsKeyboardMouseKey(M.Key))
			{
				Settings->AddAxisMapping(M);
			}
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
	RefreshControlsKeyTexts();
	if (RebindStatusText.IsValid())
	{
		RebindStatusText->SetText(NSLOCTEXT("T66.Settings", "DefaultsRestored", "Defaults restored."));
	}
	return FReply::Handled();
}

void UT66SettingsScreen::RefreshControlsKeyTexts()
{
	for (auto& Pair : ControlKeyTextMap)
	{
		const FControlRowKey& K = Pair.Key;
		if (!Pair.Value.IsValid()) continue;

		TArray<FKey> Keys;
		if (K.bIsAxis) FindAxisKeysForDevice(K.Name, K.Scale, K.bIsController, Keys);
		else FindActionKeysForDevice(K.Name, K.bIsController, Keys);
		const FKey CurrentKey = Keys.IsValidIndex(K.SlotIndex) ? Keys[K.SlotIndex] : FKey();
		Pair.Value->SetText(KeyToText(CurrentKey));
	}
}

void UT66SettingsScreen::FindActionKeysForDevice(FName ActionName, bool bIsController, TArray<FKey>& OutKeys)
{
	OutKeys.Reset();
	if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
	{
		const TArray<FInputActionKeyMapping>& M = Settings->GetActionMappings();
		for (const FInputActionKeyMapping& A : M)
		{
			if (A.ActionName != ActionName) continue;
			const bool bOk = bIsController ? IsControllerKey(A.Key) : IsKeyboardMouseKey(A.Key);
			if (bOk) OutKeys.Add(A.Key);
		}
	}
}

void UT66SettingsScreen::FindAxisKeysForDevice(FName AxisName, float Scale, bool bIsController, TArray<FKey>& OutKeys)
{
	OutKeys.Reset();
	if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
	{
		const TArray<FInputAxisKeyMapping>& M = Settings->GetAxisMappings();
		for (const FInputAxisKeyMapping& A : M)
		{
			if (A.AxisName != AxisName || !FMath::IsNearlyEqual(A.Scale, Scale)) continue;
			const bool bOk = bIsController ? IsControllerKey(A.Key) : IsKeyboardMouseKey(A.Key);
			if (bOk) OutKeys.Add(A.Key);
		}
	}
}


