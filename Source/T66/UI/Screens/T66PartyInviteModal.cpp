// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PartyInviteModal.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

UT66PartyInviteModal::UT66PartyInviteModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PartyInvite;
	bIsModal = true;
}

DEFINE_LOG_CATEGORY_STATIC(LogT66PartyInviteModal, Log, All);

namespace
{
	constexpr float PartyInviteButtonHeight = 56.f;

	struct FPartyInviteReferenceButtonBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
	};

	struct FPartyInviteReferenceButtonStyleEntry
	{
		FButtonStyle Style;
		bool bInitialized = false;
	};

	const FSlateBrush* ResolvePartyInviteReferenceBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	const TCHAR* GetPartyInviteButtonPrefix(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return TEXT("settings_toggle_on");
		case ET66ButtonType::Danger:
			return TEXT("settings_toggle_off");
		default:
			return TEXT("settings_compact_neutral");
		}
	}

	FPartyInviteReferenceButtonBrushSet& GetPartyInviteButtonBrushSet(ET66ButtonType Type)
	{
		static FPartyInviteReferenceButtonBrushSet Neutral;
		static FPartyInviteReferenceButtonBrushSet Success;
		static FPartyInviteReferenceButtonBrushSet Danger;

		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return Success;
		case ET66ButtonType::Danger:
			return Danger;
		default:
			return Neutral;
		}
	}

	FPartyInviteReferenceButtonStyleEntry& GetPartyInviteButtonStyleEntry(ET66ButtonType Type)
	{
		static FPartyInviteReferenceButtonStyleEntry Neutral;
		static FPartyInviteReferenceButtonStyleEntry Success;
		static FPartyInviteReferenceButtonStyleEntry Danger;

		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return Success;
		case ET66ButtonType::Danger:
			return Danger;
		default:
			return Neutral;
		}
	}

	const FSlateBrush* ResolvePartyInviteButtonBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* Prefix,
		const TCHAR* State,
		const TCHAR* DebugLabel)
	{
		return ResolvePartyInviteReferenceBrush(
			Entry,
			FString::Printf(TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/%s_%s.png"), Prefix, State),
			FMargin(0.16f, 0.28f, 0.16f, 0.28f),
			DebugLabel);
	}

	const FButtonStyle& GetPartyInviteButtonStyle(ET66ButtonType Type)
	{
		FPartyInviteReferenceButtonStyleEntry& StyleEntry = GetPartyInviteButtonStyleEntry(Type);
		if (!StyleEntry.bInitialized)
		{
			StyleEntry.bInitialized = true;
			StyleEntry.Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			StyleEntry.Style.SetNormalPadding(FMargin(0.f));
			StyleEntry.Style.SetPressedPadding(FMargin(0.f));

			FPartyInviteReferenceButtonBrushSet& BrushSet = GetPartyInviteButtonBrushSet(Type);
			const TCHAR* Prefix = GetPartyInviteButtonPrefix(Type);
			if (const FSlateBrush* Brush = ResolvePartyInviteButtonBrush(BrushSet.Normal, Prefix, TEXT("normal"), TEXT("PartyInviteButtonNormal")))
			{
				StyleEntry.Style.SetNormal(*Brush);
			}
			if (const FSlateBrush* Brush = ResolvePartyInviteButtonBrush(BrushSet.Hovered, Prefix, TEXT("hover"), TEXT("PartyInviteButtonHover")))
			{
				StyleEntry.Style.SetHovered(*Brush);
			}
			if (const FSlateBrush* Brush = ResolvePartyInviteButtonBrush(BrushSet.Pressed, Prefix, TEXT("pressed"), TEXT("PartyInviteButtonPressed")))
			{
				StyleEntry.Style.SetPressed(*Brush);
			}
			if (const FSlateBrush* Brush = ResolvePartyInviteButtonBrush(BrushSet.Disabled, TEXT("settings_toggle_inactive"), TEXT("normal"), TEXT("PartyInviteButtonDisabled")))
			{
				StyleEntry.Style.SetDisabled(*Brush);
			}
		}

		return StyleEntry.Style;
	}

	const FSlateBrush* GetPartyInviteShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolvePartyInviteReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_content_shell_frame.png"),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f),
			TEXT("PartyInviteShell"));
	}

	const FSlateBrush* GetPartyInviteRowBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolvePartyInviteReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_row_shell_full.png"),
			FMargin(0.055f, 0.32f, 0.055f, 0.32f),
			TEXT("PartyInviteRowShell"));
	}

	TSharedRef<SWidget> MakePartyInviteShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* ShellBrush = GetPartyInviteShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(ShellBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(Content, FT66PanelParams(ET66PanelType::Panel).SetPadding(Padding));
	}

	TSharedRef<SWidget> MakePartyInviteRow(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* RowBrush = GetPartyInviteRowBrush())
		{
			return SNew(SBorder)
				.BorderImage(RowBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66Style::MakePanel(Content, FT66PanelParams(ET66PanelType::Panel2).SetPadding(Padding));
	}

	TSharedRef<SWidget> MakePartyInviteButton(const FT66ButtonParams& Params)
	{
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : 19;
		const TAttribute<FText> ButtonText = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(FT66Style::Tokens::Text));
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(22.f, 10.f);

		FSlateFontInfo ButtonFont = FT66Style::MakeFont(*Params.FontWeight, FontSize);
		ButtonFont.LetterSpacing = 0;

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(ButtonText)
				.Font(ButtonFont)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(0.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.72f))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis));

		return SNew(SBox)
			.WidthOverride(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : PartyInviteButtonHeight)
			.Visibility(Params.Visibility)
			[
				SNew(SButton)
				.ButtonStyle(&GetPartyInviteButtonStyle(Params.Type))
				.ContentPadding(ContentPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.IsEnabled(Params.IsEnabled)
				.OnClicked(FT66Style::DebounceClick(Params.OnClicked))
				[
					Content
				]
			];
	}

	void T66RefreshInviteJoinContext(
		const UT66BackendSubsystem* Backend,
		const FString& InviteId,
		FString& InOutHostSteamId,
		FString& InOutHostLobbyId,
		FString& InOutHostAppId)
	{
		if (!Backend || InviteId.IsEmpty())
		{
			return;
		}

		for (const FT66PartyInviteEntry& Invite : Backend->GetPendingPartyInvites())
		{
			if (Invite.InviteId != InviteId)
			{
				continue;
			}

			if (!Invite.HostSteamId.IsEmpty())
			{
				InOutHostSteamId = Invite.HostSteamId;
			}

			if (!Invite.HostLobbyId.IsEmpty())
			{
				InOutHostLobbyId = Invite.HostLobbyId;
			}

			if (!Invite.HostAppId.IsEmpty())
			{
				InOutHostAppId = Invite.HostAppId;
			}

			break;
		}
	}
}

