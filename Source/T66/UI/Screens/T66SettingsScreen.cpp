// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/GameUserSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericApplication.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SOverlay.h"

UT66SettingsScreen::UT66SettingsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Settings;
	bIsModal = true;
	SetIsFocusable(true);
}

UT66LocalizationSubsystem* UT66SettingsScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66PlayerSettingsSubsystem* UT66SettingsScreen::GetPlayerSettings() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66PlayerSettingsSubsystem>();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66SettingsScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	// Use dynamic lambda for button color so it updates when tab changes
	auto MakeTabButton = [this](const FText& Text, ET66SettingsTab Tab) -> TSharedRef<SWidget>
	{
		return SNew(SBox).Padding(FMargin(2.0f, 0.0f))
			[
				FT66Style::MakeButton(
					FT66ButtonParams(Text, FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, Tab), ET66ButtonType::Neutral)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(12.f, 8.f))
					.SetColor(TAttribute<FSlateColor>::CreateLambda([this, Tab]() -> FSlateColor {
						bool bIsSelected = (CurrentTab == Tab);
						return bIsSelected ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2;
					}))
					.SetContent(SNew(STextBlock).Text(Text)
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip")))
				)
			];
	};

	// Build the widget switcher with all tab content (stored as class member)

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Tokens::Scrim)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(950.0f)
			.HeightOverride(700.0f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					// Tab bar + close button
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						FT66Style::MakePanel(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(Loc ? Loc->GetText_SettingsTabGameplay() : NSLOCTEXT("T66.Settings", "TabGameplay", "GAMEPLAY"), ET66SettingsTab::Gameplay)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(Loc ? Loc->GetText_SettingsTabGraphics() : NSLOCTEXT("T66.Settings", "TabGraphics", "GRAPHICS"), ET66SettingsTab::Graphics)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(Loc ? Loc->GetText_SettingsTabControls() : NSLOCTEXT("T66.Settings", "TabControls", "CONTROLS"), ET66SettingsTab::Controls)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(Loc ? Loc->GetText_SettingsTabAudio() : NSLOCTEXT("T66.Settings", "TabAudio", "AUDIO"), ET66SettingsTab::Audio)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(Loc ? Loc->GetText_SettingsTabCrashing() : NSLOCTEXT("T66.Settings", "TabCrashing", "CRASHING"), ET66SettingsTab::Crashing)
							]
							+ SHorizontalBox::Slot().FillWidth(1.0f)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								FT66Style::MakeButton(
								FT66ButtonParams(NSLOCTEXT("T66.Common", "CloseX", "X"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseClicked), ET66ButtonType::Danger)
								.SetMinWidth(40.f).SetHeight(40.f)
								.SetPadding(FMargin(0.f))
								.SetColor(FT66Style::Tokens::Danger)
								.SetContent(SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "CloseX", "X"))
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FT66Style::Tokens::Text))
							)
							],
							FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(15.0f, 10.0f))
						)
					]
					// Content area
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(20.0f)
					[
						SAssignNew(ContentSwitcher, SWidgetSwitcher)
						.WidgetIndex(static_cast<int32>(CurrentTab))
						+ SWidgetSwitcher::Slot()
						[
							BuildGameplayTab()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildGraphicsTab()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildControlsTab()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildAudioTab()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildCrashingTab()
						]
					],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(0.0f)
			)
			]
		];
}

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
		}
	}
	if (Pending.KeyText.IsValid())
	{
		Pending.KeyText->SetText(NSLOCTEXT("T66.Common", "Dash", "-"));
	}
}

