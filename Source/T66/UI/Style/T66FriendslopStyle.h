// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Style/T66FlatStyle.h"

class SBorder;

enum class ET66FriendslopChrome : uint8
{
	PanelLargeDark,
	ButtonLongDark,
	ButtonPrimaryRed,
	ButtonActionGreen,
	IconButtonDark,
	PillDark,
	RowDark,
	RowSelectedRed,
	PartySlotDark,
	SmallSquareDark,
	CheckboxCheckedRed,
	CheckboxEmptyDark,
	TopbarStripRound06,
	TopbarIconDarkRound06,
	TopbarTabDarkRound06,
	TopbarTabRedRound06,
	TopbarTicketRound06,
	TopbarPowerRedRound06,
	TopbarSettingsIconButtonRound06,
	TopbarCouponIconButtonRound06,
	TopbarPowerIconButtonRound06,
	LeftPanelRound06,
	ProfileRowRound06,
	SearchFieldRound06,
	SectionHeaderRound06,
	FriendRowRound06,
	InviteButtonGreenRound06,
	OfflineButtonDarkRound06,
	PartySlotRound06,
	TitleLogoRound06,
	CtaPrimaryRound06,
	CtaSecondaryRound06,
	FilterRailRound06,
	FilterPanelRound09,
	FilterIconRedRound06,
	FilterIconDarkRound06,
	FilterGlobalIconButtonRound06,
	FilterFriendsIconButtonRound06,
	FilterStreamerIconButtonRound06,
	LeaderboardPanelRound06,
	LeaderboardTabRedRound06,
	LeaderboardTabDarkRound06,
	DropdownDarkRound06,
	CheckboxCheckedRound06,
	CheckboxEmptyRound06,
	TableHeaderBandRound06,
	RankingRowRedRound06,
};

class T66_API FT66FriendslopStyle
{
public:
	static const FSlateBrush* GetChromeBrush(ET66FriendslopChrome Chrome);
	static const FSlateBrush* GetCustomBrush(
		const FString& RelativePath,
		const FMargin& Margin,
		ESlateBrushDrawType::Type DrawAs,
		const FVector2D& FallbackSize,
		const FLinearColor& FallbackTint = FLinearColor::White);

	static ET66FriendslopChrome ButtonChromeForState(ET66FlatState State);
	static ET66FriendslopChrome RowChromeForState(ET66FlatState State);
	static const FSlateBrush* GetCheckboxBrush(bool bChecked);
	static FSlateColor TextColorForState(ET66FlatState State);

	static TSharedRef<SWidget> MakeSurface(
		ET66FriendslopChrome Chrome,
		ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		TSharedPtr<SBorder>* OutBorder = nullptr,
		FName Tag = NAME_None,
		const TCHAR* Role = TEXT("FriendslopSurface"),
		bool bInteractive = false,
		FName ToggleGroup = NAME_None,
		bool bHoverCapable = false);

	static TSharedRef<SWidget> MakeCustomSurface(
		const FString& RelativePath,
		const FMargin& Margin,
		ESlateBrushDrawType::Type DrawAs,
		const FVector2D& FallbackSize,
		ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		TSharedPtr<SBorder>* OutBorder = nullptr,
		FName Tag = NAME_None,
		const TCHAR* Role = TEXT("FriendslopCustomSurface"),
		bool bInteractive = false,
		FName ToggleGroup = NAME_None,
		bool bHoverCapable = false,
		const FLinearColor& FallbackTint = FLinearColor(0.075f, 0.08f, 0.105f, 1.f));

	static TSharedRef<SWidget> MakePanel(
		ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		TSharedPtr<SBorder>* OutBorder = nullptr,
		FName Tag = NAME_None,
		ET66FriendslopChrome Chrome = ET66FriendslopChrome::PanelLargeDark);

	static TSharedRef<SWidget> MakeCustomPanel(
		const FString& RelativePath,
		const FMargin& Margin,
		const FVector2D& FallbackSize,
		ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		TSharedPtr<SBorder>* OutBorder = nullptr,
		FName Tag = NAME_None,
		const FLinearColor& FallbackTint = FLinearColor(0.075f, 0.08f, 0.105f, 1.f));

	static TSharedRef<SWidget> MakeButton(
		ET66FlatState State,
		const TAttribute<FText>& Label,
		FOnClicked OnClicked,
		const TSharedPtr<SWidget>& OptionalLeftIcon = nullptr,
		const TSharedPtr<SWidget>& OptionalRightIcon = nullptr,
		const FMargin& Padding = FMargin(14.f, 8.f),
		float MinWidth = 120.f,
		float Height = 44.f,
		TAttribute<bool> IsEnabled = true,
		int32 FontSize = 20,
		FName Tag = NAME_None,
		FName ToggleGroup = NAME_None,
		ET66FriendslopChrome Chrome = ET66FriendslopChrome::ButtonLongDark);

	static TSharedRef<SWidget> MakeButton(
		ET66FlatState State,
		const FText& Label,
		FOnClicked OnClicked,
		const TSharedPtr<SWidget>& OptionalLeftIcon = nullptr,
		const TSharedPtr<SWidget>& OptionalRightIcon = nullptr,
		const FMargin& Padding = FMargin(14.f, 8.f),
		float MinWidth = 120.f,
		float Height = 44.f,
		TAttribute<bool> IsEnabled = true,
		int32 FontSize = 20,
		FName Tag = NAME_None,
		FName ToggleGroup = NAME_None,
		ET66FriendslopChrome Chrome = ET66FriendslopChrome::ButtonLongDark);

	static TSharedRef<SWidget> MakeToggleGroupButton(
		ET66FlatState State,
		const TSharedRef<SWidget>& Content,
		FOnClicked OnClicked,
		const FMargin& Padding = FMargin(14.f, 8.f),
		float MinWidth = 120.f,
		float Height = 44.f,
		TAttribute<bool> IsEnabled = true,
		FName Tag = NAME_None,
		FName ToggleGroup = NAME_None,
		ET66FriendslopChrome Chrome = ET66FriendslopChrome::ButtonLongDark);

	static TSharedRef<SWidget> MakeCustomToggleGroupButton(
		const FString& RelativePath,
		const FMargin& Margin,
		const FVector2D& FallbackSize,
		ET66FlatState State,
		const TSharedRef<SWidget>& Content,
		FOnClicked OnClicked,
		const FMargin& Padding = FMargin(14.f, 8.f),
		float MinWidth = 120.f,
		float Height = 44.f,
		TAttribute<bool> IsEnabled = true,
		FName Tag = NAME_None,
		FName ToggleGroup = NAME_None,
		const FLinearColor& FallbackTint = FLinearColor(0.075f, 0.08f, 0.105f, 1.f),
		ESlateBrushDrawType::Type DrawAs = ESlateBrushDrawType::Box);

	static TSharedRef<SWidget> MakeFixedImage(
		ET66FriendslopChrome Chrome,
		const FVector2D& Size,
		FName Tag = NAME_None,
		const TCHAR* Role = TEXT("FriendslopFixedImage"));

	static TSharedRef<SWidget> MakeCustomFixedImage(
		const FString& RelativePath,
		const FMargin& Margin,
		ESlateBrushDrawType::Type DrawAs,
		const FVector2D& Size,
		FName Tag = NAME_None,
		const TCHAR* Role = TEXT("FriendslopCustomFixedImage"),
		const FLinearColor& FallbackTint = FLinearColor::White);
};
