// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RetroFXUI, Log, All);

using namespace T66SettingsScreenPrivate;

void UT66SettingsScreen::ConfigureAsRetroFXPreviewPopup(UT66UIManager* InUIManager)
{
	UIManager = InUIManager;
	ScreenType = ET66ScreenType::Settings;
	CurrentTab = ET66SettingsTab::RetroFX;
	bIsModal = false;
	bRetroFXPreviewPopup = true;
	bRetroFXPreviewMode = true;
	bRetroFXInitialized = false;
	bRetroFXDirty = false;
}

TSharedRef<SWidget> UT66SettingsScreen::BuildRetroFXTab()
{
	InitializeRetroFXFromUserSettingsIfNeeded();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	auto MakeSectionHeader = [](const FText& Text) -> TSharedRef<SWidget>
	{
		return MakeSettingsSectionHeader(Text, 22);
	};

	auto MakeRetroButton = [](const FText& Label, TFunction<bool()> IsSelected, FOnClicked OnClicked, float MinWidth = 88.0f) -> TSharedRef<SWidget>
	{
		return MakeSelectableSettingsButton(
			FT66ButtonParams(Label, MoveTemp(OnClicked), ET66ButtonType::Neutral)
			.SetMinWidth(MinWidth)
			.SetHeight(40.0f)
			.SetFontSize(18)
			.SetPadding(FMargin(12.0f, 6.0f))
			.SetTextColor(GetRetroButtonText()),
			MoveTemp(IsSelected),
			FSlateColor(GetRetroButtonSelectedBackground()),
			FSlateColor(GetRetroButtonBackground()));
	};

	auto MakeNumericRow = [](const FText& Label, const FText& Description, TFunction<float()> GetPercent, TFunction<void(float)> SetPercent) -> TSharedRef<SWidget>
	{
		return MakeSettingsPercentSliderRow(
			Label,
			Description,
			MoveTemp(GetPercent),
			MoveTemp(SetPercent),
			NSLOCTEXT("T66.Settings", "RetroFXSliderHint", "Slide from 0 to 100."));
	};

	FSettingsBoolToggleStyle RetroToggleStyle;
	RetroToggleStyle.ButtonMinWidth = 100.0f;
	RetroToggleStyle.ButtonFontSize = 18;
	RetroToggleStyle.ButtonPadding = FMargin(12.0f, 6.0f);
	RetroToggleStyle.OnSelectedColor = FSlateColor(GetRetroButtonSelectedBackground());
	RetroToggleStyle.OffSelectedColor = FSlateColor(GetRetroButtonSelectedBackground());
	RetroToggleStyle.UnselectedColor = FSlateColor(GetRetroButtonBackground());
	RetroToggleStyle.TextColor = FSlateColor(GetRetroButtonText());

	auto MakeToggleRow = [Loc, RetroToggleStyle](const FText& Label, const FText& Description, TFunction<bool()> GetValue, TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		return MakeSettingsToggleRow(Loc, Label, MoveTemp(GetValue), MoveTemp(SetValue), Description, RetroToggleStyle);
	};

	auto MakeFloatGetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<float()>
	{
		return [this, Field]() -> float
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			return PendingRetroFXSettings.*Field;
		};
	};

	auto MakeFloatSetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<void(float)>
	{
		return [this, Field](float Value)
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			PendingRetroFXSettings.*Field = FMath::Clamp(Value, 0.0f, 100.0f);
			MarkRetroFXEdited();
		};
	};

	auto MakeBoolGetter = [this](bool FT66RetroFXSettings::* Field) -> TFunction<bool()>
	{
		return [this, Field]() -> bool
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			return PendingRetroFXSettings.*Field;
		};
	};

	auto MakeBoolSetter = [this](bool FT66RetroFXSettings::* Field) -> TFunction<void(bool)>
	{
		return [this, Field](bool bValue)
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			PendingRetroFXSettings.*Field = bValue;
			MarkRetroFXEdited();
			if (Field == &FT66RetroFXSettings::bEnableRetroFXMaster && bRetroFXDirty)
			{
				ApplyPendingRetroFX();
			}
			ForceRebuildSlate();
		};
	};

	auto MakePercentToggleGetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<bool()>
	{
		return [this, Field]() -> bool
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			return PendingRetroFXSettings.*Field > KINDA_SMALL_NUMBER;
		};
	};

	auto MakePercentToggleSetter = [this](float FT66RetroFXSettings::* Field) -> TFunction<void(bool)>
	{
		return [this, Field](bool bValue)
		{
			InitializeRetroFXFromUserSettingsIfNeeded();
			PendingRetroFXSettings.*Field = bValue ? 100.0f : 0.0f;
			MarkRetroFXEdited();
		};
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
		const TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		if (bIncludeReset)
		{
			Row->AddSlot().AutoWidth()
			[
				MakeRetroButton(
					NSLOCTEXT("T66.Settings", "RetroFXResetDefaults", "RESET TO DEFAULTS"),
					[]() { return false; },
					FOnClicked::CreateLambda([this]()
					{
						return HandleResetRetroFXClicked();
					}),
					220.0f)
			];
		}

		Row->AddSlot().AutoWidth().Padding(bIncludeReset ? FMargin(8.0f, 0.0f, 0.0f, 0.0f) : FMargin(0.0f))
		[
			MakeRetroButton(
				NSLOCTEXT("T66.Settings", "RetroFXCancel", "CANCEL"),
				[]() { return false; },
				FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseClicked),
				140.0f)
		];

		Row->AddSlot().AutoWidth().Padding(8.0f, 0.0f, 0.0f, 0.0f)
		[
			MakeApplyButton()
		];

		return Row;
	};

	const TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	auto AddRow = [](const TSharedRef<SVerticalBox>& Box, const TSharedRef<SWidget>& Row, float BottomPadding = 8.0f)
	{
		Box->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, BottomPadding)
		[
			Row
		];
	};
	auto AddSection = [&Rows, &MakeSectionHeader, &AddRow](const FText& Title, const FText& Body)
	{
		AddRow(Rows, MakeSectionHeader(Title), 6.0f);
		AddRow(Rows,
			FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
			.Text(Body)
			.Font(SettingsRegularFont(15))
			.ColorAndOpacity(GetSettingsPageMuted())
			.AutoWrapText(true)), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true),
			10.0f);
	};
	auto AddSubsection = [&Rows, &AddRow](const FText& Title, const FText& Body)
	{
		AddRow(Rows, MakeSettingsSectionHeader(Title, 20), 4.0f);
		AddRow(Rows,
			FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
			.Text(Body)
			.Font(SettingsRegularFont(14))
			.ColorAndOpacity(GetSettingsPageMuted())
			.AutoWrapText(true)), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true),
			8.0f);
	};

	AddRow(Rows,
		MakeToggleRow(
			NSLOCTEXT("T66.Settings", "RetroFXFrontendMasterLabel", "Frontend Retro FX"),
			FText::GetEmpty(),
			MakeBoolGetter(&FT66RetroFXSettings::UIFullScreenCRTEnabled),
			MakeBoolSetter(&FT66RetroFXSettings::UIFullScreenCRTEnabled)),
		12.0f);
	AddRow(Rows,
		MakeToggleRow(
			NSLOCTEXT("T66.Settings", "RetroFXGameplayMasterLabel", "Gameplay Retro FX"),
			FText::GetEmpty(),
			MakeBoolGetter(&FT66RetroFXSettings::bEnableRetroFXMaster),
			MakeBoolSetter(&FT66RetroFXSettings::bEnableRetroFXMaster)),
		14.0f);
	AddRow(Rows, MakeActionRow(true), 0.0f);

	return SNew(SScrollBox)
		.ScrollBarStyle(GetSettingsFlatScrollBarStyle())
		.ScrollBarVisibility(EVisibility::Visible)
		.ScrollBarThickness(FVector2D(14.0f, 14.0f))
		.ScrollBarPadding(FMargin(10.0f, 0.0f, 2.0f, 0.0f))
		+ SScrollBox::Slot()
		[
			Rows
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildRetroFXPreviewPopupUI()
{
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float PanelWidth = FMath::Clamp(SafeFrameSize.X * 0.32f, 500.0f, 620.0f);
	const float PanelHeight = FMath::Clamp(SafeFrameSize.Y - 90.0f, 420.0f, 940.0f);

	const TSharedRef<SWidget> Header =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
		[
			FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(STextBlock)
			.Text(NSLOCTEXT("T66.Settings", "RetroFXPreviewPopupTitle", "RETRO FX PREVIEW"))
			.Font(SettingsBoldFont(22))
			.ColorAndOpacity(GetSettingsPageText())), NAME_None, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			MakeSettingsButton(
				FT66ButtonParams(NSLOCTEXT("T66.Common", "CloseX", "X"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseRetroFXPreviewPopupClicked), ET66ButtonType::Danger)
				.SetFontSize(18)
				.SetMinWidth(38.0f)
				.SetHeight(38.0f)
				.SetPadding(FMargin(0.0f)))
		];

	return SNew(SOverlay)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 36.0f, 28.0f, 36.0f))
		[
			SNew(SBox)
			.WidthOverride(PanelWidth)
			.HeightOverride(PanelHeight)
			[
				MakeSettingsContentShell(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
					[
						Header
					]
					+ SVerticalBox::Slot().FillHeight(1.0f)
					[
						BuildRetroFXTab()
					],
					FMargin(18.0f))
			]
		];
}

