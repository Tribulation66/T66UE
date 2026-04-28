// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AchievementsScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 T66AchievementsFontDelta = -6;
	constexpr int32 T66SecretPlaceholderRowCount = 10;
	const TCHAR* T66ProgressFillAssetPath = TEXT("SourceAssets/UI/MasterLibrary/Slices/Misc/progress_fill_dark_purple_twinkle_imagegen_20260427.png");
	TMap<FString, TStrongObjectPtr<UTexture2D>> GAchievementsGeneratedTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GAchievementsGeneratedBrushCache;
	TMap<FString, TSharedPtr<FButtonStyle>> GAchievementsGeneratedButtonStyleCache;

	bool T66IsPausedGameplayWidget(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		return PC && PC->IsPaused();
	}

	FLinearColor T66AchievementsShellFill()
	{
		return FLinearColor(0.004f, 0.005f, 0.010f, 0.985f);
	}

	FLinearColor T66AchievementsInsetFill()
	{
		return FLinearColor(0.024f, 0.025f, 0.030f, 1.0f);
	}

	FLinearColor T66AchievementsRowFill()
	{
		return FLinearColor(0.028f, 0.029f, 0.034f, 1.0f);
	}

	FLinearColor T66AchievementsUnlockedRowFill()
	{
		return FLinearColor(0.036f, 0.048f, 0.041f, 1.0f);
	}

	FLinearColor T66AchievementsTabActiveFill()
	{
		return FLinearColor(0.67f, 0.72f, 0.62f, 1.0f);
	}

	FLinearColor T66AchievementsTabInactiveFill()
	{
		return FLinearColor(0.10f, 0.11f, 0.15f, 1.0f);
	}

	FLinearColor T66AchievementsTabActiveText()
	{
		return FLinearColor(0.99f, 0.93f, 0.74f, 1.0f);
	}

	FLinearColor T66AchievementsTabInactiveText()
	{
		return FLinearColor(0.86f, 0.80f, 0.68f, 1.0f);
	}

	float T66AchievementProgress01(const FAchievementData& Achievement)
	{
		return Achievement.RequirementCount > 0
			? static_cast<float>(FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount))
				/ static_cast<float>(Achievement.RequirementCount)
			: (Achievement.bIsUnlocked ? 1.0f : 0.0f);
	}

	int32 T66AchievementRemainingCount(const FAchievementData& Achievement)
	{
		return FMath::Max(0, Achievement.RequirementCount - FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount));
	}

	int32 T66AchievementOrderKey(const FName AchievementID)
	{
		int32 NumericPart = 0;
		bool bHasDigit = false;
		for (const TCHAR Character : AchievementID.ToString())
		{
			if (FChar::IsDigit(Character))
			{
				NumericPart = (NumericPart * 10) + (Character - TEXT('0'));
				bHasDigit = true;
			}
		}

		return bHasDigit ? NumericPart : MAX_int32;
	}

	int32 AdjustAchievementsFontSize(int32 BaseSize)
	{
		return FMath::Max(8, BaseSize + T66AchievementsFontDelta);
	}

	FSlateFontInfo AchievementsBoldFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontBold(AdjustAchievementsFontSize(BaseSize));
	}

	FSlateFontInfo AchievementsRegularFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontRegular(AdjustAchievementsFontSize(BaseSize));
	}

	FString MakeSettingsAssetPath(const TCHAR* FileName)
	{
		const FString Name(FileName);
		const auto TopBarPath = [](const TCHAR* State) -> FString
		{
			return FString::Printf(TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/basic_button_%s.png"), State);
		};

		if (Name.StartsWith(TEXT("settings_toggle_on_")))
		{
			return TopBarPath(TEXT("pressed"));
		}
		if (Name.StartsWith(TEXT("settings_compact_neutral_")) || Name.StartsWith(TEXT("settings_toggle_off_")))
		{
			if (Name.Contains(TEXT("_hover"))) return TopBarPath(TEXT("hover"));
			if (Name.Contains(TEXT("_pressed"))) return TopBarPath(TEXT("pressed"));
			return TopBarPath(TEXT("normal"));
		}
		if (Name.StartsWith(TEXT("settings_toggle_inactive_")))
		{
			return TopBarPath(TEXT("disabled"));
		}
		if (Name == TEXT("settings_content_shell_frame.png"))
		{
			return TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/panel_large_normal.png");
		}
		if (Name == TEXT("settings_row_shell_full.png") || Name == TEXT("settings_row_shell_split.png"))
		{
			return TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/modal_frame_normal.png");
		}
		if (Name == TEXT("settings_dropdown_field.png"))
		{
			return TEXT("SourceAssets/UI/MasterLibrary/Slices/Controls/dropdown_field_normal.png");
		}

		return FString(TEXT("SourceAssets/UI/MasterLibrary/Slices/")) / Name;
	}

	FMargin GetAchievementsGeneratedBrushMargin(const FString& SourceRelativePath)
	{
		if (SourceRelativePath.Contains(TEXT("panel_large_normal.png")))
		{
			return FMargin(0.067f, 0.043f, 0.067f, 0.043f);
		}
		if (SourceRelativePath.Contains(TEXT("modal_frame_normal.png")))
		{
			return FMargin(0.052f, 0.094f, 0.052f, 0.094f);
		}
		if (SourceRelativePath.Contains(TEXT("dropdown_field_normal.png")))
		{
			return FMargin(0.06f, 0.34f, 0.06f, 0.34f);
		}
		if (SourceRelativePath.Contains(TEXT("progress_fill_dark_purple_twinkle")))
		{
			return FMargin(0.25f, 0.45f, 0.25f, 0.45f);
		}
		if (SourceRelativePath.Contains(TEXT("basic_button_")))
		{
			return FMargin(0.093f, 0.213f, 0.093f, 0.213f);
		}

		return FMargin(0.f);
	}

	bool IsZeroAchievementsMargin(const FMargin& Margin)
	{
		return FMath::IsNearlyZero(Margin.Left)
			&& FMath::IsNearlyZero(Margin.Top)
			&& FMath::IsNearlyZero(Margin.Right)
			&& FMath::IsNearlyZero(Margin.Bottom);
	}

	UTexture2D* LoadAchievementsGeneratedTexture(const FString& SourceRelativePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GAchievementsGeneratedTextureCache.Find(SourceRelativePath))
		{
			return CachedTexture->Get();
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				TextureFilter::TF_Trilinear,
				true,
				TEXT("AchievementsGeneratedUI"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					TEXT("AchievementsGeneratedUI"));
			}

			if (Texture)
			{
				GAchievementsGeneratedTextureCache.Add(SourceRelativePath, TStrongObjectPtr<UTexture2D>(Texture));
				return Texture;
			}
		}

		return nullptr;
	}

	const FSlateBrush* ResolveAchievementsGeneratedBrush(const FString& SourceRelativePath, const FVector2D& ImageSize = FVector2D::ZeroVector)
	{
		const FString BrushKey = FString::Printf(TEXT("%s::%.0fx%.0f"), *SourceRelativePath, ImageSize.X, ImageSize.Y);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GAchievementsGeneratedBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = LoadAchievementsGeneratedTexture(SourceRelativePath);
		if (!Texture)
		{
			return nullptr;
		}

		const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f
			? ImageSize
			: FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()));

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		const FMargin BrushMargin = GetAchievementsGeneratedBrushMargin(SourceRelativePath);
		Brush->DrawAs = IsZeroAchievementsMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolvedSize;
		Brush->Margin = BrushMargin;
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GAchievementsGeneratedBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}

	const FButtonStyle* ResolveAchievementsGeneratedButtonStyle(
		const FString& Key,
		const FString& NormalPath,
		const FString& HoverPath,
		const FString& PressedPath,
		const FString& DisabledPath)
	{
		if (const TSharedPtr<FButtonStyle>* CachedStyle = GAchievementsGeneratedButtonStyleCache.Find(Key))
		{
			return CachedStyle->Get();
		}

		const FButtonStyle& NoBorderStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		TSharedPtr<FButtonStyle> Style = MakeShared<FButtonStyle>(NoBorderStyle);
		if (const FSlateBrush* NormalBrush = ResolveAchievementsGeneratedBrush(NormalPath))
		{
			Style->SetNormal(*NormalBrush);
		}
		if (const FSlateBrush* HoverBrush = ResolveAchievementsGeneratedBrush(HoverPath))
		{
			Style->SetHovered(*HoverBrush);
		}
		if (const FSlateBrush* PressedBrush = ResolveAchievementsGeneratedBrush(PressedPath))
		{
			Style->SetPressed(*PressedBrush);
		}
		if (const FSlateBrush* DisabledBrush = ResolveAchievementsGeneratedBrush(DisabledPath))
		{
			Style->SetDisabled(*DisabledBrush);
		}
		Style->SetNormalPadding(FMargin(0.f));
		Style->SetPressedPadding(FMargin(0.f));

		GAchievementsGeneratedButtonStyleCache.Add(Key, Style);
		return Style.Get();
	}

	const FButtonStyle* ResolveAchievementsCompactButtonStyle()
	{
		return ResolveAchievementsGeneratedButtonStyle(
			TEXT("Achievements.CompactButton"),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_normal.png")),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_hover.png")),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_pressed.png")),
			MakeSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	const FButtonStyle* ResolveAchievementsToggleButtonStyle(const bool bActive)
	{
		return bActive
			? ResolveAchievementsGeneratedButtonStyle(
				TEXT("Achievements.ToggleOn"),
				MakeSettingsAssetPath(TEXT("settings_toggle_on_normal.png")),
				MakeSettingsAssetPath(TEXT("settings_toggle_on_hover.png")),
				MakeSettingsAssetPath(TEXT("settings_toggle_on_pressed.png")),
				MakeSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")))
			: ResolveAchievementsGeneratedButtonStyle(
				TEXT("Achievements.ToggleOff"),
				MakeSettingsAssetPath(TEXT("settings_toggle_off_normal.png")),
				MakeSettingsAssetPath(TEXT("settings_toggle_off_hover.png")),
				MakeSettingsAssetPath(TEXT("settings_toggle_off_pressed.png")),
				MakeSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	TSharedRef<SWidget> MakeAchievementsGeneratedPanel(
		const FString& SourceRelativePath,
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FLinearColor& Tint = FLinearColor::White,
		const FLinearColor& FallbackFill = T66AchievementsInsetFill())
	{
		if (const FSlateBrush* Brush = ResolveAchievementsGeneratedBrush(SourceRelativePath))
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(Tint)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeAchievementsGeneratedButton(
		const FT66ButtonParams& Params,
		const FButtonStyle* ButtonStyle,
		const FSlateFontInfo& Font,
		const FLinearColor& TextColor,
		const FMargin& ContentPadding)
	{
		return FT66Style::MakeBareButton(
			FT66BareButtonParams(
				Params.OnClicked,
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center))
			.SetButtonStyle(ButtonStyle ? ButtonStyle : &FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.SetPadding(ContentPadding)
			.SetHAlign(HAlign_Center)
			.SetVAlign(VAlign_Center)
			.SetEnabled(Params.IsEnabled)
			.SetMinWidth(Params.MinWidth)
			.SetHeight(Params.Height)
			.SetVisibility(Params.Visibility));
	}

	TSharedRef<SWidget> MakeAchievementsProgressBar(const float Percent, const float Height)
	{
		const float Pct = FMath::Clamp(Percent, 0.f, 1.f);
		const FSlateBrush* ShellBrush = ResolveAchievementsGeneratedBrush(MakeSettingsAssetPath(TEXT("settings_dropdown_field.png")));
		const FSlateBrush* FillBrush = ResolveAchievementsGeneratedBrush(T66ProgressFillAssetPath, FVector2D(512.f, 64.f));

		return SNew(SBorder)
			.BorderImage(ShellBrush ? ShellBrush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(ShellBrush ? FLinearColor::White : FLinearColor(0.02f, 0.02f, 0.04f, 1.0f))
			.Padding(FMargin(4.f, 3.f))
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SNew(SBox)
				.HeightOverride(Height)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(FMath::Max(Pct, 0.001f))
					[
						SNew(SBorder)
						.Visibility(Pct > 0.001f ? EVisibility::Visible : EVisibility::Collapsed)
						.BorderImage(FillBrush ? FillBrush : FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FillBrush ? FLinearColor::White : FLinearColor(0.14f, 0.02f, 0.36f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(FMath::Max(1.f - Pct, 0.001f))
					[
						SNew(SSpacer)
					]
				]
			];
	}
}

UT66AchievementsScreen::UT66AchievementsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Achievements;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66AchievementsScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66AchievementsSubsystem* UT66AchievementsScreen::GetAchievementsSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66AchievementsSubsystem>();
	}
	return nullptr;
}

UT66PlayerSettingsSubsystem* UT66AchievementsScreen::GetPlayerSettingsSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66PlayerSettingsSubsystem>();
	}
	return nullptr;
}

void UT66AchievementsScreen::RefreshAchievements()
{
	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		AllAchievements = Achievements->GetAllAchievements();
		AllAchievements.Sort([Achievements](const FAchievementData& Left, const FAchievementData& Right)
		{
			const bool bLeftClaimed = Achievements ? Achievements->IsAchievementClaimed(Left.AchievementID) : false;
			const bool bRightClaimed = Achievements ? Achievements->IsAchievementClaimed(Right.AchievementID) : false;
			if (bLeftClaimed != bRightClaimed)
			{
				return !bLeftClaimed && bRightClaimed;
			}

			const float LeftProgress = T66AchievementProgress01(Left);
			const float RightProgress = T66AchievementProgress01(Right);
			if (!FMath::IsNearlyEqual(LeftProgress, RightProgress))
			{
				return LeftProgress > RightProgress;
			}

			const int32 LeftRemaining = T66AchievementRemainingCount(Left);
			const int32 RightRemaining = T66AchievementRemainingCount(Right);
			if (LeftRemaining != RightRemaining)
			{
				return LeftRemaining < RightRemaining;
			}

			const int32 LeftOrder = T66AchievementOrderKey(Left.AchievementID);
			const int32 RightOrder = T66AchievementOrderKey(Right.AchievementID);
			if (LeftOrder != RightOrder)
			{
				return LeftOrder < RightOrder;
			}
			return Left.AchievementID.LexicalLess(Right.AchievementID);
		});
	}
	else
	{
		AllAchievements.Reset();
	}
}