void UT66PartyInviteModal::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	bJoinKickoffStarted = false;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			PartyInviteActionCompleteHandle = Backend->OnPartyInviteActionComplete().AddUObject(this, &UT66PartyInviteModal::HandlePartyInviteActionComplete);
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Party invite modal activated and bound action-complete delegate."));
		}
	}
}

void UT66PartyInviteModal::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnPartyInviteActionComplete().Remove(PartyInviteActionCompleteHandle);
		}
	}
	UE_LOG(LogT66PartyInviteModal, Log, TEXT("Party invite modal deactivated. InFlight=%d InviteId=%s"), bActionInFlight ? 1 : 0, *ActionInviteId);
	PartyInviteActionCompleteHandle.Reset();
	ActionInviteId.Reset();
	ActionHostSteamId.Reset();
	ActionHostLobbyId.Reset();
	ActionHostAppId.Reset();
	bAcceptingInvite = false;
	bActionInFlight = false;
	bJoinKickoffStarted = false;
	ActionStatusText.Reset();

	Super::OnScreenDeactivated_Implementation();
}

const FT66PartyInviteEntry* UT66PartyInviteModal::GetCurrentInvite() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			const TArray<FT66PartyInviteEntry>& PendingInvites = Backend->GetPendingPartyInvites();
			return PendingInvites.Num() > 0 ? &PendingInvites[0] : nullptr;
		}
	}

	return nullptr;
}