TSharedRef<SWidget> UT66SettingsScreen::BuildGameplayTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	auto MakeToggleRow = [this, Loc](const FText& Label, TFunction<bool()> GetValue, TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		const FText OnText = Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON");
		const FText OffText = Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF");

		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(SBox).MinDesiredWidth(200.0f)
					[
						SNew(STextBlock).Text(Label)
						.Font(FT66Style::Tokens::FontRegular(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						FT66Style::MakeButton(
							FT66ButtonParams(OnText, FOnClicked::CreateLambda([SetValue]() { SetValue(true); return FReply::Handled(); }), ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([GetValue]() -> FSlateColor { return GetValue() ? FT66Style::Tokens::Success : FT66Style::Tokens::Panel2; }))
						)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(OffText, FOnClicked::CreateLambda([SetValue]() { SetValue(false); return FReply::Handled(); }), ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([GetValue]() -> FSlateColor { return !GetValue() ? FT66Style::Tokens::Danger : FT66Style::Tokens::Panel2; }))
						)
					]
				]
			];
	};

	const FText ThemeLabel = Loc ? Loc->GetText_SettingsTheme() : NSLOCTEXT("T66.Settings", "Theme", "Theme:");
	const FText DarkText = Loc ? Loc->GetText_ThemeDark() : NSLOCTEXT("T66.Theme", "Dark", "DARK");
	const FText LightText = Loc ? Loc->GetText_ThemeLight() : NSLOCTEXT("T66.Theme", "Light", "LIGHT");

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			// Theme row (Dark / Light) at top
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FT66Style::Tokens::Panel2)
				.Padding(FMargin(15.0f, 12.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
					[
						SNew(STextBlock).Text(ThemeLabel)
						.Font(FT66Style::Tokens::FontRegular(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							FT66Style::MakeButton(
							FT66ButtonParams(DarkText, FOnClicked::CreateLambda([this, PS]()
							{
								if (PS) PS->SetLightTheme(false);
								if (UIManager) UIManager->RebuildAllVisibleUI();
								return FReply::Handled();
							}), (PS && !PS->GetLightTheme()) ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
								.SetMinWidth(100.f)
								.SetPadding(FMargin(12.f, 6.f))
							)
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
						[
							FT66Style::MakeButton(
							FT66ButtonParams(LightText, FOnClicked::CreateLambda([this, PS]()
							{
								if (PS) PS->SetLightTheme(true);
								if (UIManager) UIManager->RebuildAllVisibleUI();
								return FReply::Handled();
							}), (PS && PS->GetLightTheme()) ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
								.SetMinWidth(100.f)
								.SetPadding(FMargin(12.f, 6.f))
							)
						]
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					Loc ? Loc->GetText_PracticeMode() : NSLOCTEXT("T66.Settings.Fallback", "Practice Mode", "Practice Mode"),
					[PS]() { return PS ? PS->GetPracticeMode() : false; },
					[PS](bool b) { if (PS) PS->SetPracticeMode(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					Loc ? Loc->GetText_SubmitLeaderboardAnonymous() : NSLOCTEXT("T66.Settings.Fallback", "Submit Leaderboard as Anonymous", "Submit Leaderboard as Anonymous"),
					[PS]() { return PS ? PS->GetSubmitLeaderboardAnonymous() : false; },
					[PS](bool b) { if (PS) PS->SetSubmitLeaderboardAnonymous(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					Loc ? Loc->GetText_SpeedRunMode() : NSLOCTEXT("T66.Settings.Fallback", "Speed Run Mode", "Speed Run Mode"),
					[PS]() { return PS ? PS->GetSpeedRunMode() : false; },
					[PS](bool b) { if (PS) PS->SetSpeedRunMode(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					Loc ? Loc->GetText_GoonerMode() : NSLOCTEXT("T66.Settings.Fallback", "Gooner Mode", "Gooner Mode"),
					[PS]() { return PS ? PS->GetGoonerMode() : false; },
					[PS](bool b) { if (PS) PS->SetGoonerMode(b); }
				)
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildGraphicsTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	InitializeGraphicsFromUserSettingsIfNeeded();

	auto MakeDropdownRow = [this](const FText& Label, TFunction<FText()> GetCurrentValue, TFunction<TSharedRef<SWidget>()> MakeMenuContent) -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> TriggerContent = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(FMargin(10.0f, 4.0f))
			[
				SNew(STextBlock).Text_Lambda([GetCurrentValue]() { return GetCurrentValue(); })
				.Font(FT66Style::Tokens::FontRegular(12))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(4.0f, 0.0f))
			[
				SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "DropdownArrow", "▼"))
				.Font(FT66Style::Tokens::FontRegular(10))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			];
		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.4f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label)
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().FillWidth(0.6f)
				[
					FT66Style::MakeDropdown(FT66DropdownParams(TriggerContent, MakeMenuContent).SetHeight(32.0f))
				]
			];
	};

	auto FpsCapToText = [Loc](int32 Index) -> FText
	{
		switch (Index)
		{
		case 0: return FText::AsNumber(30);
		case 1: return FText::AsNumber(60);
		case 2: return FText::AsNumber(90);
		case 3: return FText::AsNumber(120);
		default: return Loc ? Loc->GetText_Unlimited() : NSLOCTEXT("T66.Settings", "Unlimited", "Unlimited");
		}
	};

	auto WindowModeToText = [Loc](EWindowMode::Type Mode) -> FText
	{
		if (!Loc)
		{
			switch (Mode)
			{
			case EWindowMode::Windowed: return NSLOCTEXT("T66.Settings", "Windowed", "Windowed");
			case EWindowMode::WindowedFullscreen: return NSLOCTEXT("T66.Settings", "BorderlessWindowed", "Borderless Windowed");
			default: return NSLOCTEXT("T66.Settings", "Fullscreen", "Fullscreen");
			}
		}
		switch (Mode)
		{
		case EWindowMode::Windowed: return Loc->GetText_Windowed();
		case EWindowMode::WindowedFullscreen: return Loc->GetText_BorderlessWindowed();
		default: return Loc->GetText_Fullscreen();
		}
	};

	auto DisplayModeToText = [Loc](ET66DisplayMode Mode) -> FText
	{
		if (!Loc)
		{
			return Mode == ET66DisplayMode::Widescreen ? NSLOCTEXT("T66.Settings", "Widescreen", "Widescreen") : NSLOCTEXT("T66.Settings", "Standard", "Standard");
		}
		return Mode == ET66DisplayMode::Widescreen ? Loc->GetText_DisplayModeWidescreen() : Loc->GetText_DisplayModeStandard();
	};

	auto ResToText = [](FIntPoint R) -> FText
	{
		return FText::Format(NSLOCTEXT("T66.Settings", "ResolutionFormat", "{0} x {1}"), FText::AsNumber(R.X), FText::AsNumber(R.Y));
	};

	TSharedRef<SOverlay> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(
				Loc ? Loc->GetText_Monitor() : NSLOCTEXT("T66.Settings", "Monitor", "Monitor"),
				[this, Loc]()
				{
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					const int32 Idx = FMath::Clamp(PendingGraphics.MonitorIndex, 0, FMath::Max(0, Metrics.MonitorInfo.Num() - 1));
					if (Idx == 0) return Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor");
					return FText::Format(NSLOCTEXT("T66.Settings", "MonitorN", "Monitor {0}"), FText::AsNumber(Idx + 1));
				},
				[this, Loc]()
				{
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (int32 i = 0; i < Metrics.MonitorInfo.Num(); ++i)
					{
						const FText Label = (i == 0) ? (Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor"))
							: FText::Format(NSLOCTEXT("T66.Settings", "MonitorN", "Monitor {0}"), FText::AsNumber(i + 1));
						const int32 MonitorIdx = i;
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeButton(
								FT66ButtonParams(Label, FOnClicked::CreateLambda([this, MonitorIdx]() { PendingGraphics.MonitorIndex = MonitorIdx; PendingGraphics.bDirty = true; return FReply::Handled(); }), ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetColor(FT66Style::Tokens::Panel2)
							)
						];
					}
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(
				Loc ? Loc->GetText_Resolution() : NSLOCTEXT("T66.Settings.Fallback", "Resolution", "Resolution"),
				[this, ResToText]() { return ResToText(PendingGraphics.Resolution); },
				[this, ResToText]()
				{
					static const FIntPoint ResList[] = {
						FIntPoint(1280, 720),
						FIntPoint(1600, 900),
						FIntPoint(1920, 1080),
						FIntPoint(2560, 1440),
						FIntPoint(3840, 2160)
					};
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (const FIntPoint& R : ResList)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeButton(
								FT66ButtonParams(ResToText(R), FOnClicked::CreateLambda([this, R]() { PendingGraphics.Resolution = R; PendingGraphics.bDirty = true; return FReply::Handled(); }), ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetColor(FT66Style::Tokens::Panel2)
							)
						];
					}
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(
				Loc ? Loc->GetText_WindowMode() : NSLOCTEXT("T66.Settings.Fallback", "Window Mode", "Window Mode"),
				[this, WindowModeToText]() { return WindowModeToText(PendingGraphics.WindowMode); },
				[this, WindowModeToText]()
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					auto Add = [this, &Box, WindowModeToText](EWindowMode::Type Mode)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeButton(
								FT66ButtonParams(WindowModeToText(Mode), FOnClicked::CreateLambda([this, Mode]() { PendingGraphics.WindowMode = Mode; PendingGraphics.bDirty = true; return FReply::Handled(); }), ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetColor(FT66Style::Tokens::Panel2)
							)
						];
					};
					Add(EWindowMode::Windowed);
					Add(EWindowMode::Fullscreen);
					Add(EWindowMode::WindowedFullscreen);
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(
				Loc ? Loc->GetText_DisplayMode() : NSLOCTEXT("T66.Settings.Fallback", "Display Mode", "Display Mode"),
				[this, DisplayModeToText]() { return DisplayModeToText(PendingGraphics.DisplayMode); },
				[this, DisplayModeToText]()
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					auto Add = [this, &Box, DisplayModeToText](ET66DisplayMode Mode)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeButton(
								FT66ButtonParams(DisplayModeToText(Mode), FOnClicked::CreateLambda([this, Mode]() { PendingGraphics.DisplayMode = Mode; PendingGraphics.bDirty = true; return FReply::Handled(); }), ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetColor(FT66Style::Tokens::Panel2)
							)
						];
					};
					Add(ET66DisplayMode::Standard);
					Add(ET66DisplayMode::Widescreen);
					return Box;
				}
			)
		]
		// Quality slider
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(STextBlock).Text(Loc ? Loc->GetText_BestPerformance() : NSLOCTEXT("T66.Settings.Fallback", "Best Performance", "Best Performance"))
						.Font(FT66Style::Tokens::FontRegular(11))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)
					[
						SNew(STextBlock).Text(Loc ? Loc->GetText_BestQuality() : NSLOCTEXT("T66.Settings.Fallback", "Best Quality", "Best Quality"))
						.Font(FT66Style::Tokens::FontRegular(11))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					SNew(SSlider)
					.Value_Lambda([this]() { return static_cast<float>(PendingGraphics.QualityNotch) / 3.0f; })
					.StepSize(1.0f / 3.0f)
					.OnValueChanged_Lambda([this](float V)
					{
						PendingGraphics.QualityNotch = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3);
						PendingGraphics.bDirty = true;
					})
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(
				Loc ? Loc->GetText_FpsCap() : NSLOCTEXT("T66.Settings.Fallback", "FPS Cap", "FPS Cap"),
				[this, FpsCapToText]() { return FpsCapToText(PendingGraphics.FpsCapIndex); },
				[this, FpsCapToText]()
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (int32 i = 0; i < 5; ++i)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeButton(
								FT66ButtonParams(FpsCapToText(i), FOnClicked::CreateLambda([this, i]() { PendingGraphics.FpsCapIndex = i; PendingGraphics.bDirty = true; return FReply::Handled(); }), ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetColor(FT66Style::Tokens::Panel2)
							)
						];
					}
					return Box;
				}
			)
		]
		// Apply button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f)
		[
			FT66Style::MakeButton(Loc ? Loc->GetText_Apply() : NSLOCTEXT("T66.Settings.Fallback", "APPLY", "APPLY"),
				FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleApplyGraphicsClicked), ET66ButtonType::Success, 120.f)
		]
	];

	// Confirm overlay (10s auto-revert)
	Root->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
		.Visibility_Lambda([this]() { return bVideoModeConfirmActive ? EVisibility::Visible : EVisibility::Collapsed; })
		.Padding(FMargin(30.f))
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(25.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_KeepTheseSettingsTitle() : NSLOCTEXT("T66.Settings.Fallback", "Keep these settings?", "Keep these settings?"))
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 15.f)
				[
					SAssignNew(VideoModeConfirmCountdownText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontRegular(13))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(6.f)
					[
					FT66Style::MakeButton(Loc ? Loc->GetText_Keep() : NSLOCTEXT("T66.Settings.Fallback", "KEEP", "KEEP"),
						FOnClicked::CreateLambda([this]() { EndVideoModeConfirmPrompt(true); return FReply::Handled(); }),
						ET66ButtonType::Success, 100.f)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(6.f)
					[
					FT66Style::MakeButton(Loc ? Loc->GetText_Revert() : NSLOCTEXT("T66.Settings.Fallback", "REVERT", "REVERT"),
						FOnClicked::CreateLambda([this]() { EndVideoModeConfirmPrompt(false); return FReply::Handled(); }),
						ET66ButtonType::Danger, 100.f)
					]
				]
			]
		]
	];

	return Root;
}

