// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66VersusMainMenuScreen.h"

#include "Core/T66SessionSubsystem.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UITypes.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FLinearColor VersusBackground(0.010f, 0.012f, 0.018f, 1.0f);
	const FLinearColor VersusAccent(0.32f, 0.78f, 0.94f, 1.0f);
	const FLinearColor VersusAccentWarm(0.98f, 0.64f, 0.30f, 1.0f);
	const FLinearColor VersusText(0.94f, 0.93f, 0.88f, 1.0f);
	const FLinearColor VersusMuted(0.66f, 0.68f, 0.72f, 1.0f);

	FName MakeVersusTag(const TCHAR* Prefix, const FText& Label)
	{
		FString Token = Label.ToString().ToUpper();
		for (int32 Index = Token.Len() - 1; Index >= 0; --Index)
		{
			if (!FChar::IsAlnum(Token[Index]))
			{
				Token.RemoveAt(Index, 1, EAllowShrinking::No);
			}
		}
		if (Token.IsEmpty())
		{
			Token = TEXT("ACTION");
		}
		return FName(*(FString(Prefix) + TEXT(".") + Token));
	}

	FSlateFontInfo MakeVersusFont(const TCHAR* Weight, const int32 Size)
	{
		FSlateFontInfo Font = FT66Style::MakeFont(Weight, Size);
		Font.LetterSpacing = 0;
		return Font;
	}
}

UT66VersusMainMenuScreen::UT66VersusMainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::VersusMainMenu;
	bIsModal = false;
	StatusText = NSLOCTEXT("T66Versus.MainMenu", "InitialStatus", "Choose a 1v1 setup path. Arcade game selection comes next.");
}

void UT66VersusMainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr)
	{
		SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66VersusMainMenuScreen::HandleSessionStateChanged);
		if (const UWorld* World = GetWorld(); World && World->GetNetMode() != NM_Standalone)
		{
			SessionSubsystem->HandlePartyHubScreenActivated();
		}
		SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::VersusMainMenu);
	}
}

void UT66VersusMainMenuScreen::OnScreenDeactivated_Implementation()
{
	if (UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr)
	{
		if (SessionStateChangedHandle.IsValid())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
			SessionStateChangedHandle.Reset();
		}
	}

	Super::OnScreenDeactivated_Implementation();
}

TSharedRef<SWidget> UT66VersusMainMenuScreen::BuildSlateUI()
{
	const TAttribute<FText> StatusAttribute = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UT66VersusMainMenuScreen::GetStatusText));

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(VersusBackground)
		.Padding(FMargin(34.f, 46.f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.28f))
			]
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Versus.MainMenu", "Title", "VERSUS"))
					.Font(MakeVersusFont(TEXT("Black"), 58))
					.ColorAndOpacity(VersusAccent)
					.Justification(ETextJustify::Center)
					.ShadowOffset(FVector2D(2.f, 2.f))
					.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.70f))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 26.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Versus.MainMenu", "Subtitle", "1v1 arcade gauntlet for competing against friends"))
					.Font(MakeVersusFont(TEXT("Regular"), 18))
					.ColorAndOpacity(VersusMuted)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.44f).Padding(0.f, 0.f, 22.f, 0.f)
					[
						MakeVersusPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66Versus.MainMenu", "ConceptTitle", "FORMAT"))
								.Font(MakeVersusFont(TEXT("Bold"), 24))
								.ColorAndOpacity(VersusAccentWarm)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
							[
								MakeInfoRow(
									NSLOCTEXT("T66Versus.MainMenu", "RuleRoundLabel", "Rounds"),
									NSLOCTEXT("T66Versus.MainMenu", "RuleRoundBody", "Both players roll through the same arcade cabinet challenge, then compare score, time, or survival result."))
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
							[
								MakeInfoRow(
									NSLOCTEXT("T66Versus.MainMenu", "RuleArcadeLabel", "Arcade pool"),
									NSLOCTEXT("T66Versus.MainMenu", "RuleArcadeBody", "Whack-a-Mole and the other in-world arcade games become the competitive playlist."))
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
							[
								MakeInfoRow(
									NSLOCTEXT("T66Versus.MainMenu", "RulePartyLabel", "Party"),
									NSLOCTEXT("T66Versus.MainMenu", "RulePartyBody", "The first wiring uses the existing duo lobby/session path so invite flow can be layered in next."))
							],
							FMargin(26.f, 24.f),
							VersusAccentWarm,
							FName(TEXT("Versus.Panel.Format")))
					]
					+ SHorizontalBox::Slot().FillWidth(0.56f)
					[
						MakeVersusPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66Versus.MainMenu", "SetupTitle", "SETUP"))
								.Font(MakeVersusFont(TEXT("Bold"), 24))
								.ColorAndOpacity(VersusAccent)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 22.f, 0.f, 0.f)
							[
								MakeVersusButton(
									NSLOCTEXT("T66Versus.MainMenu", "Host", "HOST 1V1"),
									FOnClicked::CreateUObject(this, &UT66VersusMainMenuScreen::HandleHostClicked),
									ET66ButtonType::Primary)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
							[
								MakeVersusButton(
									NSLOCTEXT("T66Versus.MainMenu", "Join", "JOIN FRIEND"),
									FOnClicked::CreateUObject(this, &UT66VersusMainMenuScreen::HandleJoinClicked),
									ET66ButtonType::Neutral)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
							[
								MakeVersusButton(
									NSLOCTEXT("T66Versus.MainMenu", "Practice", "LOCAL PRACTICE SHELL"),
									FOnClicked::CreateUObject(this, &UT66VersusMainMenuScreen::HandlePracticeClicked),
									ET66ButtonType::Neutral)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 24.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(StatusAttribute)
								.Font(MakeVersusFont(TEXT("Regular"), 15))
								.ColorAndOpacity(VersusText)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().FillHeight(1.f)
							[
								SNew(SBox)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 24.f, 0.f, 0.f)
							[
								SNew(SBox).WidthOverride(260.f)
								[
									MakeVersusButton(
										NSLOCTEXT("T66Versus.MainMenu", "Back", "BACK TO MINIGAMES"),
										FOnClicked::CreateUObject(this, &UT66VersusMainMenuScreen::HandleBackClicked),
										ET66ButtonType::Neutral)
								]
							],
							FMargin(26.f, 24.f),
							VersusAccent,
							FName(TEXT("Versus.Panel.Setup")))
					]
				]
			]
		];
}

