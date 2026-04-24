// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LeaderboardScreen.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	struct FT66LeaderboardShellBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	const FSlateBrush* ResolveLeaderboardShellBrush(
		FT66LeaderboardShellBrushEntry& Entry,
		const TCHAR* RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = ESlateBrushDrawType::Box;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					true,
					TEXT("LeaderboardScreenReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* GetLeaderboardModalShellBrush()
	{
		static FT66LeaderboardShellBrushEntry Entry;
		return ResolveLeaderboardShellBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_content_shell_frame.png"),
			FVector2D(1521.f, 463.f),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f));
	}

	const FSlateBrush* GetLeaderboardListShellBrush()
	{
		static FT66LeaderboardShellBrushEntry Entry;
		return ResolveLeaderboardShellBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_row_shell_full.png"),
			FVector2D(861.f, 74.f),
			FMargin(0.055f, 0.32f, 0.055f, 0.32f));
	}

	TSharedRef<SWidget> MakeLeaderboardSpritePanel(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FSlateBrush* Brush = nullptr,
		const FLinearColor& FallbackColor = FLinearColor(0.025f, 0.023f, 0.034f, 0.97f))
	{
		const FSlateBrush* ShellBrush = Brush ? Brush : GetLeaderboardModalShellBrush();
		return SNew(SBorder)
			.BorderImage(ShellBrush ? ShellBrush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(ShellBrush ? FLinearColor::White : FallbackColor)
			.Padding(Padding)
			[
				Content
			];
	}
}

UT66LeaderboardScreen::UT66LeaderboardScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Leaderboard;
	bIsModal = true;
}

void UT66LeaderboardScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}
}

TSharedRef<SWidget> UT66LeaderboardScreen::BuildSlateUI()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;

	const FText TitleText = Loc ? Loc->GetText_Leaderboard() : NSLOCTEXT("T66.Leaderboard", "Title", "LEADERBOARD");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FLinearColor ScrimColor(FT66Style::Tokens::Scrim.R, FT66Style::Tokens::Scrim.G, FT66Style::Tokens::Scrim.B, 0.95f);
	const TSharedRef<SWidget> Content =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66LeaderboardScreen::HandleBackClicked), ET66ButtonType::Neutral)
					.SetMinWidth(120.f))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(TitleText)
				.Font(FT66Style::Tokens::FontBold(36))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(120.f)
			]
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			MakeLeaderboardSpritePanel(
				SNew(SBox)
				.Padding(FMargin(8.f, 4.f))
				[
					SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
					.LocalizationSubsystem(Loc)
					.LeaderboardSubsystem(LB)
					.UIManager(UIManager)
				],
				FMargin(18.f, 16.f),
				GetLeaderboardListShellBrush(),
				FLinearColor(0.035f, 0.032f, 0.045f, 0.96f))
		];

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ScrimColor)
		.Padding(FMargin(40.f))
		[
			MakeLeaderboardSpritePanel(Content, FMargin(24.f))
		];
}

FReply UT66LeaderboardScreen::HandleBackClicked()
{
	if (!UIManager)
	{
		return FReply::Handled();
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (PC->IsPaused())
		{
			UIManager->ShowModal(ET66ScreenType::PauseMenu);
			return FReply::Handled();
		}
	}

	UIManager->CloseModal();
	return FReply::Handled();
}