TSharedRef<SWidget> UT66SettingsScreen::BuildControlsTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	auto MakeDeviceTabButton = [this, Loc](ET66ControlsDeviceTab Tab) -> TSharedRef<SWidget>
	{
		const FText Text = (Tab == ET66ControlsDeviceTab::Controller)
			? (Loc ? Loc->GetText_Controller() : NSLOCTEXT("T66.Settings.Fallback", "CONTROLLER", "CONTROLLER"))
			: (Loc ? Loc->GetText_KeyboardAndMouse() : NSLOCTEXT("T66.Settings.Fallback", "KEYBOARD & MOUSE", "KEYBOARD & MOUSE"));
		return SNew(SBox).Padding(FMargin(2.0f, 0.0f))
		[
			FT66Style::MakeButton(
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
				.SetMinWidth(0.f)
				.SetPadding(FMargin(12.f, 6.f))
				.SetColor(TAttribute<FSlateColor>::CreateLambda([this, Tab]() -> FSlateColor
				{
					const bool bIsSelected = (CurrentControlsDeviceTab == Tab);
					return bIsSelected ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2;
				}))
			)
		];
	};

	auto MakeBindingCell = [this, Loc](bool bAxis, FName Name, float Scale, bool bIsController, int32 SlotIndex) -> TSharedRef<SWidget>
	{
		TArray<FKey> Keys;
		if (bAxis) FindAxisKeysForDevice(Name, Scale, bIsController, Keys);
		else FindActionKeysForDevice(Name, bIsController, Keys);
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
			+ SHorizontalBox::Slot().FillWidth(0.45f).VAlign(VAlign_Center).Padding(4.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FT66Style::Tokens::Panel2)
				.Padding(FMargin(8.f, 4.f))
				[
					SAssignNew(KeyText, STextBlock)
					.Text(KeyToText(OldKey))
					.Font(FT66Style::Tokens::FontBold(12))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(RebindText, FOnClicked::CreateLambda([this, bAxis, Name, Scale, bIsController, SlotIndex, OldKey, KeyText]()
					{
						return bAxis ? BeginRebindAxis(Name, Scale, bIsController, SlotIndex, OldKey, KeyText)
							: BeginRebindAction(Name, bIsController, SlotIndex, OldKey, KeyText);
					}), ET66ButtonType::Primary)
					.SetMinWidth(100.f)
					.SetPadding(FMargin(12.f, 6.f))
					.SetColor(FT66Style::Tokens::Accent2)
				)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
			[
				FT66Style::MakeButton(
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
					.SetMinWidth(100.f)
					.SetPadding(FMargin(12.f, 6.f))
					.SetColor(FT66Style::Tokens::Danger)
				)
			];

		ControlKeyTextMap.Add(RowKey, KeyText);
		return Row;
	};

	auto MakeRow = [this, Loc, MakeBindingCell](const FText& Label, bool bAxis, FName Name, bool bIsController, float Scale = 1.f) -> TSharedRef<SWidget>
	{
		const FText PrimaryText = Loc ? Loc->GetText_Primary() : NSLOCTEXT("T66.Settings.Fallback", "PRIMARY", "PRIMARY");
		const FText SecondaryText = Loc ? Loc->GetText_Secondary() : NSLOCTEXT("T66.Settings.Fallback", "SECONDARY", "SECONDARY");

		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 10.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock).Text(Label)
					.Font(FT66Style::Tokens::FontRegular(13))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
					[
						SNew(STextBlock).Text(PrimaryText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
						SNew(STextBlock).Text(SecondaryText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						MakeBindingCell(bAxis, Name, Scale, bIsController, 1)
					]
				]
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
			.Font(FT66Style::Tokens::FontRegular(12))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().FillHeight(1.0f)
		[
			SAssignNew(ControlsDeviceSwitcher, SWidgetSwitcher)
			.WidgetIndex(0)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlMoveForward() : NSLOCTEXT("T66.Settings.Fallback", "Move Forward", "Move Forward"), true, FName(TEXT("MoveForward")), false, 1.f) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlMoveBack() : NSLOCTEXT("T66.Settings.Fallback", "Move Back", "Move Back"), true, FName(TEXT("MoveForward")), false, -1.f) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlMoveLeft() : NSLOCTEXT("T66.Settings.Fallback", "Move Left", "Move Left"), true, FName(TEXT("MoveRight")), false, -1.f) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlMoveRight() : NSLOCTEXT("T66.Settings.Fallback", "Move Right", "Move Right"), true, FName(TEXT("MoveRight")), false, 1.f) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlJump() : NSLOCTEXT("T66.Settings.Fallback", "Jump", "Jump"), false, FName(TEXT("Jump")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlInteract() : NSLOCTEXT("T66.Settings.Fallback", "Interact", "Interact"), false, FName(TEXT("Interact")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlPauseMenuPrimary() : NSLOCTEXT("T66.Settings.Fallback", "Pause Menu (primary)", "Pause Menu (primary)"), false, FName(TEXT("Escape")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlPauseMenuSecondary() : NSLOCTEXT("T66.Settings.Fallback", "Pause Menu (secondary)", "Pause Menu (secondary)"), false, FName(TEXT("Pause")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleHUD() : NSLOCTEXT("T66.Settings.Fallback", "Toggle HUD", "Toggle HUD"), false, FName(TEXT("ToggleHUD")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleTikTok() : NSLOCTEXT("T66.Settings.Fallback", "Toggle TikTok", "Toggle TikTok"), false, FName(TEXT("ToggleTikTok")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlOpenFullMap() : NSLOCTEXT("T66.Settings.Fallback", "Open Full Map", "Open Full Map"), false, FName(TEXT("OpenFullMap")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleMediaViewer() : NSLOCTEXT("T66.Settings.Fallback", "Toggle Media Viewer", "Toggle Media Viewer"), false, FName(TEXT("ToggleMediaViewer")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleGamerMode() : NSLOCTEXT("T66.Settings.Fallback", "Toggle Gamer Mode (Hitboxes)", "Toggle Gamer Mode (Hitboxes)"), false, FName(TEXT("ToggleGamerMode")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlRestartRun() : NSLOCTEXT("T66.Settings.Fallback", "Restart Run", "Restart Run"), false, FName(TEXT("RestartRun")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlDash() : NSLOCTEXT("T66.Settings.Fallback", "Dash", "Dash"), false, FName(TEXT("Dash")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlUltimate() : NSLOCTEXT("T66.Settings.Fallback", "Ultimate", "Ultimate"), false, FName(TEXT("Ultimate")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlAttackLock() : NSLOCTEXT("T66.Settings.Fallback", "Attack Lock", "Attack Lock"), false, FName(TEXT("AttackLock")), false) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlAttackUnlock() : NSLOCTEXT("T66.Settings.Fallback", "Attack Unlock", "Attack Unlock"), false, FName(TEXT("AttackUnlock")), false) ]
				]
			]
			+ SWidgetSwitcher::Slot()
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlMoveForward() : NSLOCTEXT("T66.Settings.Fallback", "Move Forward", "Move Forward"), true, FName(TEXT("MoveForward")), true, 1.f) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlMoveBack() : NSLOCTEXT("T66.Settings.Fallback", "Move Back", "Move Back"), true, FName(TEXT("MoveForward")), true, -1.f) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlMoveLeft() : NSLOCTEXT("T66.Settings.Fallback", "Move Left", "Move Left"), true, FName(TEXT("MoveRight")), true, -1.f) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlMoveRight() : NSLOCTEXT("T66.Settings.Fallback", "Move Right", "Move Right"), true, FName(TEXT("MoveRight")), true, 1.f) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlJump() : NSLOCTEXT("T66.Settings.Fallback", "Jump", "Jump"), false, FName(TEXT("Jump")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlInteract() : NSLOCTEXT("T66.Settings.Fallback", "Interact", "Interact"), false, FName(TEXT("Interact")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlPauseMenuPrimary() : NSLOCTEXT("T66.Settings.Fallback", "Pause Menu (primary)", "Pause Menu (primary)"), false, FName(TEXT("Escape")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlPauseMenuSecondary() : NSLOCTEXT("T66.Settings.Fallback", "Pause Menu (secondary)", "Pause Menu (secondary)"), false, FName(TEXT("Pause")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleHUD() : NSLOCTEXT("T66.Settings.Fallback", "Toggle HUD", "Toggle HUD"), false, FName(TEXT("ToggleHUD")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleTikTok() : NSLOCTEXT("T66.Settings.Fallback", "Toggle TikTok", "Toggle TikTok"), false, FName(TEXT("ToggleTikTok")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlOpenFullMap() : NSLOCTEXT("T66.Settings.Fallback", "Open Full Map", "Open Full Map"), false, FName(TEXT("OpenFullMap")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleMediaViewer() : NSLOCTEXT("T66.Settings.Fallback", "Toggle Media Viewer", "Toggle Media Viewer"), false, FName(TEXT("ToggleMediaViewer")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleGamerMode() : NSLOCTEXT("T66.Settings.Fallback", "Toggle Gamer Mode (Hitboxes)", "Toggle Gamer Mode (Hitboxes)"), false, FName(TEXT("ToggleGamerMode")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlRestartRun() : NSLOCTEXT("T66.Settings.Fallback", "Restart Run", "Restart Run"), false, FName(TEXT("RestartRun")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlDash() : NSLOCTEXT("T66.Settings.Fallback", "Dash", "Dash"), false, FName(TEXT("Dash")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlUltimate() : NSLOCTEXT("T66.Settings.Fallback", "Ultimate", "Ultimate"), false, FName(TEXT("Ultimate")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlAttackLock() : NSLOCTEXT("T66.Settings.Fallback", "Attack Lock", "Attack Lock"), false, FName(TEXT("AttackLock")), true) ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlAttackUnlock() : NSLOCTEXT("T66.Settings.Fallback", "Attack Unlock", "Attack Unlock"), false, FName(TEXT("AttackUnlock")), true) ]
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f)
		[
			FT66Style::MakeButton(Loc ? Loc->GetText_RestoreDefaults() : NSLOCTEXT("T66.Settings.Fallback", "Restore Defaults", "RESTORE DEFAULTS"),
				FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleRestoreDefaultsClicked), ET66ButtonType::Danger, 180.f)
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildAudioTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	auto MakeSliderRow = [this](const FText& Label, TFunction<float()> GetValue, TFunction<void(float)> SetValue) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.3f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label)
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().FillWidth(0.55f).VAlign(VAlign_Center).Padding(10.0f, 0.0f)
				[
					SNew(SSlider)
					.Value_Lambda([GetValue]() { return GetValue(); })
					.OnValueChanged_Lambda([SetValue](float V) { SetValue(V); })
				]
				+ SHorizontalBox::Slot().FillWidth(0.15f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([GetValue]()
					{
						const int32 Percent = FMath::RoundToInt(GetValue() * 100.0f);
						return FText::FromString(FString::Printf(TEXT("%d%%"), Percent));
					})
					.Font(FT66Style::Tokens::FontRegular(13))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Right)
				]
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSliderRow(
				Loc ? Loc->GetText_MasterVolume() : NSLOCTEXT("T66.Settings.Fallback", "Master Volume", "Master Volume"),
				[PS]() { return PS ? PS->GetMasterVolume() : 0.8f; },
				[PS](float V) { if (PS) PS->SetMasterVolume(V); }
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSliderRow(
				Loc ? Loc->GetText_MusicVolume() : NSLOCTEXT("T66.Settings.Fallback", "Music Volume", "Music Volume"),
				[PS]() { return PS ? PS->GetMusicVolume() : 0.6f; },
				[PS](float V) { if (PS) PS->SetMusicVolume(V); }
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSliderRow(
				Loc ? Loc->GetText_SfxVolume() : NSLOCTEXT("T66.Settings.Fallback", "SFX Volume", "SFX Volume"),
				[PS]() { return PS ? PS->GetSfxVolume() : 0.8f; },
				[PS](float V) { if (PS) PS->SetSfxVolume(V); }
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 8.f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_MuteWhenUnfocused() : NSLOCTEXT("T66.Settings.Fallback", "Mute when unfocused", "Mute when unfocused"))
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					FT66Style::MakeButton(
						FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([PS]() { if (PS) PS->SetMuteWhenUnfocused(!PS->GetMuteWhenUnfocused()); return FReply::Handled(); }), ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetColor(TAttribute<FSlateColor>::CreateLambda([PS]() -> FSlateColor
						{
							const bool bOn = PS ? PS->GetMuteWhenUnfocused() : false;
							return FSlateColor(bOn ? FT66Style::Tokens::Success : FT66Style::Tokens::Danger);
						}))
						.SetDynamicLabel(TAttribute<FText>::CreateLambda([Loc, PS]() -> FText
						{
							const bool bOn = PS ? PS->GetMuteWhenUnfocused() : false;
							return bOn ? (Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON")) : (Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF"));
						}))
					)
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.4f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_OutputDevice() : NSLOCTEXT("T66.Settings.Fallback", "Output Device", "Output Device"))
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().FillWidth(0.6f)
				[
					FT66Style::MakeDropdown(FT66DropdownParams(
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(FMargin(10.0f, 4.0f))
							[
								SNew(STextBlock)
								.Text_Lambda([PS, Loc]()
								{
									if (!PS || PS->GetOutputDeviceId().IsEmpty()) return NSLOCTEXT("T66.Settings.Fallback", "Default", "Default");
									return FText::FromString(PS->GetOutputDeviceId());
								})
								.Font(FT66Style::Tokens::FontRegular(12))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(4.0f, 0.0f))
							[
								SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "DropdownArrow", "▼"))
								.Font(FT66Style::Tokens::FontRegular(10))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							],
						[PS, Loc]()
						{
							const FText DefaultLabel = NSLOCTEXT("T66.Settings.Fallback", "Default", "Default");
							return SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									FT66Style::MakeButton(
										FT66ButtonParams(DefaultLabel, FOnClicked::CreateLambda([PS]() { if (PS) PS->SetOutputDeviceId(FString()); return FReply::Handled(); }), ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetColor(FT66Style::Tokens::Panel2)
									)
								];
						}).SetHeight(32.0f))
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_SubtitlesAlwaysOn() : NSLOCTEXT("T66.Settings.Fallback", "Subtitles: always on", "Subtitles: always on"))
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					FT66Style::MakeButton(
						FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([PS]() { if (PS) PS->SetSubtitlesAlwaysOn(!PS->GetSubtitlesAlwaysOn()); return FReply::Handled(); }), ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetColor(TAttribute<FSlateColor>::CreateLambda([PS]() -> FSlateColor
						{
							const bool bOn = PS ? PS->GetSubtitlesAlwaysOn() : false;
							return FSlateColor(bOn ? FT66Style::Tokens::Success : FT66Style::Tokens::Danger);
						}))
						.SetDynamicLabel(TAttribute<FText>::CreateLambda([Loc, PS]() -> FText
						{
							const bool bOn = PS ? PS->GetSubtitlesAlwaysOn() : false;
							return bOn ? (Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON")) : (Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF"));
						}))
					)
				]
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildCrashingTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	return SNew(SVerticalBox)
		// Header
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 20.0f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_CrashingChecklistHeader() : NSLOCTEXT("T66.Settings.Fallback", "CrashingChecklistHeader", "If your game is crashing, try these steps:"))
			.Font(FT66Style::Tokens::FontBold(16))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		// Checklist
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(20.0f, 15.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_CrashingChecklistBody() : FText::GetEmpty())
					.Font(FT66Style::Tokens::FontRegular(13))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.AutoWrapText(true)
				]
			]
		]
		// Safe Mode button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 15.0f, 0.0f, 20.0f)
		[
			FT66Style::MakeButton(Loc ? Loc->GetText_ApplySafeModeSettings() : NSLOCTEXT("T66.Settings.Fallback", "Apply Safe Mode Settings", "Apply Safe Mode Settings"),
				FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleSafeModeClicked), ET66ButtonType::Danger, 220.f)
		]
		// Separator
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 20.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			[
				SNew(SBox).HeightOverride(1.0f)
			]
		]
		// Report Bug section
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_StillHavingIssues() : NSLOCTEXT("T66.Settings.Fallback", "Still having issues?", "Still having issues?"))
			.Font(FT66Style::Tokens::FontBold(14))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_ReportBugDescription() : NSLOCTEXT("T66.Settings.Fallback", "ReportBugDescription", "Report a bug to help us fix it. Your report will include basic system info."))
			.Font(FT66Style::Tokens::FontRegular(12))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			FT66Style::MakeButton(Loc ? Loc->GetText_ReportBug() : NSLOCTEXT("T66.ReportBug", "Title", "REPORT BUG"),
				FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleReportBugClicked), ET66ButtonType::Primary, 150.f)
		];
}