FReply UT66VersusMainMenuScreen::HandleHostClicked()
{
	if (UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr)
	{
		const bool bStarted = SessionSubsystem->EnsurePartySessionReady(ET66PartySize::Duo, ET66ScreenType::VersusMainMenu);
		if (bStarted)
		{
			StatusText = NSLOCTEXT("T66Versus.MainMenu", "HostStarted", "Duo lobby is being prepared for Versus. Invite selection and game draft are the next layer.");
		}
		else
		{
			const FString LastStatus = SessionSubsystem->GetLastStatusText();
			StatusText = FText::FromString(LastStatus.IsEmpty() ? FString(TEXT("Could not start a Versus lobby.")) : LastStatus);
		}
	}
	else
	{
		StatusText = NSLOCTEXT("T66Versus.MainMenu", "NoSessionSubsystem", "Party session subsystem is unavailable.");
	}

	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66VersusMainMenuScreen::HandleJoinClicked()
{
	StatusText = NSLOCTEXT("T66Versus.MainMenu", "JoinPending", "Join will use the existing party invite acceptance path. Waiting for the friend-list picker layer.");
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66VersusMainMenuScreen::HandlePracticeClicked()
{
	StatusText = NSLOCTEXT("T66Versus.MainMenu", "PracticePending", "Local practice shell is wired. Arcade round draft and score arbitration are not implemented yet.");
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66VersusMainMenuScreen::HandleBackClicked()
{
	NavigateTo(ET66ScreenType::Minigames);
	return FReply::Handled();
}

void UT66VersusMainMenuScreen::HandleSessionStateChanged()
{
	if (const UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr)
	{
		const FString& LastStatus = SessionSubsystem->GetLastStatusText();
		if (!LastStatus.IsEmpty())
		{
			StatusText = FText::FromString(LastStatus);
			ForceRebuildSlate();
		}
	}
}

FText UT66VersusMainMenuScreen::GetStatusText() const
{
	return StatusText;
}

TSharedRef<SWidget> UT66VersusMainMenuScreen::MakeVersusPanel(const TSharedRef<SWidget>& Content, const FMargin& ContentPadding, const FLinearColor& Accent, const FName Tag) const
{
	const FMargin ResolvedPadding(
		ContentPadding.Left + 58.f,
		ContentPadding.Top + 42.f,
		ContentPadding.Right + 34.f,
		ContentPadding.Bottom + 24.f);
	const ET66FlatState PanelState = Accent.R > 0.90f ? ET66FlatState::Selected : ET66FlatState::Default;
	return FT66FlatStyle::MakeFlatPanel(PanelState, ResolvedPadding, Content, nullptr, Tag);
}

TSharedRef<SWidget> UT66VersusMainMenuScreen::MakeVersusButton(const FText& Text, const FOnClicked& Handler, const ET66ButtonType Type, const bool bEnabled) const
{
	const bool bPrimary = Type == ET66ButtonType::Primary;
	const ET66FlatState State = !bEnabled ? ET66FlatState::Disabled : (bPrimary ? ET66FlatState::Selected : ET66FlatState::Default);

	return SNew(SBox)
		.HeightOverride(58.f)
		[
			FT66FlatStyle::MakeFlatButton(
				State,
				Text,
				Handler,
				nullptr,
				nullptr,
				FMargin(18.f, 10.f),
				220.f,
				58.f,
				bEnabled,
				18,
				MakeVersusTag(TEXT("Versus.Button"), Text))
		];
}

TSharedRef<SWidget> UT66VersusMainMenuScreen::MakeInfoRow(const FText& Label, const FText& Body) const
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(MakeVersusFont(TEXT("Bold"), 15))
			.ColorAndOpacity(VersusText)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(Body)
			.Font(MakeVersusFont(TEXT("Regular"), 14))
			.ColorAndOpacity(VersusMuted)
			.AutoWrapText(true)
		];
}
