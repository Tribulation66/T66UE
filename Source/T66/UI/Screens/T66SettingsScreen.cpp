// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
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
#include "Misc/DefaultValueHelper.h"
#include "UI/Style/T66Style.h"
#include "UI/Dota/T66DotaTheme.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SOverlay.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RetroFXUI, Log, All);

namespace
{
	const FLinearColor ClassicSettingsPageBackground(0.64f, 0.64f, 0.64f, 1.0f);
	const FLinearColor ClassicSettingsPageText(0.08f, 0.08f, 0.08f, 1.0f);
	const FLinearColor ClassicSettingsPageMuted(0.35f, 0.35f, 0.35f, 1.0f);
	const FLinearColor ClassicRetroButtonBackground(1.0f, 1.0f, 1.0f, 1.0f);
	const FLinearColor ClassicRetroButtonSelectedBackground(0.70f, 0.70f, 0.70f, 1.0f);
	const FLinearColor ClassicRetroButtonOutline(0.10f, 0.10f, 0.10f, 1.0f);
	const FLinearColor ClassicRetroButtonText(0.05f, 0.05f, 0.05f, 1.0f);

	FLinearColor GetSettingsPageBackground()
	{
		return FT66Style::IsDotaTheme() ? FT66DotaTheme::ScreenBackground() : ClassicSettingsPageBackground;
	}

	FLinearColor GetSettingsPageText()
	{
		return FT66Style::IsDotaTheme() ? FT66DotaTheme::ScreenText() : ClassicSettingsPageText;
	}

	FLinearColor GetSettingsPageMuted()
	{
		return FT66Style::IsDotaTheme() ? FT66DotaTheme::ScreenMuted() : ClassicSettingsPageMuted;
	}

	FLinearColor GetRetroButtonBackground()
	{
		return FT66Style::IsDotaTheme() ? FT66DotaTheme::ButtonNeutral() : ClassicRetroButtonBackground;
	}

	FLinearColor GetRetroButtonSelectedBackground()
	{
		return FT66Style::IsDotaTheme() ? FT66DotaTheme::SelectionFill() : ClassicRetroButtonSelectedBackground;
	}

	FLinearColor GetRetroButtonOutline()
	{
		return FT66Style::IsDotaTheme() ? FT66DotaTheme::Border() : ClassicRetroButtonOutline;
	}

	FLinearColor GetRetroButtonText()
	{
		return FT66Style::IsDotaTheme() ? FT66DotaTheme::Text() : ClassicRetroButtonText;
	}