TSharedRef<SWidget> UT66PartyInviteModal::BuildSlateUI()
{
	const FT66PartyInviteEntry* Invite = GetCurrentInvite();
	const FText TitleText = NSLOCTEXT("T66.PartyInvite", "Title", "PARTY INVITE");
	const FText EmptyText = NSLOCTEXT("T66.PartyInvite", "Empty", "No pending party invites.");
	const FText AcceptText = bActionInFlight
		? NSLOCTEXT("T66.PartyInvite", "Working", "WORKING...")
		: NSLOCTEXT("T66.PartyInvite", "Accept", "ACCEPT");
	const FText RejectText = bActionInFlight
		? NSLOCTEXT("T66.PartyInvite", "PleaseWait", "PLEASE WAIT")
		: NSLOCTEXT("T66.PartyInvite", "Reject", "REJECT");

	const FText InviteBodyText = Invite
		? FText::Format(
			NSLOCTEXT("T66.PartyInvite", "InviteBody", "{0} invited you to join their party."),
			FText::FromString(Invite->HostDisplayName.IsEmpty() ? Invite->HostSteamId : Invite->HostDisplayName))
		: EmptyText;

	const FText StatusText = ActionStatusText.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(ActionStatusText);
	const bool bCanActOnInvite = Invite != nullptr && !bActionInFlight;

	constexpr float T66InviteButtonMinWidth = 280.f;
	constexpr float T66InviteButtonHeight = 52.f;
	const FMargin T66InviteButtonPadding(24.f, 12.f);

	return SNew(SBorder)
		.BorderBackgroundColor(FT66Style::Scrim())
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakePartyInviteShell(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(34))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 24.f)
					[
						MakePartyInviteRow(
							SNew(SBox)
							.WidthOverride(560.f)
							[
								SNew(STextBlock)
								.Text(InviteBodyText)
								.Font(FT66Style::Tokens::FontRegular(18))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							],
							FMargin(24.f, 18.f))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, StatusText.IsEmpty() ? 0.f : 18.f)
					[
						SNew(SBox)
						.Visibility(StatusText.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
						[
							MakePartyInviteRow(
								SNew(SBox)
								.WidthOverride(560.f)
								[
									SNew(STextBlock)
									.Text(StatusText)
									.Font(FT66Style::Tokens::FontRegular(14))
									.ColorAndOpacity(FLinearColor(0.90f, 0.77f, 0.43f, 1.0f))
									.Justification(ETextJustify::Center)
									.AutoWrapText(true)
								],
								FMargin(18.f, 10.f))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.f, 0.f)
						[
							MakePartyInviteButton(
								FT66ButtonParams(
									AcceptText,
									FOnClicked::CreateUObject(this, &UT66PartyInviteModal::HandleAcceptClicked),
									ET66ButtonType::Success)
								.SetMinWidth(T66InviteButtonMinWidth)
								.SetHeight(T66InviteButtonHeight)
								.SetPadding(T66InviteButtonPadding)
								.SetEnabled(TAttribute<bool>(bCanActOnInvite)))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.f, 0.f)
						[
							MakePartyInviteButton(
								FT66ButtonParams(
									RejectText,
									FOnClicked::CreateUObject(this, &UT66PartyInviteModal::HandleRejectClicked),
									ET66ButtonType::Danger)
								.SetMinWidth(T66InviteButtonMinWidth)
								.SetHeight(T66InviteButtonHeight)
								.SetPadding(T66InviteButtonPadding)
								.SetEnabled(TAttribute<bool>(bCanActOnInvite)))
						]
					]
				,
				FMargin(42.f, 30.f))
			]
		];
}

FReply UT66PartyInviteModal::HandleAcceptClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("MP-03 PartyInviteModal::AcceptClick"));

	if (bActionInFlight)
	{
		return FReply::Handled();
	}

	const FT66PartyInviteEntry* Invite = GetCurrentInvite();
	if (!Invite)
	{
		CloseModal();
		return FReply::Handled();
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			ActionInviteId = Invite->InviteId;
			ActionHostSteamId = Invite->HostSteamId;
			ActionHostLobbyId = Invite->HostLobbyId;
			ActionHostAppId = Invite->HostAppId;
			ActionStatusText = TEXT("Accepting invite...");
			bAcceptingInvite = true;
			FT66MultiplayerDiagnosticContext Diagnostic;
			Diagnostic.EventName = TEXT("invite_accept_click");
			Diagnostic.Severity = TEXT("info");
			Diagnostic.Message = TEXT("Player accepted a party invite.");
			Diagnostic.InviteId = ActionInviteId;
			Diagnostic.HostSteamId = ActionHostSteamId;
			Diagnostic.LobbyId = ActionHostLobbyId;
			Diagnostic.SourceAppId = ActionHostAppId;
			Backend->SubmitMultiplayerDiagnostic(Diagnostic);
			bActionInFlight = Backend->RespondToPartyInvite(Invite->InviteId, true);
			bJoinKickoffStarted = false;
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Accept clicked for invite %s host=%s lobby=%s app=%s"), *ActionInviteId, *ActionHostSteamId, *ActionHostLobbyId, *ActionHostAppId);

			if (TryStartJoinKickoff())
			{
				bJoinKickoffStarted = true;
				ActionStatusText = TEXT("Joining party...");
				CloseModal();
				return FReply::Handled();
			}

			if (bActionInFlight)
			{
				ForceRebuildSlate();
			}
			else
			{
				ActionStatusText = TEXT("Invite accept could not be sent.");
				ForceRebuildSlate();
			}
		}
	}

	return FReply::Handled();
}