FReply UT66SettingsScreen::HandleApplyRetroFXClicked()
{
	UE_LOG(LogT66RetroFXUI, Log, TEXT("HandleApplyRetroFXClicked: dirty=%s world=%s"), bRetroFXDirty ? TEXT("true") : TEXT("false"), *GetNameSafe(GetWorld()));
	ApplyPendingRetroFX();
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleResetRetroFXClicked()
{
	ResetPendingRetroFXToDefaults();
	if (ShouldLiveApplyRetroFX())
	{
		ApplyPendingRetroFX();
	}
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleRetroFXPreviewModeClicked(bool bEnabled)
{
	if (bRetroFXPreviewPopup && !bEnabled)
	{
		if (UIManager)
		{
			UIManager->HideRetroFXPreviewPopup();
		}
		return FReply::Handled();
	}

	bRetroFXPreviewMode = bEnabled;
	if (UIManager)
	{
		if (bEnabled)
		{
			UIManager->ShowRetroFXPreviewPopup();
		}
		else
		{
			UIManager->HideRetroFXPreviewPopup();
		}
	}

	if (bEnabled)
	{
		ApplyPendingRetroFX();
	}
	return FReply::Handled();
}

FReply UT66SettingsScreen::HandleCloseRetroFXPreviewPopupClicked()
{
	if (UIManager)
	{
		UIManager->HideRetroFXPreviewPopup();
	}
	return FReply::Handled();
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
		TEXT("ApplyPendingRetroFX: dirty=%s world=%s MasterEnabled=%s PS1Blend=%.2f WorldPixelation=%.2f CharacterPixelation=%.2f ChromePixel=%.2f ChromeDither=%.2f ChromeSnap=%.2f BackgroundPixel=%.2f BackgroundDither=%.2f BackgroundSnap=%.2f"),
		bRetroFXDirty ? TEXT("true") : TEXT("false"),
		*GetNameSafe(GetWorld()),
		PendingRetroFXSettings.bEnableRetroFXMaster ? TEXT("true") : TEXT("false"),
		PendingRetroFXSettings.PS1BlendPercent,
		PendingRetroFXSettings.WorldPixelationPercent,
		PendingRetroFXSettings.CharacterPixelationPercent,
		PendingRetroFXSettings.UIChromePixelationPercent,
		PendingRetroFXSettings.UIChromeDitheringPercent,
		PendingRetroFXSettings.UIChromeVertexSnapPercent,
		PendingRetroFXSettings.UIBackgroundImagePixelationPercent,
		PendingRetroFXSettings.UIBackgroundImageDitheringPercent,
		PendingRetroFXSettings.UIBackgroundImageVertexSnapPercent);

	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->SetRetroFXSettings(PendingRetroFXSettings);
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().InvalidateAllWidgets(false);
		}

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
			{
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
	}
	else
	{
		UE_LOG(LogT66RetroFXUI, Warning, TEXT("ApplyPendingRetroFX: Player settings subsystem was null"));
	}
}

void UT66SettingsScreen::CommitPendingRetroFXOnClose()
{
	if (!bRetroFXInitialized || !bRetroFXDirty)
	{
		return;
	}

	UE_LOG(LogT66RetroFXUI, Log, TEXT("CommitPendingRetroFXOnClose: applying unsaved Retro FX changes before leaving Settings"));
	ApplyPendingRetroFX();
}

void UT66SettingsScreen::ResetPendingRetroFXToDefaults()
{
	InitializeRetroFXFromUserSettingsIfNeeded();
	PendingRetroFXSettings = FT66RetroFXSettings();
	bRetroFXDirty = true;
}

void UT66SettingsScreen::MarkRetroFXEdited()
{
	bRetroFXDirty = true;
	if (ShouldLiveApplyRetroFX())
	{
		ApplyPendingRetroFX();
	}
}

bool UT66SettingsScreen::ShouldLiveApplyRetroFX() const
{
	return bRetroFXPreviewPopup || bRetroFXPreviewMode;
}
