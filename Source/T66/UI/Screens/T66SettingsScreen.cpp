// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SComboBox.h"
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

TSharedRef<SWidget> UT66SettingsScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	FText SettingsText = Loc ? Loc->GetText_Settings() : FText::FromString(TEXT("SETTINGS"));

	// Use dynamic lambda for button color so it updates when tab changes
	auto MakeTabButton = [this](const FText& Text, ET66SettingsTab Tab) -> TSharedRef<SWidget>
	{
		return SNew(SBox).HeightOverride(40.0f).Padding(FMargin(2.0f, 0.0f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, Tab))
				.ButtonColorAndOpacity_Lambda([this, Tab]() -> FSlateColor {
					bool bIsSelected = (CurrentTab == Tab);
					return bIsSelected ? FLinearColor(0.25f, 0.4f, 0.7f, 1.0f) : FLinearColor(0.12f, 0.12f, 0.18f, 1.0f);
				})
				[
					SNew(STextBlock).Text(Text)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
					.ColorAndOpacity(FLinearColor::White)
				]
			];
	};

	// Build the widget switcher with all tab content (stored as class member)

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.WidthOverride(950.0f)
			.HeightOverride(700.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.1f, 1.0f))
				.Padding(FMargin(0.0f))
				[
					SNew(SVerticalBox)
					// Tab bar + close button
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBorder)
						.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.14f, 1.0f))
						.Padding(FMargin(15.0f, 10.0f))
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(FText::FromString(TEXT("GAMEPLAY")), ET66SettingsTab::Gameplay)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(FText::FromString(TEXT("GRAPHICS")), ET66SettingsTab::Graphics)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(FText::FromString(TEXT("CONTROLS")), ET66SettingsTab::Controls)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(FText::FromString(TEXT("AUDIO")), ET66SettingsTab::Audio)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeTabButton(FText::FromString(TEXT("CRASHING")), ET66SettingsTab::Crashing)
							]
							+ SHorizontalBox::Slot().FillWidth(1.0f)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox).WidthOverride(40.0f).HeightOverride(40.0f)
								[
									SNew(SButton)
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseClicked))
									.ButtonColorAndOpacity(FLinearColor(0.5f, 0.2f, 0.2f, 1.0f))
									[
										SNew(STextBlock).Text(FText::FromString(TEXT("X")))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
										.ColorAndOpacity(FLinearColor::White)
									]
								]
							]
						]
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
		if (K == EKeys::Escape)
		{
			bWaitingForRebind = false;
			if (RebindStatusText.IsValid())
			{
				RebindStatusText->SetText(FText::FromString(TEXT("Rebind cancelled.")));
			}
			return FReply::Handled();
		}
		ApplyRebindToInputSettings(K);
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
	return K.IsValid() ? K.GetDisplayName() : FText::FromString(TEXT("-"));
}