	FText FormatRetroPercent(float Value)
	{
		const float ClampedValue = FMath::Clamp(Value, 0.0f, 100.0f);
		if (FMath::IsNearlyEqual(ClampedValue, FMath::RoundToFloat(ClampedValue)))
		{
			return FText::AsNumber(FMath::RoundToInt(ClampedValue));
		}
		return FText::FromString(FString::Printf(TEXT("%.2f"), ClampedValue));
	}
}

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

	// Selected tab uses ToggleActive for pressed effect; rebuild on tab change so type updates.
	auto MakeTabButton = [this](const FText& Text, ET66SettingsTab Tab) -> TSharedRef<SWidget>
	{
		const bool bIsSelected = (CurrentTab == Tab);
		return SNew(SBox).Padding(FMargin(2.0f, 4.0f))
			[
				FT66Style::MakeButton(
					FT66ButtonParams(Text, FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, Tab), bIsSelected ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(12.f, 8.f))
					.SetColor(TAttribute<FSlateColor>::CreateLambda([this, Tab]() -> FSlateColor {
						bool bSel = (CurrentTab == Tab);
						return bSel ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2;
					}))
					.SetContent(SNew(STextBlock).Text(Text)
						.Font(FT66Style::Tokens::FontBold(22))
						.ColorAndOpacity(FT66Style::Tokens::Text))
				)
			];
	};

	// Build the widget switcher with all tab content (stored as class member)
	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetCachedDisplayMetrics(DisplayMetrics);
	const float PanelW = 0.8f * static_cast<float>(DisplayMetrics.PrimaryDisplayWidth);
	const float PanelH = 0.8f * static_cast<float>(DisplayMetrics.PrimaryDisplayHeight);

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Tokens::Scrim)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(0.0f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
							.WidthOverride(PanelW)
							.HeightOverride(PanelH)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(GetSettingsPageBackground())
								.Padding(0.0f)
								[
					SNew(SVerticalBox)
					// Tab bar + close button (extra vertical padding so tab buttons are not clipped)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 6.0f)
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
								MakeTabButton(Loc ? Loc->GetText_SettingsTabHUD() : NSLOCTEXT("T66.Settings", "TabHUD", "HUD"), ET66SettingsTab::HUD)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(Loc ? Loc->GetText_SettingsTabMediaViewer() : NSLOCTEXT("T66.Settings", "TabMediaViewer", "TIKTOK & SHORTS"), ET66SettingsTab::MediaViewer)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(Loc ? Loc->GetText_SettingsTabAudio() : NSLOCTEXT("T66.Settings", "TabAudio", "AUDIO"), ET66SettingsTab::Audio)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(Loc ? Loc->GetText_SettingsTabCrashing() : NSLOCTEXT("T66.Settings", "TabCrashing", "CRASHING"), ET66SettingsTab::Crashing)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(NSLOCTEXT("T66.Settings", "TabRetroFX", "RETRO FX"), ET66SettingsTab::RetroFX)
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
									.Font(FT66Style::Tokens::FontBold(36))
									.ColorAndOpacity(FT66Style::Tokens::Text))
							)
							],
							FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(15.0f, 14.0f))
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
							BuildHUDTab()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildMediaViewerTab()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildAudioTab()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildCrashingTab()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildRetroFXTab()
						]
					]
								]
						]
					]
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
						.Font(FT66Style::Tokens::FontRegular(28))
						.ColorAndOpacity(GetSettingsPageText())
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						FT66Style::MakeButton(
							FT66ButtonParams(OnText, FOnClicked::CreateLambda([this, SetValue]() { SetValue(true); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }), GetValue() ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([GetValue]() -> FSlateColor { return GetValue() ? FT66Style::Tokens::Success : FT66Style::Tokens::Panel2; }))
						)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(OffText, FOnClicked::CreateLambda([this, SetValue]() { SetValue(false); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }), !GetValue() ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([GetValue]() -> FSlateColor { return !GetValue() ? FT66Style::Tokens::Danger : FT66Style::Tokens::Panel2; }))
						)
					]
				]
			];
	};

	auto MakeNumericRow = [this](const FText& Label, const FText& Description, TFunction<float()> GetPercent, TFunction<void(float)> SetPercent) -> TSharedRef<SWidget>
	{
		auto CommitNumericValue = [this, SetPercent](const FText& NewText)
		{
			const FString TrimmedText = NewText.ToString().TrimStartAndEnd();
			float ParsedValue = 0.0f;
			if (TrimmedText.IsEmpty())
			{
				SetPercent(0.0f);
			}
			else if (FDefaultValueHelper::ParseFloat(TrimmedText, ParsedValue))
			{
				SetPercent(FMath::Clamp(ParsedValue, 0.0f, 100.0f));
			}
			FT66Style::DeferRebuild(this, 0);
		};

		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.46f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Label)
						.Font(FT66Style::Tokens::FontRegular(28))
						.ColorAndOpacity(GetSettingsPageText())
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 18.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(Description)
						.Font(FT66Style::Tokens::FontRegular(20))
						.ColorAndOpacity(GetSettingsPageMuted())
						.AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot().FillWidth(0.54f).VAlign(VAlign_Center).Padding(10.0f, 0.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox)
						.WidthOverride(170.0f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(GetRetroButtonOutline())
							.Padding(1.0f)
							[
								SNew(SEditableTextBox)
								.Text(FormatRetroPercent(GetPercent()))
								.Font(FT66Style::Tokens::FontRegular(24))
								.ForegroundColor(GetRetroButtonText())
								.BackgroundColor(FLinearColor::White)
								.Justification(ETextJustify::Center)
								.HintText(NSLOCTEXT("T66.Settings", "GameplayNumericHint", "0-100"))
								.OnTextCommitted_Lambda([CommitNumericValue](const FText& NewText, ETextCommit::Type)
								{
									CommitNumericValue(NewText);
								})
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Settings", "GameplayNumericHelp", "Enter a value from 0 to 100."))
						.Font(FT66Style::Tokens::FontRegular(18))
						.ColorAndOpacity(GetSettingsPageMuted())
						.AutoWrapText(true)
					]
				]
			];
	};

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
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
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(
					NSLOCTEXT("T66.Settings", "NativeFogIntensityLabel", "Native Fog Intensity"),
					NSLOCTEXT("T66.Settings", "NativeFogIntensityBody", "Controls the strength of the gameplay haze from 0 to 100. 0 disables native fog entirely, 100 is intentionally very heavy."),
					[PS]() { return PS ? PS->GetFogIntensityPercent() : 55.0f; },
					[PS](float Value) { if (PS) PS->SetFogIntensityPercent(Value); }
				)
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildGraphicsTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	InitializeGraphicsFromUserSettingsIfNeeded();

	auto MakeDropdownRow = [this](const FText& Label, TFunction<FText()> GetCurrentValue, TFunction<TSharedRef<SWidget>()> MakeMenuContent) -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> TriggerContent = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(FMargin(10.0f, 4.0f))
			[
				SNew(STextBlock).Text_Lambda([GetCurrentValue]() { return GetCurrentValue(); })
				.Font(FT66Style::Tokens::FontRegular(24))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(4.0f, 0.0f))
			[
				SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "DropdownArrow", "???"))
				.Font(FT66Style::Tokens::FontRegular(20))
				.ColorAndOpacity(GetSettingsPageMuted())
			];
		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.4f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label)
					.Font(FT66Style::Tokens::FontRegular(28))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SHorizontalBox::Slot().FillWidth(0.6f)
				[
					FT66Style::MakeDropdown(FT66DropdownParams(TriggerContent, MakeMenuContent).SetHeight(0))
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
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(SBox).MinDesiredWidth(200.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Settings", "UiTheme", "UI Theme"))
						.Font(FT66Style::Tokens::FontRegular(28))
						.ColorAndOpacity(GetSettingsPageText())
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						FT66Style::MakeButton(
							FT66ButtonParams(NSLOCTEXT("T66.Settings", "UiThemeClassic", "CLASSIC"),
								FOnClicked::CreateLambda([PS]()
								{
									if (PS)
									{
										PS->SetUITheme(ET66UITheme::Classic);
									}
									return FReply::Handled();
								}),
								(PS && PS->GetUITheme() == ET66UITheme::Classic) ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
							.SetMinWidth(140.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([PS]() -> FSlateColor
							{
								return (PS && PS->GetUITheme() == ET66UITheme::Classic) ? FT66Style::Tokens::Success : FT66Style::Tokens::Panel2;
							}))
						)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(NSLOCTEXT("T66.Settings", "UiThemeDota", "DOTA"),
								FOnClicked::CreateLambda([PS]()
								{
									if (PS)
									{
										PS->SetUITheme(ET66UITheme::Dota);
									}
									return FReply::Handled();
								}),
								(PS && PS->GetUITheme() == ET66UITheme::Dota) ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
							.SetMinWidth(140.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([PS]() -> FSlateColor
							{
								return (PS && PS->GetUITheme() == ET66UITheme::Dota) ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2;
							}))
						)
					]
				]
			]
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
						.Font(FT66Style::Tokens::FontRegular(22))
						.ColorAndOpacity(GetSettingsPageMuted())
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)
					[
						SNew(STextBlock).Text(Loc ? Loc->GetText_BestQuality() : NSLOCTEXT("T66.Settings.Fallback", "Best Quality", "Best Quality"))
						.Font(FT66Style::Tokens::FontRegular(22))
						.ColorAndOpacity(GetSettingsPageMuted())
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
		// Fog toggle
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(SBox).MinDesiredWidth(200.0f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_SettingsFog() : NSLOCTEXT("T66.Settings", "Fog", "Fog"))
						.Font(FT66Style::Tokens::FontRegular(28))
						.ColorAndOpacity(GetSettingsPageText())
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						FT66Style::MakeButton(
							FT66ButtonParams(Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON"), FOnClicked::CreateLambda([this, PS]() { if (PS) PS->SetFogEnabled(true); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }), (PS && PS->GetFogEnabled()) ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([PS]() -> FSlateColor { return (PS && PS->GetFogEnabled()) ? FT66Style::Tokens::Success : FT66Style::Tokens::Panel2; }))
						)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF"), FOnClicked::CreateLambda([this, PS]() { if (PS) PS->SetFogEnabled(false); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }), (PS && !PS->GetFogEnabled()) ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([PS]() -> FSlateColor { return (PS && !PS->GetFogEnabled()) ? FT66Style::Tokens::Danger : FT66Style::Tokens::Panel2; }))
						)
					]
				]
			]
		]
		// Apply button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(Loc ? Loc->GetText_Apply() : NSLOCTEXT("T66.Settings.Fallback", "APPLY", "APPLY"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleApplyGraphicsClicked), ET66ButtonType::Success)
				.SetFontSize(32).SetMinWidth(120.f))
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
					.Font(FT66Style::Tokens::FontBold(36))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 15.f)
				[
					SAssignNew(VideoModeConfirmCountdownText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontRegular(26))
					.ColorAndOpacity(GetSettingsPageMuted())
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
				.SetFontSize(32)
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
					.Font(FT66Style::Tokens::FontBold(24))
					.ColorAndOpacity(GetSettingsPageText())
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
					.Font(FT66Style::Tokens::FontRegular(26))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
					[
						SNew(STextBlock).Text(PrimaryText).Font(FT66Style::Tokens::FontRegular(20)).ColorAndOpacity(GetSettingsPageMuted())
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
						SNew(STextBlock).Text(SecondaryText).Font(FT66Style::Tokens::FontRegular(20)).ColorAndOpacity(GetSettingsPageMuted())
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
			.Font(FT66Style::Tokens::FontRegular(24))
			.ColorAndOpacity(GetSettingsPageMuted())
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
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[ MakeRow(Loc ? Loc->GetText_ControlToggleMouseLock() : NSLOCTEXT("T66.Settings.Fallback", "Toggle Mouse Lock", "Toggle Mouse Lock"), false, FName(TEXT("ToggleMouseLock")), false) ]
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
			FT66Style::MakeButton(
				FT66ButtonParams(Loc ? Loc->GetText_RestoreDefaults() : NSLOCTEXT("T66.Settings.Fallback", "Restore Defaults", "RESTORE DEFAULTS"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleRestoreDefaultsClicked), ET66ButtonType::Danger)
				.SetFontSize(32).SetMinWidth(180.f))
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildHUDTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	auto MakeToggleRow = [this, Loc](const FText& Label, TFunction<bool()> GetValue, TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		const FText OnText = Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON");
		const FText OffText = Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF");
		const bool bOn = GetValue();

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
						.Font(FT66Style::Tokens::FontRegular(28))
						.ColorAndOpacity(GetSettingsPageText())
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						FT66Style::MakeButton(
							FT66ButtonParams(OnText, FOnClicked::CreateLambda([this, SetValue]() { SetValue(true); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }), bOn ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([GetValue]() -> FSlateColor { return GetValue() ? FT66Style::Tokens::Success : FT66Style::Tokens::Panel2; }))
						)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(OffText, FOnClicked::CreateLambda([this, SetValue]() { SetValue(false); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }), !bOn ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetPadding(FMargin(12.f, 6.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([GetValue]() -> FSlateColor { return !GetValue() ? FT66Style::Tokens::Danger : FT66Style::Tokens::Panel2; }))
						)
					]
				]
			];
	};

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SettingsHudToggleIntro() : NSLOCTEXT("T66.Settings", "HudToggleIntro", "When you press the HUD toggle key, the following elements will show or hide:"))
				.Font(FT66Style::Tokens::FontRegular(26))
				.ColorAndOpacity(GetSettingsPageText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					Loc ? Loc->GetText_SettingsHudInventory() : NSLOCTEXT("T66.Settings", "HudInventory", "Inventory"),
					[PS]() { return PS ? PS->GetHudToggleAffectsInventory() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsInventory(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					Loc ? Loc->GetText_SettingsHudMinimap() : NSLOCTEXT("T66.Settings", "HudMinimap", "Minimap"),
					[PS]() { return PS ? PS->GetHudToggleAffectsMinimap() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsMinimap(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					Loc ? Loc->GetText_SettingsHudIdolSlots() : NSLOCTEXT("T66.Settings", "HudIdolSlots", "Idol slots"),
					[PS]() { return PS ? PS->GetHudToggleAffectsIdolSlots() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsIdolSlots(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					Loc ? Loc->GetText_SettingsHudPortraitStats() : NSLOCTEXT("T66.Settings", "HudPortraitStats", "Portrait & stats"),
					[PS]() { return PS ? PS->GetHudToggleAffectsPortraitStats() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsPortraitStats(b); }
				)
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildMediaViewerTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	const FText OnText = Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON");
	const FText OffText = Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF");
	const bool bMediaOn = PS ? PS->GetMediaViewerEnabled() : true;

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SettingsMediaViewerPrivacyBody() : NSLOCTEXT("T66.Settings", "MediaViewerPrivacyBody", "The Media Viewer runs only on your computer. We do not receive or store the videos you watch or any data from TikTok or YouTube. To open it, use the key bound to \"Toggle Media Viewer\" or \"Toggle TikTok\" in the Controls tab."))
				.Font(FT66Style::Tokens::FontRegular(26))
				.ColorAndOpacity(GetSettingsPageText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 16.0f, 0.0f, 8.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FT66Style::Tokens::Panel2)
				.Padding(FMargin(15.0f, 12.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
					[
						SNew(SBox).MinDesiredWidth(200.0f)
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_SettingsMediaViewerEnable() : NSLOCTEXT("T66.Settings", "MediaViewerEnable", "Enable Media Viewer (TikTok / YouTube Shorts)"))
							.Font(FT66Style::Tokens::FontRegular(28))
							.ColorAndOpacity(GetSettingsPageText())
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							FT66Style::MakeButton(
								FT66ButtonParams(OnText, FOnClicked::CreateLambda([this, PS]() { if (PS) PS->SetMediaViewerEnabled(true); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }), bMediaOn ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
								.SetMinWidth(100.f)
								.SetPadding(FMargin(12.f, 6.f))
								.SetColor(TAttribute<FSlateColor>::CreateLambda([PS]() -> FSlateColor { return (PS && PS->GetMediaViewerEnabled()) ? FT66Style::Tokens::Success : FT66Style::Tokens::Panel2; }))
							)
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(OffText, FOnClicked::CreateLambda([this, PS]() { if (PS) PS->SetMediaViewerEnabled(false); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }), !bMediaOn ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
								.SetMinWidth(100.f)
								.SetPadding(FMargin(12.f, 6.f))
								.SetColor(TAttribute<FSlateColor>::CreateLambda([PS]() -> FSlateColor { return (PS && !PS->GetMediaViewerEnabled()) ? FT66Style::Tokens::Danger : FT66Style::Tokens::Panel2; }))
							)
						]
					]
				]
			]
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
					.Font(FT66Style::Tokens::FontRegular(28))
					.ColorAndOpacity(GetSettingsPageText())
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
					.Font(FT66Style::Tokens::FontRegular(26))
					.ColorAndOpacity(GetSettingsPageText())
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
					.Font(FT66Style::Tokens::FontRegular(28))
					.ColorAndOpacity(GetSettingsPageText())
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
					.Font(FT66Style::Tokens::FontRegular(28))
					.ColorAndOpacity(GetSettingsPageText())
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
								.Font(FT66Style::Tokens::FontRegular(24))
								.ColorAndOpacity(GetSettingsPageText())
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(4.0f, 0.0f))
							[
								SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "DropdownArrow", "???"))
								.Font(FT66Style::Tokens::FontRegular(20))
								.ColorAndOpacity(GetSettingsPageMuted())
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
						}).SetHeight(0))
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
					.Font(FT66Style::Tokens::FontRegular(28))
					.ColorAndOpacity(GetSettingsPageText())
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