int32 UT66AchievementsScreen::GetUnlockedAchievementCount() const
{
	int32 UnlockedCount = 0;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.bIsUnlocked)
		{
			++UnlockedCount;
		}
	}
	return UnlockedCount;
}

int32 UT66AchievementsScreen::GetUnlockedAchievementCountForCategory(const ET66AchievementCategory Category) const
{
	int32 UnlockedCount = 0;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.Category == Category && Achievement.bIsUnlocked)
		{
			++UnlockedCount;
		}
	}
	return UnlockedCount;
}

TArray<FAchievementData> UT66AchievementsScreen::GetAchievementsForCategory(const ET66AchievementCategory Category) const
{
	TArray<FAchievementData> FilteredAchievements;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.Category == Category)
		{
			FilteredAchievements.Add(Achievement);
		}
	}
	return FilteredAchievements;
}

FText UT66AchievementsScreen::BuildAchievementRewardText(const FAchievementData& Achievement) const
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	if (Achievement.RewardType == ET66AchievementRewardType::SkinUnlock)
	{
		const FText SkinName = Loc
			? Loc->GetText_SkinName(Achievement.RewardSkinID)
			: FText::FromName(Achievement.RewardSkinID);

		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (Achievement.RewardEntityType == ET66AchievementRewardEntityType::Hero)
			{
				FHeroData HeroData;
				if (T66GI->GetHeroData(Achievement.RewardEntityID, HeroData))
				{
					const FText HeroName = Loc
						? Loc->GetHeroDisplayName(HeroData)
						: (!HeroData.DisplayName.IsEmpty() ? HeroData.DisplayName : FText::FromName(Achievement.RewardEntityID));
					return FText::Format(
						NSLOCTEXT("T66.Achievements", "SkinRewardHeroFormat", "{0} Outfit: {1}"),
						SkinName,
						HeroName);
				}
			}
			else if (Achievement.RewardEntityType == ET66AchievementRewardEntityType::Companion)
			{
				FCompanionData CompanionData;
				if (T66GI->GetCompanionData(Achievement.RewardEntityID, CompanionData))
				{
					const FText CompanionName = Loc
						? Loc->GetCompanionDisplayName(CompanionData)
						: (!CompanionData.DisplayName.IsEmpty() ? CompanionData.DisplayName : FText::FromName(Achievement.RewardEntityID));
					return FText::Format(
						NSLOCTEXT("T66.Achievements", "SkinRewardCompanionFormat", "{0} Outfit: {1}"),
						SkinName,
						CompanionName);
				}
			}
		}

		return FText::Format(
			NSLOCTEXT("T66.Achievements", "SkinRewardFormat", "{0} Outfit"),
			SkinName);
	}

	if (Loc)
	{
		return FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Achievement.RewardCoins));
	}

	return FText::Format(NSLOCTEXT("T66.Achievements", "CoinsFormat", "{0} CC"), FText::AsNumber(Achievement.RewardCoins));
}

