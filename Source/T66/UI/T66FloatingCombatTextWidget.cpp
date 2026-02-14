// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FloatingCombatTextWidget.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Text/STextBlock.h"
#include "Core/T66FloatingCombatTextSubsystem.h"

void UT66FloatingCombatTextWidget::SetDamageNumber(int32 Amount, FName EventType)
{
	FText DisplayText = FText::AsNumber(Amount);
	int32 Size = 18;
	FLinearColor Color(1.f, 0.95f, 0.7f, 1.f); // default: light yellow

	if (EventType == UT66FloatingCombatTextSubsystem::EventType_Crit)
	{
		Size = 24;
		Color = FLinearColor(1.f, 0.4f, 0.2f, 1.f); // orange-red for crit
	}
	else if (EventType == UT66FloatingCombatTextSubsystem::EventType_DoT)
	{
		Size = 16;
		Color = FLinearColor(0.85f, 0.5f, 1.f, 1.f); // purple for DoT
	}

	UpdateTextBlock(DisplayText, Size, Color);
}

void UT66FloatingCombatTextWidget::SetStatusEvent(FName EventType)
{
	FText DisplayText;
	int32 Size = 20;
	FLinearColor Color(1.f, 0.95f, 0.7f, 1.f);

	if (EventType == UT66FloatingCombatTextSubsystem::EventType_Crit)
	{
		DisplayText = NSLOCTEXT("T66.FloatingCombatText", "Status_Crit", "CRIT!");
		Size = 26;
		Color = FLinearColor(1.f, 0.35f, 0.15f, 1.f);
	}
	else if (EventType == UT66FloatingCombatTextSubsystem::EventType_DoT)
	{
		DisplayText = NSLOCTEXT("T66.FloatingCombatText", "Status_DoT", "DoT");
		Size = 18;
		Color = FLinearColor(0.85f, 0.5f, 1.f, 1.f);
	}
	else if (EventType == UT66FloatingCombatTextSubsystem::EventType_Burn)
	{
		DisplayText = NSLOCTEXT("T66.FloatingCombatText", "Status_Burn", "BURN!");
		Size = 22;
		Color = FLinearColor(0.95f, 0.25f, 0.1f, 1.f);
	}
	else if (EventType == UT66FloatingCombatTextSubsystem::EventType_Chill)
	{
		DisplayText = NSLOCTEXT("T66.FloatingCombatText", "Status_Chill", "CHILL!");
		Size = 22;
		Color = FLinearColor(0.2f, 0.6f, 0.95f, 1.f);
	}
	else if (EventType == UT66FloatingCombatTextSubsystem::EventType_Curse)
	{
		DisplayText = NSLOCTEXT("T66.FloatingCombatText", "Status_Curse", "CURSE!");
		Size = 22;
		Color = FLinearColor(0.65f, 0.2f, 0.9f, 1.f);
	}
	else if (EventType == UT66FloatingCombatTextSubsystem::EventType_LevelUp)
	{
		DisplayText = NSLOCTEXT("T66.FloatingCombatText", "Status_LevelUp", "LEVEL UP!");
		Size = 26;
		Color = FLinearColor(0.2f, 1.f, 0.5f, 1.f);
	}
	else
	{
		DisplayText = FText::FromName(EventType);
	}

	UpdateTextBlock(DisplayText, Size, Color);
}

void UT66FloatingCombatTextWidget::UpdateTextBlock(const FText& InText, int32 FontSize, const FLinearColor& Color)
{
	CachedText = InText;
	CachedFontSize = FontSize;
	CachedColor = Color;

	if (TextBlock)
	{
		TextBlock->SetText(InText);
		TextBlock->SetFont(FT66Style::Tokens::FontBold(FontSize));
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
	}
}

TSharedRef<SWidget> UT66FloatingCombatTextWidget::RebuildWidget()
{
	TextBlock = SNew(STextBlock)
		.Text(CachedText)
		.Font(FT66Style::Tokens::FontBold(CachedFontSize))
		.ColorAndOpacity(FSlateColor(CachedColor));

	return TextBlock.ToSharedRef();
}