FReply UT66SettingsScreen::HandleCloseClicked()
{
	OnCloseClicked();
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleTabClicked(ET66SettingsTab Tab)
{
	SwitchToTab(Tab);
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleApplyGraphicsClicked()
{
	OnApplyGraphicsClicked();
	return FReply::Handled();
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
		}
	}
	RefreshControlsKeyTexts();
	if (RebindStatusText.IsValid())
	{
		RebindStatusText->SetText(NSLOCTEXT("T66.Settings", "DefaultsRestored", "Defaults restored."));
	}
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleReportBugClicked()
{
	ShowModal(ET66ScreenType::ReportBug);
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleSafeModeClicked()
{
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->ApplySafeModeSettings();
	}
	return FReply::Handled();
}

void UT66SettingsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	// Remember last tab (Bible 1.17).
	int32 TabIdx = 0;
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		TabIdx = PS->GetLastSettingsTabIndex();
	}
	CurrentTab = static_cast<ET66SettingsTab>(FMath::Clamp(TabIdx, 0, 4));
	OnTabChanged(CurrentTab);
	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(CurrentTab));
	}

	SetKeyboardFocus();
}

void UT66SettingsScreen::SwitchToTab(ET66SettingsTab Tab)
{
	CurrentTab = Tab;
	OnTabChanged(Tab);
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->SetLastSettingsTabIndex(static_cast<int32>(Tab));
	}
	// Switch content via the widget switcher (buttons update via lambdas)
	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(Tab));
	}
}