FText UT66AchievementsScreen::GetAchievementActionText(const FAchievementData& Achievement) const
{
	if (Achievement.RewardType == ET66AchievementRewardType::SkinUnlock)
	{
		return NSLOCTEXT("T66.Achievements", "UnlockReward", "UNLOCK");
	}

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		return Loc->GetText_Claim();
	}

	return NSLOCTEXT("T66.Achievements", "Claim", "CLAIM");
}

TSharedRef<SWidget> UT66AchievementsScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
	const FText SteamText = NSLOCTEXT("T66.Achievements", "SteamTab", "STEAM");
	const FText SecretText = NSLOCTEXT("T66.Achievements", "SecretTab", "SECRET");
	const FLinearColor ShellFill = T66AchievementsShellFill();
	const FLinearColor InsetFill = T66AchievementsInsetFill();
	const float TopInset = T66ScreenSlateHelpers::GetFrontendChromeTopInset(UIManager);

	RefreshAchievements();
	const TArray<FAchievementData> StandardAchievements = GetAchievementsForCategory(ET66AchievementCategory::Standard);
	const int32 UnlockedStandardAchievements = GetUnlockedAchievementCountForCategory(ET66AchievementCategory::Standard);
	const float StandardProgress = StandardAchievements.Num() > 0
		? static_cast<float>(UnlockedStandardAchievements) / static_cast<float>(StandardAchievements.Num())
		: 0.0f;
	const bool bShowingSecret = ActiveTab == EAchievementTab::Secret;

	auto MakeTabButton = [this](const FText& Label, bool bActive, FReply(UT66AchievementsScreen::*Handler)()) -> TSharedRef<SWidget>
	{
		return MakeAchievementsGeneratedButton(
			FT66ButtonParams(Label, FOnClicked::CreateUObject(this, Handler), bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
			.SetMinWidth(T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabMinWidth)
			.SetHeight(T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabHeight),
			ResolveAchievementsToggleButtonStyle(bActive),
			T66ScreenSlateHelpers::MakeFrontendChromeTabFont(),
			bActive ? T66AchievementsTabActiveText() : T66AchievementsTabInactiveText(),
			T66ScreenSlateHelpers::GetFrontendChromeMetrics().TabPadding);
	};

	const TSharedRef<SWidget> Root =
		SNew(SBox)
		.Padding(FMargin(14.f, TopInset, 14.f, 0.f))
		[
			MakeAchievementsGeneratedPanel(
				MakeSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(T66ScreenSlateHelpers::GetFrontendChromeMetrics().HeaderPadding)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(AchievementsText)
							.Font(T66ScreenSlateHelpers::MakeFrontendChromeTitleFont())
							.ColorAndOpacity(FLinearColor(0.83f, 0.68f, 0.34f, 1.0f))
							.ShadowOffset(FVector2D(0.f, 2.f))
							.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.75f))
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.f, 0.f, 8.f, 0.f)
							[
								MakeTabButton(
									SteamText,
									ActiveTab == EAchievementTab::Achievements,
									&UT66AchievementsScreen::HandleAchievementsTabClicked)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								MakeTabButton(
									SecretText,
									ActiveTab == EAchievementTab::Secret,
									&UT66AchievementsScreen::HandleSecretTabClicked)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 6.f, 0.f, 14.f)
					[
						MakeAchievementsGeneratedPanel(
							MakeSettingsAssetPath(TEXT("settings_row_shell_split.png")),
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(12.f, 0.f, 12.f, 8.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.f)
								[
									SNew(STextBlock)
									.Text_Lambda([bShowingSecret, UnlockedStandardAchievements, StandardAchievements]() -> FText
									{
										if (bShowingSecret)
										{
											return NSLOCTEXT("T66.Achievements", "SecretProgressMaskedLabel", "???");
										}

										return FText::Format(
											NSLOCTEXT("T66.Achievements", "CompletionLabel", "{0}/{1} ACHIEVEMENTS"),
											FText::AsNumber(UnlockedStandardAchievements),
											FText::AsNumber(StandardAchievements.Num()));
									})
									.Font(AchievementsBoldFont(18))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.HAlign(HAlign_Right)
								[
									SNew(STextBlock)
									.Text_Lambda([bShowingSecret, StandardProgress]() -> FText
									{
										return bShowingSecret
											? NSLOCTEXT("T66.Achievements", "SecretProgressMaskedPercent", "???")
											: FText::AsPercent(StandardProgress);
									})
									.Font(AchievementsBoldFont(17))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								]
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								MakeAchievementsProgressBar(bShowingSecret ? 0.0f : StandardProgress, 13.f)
							],
							FMargin(20.f, 18.f),
							FLinearColor::White,
							InsetFill)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					.Padding(0.f, 0.f, 0.f, 16.f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(AchievementListBox, SVerticalBox)
						]
					]
				],
				FMargin(18.f),
				FLinearColor::White,
				ShellFill)
		];

	RebuildAchievementList();
	if (const FSlateBrush* SceneBackgroundBrush = ResolveAchievementsGeneratedBrush(TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/main_menu_scene_plate_imagegen_20260425_v1.png")))
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(SceneBackgroundBrush)
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.48f))
			]
			+ SOverlay::Slot()
			[
				Root
			];
	}

	return Root;
}