bool UT66SettingsScreen::IsKeyboardMouseKey(const FKey& K)
{
	if (!K.IsValid()) return false;
	if (K.IsMouseButton()) return true;
	// Treat any non-gamepad, non-touch key as "keyboard/mouse" for our purposes.
	return !K.IsGamepadKey() && !K.IsTouch();
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

FReply UT66SettingsScreen::BeginRebindAction(FName ActionName, const FKey& OldKey, TSharedPtr<STextBlock> KeyText)
{
	bWaitingForRebind = true;
	Pending = {};
	Pending.bIsAxis = false;
	Pending.Name = ActionName;
	Pending.Scale = 1.f;
	Pending.OldKey = OldKey;
	Pending.KeyText = KeyText;
	if (RebindStatusText.IsValid())
	{
		RebindStatusText->SetText(FText::FromString(FString::Printf(TEXT("Press a key for %s (Esc cancels)"), *ActionName.ToString())));
	}
	return FReply::Handled();
}

FReply UT66SettingsScreen::BeginRebindAxis(FName AxisName, float Scale, const FKey& OldKey, TSharedPtr<STextBlock> KeyText)
{
	bWaitingForRebind = true;
	Pending = {};
	Pending.bIsAxis = true;
	Pending.Name = AxisName;
	Pending.Scale = Scale;
	Pending.OldKey = OldKey;
	Pending.KeyText = KeyText;
	if (RebindStatusText.IsValid())
	{
		RebindStatusText->SetText(FText::FromString(FString::Printf(TEXT("Press a key for %s (Esc cancels)"), *AxisName.ToString())));
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
		RebindStatusText->SetText(FText::FromString(TEXT("Rebind saved.")));
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
		Pending.KeyText->SetText(FText::FromString(TEXT("-")));
	}
}

TSharedRef<SWidget> UT66SettingsScreen::BuildGameplayTab()
{
	auto MakeToggleRow = [](const FText& Label, bool bDefaultOn) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBox).WidthOverride(60.0f).HeightOverride(32.0f)
						[
							SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
							.ButtonColorAndOpacity(bDefaultOn ? FLinearColor(0.2f, 0.5f, 0.3f, 1.0f) : FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("ON")))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(60.0f).HeightOverride(32.0f)
						[
							SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
							.ButtonColorAndOpacity(!bDefaultOn ? FLinearColor(0.5f, 0.2f, 0.2f, 1.0f) : FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("OFF")))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
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
				MakeToggleRow(FText::FromString(TEXT("Intense Visuals")), true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(FText::FromString(TEXT("Auto Sprint")), false)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(FText::FromString(TEXT("Submit Scores to Leaderboard")), true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(FText::FromString(TEXT("Screen Shake")), true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeToggleRow(FText::FromString(TEXT("Camera Smoothing")), true)
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildGraphicsTab()
{
	auto MakeDropdownRow = [](const FText& Label, const FText& CurrentValue) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.4f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot().FillWidth(0.6f)
				[
					SNew(SBox).HeightOverride(32.0f)
					[
						SNew(SBorder)
						.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.18f, 1.0f))
						.Padding(FMargin(10.0f, 0.0f))
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(CurrentValue)
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
								.ColorAndOpacity(FLinearColor::White)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("\u25BC")))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
								.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
							]
						]
					]
				]
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(FText::FromString(TEXT("Monitor")), FText::FromString(TEXT("Primary Monitor")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(FText::FromString(TEXT("Resolution")), FText::FromString(TEXT("1920 x 1080")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(FText::FromString(TEXT("Window Mode")), FText::FromString(TEXT("Fullscreen")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(FText::FromString(TEXT("Display Mode")), FText::FromString(TEXT("Standard")))
		]
		// Quality slider
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(STextBlock).Text(FText::FromString(TEXT("Best Performance")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)
					[
						SNew(STextBlock).Text(FText::FromString(TEXT("Best Quality")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					SNew(SSlider)
					.Value(0.75f)
					.StepSize(0.25f)
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeDropdownRow(FText::FromString(TEXT("FPS Cap")), FText::FromString(TEXT("60")))
		]
		// Apply button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f)
		[
			SNew(SBox).WidthOverride(120.0f).HeightOverride(40.0f)
			[
				SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleApplyGraphicsClicked))
				.ButtonColorAndOpacity(FLinearColor(0.2f, 0.5f, 0.3f, 1.0f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("APPLY")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildControlsTab()
{
	auto MakeRow = [this](const FText& Label, bool bAxis, FName Name, float Scale = 1.f) -> TSharedRef<SWidget>
	{
		const FKey OldKey = bAxis ? FindAxisKey(Name, Scale, FKey()) : FindActionKey(Name, FKey());
		TSharedPtr<STextBlock> KeyText;

		return SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
			.Padding(FMargin(15.0f, 10.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.45f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot().FillWidth(0.20f).Padding(5.0f, 0.0f).VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
					.Padding(FMargin(8.f, 4.f))
					[
						SAssignNew(KeyText, STextBlock)
						.Text(KeyToText(OldKey))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
				+ SHorizontalBox::Slot().FillWidth(0.20f).Padding(5.0f, 0.0f)
				[
					SNew(SBox).HeightOverride(30.0f)
					[
						SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
						.OnClicked_Lambda([this, bAxis, Name, Scale, OldKey, KeyText]()
						{
							return bAxis ? BeginRebindAxis(Name, Scale, OldKey, KeyText) : BeginRebindAction(Name, OldKey, KeyText);
						})
						.ButtonColorAndOpacity(FLinearColor(0.25f, 0.4f, 0.7f, 1.0f))
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("Rebind")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
				+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(5.0f, 0.0f)
				[
					SNew(SBox).HeightOverride(30.0f)
					[
						SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
						.OnClicked_Lambda([this, bAxis, Name, Scale, OldKey, KeyText]()
						{
							Pending = {};
							Pending.bIsAxis = bAxis;
							Pending.Name = Name;
							Pending.Scale = Scale;
							Pending.OldKey = OldKey;
							Pending.KeyText = KeyText;
							ClearBindingInInputSettings();
							return FReply::Handled();
						})
						.ButtonColorAndOpacity(FLinearColor(0.3f, 0.2f, 0.2f, 1.0f))
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("Clear")))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SAssignNew(RebindStatusText, STextBlock)
			.Text(FText::FromString(TEXT("Click Rebind, then press a key (Esc cancels).")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.85f, 1.f))
		]
		+ SVerticalBox::Slot().FillHeight(1.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Move Forward")), true, FName(TEXT("MoveForward")), 1.f) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Move Back")), true, FName(TEXT("MoveForward")), -1.f) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Move Left")), true, FName(TEXT("MoveRight")), -1.f) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Move Right")), true, FName(TEXT("MoveRight")), 1.f) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Jump")), false, FName(TEXT("Jump"))) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Interact")), false, FName(TEXT("Interact"))) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Pause Menu (primary)")), false, FName(TEXT("Escape"))) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Pause Menu (secondary)")), false, FName(TEXT("Pause"))) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Toggle HUD")), false, FName(TEXT("ToggleHUD"))) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Dash")), false, FName(TEXT("Dash"))) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Attack Lock")), false, FName(TEXT("AttackLock"))) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[ MakeRow(FText::FromString(TEXT("Attack Unlock")), false, FName(TEXT("AttackUnlock"))) ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f)
		[
			SNew(SBox).WidthOverride(160.0f).HeightOverride(40.0f)
			[
				SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleRestoreDefaultsClicked))
				.ButtonColorAndOpacity(FLinearColor(0.4f, 0.3f, 0.2f, 1.0f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Restore Defaults")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildAudioTab()
{
	auto MakeSliderRow = [](const FText& Label, float DefaultValue) -> TSharedRef<SWidget>
	{
		int32 Percent = FMath::RoundToInt(DefaultValue * 100);
		return SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.3f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot().FillWidth(0.55f).VAlign(VAlign_Center).Padding(10.0f, 0.0f)
				[
					SNew(SSlider)
					.Value(DefaultValue)
				]
				+ SHorizontalBox::Slot().FillWidth(0.15f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::Format(FText::FromString(TEXT("{0}%")), FText::AsNumber(Percent)))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Right)
				]
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSliderRow(FText::FromString(TEXT("Master Volume")), 0.8f)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSliderRow(FText::FromString(TEXT("Music Volume")), 0.6f)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSliderRow(FText::FromString(TEXT("SFX Volume")), 0.8f)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSliderRow(FText::FromString(TEXT("Voice Volume")), 1.0f)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSliderRow(FText::FromString(TEXT("Ambient Volume")), 0.7f)
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildCrashingTab()
{
	return SNew(SVerticalBox)
		// Header
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 20.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("If your game is crashing, try these steps:")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			.ColorAndOpacity(FLinearColor::White)
		]
		// Checklist
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
			.Padding(FMargin(20.0f, 15.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("1. Lower Quality to \"Best Performance\" in Graphics tab")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("2. Disable Intense Visuals in Gameplay tab")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("3. Reduce FPS Cap to 60 in Graphics tab")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("4. Try Windowed mode instead of Fullscreen")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
				]
			]
		]
		// Safe Mode button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 15.0f, 0.0f, 20.0f)
		[
			SNew(SBox).WidthOverride(220.0f).HeightOverride(45.0f)
			[
				SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleSafeModeClicked))
				.ButtonColorAndOpacity(FLinearColor(0.5f, 0.4f, 0.1f, 1.0f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Apply Safe Mode Settings")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		]
		// Separator
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 20.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f))
			[
				SNew(SBox).HeightOverride(1.0f)
			]
		]
		// Report Bug section
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Still having issues?")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.ColorAndOpacity(FLinearColor::White)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Report a bug to help us fix it. Your report will include basic system info.")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SBox).WidthOverride(150.0f).HeightOverride(45.0f)
			[
				SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleReportBugClicked))
				.ButtonColorAndOpacity(FLinearColor(0.3f, 0.4f, 0.6f, 1.0f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Report Bug")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
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
	UE_LOG(LogTemp, Log, TEXT("Restore Defaults clicked - not implemented"));
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleReportBugClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Report Bug clicked - not implemented"));
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleSafeModeClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Apply Safe Mode clicked - not implemented"));
	return FReply::Handled();
}

void UT66SettingsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	CurrentTab = ET66SettingsTab::Gameplay;
	OnTabChanged(CurrentTab);
	SetKeyboardFocus();
}

void UT66SettingsScreen::SwitchToTab(ET66SettingsTab Tab)
{
	CurrentTab = Tab;
	OnTabChanged(Tab);
	// Switch content via the widget switcher (buttons update via lambdas)
	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(Tab));
	}
}

void UT66SettingsScreen::OnApplyGraphicsClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Apply Graphics - not implemented"));
}

void UT66SettingsScreen::OnCloseClicked()
{
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