FReply UT66PartyInviteModal::HandleRejectClicked()
{
	if (bActionInFlight)
	{
		return FReply::Handled();
	}

	const FT66PartyInviteEntry* Invite = GetCurrentInvite();
	if (!Invite)
	{
		CloseModal();
		return FReply::Handled();
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			ActionInviteId = Invite->InviteId;
			ActionHostSteamId.Reset();
			ActionHostLobbyId.Reset();
			ActionHostAppId.Reset();
			ActionStatusText = TEXT("Rejecting invite...");
			bAcceptingInvite = false;
			bActionInFlight = Backend->RespondToPartyInvite(Invite->InviteId, false);
			bJoinKickoffStarted = false;
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Reject clicked for invite %s"), *ActionInviteId);
			if (bActionInFlight)
			{
				ForceRebuildSlate();
			}
			else
			{
				ActionStatusText = TEXT("Invite reject could not be sent.");
				ForceRebuildSlate();
			}
		}
	}

	return FReply::Handled();
}

void UT66PartyInviteModal::HandlePartyInviteActionComplete(bool bSuccess, const FString& Action, const FString& InviteId, const FString& Message)
{
	FLagScopedScope LagScope(GetWorld(), TEXT("MP-03 PartyInviteModal::ActionComplete"));

	if (InviteId != ActionInviteId || bActionInFlight == false)
	{
		UE_LOG(LogT66PartyInviteModal, Verbose, TEXT("Ignoring action-complete callback success=%d action=%s invite=%s current=%s inFlight=%d"), bSuccess ? 1 : 0, *Action, *InviteId, *ActionInviteId, bActionInFlight ? 1 : 0);
		return;
	}

	bActionInFlight = false;
	ActionStatusText = Message;
	UE_LOG(LogT66PartyInviteModal, Log, TEXT("Invite action complete success=%d action=%s invite=%s message=%s"), bSuccess ? 1 : 0, *Action, *InviteId, *Message);

	if (!bSuccess)
	{
		ForceRebuildSlate();
		return;
	}

	if (bAcceptingInvite)
	{
		if (bJoinKickoffStarted)
		{
			CloseModal();
			return;
		}

		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
			{
				T66RefreshInviteJoinContext(Backend, ActionInviteId, ActionHostSteamId, ActionHostLobbyId, ActionHostAppId);
			}

			if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
			{
				const bool bJoinStarted =
					!ActionHostLobbyId.IsEmpty()
						? SessionSubsystem->JoinPartySessionByLobbyId(ActionHostLobbyId, ActionHostSteamId, ActionHostAppId, ActionInviteId)
						: SessionSubsystem->JoinFriendPartySessionBySteamId(ActionHostSteamId, ActionInviteId, ActionHostAppId);
				UE_LOG(LogT66PartyInviteModal, Log, TEXT("Invite accept join start result=%d host=%s lobby=%s app=%s"), bJoinStarted ? 1 : 0, *ActionHostSteamId, *ActionHostLobbyId, *ActionHostAppId);
				if (!bJoinStarted)
				{
					ActionStatusText = TEXT("Invite accepted, but the party join could not start.");
					ForceRebuildSlate();
					return;
				}
			}
		}
	}

	CloseModal();
}

bool UT66PartyInviteModal::TryStartJoinKickoff()
{
	if (!bAcceptingInvite)
	{
		return false;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			T66RefreshInviteJoinContext(Backend, ActionInviteId, ActionHostSteamId, ActionHostLobbyId, ActionHostAppId);
		}

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			const bool bJoinStarted =
				!ActionHostLobbyId.IsEmpty()
					? SessionSubsystem->JoinPartySessionByLobbyId(ActionHostLobbyId, ActionHostSteamId, ActionHostAppId, ActionInviteId)
					: SessionSubsystem->JoinFriendPartySessionBySteamId(ActionHostSteamId, ActionInviteId, ActionHostAppId);
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Invite join kickoff result=%d host=%s lobby=%s app=%s"), bJoinStarted ? 1 : 0, *ActionHostSteamId, *ActionHostLobbyId, *ActionHostAppId);
			return bJoinStarted;
		}
	}

	return false;
}