void UT66SettingsScreen::OnApplyGraphicsClicked()
{
	ApplyPendingGraphics(false);
}

void UT66SettingsScreen::OnCloseClicked()
{
	// If a video-mode confirm is active, closing should not keep the new settings.
	if (bVideoModeConfirmActive)
	{
		EndVideoModeConfirmPrompt(false);
	}

	CloseModal();

	// In gameplay, Settings is opened from Pause Menu. Our UIManager is single-modal, so showing Settings
	// replaces Pause Menu. When Settings closes, re-open Pause Menu so the player can resume/unpause.
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && PC->IsPaused())
		{
			ShowModal(ET66ScreenType::PauseMenu);
		}
	}
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

void UT66SettingsScreen::InitializeGraphicsFromUserSettingsIfNeeded()
{
	if (bGraphicsInitialized) return;
	bGraphicsInitialized = true;

	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		PendingGraphics.Resolution = GUS->GetScreenResolution();
		PendingGraphics.WindowMode = GUS->GetFullscreenMode();
		PendingGraphics.QualityNotch = FMath::Clamp(GUS->GetOverallScalabilityLevel(), 0, 3);
		const float Limit = GUS->GetFrameRateLimit();
		if (Limit <= 0.0f) PendingGraphics.FpsCapIndex = 4;
		else if (Limit <= 30.0f) PendingGraphics.FpsCapIndex = 0;
		else if (Limit <= 60.0f) PendingGraphics.FpsCapIndex = 1;
		else if (Limit <= 90.0f) PendingGraphics.FpsCapIndex = 2;
		else PendingGraphics.FpsCapIndex = 3;
	}
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PendingGraphics.DisplayMode = (PS->GetDisplayModeIndex() == 1) ? ET66DisplayMode::Widescreen : ET66DisplayMode::Standard;
		PendingGraphics.MonitorIndex = PS->GetMonitorIndex();
	}
	else
	{
		PendingGraphics.DisplayMode = ET66DisplayMode::Standard;
		PendingGraphics.MonitorIndex = 0;
	}
	PendingGraphics.bDirty = false;
}