void UT66AchievementsScreen::RebuildAchievementList()
{
	if (!AchievementListBox.IsValid())
	{
		return;
	}

	AchievementListBox->ClearChildren();
	RefreshAchievements();

	UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem();
	UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettingsSubsystem();
	const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
	const FText FavoriteAchievementTooltip = NSLOCTEXT("T66.Achievements", "FavoriteAchievementTooltip", "Favorite achievement");
	const FText UnfavoriteAchievementTooltip = NSLOCTEXT("T66.Achievements", "UnfavoriteAchievementTooltip", "Remove favorite");

	auto AddAchievementSection = [this, Achievements, PlayerSettings, &NoBorderButtonStyle, &FavoriteAchievementTooltip, &UnfavoriteAchievementTooltip](const FText& SectionTitle, const TArray<FAchievementData>& SectionAchievements)
	{
		if (!AchievementListBox.IsValid())
		{
			return;
		}

		int32 UnlockedInSection = 0;
		for (const FAchievementData& Achievement : SectionAchievements)
		{
			if (Achievement.bIsUnlocked)
			{
				++UnlockedInSection;
			}
		}

		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(FText::Format(
				NSLOCTEXT("T66.Achievements", "SectionTitleFormat", "{0}  {1}/{2}"),
				SectionTitle,
				FText::AsNumber(UnlockedInSection),
				FText::AsNumber(SectionAchievements.Num())))
			.Font(AchievementsBoldFont(22))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		];

		for (const FAchievementData& Achievement : SectionAchievements)
		{
			const FLinearColor RowBackground = Achievement.bIsUnlocked
				? T66AchievementsUnlockedRowFill()
				: T66AchievementsRowFill();
			const FLinearColor ProgressColor = Achievement.bIsUnlocked
				? FT66Style::Tokens::Success
				: FT66Style::Tokens::TextMuted;
			const FString ProgressString = FString::Printf(
				TEXT("%d/%d"),
				FMath::Min(Achievement.CurrentProgress, Achievement.RequirementCount),
				FMath::Max(1, Achievement.RequirementCount));
			const bool bClaimed = Achievements ? Achievements->IsAchievementClaimed(Achievement.AchievementID) : false;
			const bool bCanClaim = Achievement.bIsUnlocked && !bClaimed;
			const bool bIsFavorited = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Achievement.AchievementID);
			const FText RewardText = BuildAchievementRewardText(Achievement);
			const FText ActionText = GetAchievementActionText(Achievement);
			const FText FavoriteGlyph = FText::FromString(bIsFavorited ? TEXT("\u2605") : TEXT("\u2606"));
			const FLinearColor FavoriteGlyphColor = bIsFavorited
				? FLinearColor(0.94f, 0.78f, 0.28f, 1.0f)
				: FLinearColor(0.55f, 0.58f, 0.62f, 1.0f);

			AchievementListBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 5.f)
			[
				MakeAchievementsGeneratedPanel(
					MakeSettingsAssetPath(TEXT("settings_row_shell_full.png")),
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.55f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(Achievement.DisplayName)
							.Font(AchievementsBoldFont(19))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 3.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(Achievement.Description)
							.Font(AchievementsRegularFont(16))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.AutoWrapText(true)
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.17f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(ProgressString))
						.Font(AchievementsBoldFont(19))
						.ColorAndOpacity(ProgressColor)
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[  
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Right)
							[
								SNew(STextBlock)
								.Text(bClaimed
									? (GetLocSubsystem() ? GetLocSubsystem()->GetText_Claimed() : NSLOCTEXT("T66.Achievements", "Claimed", "CLAIMED"))
									: RewardText)
								.Font(AchievementsBoldFont(19))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(10.f, 0.f, 0.f, 0.f)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(28.f)
								.HeightOverride(28.f)
								[
									FT66Style::MakeBareButton(
										FT66BareButtonParams(
											FOnClicked::CreateLambda([this, PlayerSettings, AchievementID = Achievement.AchievementID]()
											{
												if (PlayerSettings)
												{
													PlayerSettings->SetFavoriteAchievement(AchievementID, !PlayerSettings->IsFavoriteAchievement(AchievementID));
													ForceRebuildSlate();
												}
												return FReply::Handled();
											}),
											SNew(SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(FLinearColor(0.10f, 0.11f, 0.14f, 0.94f))
											.Padding(FMargin(0.f))
											[
												SNew(STextBlock)
												.Text(FavoriteGlyph)
												.Font(FT66Style::Tokens::FontRegular(20))
												.ColorAndOpacity(FavoriteGlyphColor)
												.Justification(ETextJustify::Center)
											])
										.SetButtonStyle(&NoBorderButtonStyle)
										.SetPadding(FMargin(0.f))
										.SetEnabled(PlayerSettings != nullptr)
										.SetToolTipText(bIsFavorited ? UnfavoriteAchievementTooltip : FavoriteAchievementTooltip))
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Right)
						.Padding(0.f, 4.f, 0.f, 0.f)
						[
							MakeAchievementsGeneratedButton(
								FT66ButtonParams(
									ActionText,
									FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleClaimClicked, Achievement.AchievementID),
									ET66ButtonType::Primary)
								.SetMinWidth(0.f)
								.SetHeight(38.f)
								.SetEnabled(bCanClaim)
								.SetVisibility(bCanClaim ? EVisibility::Visible : EVisibility::Collapsed),
								ResolveAchievementsCompactButtonStyle(),
								AchievementsBoldFont(18),
								FT66Style::Tokens::Text,
								FMargin(16.f, 7.f, 16.f, 6.f))
						]
					]
					,
					FMargin(20.f, 14.f),
					Achievement.bIsUnlocked ? FLinearColor(0.92f, 1.0f, 0.88f, 1.0f) : FLinearColor::White,
					RowBackground)
			];
		}
	};

	if (ActiveTab == EAchievementTab::Achievements)
	{
		AddAchievementSection(
			NSLOCTEXT("T66.Achievements", "AchievementsHeader", "ACHIEVEMENTS"),
			GetAchievementsForCategory(ET66AchievementCategory::Standard));
		return;
	}

	const FText MaskedText = NSLOCTEXT("T66.Achievements", "SecretMaskedText", "???");
	AchievementListBox->AddSlot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 10.f)
	[
		SNew(STextBlock)
		.Text(MaskedText)
		.Font(AchievementsBoldFont(22))
		.ColorAndOpacity(FT66Style::Tokens::Text)
	];

	for (int32 RowIndex = 0; RowIndex < T66SecretPlaceholderRowCount; ++RowIndex)
	{
		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 5.f)
		[
			MakeAchievementsGeneratedPanel(
				MakeSettingsAssetPath(TEXT("settings_row_shell_full.png")),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.55f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(MaskedText)
						.Font(AchievementsBoldFont(19))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 3.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(MaskedText)
						.Font(AchievementsRegularFont(16))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.17f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(MaskedText)
					.Font(AchievementsBoldFont(19))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.28f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text(MaskedText)
						.Font(AchievementsBoldFont(19))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						MakeAchievementsGeneratedButton(
							FT66ButtonParams(
								MaskedText,
								FOnClicked::CreateLambda([]()
								{
									return FReply::Handled();
								}),
								ET66ButtonType::Primary)
							.SetMinWidth(0.f)
							.SetHeight(38.f)
							.SetEnabled(false),
							ResolveAchievementsCompactButtonStyle(),
							AchievementsBoldFont(18),
							FT66Style::Tokens::TextMuted,
							FMargin(16.f, 7.f, 16.f, 6.f))
					]
				]
				,
				FMargin(20.f, 14.f),
				RowIndex % 2 == 0 ? FLinearColor::White : FLinearColor(0.92f, 1.0f, 0.88f, 1.0f),
				RowIndex % 2 == 0 ? T66AchievementsRowFill() : T66AchievementsUnlockedRowFill())
		];
	}
}