TSharedRef<SWidget> UT66SettingsScreen::BuildRetroFXTab()
{
	InitializeRetroFXFromUserSettingsIfNeeded();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText OnText = Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON");
	const FText OffText = Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF");

	auto MakeSectionHeader = [](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(FT66Style::Tokens::FontBold(28))
			.ColorAndOpacity(FT66Style::Tokens::Accent2);
	};

	auto MakeRetroButton = [this](const FText& Label, TFunction<bool()> IsSelected, FOnClicked OnClicked, float MinWidth = 88.0f) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.MinDesiredWidth(MinWidth)
			[
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.ContentPadding(FMargin(0.0f))
				.OnClicked(MoveTemp(OnClicked))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(GetRetroButtonOutline())
					.Padding(1.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor_Lambda([IsSelected]()
						{
							return IsSelected() ? GetRetroButtonSelectedBackground() : GetRetroButtonBackground();
						})
						.Padding(FMargin(12.0f, 6.0f))
						[
							SNew(STextBlock)
							.Text(Label)
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(GetRetroButtonText())
							.Justification(ETextJustify::Center)
						]
					]
				]
			];
	};

	auto MakeNumericRow = [this](const FText& Label, const FText& Description, TFunction<float()> GetPercent, TFunction<void(float)> SetPercent) -> TSharedRef<SWidget>
	{
		auto CommitNumericValue = [this, SetPercent](const FText& NewText)
		{
			const FString TrimmedText = NewText.ToString().TrimStartAndEnd();
			float ParsedValue = 0.0f;
			if (TrimmedText.IsEmpty())
			{
				SetPercent(0.0f);
			}
			else if (FDefaultValueHelper::ParseFloat(TrimmedText, ParsedValue))
			{
				SetPercent(FMath::Clamp(ParsedValue, 0.0f, 100.0f));
			}
			FT66Style::DeferRebuild(this, 0);
		};

		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.46f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Label)
						.Font(FT66Style::Tokens::FontRegular(28))
						.ColorAndOpacity(GetSettingsPageText())
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 18.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(Description)
						.Font(FT66Style::Tokens::FontRegular(20))
						.ColorAndOpacity(GetSettingsPageMuted())
						.AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot().FillWidth(0.54f).VAlign(VAlign_Center).Padding(10.0f, 0.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox)
						.WidthOverride(170.0f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(GetRetroButtonOutline())
							.Padding(1.0f)
							[
								SNew(SEditableTextBox)
								.Text(FormatRetroPercent(GetPercent()))
								.Font(FT66Style::Tokens::FontRegular(24))
								.ForegroundColor(GetRetroButtonText())
								.BackgroundColor(FLinearColor::White)
								.Justification(ETextJustify::Center)
								.HintText(NSLOCTEXT("T66.Settings", "RetroFXPercentHint", "0-100"))
								.OnTextCommitted_Lambda([CommitNumericValue](const FText& NewText, ETextCommit::Type)
								{
									CommitNumericValue(NewText);
								})
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Settings", "RetroFXNumericHint", "Enter a value from 0 to 100."))
						.Font(FT66Style::Tokens::FontRegular(18))
						.ColorAndOpacity(GetSettingsPageMuted())
						.AutoWrapText(true)
					]
				]
			];
	};

	auto MakeToggleRow = [this, &MakeRetroButton, OnText, OffText](const FText& Label, const FText& Description, TFunction<bool()> GetValue, TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Label)
						.Font(FT66Style::Tokens::FontRegular(28))
						.ColorAndOpacity(GetSettingsPageText())
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 18.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(Description)
						.Font(FT66Style::Tokens::FontRegular(20))
						.ColorAndOpacity(GetSettingsPageMuted())
						.AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeRetroButton(
							OnText,
							[GetValue]() { return GetValue(); },
							FOnClicked::CreateLambda([this, SetValue]() { SetValue(true); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }),
							100.0f)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						MakeRetroButton(
							OffText,
							[GetValue]() { return !GetValue(); },
							FOnClicked::CreateLambda([this, SetValue]() { SetValue(false); FT66Style::DeferRebuild(this, 0); return FReply::Handled(); }),
							100.0f)
					]
				]
			];
	};

	auto MakeApplyButton = [this, &MakeRetroButton]() -> TSharedRef<SWidget>
	{
		return MakeRetroButton(
			NSLOCTEXT("T66.Settings", "RetroFXApply", "APPLY"),
			[]() { return false; },
			FOnClicked::CreateLambda([this]()
			{
				return HandleApplyRetroFXClicked();
			}),
			180.0f);
	};

	auto MakeActionRow = [this, &MakeRetroButton, &MakeApplyButton](bool bIncludeReset) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				bIncludeReset
					? StaticCastSharedRef<SWidget>(MakeRetroButton(
						NSLOCTEXT("T66.Settings", "RetroFXResetDefaults", "RESET TO DEFAULTS"),
						[]() { return false; },
						FOnClicked::CreateLambda([this]()
						{
							return HandleResetRetroFXClicked();
						}),
						220.0f))
					: SNullWidget::NullWidget
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(bIncludeReset ? FMargin(8.0f, 0.0f, 0.0f, 0.0f) : FMargin(0.0f))
			[
				MakeApplyButton()
			];
	};

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Settings", "RetroFXHeader", "Retro FX"))
				.Font(FT66Style::Tokens::FontBold(32))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 14.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Settings", "RetroFXBodyExpanded", "This panel drives the currently wired UE5RFX screen-space stack and the safe retro geometry swap path for T66's world and character unlit materials. Scalar settings use direct 0-100 numeric input, while binary settings use ON and OFF toggles. The master toggle disables the entire Retro FX stack without erasing your saved values. PS1 sub-settings require PS1 Master Blend above 0 to show on screen."))
				.Font(FT66Style::Tokens::FontRegular(22))
				.ColorAndOpacity(GetSettingsPageMuted())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(
					NSLOCTEXT("T66.Settings", "RetroFXMasterEnableLabel", "Retro FX Master Enable"),
					NSLOCTEXT("T66.Settings", "RetroFXMasterEnableBody", "Turns the entire Retro FX stack on or off without changing the individual values below."),
					[this]() { return PendingRetroFXSettings.bEnableRetroFXMaster; },
					[this](bool bValue) { PendingRetroFXSettings.bEnableRetroFXMaster = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(bRetroFXDirty ? NSLOCTEXT("T66.Settings", "RetroFXPendingDirty", "Pending changes have not been applied yet.") : NSLOCTEXT("T66.Settings", "RetroFXPendingClean", "Pending values match the saved Retro FX profile."))
				.Font(FT66Style::Tokens::FontRegular(20))
				.ColorAndOpacity(bRetroFXDirty ? FT66Style::Tokens::Accent2 : GetSettingsPageMuted())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 0.0f, 0.0f, 18.0f)
			[
				MakeActionRow(false)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionPS1", "PS1 Stack"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1BlendLabel", "PS1 Master Blend"), NSLOCTEXT("T66.Settings", "RetroFXPs1BlendBody", "Overall blend weight for the UE5RFX PS1 post-process stack."), [this]() { return PendingRetroFXSettings.PS1BlendPercent; }, [this](float V) { PendingRetroFXSettings.PS1BlendPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1DitherLabel", "PS1 Dithering"), NSLOCTEXT("T66.Settings", "RetroFXPs1DitherBody", "Controls the strength of the imported UE5RFX PS1 dithering pattern."), [this]() { return PendingRetroFXSettings.PS1DitheringPercent; }, [this](float V) { PendingRetroFXSettings.PS1DitheringPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1BayerLabel", "PS1 Bayer Dithering"), NSLOCTEXT("T66.Settings", "RetroFXPs1BayerBody", "Switches the PS1 post-process stack to the imported Bayer dithering path."), [this]() { return PendingRetroFXSettings.PS1BayerDitheringPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.PS1BayerDitheringPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1ColorLUTLabel", "PS1 Color LUT"), NSLOCTEXT("T66.Settings", "RetroFXPs1ColorLUTBody", "Enables or disables the imported UE5RFX color LUT stage. Color Boost only shows when this is enabled."), [this]() { return PendingRetroFXSettings.PS1ColorLUTPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.PS1ColorLUTPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1ColorBoostLabel", "PS1 Color Boost"), NSLOCTEXT("T66.Settings", "RetroFXPs1ColorBoostBody", "Strength of the imported UE5RFX PS1 LUT color boost."), [this]() { return PendingRetroFXSettings.PS1ColorBoostPercent; }, [this](float V) { PendingRetroFXSettings.PS1ColorBoostPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionWorldGeometry", "World Geometry"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXWorldGeometryEnableLabel", "World Geometry Enable"), NSLOCTEXT("T66.Settings", "RetroFXWorldGeometryEnableBody", "Turns the safe runtime retro-geometry swap on or off for world and environment materials that inherit from T66's shared unlit masters."), [this]() { return PendingRetroFXSettings.bEnableWorldGeometry; }, [this](bool bValue) { PendingRetroFXSettings.bEnableWorldGeometry = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapLabel", "World Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapBody", "Strength for geometry snapping on world and environment materials patched into the retro stack."), [this]() { return PendingRetroFXSettings.WorldVertexSnapPercent; }, [this](float V) { PendingRetroFXSettings.WorldVertexSnapPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapResLabel", "World Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexSnapResBody", "Higher values lower the target snap resolution, making the world wobble and step more aggressively."), [this]() { return PendingRetroFXSettings.WorldVertexSnapResolutionPercent; }, [this](float V) { PendingRetroFXSettings.WorldVertexSnapResolutionPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldVertexNoiseLabel", "World Vertex Noise"), NSLOCTEXT("T66.Settings", "RetroFXWorldVertexNoiseBody", "Adds extra world-position noise on top of snapping for rougher retro surfaces."), [this]() { return PendingRetroFXSettings.WorldVertexNoisePercent; }, [this](float V) { PendingRetroFXSettings.WorldVertexNoisePercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineBlendLabel", "World Affine Blend"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineBlendBody", "Blends world UVs into the affine mapping path. 0 keeps stable UVs, 100 fully commits to the retro perspective error."), [this]() { return PendingRetroFXSettings.WorldAffineBlendPercent; }, [this](float V) { PendingRetroFXSettings.WorldAffineBlendPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD1Label", "World Affine Distance 1"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD1Body", "Near-distance threshold for world affine mapping."), [this]() { return PendingRetroFXSettings.WorldAffineDistance1Percent; }, [this](float V) { PendingRetroFXSettings.WorldAffineDistance1Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD2Label", "World Affine Distance 2"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD2Body", "Mid-distance threshold for world affine mapping."), [this]() { return PendingRetroFXSettings.WorldAffineDistance2Percent; }, [this](float V) { PendingRetroFXSettings.WorldAffineDistance2Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD3Label", "World Affine Distance 3"), NSLOCTEXT("T66.Settings", "RetroFXWorldAffineD3Body", "Far-distance threshold for world affine mapping."), [this]() { return PendingRetroFXSettings.WorldAffineDistance3Percent; }, [this](float V) { PendingRetroFXSettings.WorldAffineDistance3Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionCharacterGeometry", "Character Geometry"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterGeometryEnableLabel", "Character Geometry Enable"), NSLOCTEXT("T66.Settings", "RetroFXCharacterGeometryEnableBody", "Turns the safe runtime retro-geometry swap on or off for character-facing materials that inherit from T66's shared unlit masters."), [this]() { return PendingRetroFXSettings.bEnableCharacterGeometry; }, [this](bool bValue) { PendingRetroFXSettings.bEnableCharacterGeometry = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapLabel", "Character Vertex Snap Strength"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapBody", "Strength for geometry snapping on character-facing unlit materials."), [this]() { return PendingRetroFXSettings.CharacterVertexSnapPercent; }, [this](float V) { PendingRetroFXSettings.CharacterVertexSnapPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapResLabel", "Character Vertex Snap Resolution"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexSnapResBody", "Higher values lower the target snap resolution, making hero and enemy geometry wobble more aggressively."), [this]() { return PendingRetroFXSettings.CharacterVertexSnapResolutionPercent; }, [this](float V) { PendingRetroFXSettings.CharacterVertexSnapResolutionPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexNoiseLabel", "Character Vertex Noise"), NSLOCTEXT("T66.Settings", "RetroFXCharacterVertexNoiseBody", "Adds extra character-space noise on top of snapping. Keep it lower if you want readable combat silhouettes."), [this]() { return PendingRetroFXSettings.CharacterVertexNoisePercent; }, [this](float V) { PendingRetroFXSettings.CharacterVertexNoisePercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineBlendLabel", "Character Affine Blend"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineBlendBody", "Blends character UVs into the affine mapping path. Push this carefully to avoid readability loss in combat."), [this]() { return PendingRetroFXSettings.CharacterAffineBlendPercent; }, [this](float V) { PendingRetroFXSettings.CharacterAffineBlendPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD1Label", "Character Affine Distance 1"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD1Body", "Near-distance threshold for character affine mapping."), [this]() { return PendingRetroFXSettings.CharacterAffineDistance1Percent; }, [this](float V) { PendingRetroFXSettings.CharacterAffineDistance1Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD2Label", "Character Affine Distance 2"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD2Body", "Mid-distance threshold for character affine mapping."), [this]() { return PendingRetroFXSettings.CharacterAffineDistance2Percent; }, [this](float V) { PendingRetroFXSettings.CharacterAffineDistance2Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD3Label", "Character Affine Distance 3"), NSLOCTEXT("T66.Settings", "RetroFXCharacterAffineD3Body", "Far-distance threshold for character affine mapping."), [this]() { return PendingRetroFXSettings.CharacterAffineDistance3Percent; }, [this](float V) { PendingRetroFXSettings.CharacterAffineDistance3Percent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionN64", "N64 Blur"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXN64BlendLabel", "N64 Blur Blend"), NSLOCTEXT("T66.Settings", "RetroFXN64BlendBody", "Overall blend weight for the UE5RFX N64 blur material."), [this]() { return PendingRetroFXSettings.N64BlurBlendPercent; }, [this](float V) { PendingRetroFXSettings.N64BlurBlendPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXN64StepsLabel", "N64 Blur Steps"), NSLOCTEXT("T66.Settings", "RetroFXN64StepsBody", "Higher values increase the blur sample count in the N64 pass."), [this]() { return PendingRetroFXSettings.N64BlurStepsPercent; }, [this](float V) { PendingRetroFXSettings.N64BlurStepsPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXN64LowResLabel", "N64 Low Fake Resolution"), NSLOCTEXT("T66.Settings", "RetroFXN64LowResBody", "Enables or disables the low-fake-resolution path used by the N64 blur pass."), [this]() { return PendingRetroFXSettings.N64LowFakeResolutionPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.N64LowFakeResolutionPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXN64ReplaceTMLabel", "N64 Replace Tonemapper"), NSLOCTEXT("T66.Settings", "RetroFXN64ReplaceTMBody", "Switch between the standard N64 blur material and the replace-tonemapper variant."), [this]() { return PendingRetroFXSettings.bUseUE5RFXN64BlurReplaceTonemapper; }, [this](bool bValue) { PendingRetroFXSettings.bUseUE5RFXN64BlurReplaceTonemapper = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionChromatic", "Chromatic Aberration And Pixelation"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticStrengthLabel", "Chromatic Aberration Strength"), NSLOCTEXT("T66.Settings", "RetroFXChromaticStrengthBody", "Controls the radial RGB split strength for the custom chromatic-aberration post-process pass."), [this]() { return PendingRetroFXSettings.ChromaticAberrationPercent; }, [this](float V) { PendingRetroFXSettings.ChromaticAberrationPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticDistortionLabel", "Distortion Strength"), NSLOCTEXT("T66.Settings", "RetroFXChromaticDistortionBody", "Controls the radial screen distortion strength used by the chromatic-aberration pass."), [this]() { return PendingRetroFXSettings.ChromaticDistortionPercent; }, [this](float V) { PendingRetroFXSettings.ChromaticDistortionPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXChromaticInvertLabel", "Invert Distortion"), NSLOCTEXT("T66.Settings", "RetroFXChromaticInvertBody", "Flips the radial distortion direction used by the chromatic-aberration pass."), [this]() { return PendingRetroFXSettings.bInvertChromaticDistortion; }, [this](bool bValue) { PendingRetroFXSettings.bInvertChromaticDistortion = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 18.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPixelationLabel", "T66 Pixelation"), NSLOCTEXT("T66.Settings", "RetroFXPixelationBody", "Drives the existing T66 pixelation subsystem from Off to full strength."), [this]() { return PendingRetroFXSettings.T66PixelationPercent; }, [this](float V) { PendingRetroFXSettings.T66PixelationPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionResolution", "Shared Resolution"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXRealLowResLabel", "Real Low Resolution"), NSLOCTEXT("T66.Settings", "RetroFXRealLowResBody", "Lowers the actual runtime screen percentage for a stronger low-resolution look. This is the most aggressive shared-resolution mode."), [this]() { return PendingRetroFXSettings.bUseRealLowResolution; }, [this](bool bValue) { PendingRetroFXSettings.bUseRealLowResolution = bValue; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXResSizeLabel", "Fake Resolution Size Switch"), NSLOCTEXT("T66.Settings", "RetroFXResSizeBody", "Enables or disables the fake screen-size resolution switch."), [this]() { return PendingRetroFXSettings.FakeResolutionSwitchSizePercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.FakeResolutionSwitchSizePercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXResUVLabel", "Fake Resolution UV Switch"), NSLOCTEXT("T66.Settings", "RetroFXResUVBody", "Enables or disables the fake UV resolution switch."), [this]() { return PendingRetroFXSettings.FakeResolutionSwitchUVPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.FakeResolutionSwitchUVPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXTargetResLabel", "Target Resolution Height"), NSLOCTEXT("T66.Settings", "RetroFXTargetResBody", "Higher values drive the target height lower, which makes the scene feel more aggressively low-res."), [this]() { return PendingRetroFXSettings.TargetResolutionHeightPercent; }, [this](float V) { PendingRetroFXSettings.TargetResolutionHeightPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f)
			[
				MakeSectionHeader(NSLOCTEXT("T66.Settings", "RetroFXSectionFog", "PS1 Fog"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogLabel", "PS1 Fog Enable"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogBody", "Enables or disables the visible PS1 fog contribution by gating its density."), [this]() { return PendingRetroFXSettings.PS1FogPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.PS1FogPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(NSLOCTEXT("T66.Settings", "RetroFXPs1SceneDepthFogLabel", "PS1 Scene Depth Fog"), NSLOCTEXT("T66.Settings", "RetroFXPs1SceneDepthFogBody", "Switches the fog calculation to the imported scene-depth-based PS1 fog path."), [this]() { return PendingRetroFXSettings.PS1SceneDepthFogPercent > KINDA_SMALL_NUMBER; }, [this](bool bValue) { PendingRetroFXSettings.PS1SceneDepthFogPercent = bValue ? 100.0f : 0.0f; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogDensityLabel", "PS1 Fog Density"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogDensityBody", "Controls how thick the UE5RFX fog becomes once fog is enabled."), [this]() { return PendingRetroFXSettings.PS1FogDensityPercent; }, [this](float V) { PendingRetroFXSettings.PS1FogDensityPercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogStartLabel", "PS1 Fog Start Distance"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogStartBody", "Higher values pull the fog closer to the camera for a heavier retro atmosphere."), [this]() { return PendingRetroFXSettings.PS1FogStartDistancePercent; }, [this](float V) { PendingRetroFXSettings.PS1FogStartDistancePercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 18.0f)
			[
				MakeNumericRow(NSLOCTEXT("T66.Settings", "RetroFXPs1FogFalloffLabel", "PS1 Fog Falloff"), NSLOCTEXT("T66.Settings", "RetroFXPs1FogFalloffBody", "Higher values tighten the fog falloff distance for a denser wall of haze."), [this]() { return PendingRetroFXSettings.PS1FogFallOffDistancePercent; }, [this](float V) { PendingRetroFXSettings.PS1FogFallOffDistancePercent = V; bRetroFXDirty = true; })
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
			[
				MakeActionRow(true)
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
			.Font(FT66Style::Tokens::FontBold(32))
			.ColorAndOpacity(GetSettingsPageText())
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
					.Font(FT66Style::Tokens::FontRegular(26))
					.ColorAndOpacity(GetSettingsPageText())
					.AutoWrapText(true)
				]
			]
		]
		// Safe Mode button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 15.0f, 0.0f, 20.0f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(Loc ? Loc->GetText_ApplySafeModeSettings() : NSLOCTEXT("T66.Settings.Fallback", "Apply Safe Mode Settings", "Apply Safe Mode Settings"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleSafeModeClicked), ET66ButtonType::Danger)
				.SetFontSize(32).SetMinWidth(220.f))
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
			.Font(FT66Style::Tokens::FontBold(28))
			.ColorAndOpacity(GetSettingsPageText())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_ReportBugDescription() : NSLOCTEXT("T66.Settings.Fallback", "ReportBugDescription", "Report a bug to help us fix it. Your report will include basic system info."))
			.Font(FT66Style::Tokens::FontRegular(24))
			.ColorAndOpacity(GetSettingsPageMuted())
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(Loc ? Loc->GetText_ReportBug() : NSLOCTEXT("T66.ReportBug", "Title", "REPORT BUG"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleReportBugClicked), ET66ButtonType::Primary)
				.SetFontSize(32).SetMinWidth(150.f))
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
	// Rebuild so tab bar shows ToggleActive (pressed) on the selected tab.
	FT66Style::DeferRebuild(this, 0);
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

FReply UT66SettingsScreen::HandleApplyRetroFXClicked()
{
	UE_LOG(LogT66RetroFXUI, Log, TEXT("HandleApplyRetroFXClicked: dirty=%s world=%s"), bRetroFXDirty ? TEXT("true") : TEXT("false"), *GetNameSafe(GetWorld()));
	ApplyPendingRetroFX();
	FT66Style::DeferRebuild(this, 0);
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleResetRetroFXClicked()
{
	ResetPendingRetroFXToDefaults();
	FT66Style::DeferRebuild(this, 0);
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
	CurrentTab = static_cast<ET66SettingsTab>(FMath::Clamp(TabIdx, 0, 7));
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

void UT66SettingsScreen::InitializeRetroFXFromUserSettingsIfNeeded()
{
	if (bRetroFXInitialized) return;
	bRetroFXInitialized = true;

	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PendingRetroFXSettings = PS->GetRetroFXSettings();
	}
	else
	{
		PendingRetroFXSettings = FT66RetroFXSettings();
	}

	bRetroFXDirty = false;
}

void UT66SettingsScreen::ApplyPendingRetroFX()
{
	InitializeRetroFXFromUserSettingsIfNeeded();

	UE_LOG(LogT66RetroFXUI, Log,
		TEXT("ApplyPendingRetroFX: begin dirty=%s world=%s MasterEnabled=%s PS1Blend=%.2f PS1Dither=%.2f PS1Bayer=%.2f PS1ColorLUT=%.2f PS1ColorBoost=%.2f ChromaticStrength=%.2f DistortionStrength=%.2f InvertDistortion=%s PS1FogEnable=%.2f PS1SceneDepthFog=%.2f PS1FogDensity=%.2f PS1FogStart=%.2f PS1FogFalloff=%.2f RealLowRes=%s FakeSize=%.2f FakeUV=%.2f TargetRes=%.2f"),
		bRetroFXDirty ? TEXT("true") : TEXT("false"),
		*GetNameSafe(GetWorld()),
		PendingRetroFXSettings.bEnableRetroFXMaster ? TEXT("true") : TEXT("false"),
		PendingRetroFXSettings.PS1BlendPercent,
		PendingRetroFXSettings.PS1DitheringPercent,
		PendingRetroFXSettings.PS1BayerDitheringPercent,
		PendingRetroFXSettings.PS1ColorLUTPercent,
		PendingRetroFXSettings.PS1ColorBoostPercent,
		PendingRetroFXSettings.ChromaticAberrationPercent,
		PendingRetroFXSettings.ChromaticDistortionPercent,
		PendingRetroFXSettings.bInvertChromaticDistortion ? TEXT("true") : TEXT("false"),
		PendingRetroFXSettings.PS1FogPercent,
		PendingRetroFXSettings.PS1SceneDepthFogPercent,
		PendingRetroFXSettings.PS1FogDensityPercent,
		PendingRetroFXSettings.PS1FogStartDistancePercent,
		PendingRetroFXSettings.PS1FogFallOffDistancePercent,
		PendingRetroFXSettings.bUseRealLowResolution ? TEXT("true") : TEXT("false"),
		PendingRetroFXSettings.FakeResolutionSwitchSizePercent,
		PendingRetroFXSettings.FakeResolutionSwitchUVPercent,
		PendingRetroFXSettings.TargetResolutionHeightPercent);

	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		UE_LOG(LogT66RetroFXUI, Log, TEXT("ApplyPendingRetroFX: saving pending settings to UT66PlayerSettingsSubsystem"));
		PS->SetRetroFXSettings(PendingRetroFXSettings);

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
			{
				UE_LOG(LogT66RetroFXUI, Log, TEXT("ApplyPendingRetroFX: directly applying pending settings to runtime subsystem"));
				RetroFX->ApplySettings(PendingRetroFXSettings, GetWorld());
			}
			else
			{
				UE_LOG(LogT66RetroFXUI, Warning, TEXT("ApplyPendingRetroFX: Retro FX subsystem was null"));
			}
		}
		else
		{
			UE_LOG(LogT66RetroFXUI, Warning, TEXT("ApplyPendingRetroFX: GameInstance was null"));
		}

		bRetroFXDirty = false;
		UE_LOG(LogT66RetroFXUI, Log, TEXT("ApplyPendingRetroFX: completed successfully"));
	}
	else
	{
		UE_LOG(LogT66RetroFXUI, Warning, TEXT("ApplyPendingRetroFX: Player settings subsystem was null"));
	}
}

void UT66SettingsScreen::ResetPendingRetroFXToDefaults()
{
	InitializeRetroFXFromUserSettingsIfNeeded();
	PendingRetroFXSettings = FT66RetroFXSettings();
	bRetroFXDirty = true;
}