void UT66SettingsScreen::ApplyPendingGraphics(bool bForceConfirmPrompt)
{
	if (!PendingGraphics.bDirty && !bForceConfirmPrompt) return;

	UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GUS) return;

	const FIntPoint OldRes = GUS->GetScreenResolution();
	const EWindowMode::Type OldMode = GUS->GetFullscreenMode();

	GUS->SetScreenResolution(PendingGraphics.Resolution);
	GUS->SetFullscreenMode(PendingGraphics.WindowMode);
	GUS->SetOverallScalabilityLevel(FMath::Clamp(PendingGraphics.QualityNotch, 0, 3));

	float NewLimit = 60.0f;
	switch (PendingGraphics.FpsCapIndex)
	{
	case 0: NewLimit = 30.0f; break;
	case 1: NewLimit = 60.0f; break;
	case 2: NewLimit = 90.0f; break;
	case 3: NewLimit = 120.0f; break;
	default: NewLimit = 0.0f; break; // Unlimited
	}
	GUS->SetFrameRateLimit(NewLimit);

	GUS->ApplySettings(false);

	// Persist display mode (Standard/Widescreen) for future viewport/camera use.
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->SetDisplayModeIndex(static_cast<int32>(PendingGraphics.DisplayMode));
		PS->SetMonitorIndex(PendingGraphics.MonitorIndex);
	}

	// Move game window to selected monitor when multiple monitors are available.
	FDisplayMetrics Metrics;
	FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
	if (Metrics.MonitorInfo.IsValidIndex(PendingGraphics.MonitorIndex))
	{
		const FMonitorInfo& Mon = Metrics.MonitorInfo[PendingGraphics.MonitorIndex];
		TArray<TSharedRef<SWindow>> TopLevelWindows = FSlateApplication::Get().GetTopLevelWindows();
		if (TopLevelWindows.Num() > 0)
		{
			TSharedRef<SWindow> GameWindow = TopLevelWindows[0];
			const FVector2D Pos(static_cast<float>(Mon.WorkArea.Left), static_cast<float>(Mon.WorkArea.Top));
			GameWindow->MoveWindowTo(Pos);
		}
	}

	const bool bNeedsConfirm = bForceConfirmPrompt || (OldRes != PendingGraphics.Resolution) || (OldMode != PendingGraphics.WindowMode);
	if (bNeedsConfirm)
	{
		StartVideoModeConfirmPrompt();
	}
	else
	{
		GUS->SaveSettings();
		PendingGraphics.bDirty = false;
	}
}

