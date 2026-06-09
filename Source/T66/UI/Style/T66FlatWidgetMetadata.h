// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/ISlateMetaData.h"
#include "UI/Style/T66FlatStyle.h"

/**
 * Runtime metadata attached by FT66FlatStyle helpers for the UI fidelity loop.
 * The Slate tag remains the primary lookup key; this carries T66-specific intent.
 */
class T66_API FT66FlatWidgetMetadata : public ISlateMetaData
{
public:
	SLATE_METADATA_TYPE(FT66FlatWidgetMetadata, ISlateMetaData)

	FT66FlatWidgetMetadata(
		const FName InTag,
		FString InIntendedRole,
		const ET66FlatState InIntendedState,
		const TOptional<FLinearColor>& InBorderColor = TOptional<FLinearColor>(),
		const bool bInHasClickHandler = false,
		const FName InToggleGroup = NAME_None,
		const bool bInIsLabel = false,
		const bool bInHoverCapable = false,
		const FName InTooltipId = NAME_None,
		FString InTooltipKind = FString(),
		const bool bInHasTooltip = false,
		const bool bInTooltipRequired = false)
		: Tag(InTag)
		, IntendedRole(MoveTemp(InIntendedRole))
		, IntendedState(InIntendedState)
		, BorderColor(InBorderColor)
		, bHasClickHandler(bInHasClickHandler)
		, ToggleGroup(InToggleGroup)
		, bIsLabel(bInIsLabel)
		, bHoverCapable(bInHoverCapable)
		, TooltipId(InTooltipId)
		, TooltipKind(MoveTemp(InTooltipKind))
		, bHasTooltip(bInHasTooltip)
		, bTooltipRequired(bInTooltipRequired)
	{
	}

	FName Tag = NAME_None;
	FString IntendedRole;
	ET66FlatState IntendedState = ET66FlatState::Default;
	TOptional<FLinearColor> BorderColor;
	bool bHasClickHandler = false;
	FName ToggleGroup = NAME_None;
	bool bIsLabel = false;
	bool bHoverCapable = false;
	FName TooltipId = NAME_None;
	FString TooltipKind;
	bool bHasTooltip = false;
	bool bTooltipRequired = false;
};