void UT66AchievementsScreen::OnScreenActivated_Implementation()
{
	FString RequestedAchievementsTab;
	bool bShouldRebuildForRequestedTab = false;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66AchievementsTab="), RequestedAchievementsTab))
	{
		EAchievementTab RequestedTab = ActiveTab;
		bool bHasValidRequestedTab = true;
		if (RequestedAchievementsTab.Equals(TEXT("Secret"), ESearchCase::IgnoreCase))
		{
			RequestedTab = EAchievementTab::Secret;
		}
		else if (
			RequestedAchievementsTab.Equals(TEXT("Achievements"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Achievement"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Steam"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Standard"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Normal"), ESearchCase::IgnoreCase))
		{
			RequestedTab = EAchievementTab::Achievements;
		}
		else
		{
			bHasValidRequestedTab = false;
		}

		if (bHasValidRequestedTab && ActiveTab != RequestedTab)
		{
			ActiveTab = RequestedTab;
			bShouldRebuildForRequestedTab = HasBuiltSlateUI();
		}
	}

	Super::OnScreenActivated_Implementation();
	RebuildAchievementList();

	if (bShouldRebuildForRequestedTab)
	{
		ForceRebuildSlate();
	}

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66AchievementsScreen::HandleLanguageChanged);
	}

	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->AchievementsStateChanged.AddUniqueDynamic(this, &UT66AchievementsScreen::HandleAchievementsStateChanged);
	}
}

void UT66AchievementsScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66AchievementsScreen::HandleLanguageChanged);
	}

	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->AchievementsStateChanged.RemoveDynamic(this, &UT66AchievementsScreen::HandleAchievementsStateChanged);
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66AchievementsScreen::OnBackClicked()
{
	if (T66IsPausedGameplayWidget(this) && UIManager)
	{
		ShowModal(ET66ScreenType::PauseMenu);
		return;
	}

	NavigateBack();
}

FReply UT66AchievementsScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleClaimClicked(FName AchievementID)
{
	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->TryClaimAchievement(AchievementID);
	}
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleAchievementsTabClicked()
{
	if (ActiveTab == EAchievementTab::Achievements)
	{
		return FReply::Handled();
	}

	ActiveTab = EAchievementTab::Achievements;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleSecretTabClicked()
{
	if (ActiveTab == EAchievementTab::Secret)
	{
		return FReply::Handled();
	}

	ActiveTab = EAchievementTab::Secret;
	ForceRebuildSlate();
	return FReply::Handled();
}

void UT66AchievementsScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}

void UT66AchievementsScreen::HandleAchievementsStateChanged()
{
	if (!HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	RebuildAchievementList();
}