void UT66SettingsScreen::StartVideoModeConfirmPrompt()
{
	if (bVideoModeConfirmActive) return;
	bVideoModeConfirmActive = true;
	VideoModeConfirmSecondsRemaining = 10;

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		if (VideoModeConfirmCountdownText.IsValid())
		{
			VideoModeConfirmCountdownText->SetText(FText::Format(Loc->GetText_KeepTheseSettingsBodyFormat(), FText::AsNumber(VideoModeConfirmSecondsRemaining)));
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VideoModeConfirmTimerHandle);
		FTimerDelegate TickDelegate;
		TickDelegate.BindLambda([this]()
		{
			VideoModeConfirmSecondsRemaining = FMath::Max(0, VideoModeConfirmSecondsRemaining - 1);
			if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
			{
				if (VideoModeConfirmCountdownText.IsValid())
				{
					VideoModeConfirmCountdownText->SetText(FText::Format(Loc->GetText_KeepTheseSettingsBodyFormat(), FText::AsNumber(VideoModeConfirmSecondsRemaining)));
				}
			}
			if (VideoModeConfirmSecondsRemaining <= 0)
			{
				EndVideoModeConfirmPrompt(false);
			}
		});
		World->GetTimerManager().SetTimer(VideoModeConfirmTimerHandle, TickDelegate, 1.0f, true);
	}
}

void UT66SettingsScreen::EndVideoModeConfirmPrompt(bool bKeepNewSettings)
{
	if (!bVideoModeConfirmActive) return;
	bVideoModeConfirmActive = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VideoModeConfirmTimerHandle);
	}

	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		if (bKeepNewSettings)
		{
			GUS->ConfirmVideoMode();
			GUS->SaveSettings();
			PendingGraphics.bDirty = false;
		}
		else
		{
			GUS->RevertVideoMode();
			GUS->ApplySettings(false);
			// Sync pending back to current applied.
			PendingGraphics.Resolution = GUS->GetScreenResolution();
			PendingGraphics.WindowMode = GUS->GetFullscreenMode();
			PendingGraphics.bDirty = false;
		}
	}
}
