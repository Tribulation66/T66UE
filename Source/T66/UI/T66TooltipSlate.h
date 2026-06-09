// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66TooltipTypes.h"

class IToolTip;
class SWidget;

namespace T66TooltipSlate
{
	T66_API TSharedRef<SWidget> MakeTooltipContent(const FT66TooltipPayload& Payload);
	T66_API TSharedPtr<IToolTip> MakeTooltip(const FT66TooltipPayload& Payload);
	T66_API TSharedPtr<IToolTip> MakeRichTooltip(const FText& Title, const FText& Description, int32 FontSizeAdjustment = 0);
	T66_API TSharedPtr<IToolTip> MakeTextTooltip(const FText& Text, int32 FontSizeAdjustment = 0);

	T66_API void SetTooltip(const TSharedPtr<SWidget>& Widget, const FT66TooltipPayload& Payload, bool bTooltipRequired = true);
	T66_API void SetTooltip(const TSharedRef<SWidget>& Widget, const FT66TooltipPayload& Payload, bool bTooltipRequired = true);
	T66_API TSharedRef<SWidget> WithTooltip(const TSharedRef<SWidget>& Widget, const FT66TooltipPayload& Payload, bool bTooltipRequired = true);
}
