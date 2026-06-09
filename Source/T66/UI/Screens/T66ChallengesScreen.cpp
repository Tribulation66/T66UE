// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ChallengesScreen.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66WebImageCache.h"
#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	enum class ETabIndex : int32
	{
		Challenges = 0,
		Mods = 1,
		Count,
	};

	enum class ESourceTabIndex : int32
	{
		Official = 0,
		Community = 1,
		Count,
	};

	enum class ET66ChallengeButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		ToggleInactive
	};


	struct FT66ChallengeSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
		bool bSimpleFallback = false;
	};


	UT66GameInstance* GetT66GameInstance(const UObject* Context)
	{
		return Context ? Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(Context)) : nullptr;
	}

	FLinearColor ChallengeShellFill()
	{
		return FT66FlatStyle::BackgroundColor();
	}

	FLinearColor ChallengePanelFill()
	{
		return FT66FlatStyle::DefaultFill();
	}

	FLinearColor ChallengePanelInsetFill()
	{
		return FT66FlatStyle::DefaultFill();
	}


	FLinearColor ChallengeRewardTint()
	{
		return FT66FlatStyle::SelectedText();
	}

	FLinearColor ChallengeDangerTint()
	{
		return FLinearColor(0.89f, 0.29f, 0.25f, 1.0f);
	}

	FLinearColor ChallengeSuccessTint()
	{
		return FT66FlatStyle::SelectedText();
	}

	FLinearColor ChallengeMutedBadgeTint()
	{
		return FT66FlatStyle::DefaultFill();
	}

	const FLinearColor ChallengeFantasyText(0.863f, 0.843f, 0.922f, 1.0f);
	const FLinearColor ChallengeFantasyMuted(0.541f, 0.541f, 0.584f, 1.0f);
	const FLinearColor ChallengePaperText(0.863f, 0.843f, 0.922f, 1.0f);
	const FLinearColor ChallengePaperMuted(0.541f, 0.541f, 0.584f, 1.0f);
	const FLinearColor ChallengeGoldText(1.0f, 0.314f, 0.373f, 1.0f);

	const FSlateBrush* ResolveChallengeSpriteBrush(
		FT66ChallengeSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = DrawAs;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid() && !Entry.bSimpleFallback)
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("ChallengesReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		if (Entry.Texture.IsValid())
		{
			Entry.bSimpleFallback = false;
			Entry.Brush->SetResourceObject(Entry.Texture.Get());
			return Entry.Brush.Get();
		}

		if (T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(RelativePath))
		{
			Entry.bSimpleFallback = true;
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Entry.Brush,
				RelativePath,
				ImageSize,
				Margin,
				DrawAs);
			return Entry.Brush.Get();
		}

		Entry.bSimpleFallback = false;
		Entry.Brush->SetResourceObject(nullptr);
		return nullptr;
	}

	const FScrollBarStyle* GetChallengeReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static FSlateColorBrush TrackBrush(FT66FlatStyle::DisabledBorder());
		static FSlateColorBrush ThumbBrush(FT66FlatStyle::SelectedBorder());
		static FSlateColorBrush HoverBrush(FT66FlatStyle::SelectedText());

		Style
			.SetVerticalBackgroundImage(TrackBrush)
			.SetVerticalTopSlotImage(TrackBrush)
			.SetVerticalBottomSlotImage(TrackBrush)
			.SetNormalThumbImage(ThumbBrush)
			.SetHoveredThumbImage(HoverBrush)
			.SetDraggedThumbImage(HoverBrush)
			.SetThickness(14.f);

		return &Style;
	}

	TSharedRef<SWidget> MakeChallengeSpritePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FLinearColor& FallbackColor)
	{
		(void)Brush;
		(void)FallbackColor;
		return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, Padding, Content);
	}

	TSharedRef<SWidget> MakeChallengeHorizontalSlicedPanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const float Height,
		const FMargin& Padding,
		const FLinearColor& FallbackColor,
		const float SourceCapFraction = 0.105f)
	{
		(void)Brush;
		(void)FallbackColor;
		(void)SourceCapFraction;
		return SNew(SBox)
			.HeightOverride(Height)
			[
				FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, Padding, Content)
			];
	}

	TSharedRef<SWidget> MakeChallengeTagPill(const FText& Label)
	{
		return SNew(SBox)
			.WidthOverride(92.f)
			.HeightOverride(28.f)
			[
				MakeChallengeHorizontalSlicedPanel(
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66FlatStyle::Tokens::FontBold(10))
					.ColorAndOpacity(ChallengeGoldText)
					.Justification(ETextJustify::Center)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds),
					nullptr,
					28.f,
					FMargin(8.f, 4.f, 8.f, 3.f),
					ChallengeMutedBadgeTint(),
					0.180f)
			];
	}

	TSharedRef<SWidget> MakeChallengeSpriteButtonContent(
		const TSharedRef<SWidget>& Content,
		const FOnClicked& OnClicked,
		const ET66ChallengeButtonFamily Family,
		const float MinWidth,
		const float Height,
		const FMargin& ContentPadding)
	{
		const ET66FlatState State = Family == ET66ChallengeButtonFamily::ToggleOn
			? ET66FlatState::Selected
			: (Family == ET66ChallengeButtonFamily::ToggleInactive ? ET66FlatState::Disabled : ET66FlatState::Default);
		const bool bEnabled = Family != ET66ChallengeButtonFamily::ToggleInactive && OnClicked.IsBound();
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				Content
			],
			OnClicked,
			ContentPadding,
			MinWidth,
			Height,
			bEnabled);
	}

	TSharedRef<SWidget> MakeChallengeSpriteButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ChallengeButtonFamily Family,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const FMargin& ContentPadding = FMargin(12.f, 7.f, 12.f, 6.f))
	{
		return MakeChallengeSpriteButtonContent(
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::DownOnly)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FT66FlatStyle::Tokens::FontBold(FontSize))
				.ColorAndOpacity(Family == ET66ChallengeButtonFamily::ToggleInactive ? ChallengeFantasyMuted : ChallengeFantasyText)
				.Justification(ETextJustify::Center)
				.AutoWrapText(false)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			],
			OnClicked,
			Family,
			MinWidth,
			Height,
			ContentPadding);
	}

	ET66CommunityContentKind TabIndexToKind(const int32 TabIndex)
	{
		(void)TabIndex;
		return ET66CommunityContentKind::Challenge;
	}
}

UT66ChallengesScreen::UT66ChallengesScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Challenges;
	bIsModal = false;
}

void UT66ChallengesScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (!bCommunityDelegateBound)
		{
			Community->OnContentChanged().AddUObject(this, &UT66ChallengesScreen::HandleCommunityContentChanged);
			bCommunityDelegateBound = true;
		}
	}

	ApplyCommandLineStartupMode();
}

void UT66ChallengesScreen::OnScreenDeactivated_Implementation()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		Community->OnContentChanged().RemoveAll(this);
	}

	bCommunityDelegateBound = false;
	Super::OnScreenDeactivated_Implementation();
}

UT66CommunityContentSubsystem* UT66ChallengesScreen::GetCommunitySubsystem() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UT66CommunityContentSubsystem>();
	}

	return nullptr;
}

ET66CommunityContentKind UT66ChallengesScreen::GetActiveKind() const
{
	return TabIndexToKind(ActiveTabIndex);
}

void UT66ChallengesScreen::OpenContentKind(const ET66CommunityContentKind ContentKind)
{
	(void)ContentKind;
	InitializeSelectionState();
	ActiveTabIndex = static_cast<int32>(ETabIndex::Challenges);
	EndDraftEditor();
	RequestDeferredSlateRebuild();
}

void UT66ChallengesScreen::ApplyCommandLineStartupMode()
{
	if (bAppliedCommandLineStartupMode)
	{
		return;
	}

	FString RequestedMode;
	if (!FParse::Value(FCommandLine::Get(), TEXT("T66ChallengesMode="), RequestedMode) || RequestedMode.IsEmpty())
	{
		return;
	}

	bAppliedCommandLineStartupMode = true;
	InitializeSelectionState();

	if (RequestedMode.Equals(TEXT("Mods"), ESearchCase::IgnoreCase))
	{
		OpenContentKind(ET66CommunityContentKind::Challenge);
		return;
	}

	const bool bDeprecatedCreateMod = RequestedMode.Equals(TEXT("CreateMod"), ESearchCase::IgnoreCase)
		|| RequestedMode.Equals(TEXT("ModEditor"), ESearchCase::IgnoreCase);
	const bool bCreateChallenge = RequestedMode.Equals(TEXT("CreateChallenge"), ESearchCase::IgnoreCase)
		|| RequestedMode.Equals(TEXT("ChallengeEditor"), ESearchCase::IgnoreCase)
		|| bDeprecatedCreateMod;
	if (!bCreateChallenge)
	{
		return;
	}

	const ET66CommunityContentKind DraftKind = ET66CommunityContentKind::Challenge;
	ActiveTabIndex = static_cast<int32>(ETabIndex::Challenges);
	ActiveSourceTabIndex[ActiveTabIndex] = static_cast<int32>(ESourceTabIndex::Community);

	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		BeginDraftEditor(Community->CreateDraftTemplate(DraftKind));
		RequestDeferredSlateRebuild();
	}
}

TArray<FT66CommunityContentEntry> UT66ChallengesScreen::GetEntriesForView(const int32 TabIndex, const int32 SourceTabIndex) const
{
	const UT66CommunityContentSubsystem* Community = GetCommunitySubsystem();
	if (!Community)
	{
		return {};
	}

	const ET66CommunityContentKind Kind = TabIndexToKind(TabIndex);
	return SourceTabIndex == static_cast<int32>(ESourceTabIndex::Official)
		? TArray<FT66CommunityContentEntry>(Community->GetOfficialEntries(Kind))
		: Community->GetCommunityBrowserEntries(Kind);
}

bool UT66ChallengesScreen::FindSelectedEntryForView(const int32 TabIndex, const int32 SourceTabIndex, FT66CommunityContentEntry& OutEntry)
{
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(TabIndex, SourceTabIndex);
	if (Entries.Num() <= 0)
	{
		return false;
	}

	const FName SelectedId = GetSelectedEntryIdForView(TabIndex, SourceTabIndex);
	const FT66CommunityContentEntry* Found = Entries.FindByPredicate([SelectedId](const FT66CommunityContentEntry& Entry)
	{
		return Entry.LocalId == SelectedId;
	});

	OutEntry = Found ? *Found : Entries[0];
	return true;
}

bool UT66ChallengesScreen::FindCurrentSelectedEntry(FT66CommunityContentEntry& OutEntry)
{
	return FindSelectedEntryForView(
		ActiveTabIndex,
		ActiveSourceTabIndex[ActiveTabIndex],
		OutEntry);
}

bool UT66ChallengesScreen::FindConfirmedEntry(FT66CommunityContentEntry& OutEntry) const
{
	if (const UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		return Community->GetActiveEntry(OutEntry);
	}

	return false;
}

FName UT66ChallengesScreen::GetSelectedEntryIdForView(const int32 TabIndex, const int32 SourceTabIndex)
{
	InitializeSelectionState();

	const int32 SafeTabIndex = FMath::Clamp(TabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
	const int32 SafeSourceIndex = FMath::Clamp(SourceTabIndex, 0, static_cast<int32>(ESourceTabIndex::Count) - 1);
	FName& SelectedId = PendingSelections[SafeTabIndex][SafeSourceIndex];
	if (!SelectedId.IsNone())
	{
		return SelectedId;
	}

	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(SafeTabIndex, SafeSourceIndex);
	if (Entries.Num() > 0)
	{
		SelectedId = Entries[0].LocalId;
	}

	return SelectedId;
}

FString UT66ChallengesScreen::GetOriginLabel(const FT66CommunityContentEntry& Entry) const
{
	switch (Entry.Origin)
	{
	case ET66CommunityContentOrigin::Draft:
		return TEXT("Draft");
	case ET66CommunityContentOrigin::Community:
		return TEXT("Community");
	case ET66CommunityContentOrigin::Official:
	default:
		return TEXT("Official");
	}
}

FString UT66ChallengesScreen::GetDraftSubmissionLabel(const FT66CommunityContentEntry& Entry) const
{
	if (!Entry.SubmissionStatus.IsEmpty())
	{
		return Entry.SubmissionStatus;
	}

	if (!Entry.ReviewNote.IsEmpty())
	{
		return Entry.ReviewNote;
	}

	if (!Entry.ModerationStatus.IsEmpty())
	{
		return Entry.ModerationStatus;
	}

	return TEXT("Not submitted");
}

FString UT66ChallengesScreen::GetPassiveLabel(const ET66PassiveType PassiveType) const
{
	if (const UEnum* Enum = StaticEnum<ET66PassiveType>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(PassiveType)).ToString();
	}

	return TEXT("None");
}

FString UT66ChallengesScreen::GetUltimateLabel(const ET66UltimateType UltimateType) const
{
	if (const UEnum* Enum = StaticEnum<ET66UltimateType>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(UltimateType)).ToString();
	}

	return TEXT("None");
}

FString UT66ChallengesScreen::GetItemLabel(const FName ItemId) const
{
	return ItemId.IsNone() ? TEXT("None") : ItemId.ToString();
}

TArray<FName> UT66ChallengesScreen::GetSelectableItemIds() const
{
	TArray<FName> Result;
	Result.Add(NAME_None);

	if (UT66GameInstance* T66GI = GetT66GameInstance(this))
	{
		if (UDataTable* ItemsTable = T66GI->GetItemsDataTable())
		{
			TArray<FName> RowNames = ItemsTable->GetRowNames();
			RowNames.Sort([](const FName& A, const FName& B)
			{
				return A.LexicalLess(B);
			});
			Result.Append(RowNames);
		}
	}

	return Result;
}

const FSlateBrush* UT66ChallengesScreen::GetOrCreateAvatarBrush(const FString& AvatarUrl)
{
	if (!DefaultAvatarBrush.IsValid())
	{
		DefaultAvatarBrush = MakeShared<FSlateBrush>();
		DefaultAvatarBrush->DrawAs = ESlateBrushDrawType::Image;
		DefaultAvatarBrush->ImageSize = FVector2D(52.0f, 52.0f);
		DefaultAvatarBrush->TintColor = FSlateColor(FLinearColor(0.14f, 0.15f, 0.17f, 0.32f));
	}

	if (AvatarUrl.IsEmpty())
	{
		return DefaultAvatarBrush.Get();
	}

	if (TSharedPtr<FSlateBrush>* Found = AvatarBrushes.Find(AvatarUrl))
	{
		return Found->Get();
	}

	const UGameInstance* GI = GetGameInstance();
	UT66WebImageCache* ImageCache = GI ? GI->GetSubsystem<UT66WebImageCache>() : nullptr;
	if (!ImageCache)
	{
		return DefaultAvatarBrush.Get();
	}

	if (UTexture2D* CachedTexture = ImageCache->GetCachedImage(AvatarUrl))
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->SetResourceObject(CachedTexture);
		Brush->ImageSize = FVector2D(52.0f, 52.0f);
		AvatarBrushes.Add(AvatarUrl, Brush);
		return Brush.Get();
	}

	TWeakObjectPtr<UT66ChallengesScreen> WeakScreen(this);
	ImageCache->RequestImage(AvatarUrl, [WeakScreen](UTexture2D* Texture)
	{
		if (Texture)
		{
			if (UT66ChallengesScreen* Screen = WeakScreen.Get())
			{
				Screen->RequestDeferredSlateRebuild();
			}
		}
	});

	return DefaultAvatarBrush.Get();
}

void UT66ChallengesScreen::InitializeSelectionState()
{
	if (bSelectionStateInitialized)
	{
		return;
	}

	bSelectionStateInitialized = true;
	ActiveTabIndex = static_cast<int32>(ETabIndex::Challenges);
	ActiveSourceTabIndex[static_cast<int32>(ETabIndex::Challenges)] = static_cast<int32>(ESourceTabIndex::Official);
	ActiveSourceTabIndex[static_cast<int32>(ETabIndex::Mods)] = static_cast<int32>(ESourceTabIndex::Official);

	for (int32 TabIndex = 0; TabIndex < static_cast<int32>(ETabIndex::Count); ++TabIndex)
	{
		for (int32 SourceTabIndex = 0; SourceTabIndex < static_cast<int32>(ESourceTabIndex::Count); ++SourceTabIndex)
		{
			const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(TabIndex, SourceTabIndex);
			PendingSelections[TabIndex][SourceTabIndex] = Entries.Num() > 0 ? Entries[0].LocalId : NAME_None;
		}
	}

	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		FT66CommunityContentEntry ActiveEntry;
		if (Community->GetActiveEntry(ActiveEntry))
		{
			if (ActiveEntry.Kind == ET66CommunityContentKind::Mod)
			{
				Community->ClearActiveEntry();
			}
			else
			{
				ActiveTabIndex = static_cast<int32>(ETabIndex::Challenges);
				ActiveSourceTabIndex[ActiveTabIndex] = ActiveEntry.Origin == ET66CommunityContentOrigin::Official
					? static_cast<int32>(ESourceTabIndex::Official)
					: static_cast<int32>(ESourceTabIndex::Community);
				PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = ActiveEntry.LocalId;
				return;
			}
		}
	}

	if (UT66GameInstance* T66GI = GetT66GameInstance(this))
	{
		if (T66GI->SelectedRunModifierKind == ET66RunModifierKind::Mod)
		{
			T66GI->SelectedRunModifierKind = ET66RunModifierKind::None;
			T66GI->SelectedRunModifierID = NAME_None;
		}
	}
}

void UT66ChallengesScreen::BeginDraftEditor(const FT66CommunityContentEntry& DraftEntry)
{
	bDraftEditorActive = true;
	DraftEditorEntry = DraftEntry;
	DraftEditorEntry.Kind = ET66CommunityContentKind::Challenge;
	ActiveTabIndex = static_cast<int32>(ETabIndex::Challenges);
	ActiveSourceTabIndex[ActiveTabIndex] = static_cast<int32>(ESourceTabIndex::Community);
	PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = DraftEntry.LocalId;
}

void UT66ChallengesScreen::EndDraftEditor()
{
	bDraftEditorActive = false;
	DraftEditorEntry = FT66CommunityContentEntry{};
}

void UT66ChallengesScreen::CycleDraftPassive(const int32 Direction)
{
	const UEnum* Enum = StaticEnum<ET66PassiveType>();
	if (!Enum)
	{
		return;
	}

	TArray<ET66PassiveType> Values;
	for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
	{
		Values.Add(static_cast<ET66PassiveType>(Enum->GetValueByIndex(EnumIndex)));
	}

	const int32 CurrentIndex = Values.IndexOfByKey(DraftEditorEntry.Rules.PassiveOverride);
	const int32 NextIndex = Values.IsValidIndex(CurrentIndex)
		? (CurrentIndex + Direction + Values.Num()) % Values.Num()
		: 0;
	DraftEditorEntry.Rules.PassiveOverride = Values.IsValidIndex(NextIndex) ? Values[NextIndex] : ET66PassiveType::None;
}

void UT66ChallengesScreen::CycleDraftUltimate(const int32 Direction)
{
	const UEnum* Enum = StaticEnum<ET66UltimateType>();
	if (!Enum)
	{
		return;
	}

	TArray<ET66UltimateType> Values;
	for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
	{
		Values.Add(static_cast<ET66UltimateType>(Enum->GetValueByIndex(EnumIndex)));
	}

	const int32 CurrentIndex = Values.IndexOfByKey(DraftEditorEntry.Rules.UltimateOverride);
	const int32 NextIndex = Values.IsValidIndex(CurrentIndex)
		? (CurrentIndex + Direction + Values.Num()) % Values.Num()
		: 0;
	DraftEditorEntry.Rules.UltimateOverride = Values.IsValidIndex(NextIndex) ? Values[NextIndex] : ET66UltimateType::None;
}

void UT66ChallengesScreen::CycleDraftStartingItem(const int32 Direction)
{
	const TArray<FName> ItemIds = GetSelectableItemIds();
	if (ItemIds.Num() <= 0)
	{
		DraftEditorEntry.Rules.StartingItemId = NAME_None;
		return;
	}

	const int32 CurrentIndex = ItemIds.IndexOfByKey(DraftEditorEntry.Rules.StartingItemId);
	const int32 SafeCurrentIndex = CurrentIndex != INDEX_NONE ? CurrentIndex : 0;
	const int32 NextIndex = (SafeCurrentIndex + Direction + ItemIds.Num()) % ItemIds.Num();
	DraftEditorEntry.Rules.StartingItemId = ItemIds[NextIndex];
}

TSharedRef<SWidget> UT66ChallengesScreen::BuildSlateUI()
{
	InitializeSelectionState();

	UT66CommunityContentSubsystem* Community = GetCommunitySubsystem();
	if (Community && !bRequestedCommunityRefresh)
	{
		bRequestedCommunityRefresh = true;
		Community->RefreshCommunityCatalog(false);
		Community->RefreshMySubmissionStates(false);
	}

	const float ListPanelWidth = 895.0f;
	const float DetailPanelWidth = 893.0f;
	const float BodyPanelHeight = 760.0f;
	const float DetailColumnWidth = DetailPanelWidth - 66.0f;
	const float ListColumnWidth = ListPanelWidth - 44.0f;
	const int32 CurrentSourceTabIndex = ActiveSourceTabIndex[ActiveTabIndex];
	const ET66CommunityContentKind ActiveKind = GetActiveKind();
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(ActiveTabIndex, CurrentSourceTabIndex);

	FT66CommunityContentEntry SelectedEntry;
	const bool bHasSelectedEntry = !bDraftEditorActive && FindCurrentSelectedEntry(SelectedEntry);

	FT66CommunityContentEntry ConfirmedEntry;
	const bool bHasConfirmedEntry = FindConfirmedEntry(ConfirmedEntry);
	const bool bSelectedEntryConfirmed = bHasSelectedEntry && bHasConfirmedEntry && SelectedEntry.LocalId == ConfirmedEntry.LocalId;

	const FString HeaderTitle = ActiveKind == ET66CommunityContentKind::Mod ? TEXT("Mods") : TEXT("Challenges");
	const FString DetailListHeader = ActiveKind == ET66CommunityContentKind::Mod ? TEXT("Rules") : TEXT("Rules And Requirements");
	const bool bCanConfirmSelectedEntry = bHasSelectedEntry && !SelectedEntry.IsDraft();
	const FText FooterConfirmLabel = FText::FromString(bSelectedEntryConfirmed ? TEXT("SELECTED") : TEXT("CONFIRM"));
	const ET66ChallengeButtonFamily FooterConfirmFamily = bCanConfirmSelectedEntry
		? ET66ChallengeButtonFamily::ToggleOn
		: ET66ChallengeButtonFamily::ToggleInactive;

	auto MakeConstraintRow = [DetailColumnWidth](const FString& ConstraintText) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 1.f, 8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Challenges", "ConstraintBullet", "\u25C6"))
				.Font(FT66FlatStyle::Tokens::FontBold(15))
				.ColorAndOpacity(ChallengePaperMuted)
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(ConstraintText))
				.Font(FT66FlatStyle::Tokens::FontBold(15))
				.ColorAndOpacity(ChallengePaperText)
				.AutoWrapText(true)
				.WrapTextAt(FMath::Max(260.f, DetailColumnWidth - 104.f))
				.Clipping(EWidgetClipping::ClipToBounds)
			];
	};

	auto MakeEntryRow = [this, CurrentSourceTabIndex, Community, ListColumnWidth](const FT66CommunityContentEntry& Entry, const int32 EntryIndex) -> TSharedRef<SWidget>
	{
		const FName SelectedId = GetSelectedEntryIdForView(ActiveTabIndex, CurrentSourceTabIndex);
		const bool bSelected = Entry.LocalId == SelectedId;
		const FT66CommunityContentEntry ActiveEntry = [this]()
		{
			FT66CommunityContentEntry Result;
			FindConfirmedEntry(Result);
			return Result;
		}();
		const bool bConfirmed = !ActiveEntry.LocalId.IsNone() && ActiveEntry.LocalId == Entry.LocalId;

		const TSharedRef<SWidget> StateSocket =
			SNew(SBox)
			.WidthOverride(52.f)
			.HeightOverride(52.f)
			[
				MakeChallengeSpritePanel(
					SNew(STextBlock)
					.Text(bConfirmed ? NSLOCTEXT("T66.Challenges", "ConfirmedMarker", "X") : FText::GetEmpty())
					.Font(FT66FlatStyle::Tokens::FontBold(14))
					.ColorAndOpacity(bConfirmed ? ChallengeGoldText : FLinearColor::Transparent)
					.Justification(ETextJustify::Center),
					nullptr,
					FMargin(0.f),
					ChallengeMutedBadgeTint())
			];

		const TSharedRef<SWidget> RowContent =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 18.f, 0.f)
			[
				StateSocket
			]
			+ SHorizontalBox::Slot().FillWidth(0.44f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry.Title))
					.Font(FT66FlatStyle::Tokens::FontBold(21))
					.ColorAndOpacity(bSelected ? ChallengePaperText : ChallengePaperMuted)
					.WrapTextAt(FMath::Max(210.f, ListColumnWidth * 0.34f))
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					MakeChallengeTagPill(FText::FromString(GetOriginLabel(Entry).ToUpper()))
				]
			]
			+ SHorizontalBox::Slot().FillWidth(0.26f).VAlign(VAlign_Center).Padding(20.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Community ? Community->BuildRewardSummary(Entry) : TEXT("No reward data")))
				.Font(FT66FlatStyle::Tokens::FontBold(14))
				.ColorAndOpacity(ChallengeRewardTint())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SHorizontalBox::Slot().FillWidth(0.24f).VAlign(VAlign_Center).Padding(20.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry.IsDraft() ? GetDraftSubmissionLabel(Entry) : Entry.AuthorDisplayName))
				.Font(FT66FlatStyle::Tokens::FontBold(13))
				.ColorAndOpacity(ChallengePaperMuted)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			];

		return FT66FlatStyle::MakeBareButton(
			FT66BareButtonParams(
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleEntrySelected, EntryIndex),
				SNew(SBox)
				.HeightOverride(112.f)
				[
					MakeChallengeSpritePanel(
						RowContent,
						nullptr,
						FMargin(28.f, 18.f, 28.f, 16.f),
						ChallengePanelInsetFill())
				])
				.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.SetPadding(FMargin(0.f))
				.SetHAlign(HAlign_Fill)
				.SetVAlign(VAlign_Fill)
				.SetHeight(112.f));
	};

	auto MakeDraftStepRow = [this](const FString& Label, const int32 Value, const FOnClicked& OnMinus, const FOnClicked& OnPlus) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("-")), OnMinus, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Value))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeRewardTint())
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("+")), OnPlus, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			];
	};

	auto MakeCycleRow = [this](const FString& Label, const FString& Value, const FOnClicked& OnPrev, const FOnClicked& OnNext) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("<")), OnPrev, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Value))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeRewardTint())
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT(">")), OnNext, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			];
	};

	TSharedRef<SVerticalBox> EntryList = SNew(SVerticalBox);
	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		EntryList->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			MakeEntryRow(Entries[EntryIndex], EntryIndex)
		];
	}

	const TSharedRef<SWidget> ListPanelContent = Entries.Num() > 0
		? StaticCastSharedRef<SWidget>(
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				EntryList
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(CurrentSourceTabIndex == static_cast<int32>(ESourceTabIndex::Community)
					? TEXT("No community entries yet. Create the first one.")
					: TEXT("No official entries were found.")))
				.Font(FT66FlatStyle::Tokens::FontBold(16))
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]);

	TSharedRef<SWidget> DetailPanelContent = SNew(STextBlock)
		.Text(NSLOCTEXT("T66.Challenges", "NoSelection", "Select an entry or create a new draft."))
		.Font(FT66FlatStyle::Tokens::FontRegular(13))
		.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
		.AutoWrapText(true);

	if (bDraftEditorActive)
	{
		TSharedRef<SVerticalBox> EditorRows = SNew(SVerticalBox);

		auto AddEditorSpacer = [&EditorRows](float Height)
		{
			EditorRows->AddSlot().AutoHeight().Padding(0.f, Height, 0.f, 0.f)
			[
				SNew(SSpacer)
				.Size(FVector2D(1.f, 1.f))
			];
		};

		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(DraftEditorEntry.Kind == ET66CommunityContentKind::Mod ? TEXT("Create Mod") : TEXT("Create Challenge")))
			.Font(FT66FlatStyle::Tokens::FontBold(28))
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
		];
		AddEditorSpacer(10.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Title")))
			.Font(FT66FlatStyle::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			MakeChallengeSpritePanel(
				SNew(SEditableTextBox)
				.Text(FText::FromString(DraftEditorEntry.Title))
				.ForegroundColor(ChallengeFantasyText)
				.BackgroundColor(FLinearColor::Transparent)
				.OnTextChanged_UObject(this, &UT66ChallengesScreen::HandleDraftTitleChanged),
				nullptr,
				FMargin(10.f, 6.f),
				ChallengePanelInsetFill())
		];
		AddEditorSpacer(10.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Description")))
			.Font(FT66FlatStyle::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			MakeChallengeSpritePanel(
				SNew(SMultiLineEditableTextBox)
				.Text(FText::FromString(DraftEditorEntry.Description))
				.ForegroundColor(ChallengeFantasyText)
				.OnTextChanged_UObject(this, &UT66ChallengesScreen::HandleDraftDescriptionChanged),
				nullptr,
				FMargin(10.f, 6.f),
				ChallengePanelInsetFill())
		];

		if (DraftEditorEntry.Kind == ET66CommunityContentKind::Challenge)
		{
			AddEditorSpacer(12.f);
			EditorRows->AddSlot().AutoHeight()
			[
				MakeDraftStepRow(
					TEXT("Suggested Chad Coupons"),
					DraftEditorEntry.SuggestedRewardChadCoupons,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftReward, -5),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftReward, +5))
			];
		}

		AddEditorSpacer(12.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Gameplay Rules")))
			.Font(FT66FlatStyle::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Starting Item"),
				GetItemLabel(DraftEditorEntry.Rules.StartingItemId),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftStartingItemClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftStartingItemClicked, +1))
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Passive Override"),
				GetPassiveLabel(DraftEditorEntry.Rules.PassiveOverride),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftPassiveClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftPassiveClicked, +1))
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Ultimate Override"),
				GetUltimateLabel(DraftEditorEntry.Rules.UltimateOverride),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftUltimateClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftUltimateClicked, +1))
		];

		if (DraftEditorEntry.Kind == ET66CommunityContentKind::Challenge)
		{
			AddEditorSpacer(12.f);
			EditorRows->AddSlot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Completion Requirements")))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeSuccessTint())
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatCheckbox(
					DraftEditorEntry.Rules.bRequireFullClear ? ET66FlatState::Selected : ET66FlatState::Default,
					TAttribute<ECheckBoxState>::CreateLambda([this]() { return DraftEditorEntry.Rules.bRequireFullClear ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }),
					FOnCheckStateChanged::CreateUObject(this, &UT66ChallengesScreen::HandleDraftFullClearChanged),
					TAttribute<FText>(FText::FromString(TEXT("Require full clear"))))
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatCheckbox(
					DraftEditorEntry.Rules.bRequireNoDamage ? ET66FlatState::Selected : ET66FlatState::Default,
					TAttribute<ECheckBoxState>::CreateLambda([this]() { return DraftEditorEntry.Rules.bRequireNoDamage ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }),
					FOnCheckStateChanged::CreateUObject(this, &UT66ChallengesScreen::HandleDraftNoDamageChanged),
					TAttribute<FText>(FText::FromString(TEXT("Require no damage"))))
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				MakeDraftStepRow(
					TEXT("Minimum Stage Reached"),
					DraftEditorEntry.Rules.RequiredStageReached,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftRequiredStage, -1),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftRequiredStage, +1))
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				MakeDraftStepRow(
					TEXT("Max Run Time (Seconds)"),
					DraftEditorEntry.Rules.MaxRunTimeSeconds,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftMaxRunTime, -30),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftMaxRunTime, +30))
			];
		}

		AddEditorSpacer(16.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SaveDraft", "SAVE DRAFT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSaveDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 136.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SubmitDraft", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSubmitDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 120.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "PlayDraft", "PLAY DRAFT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandlePlayDraftClicked), ET66ChallengeButtonFamily::CompactNeutral, 120.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "CancelDraft", "CANCEL"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCancelDraftEditorClicked), ET66ChallengeButtonFamily::ToggleOff, 112.f, 34.f, 12)
			]
		];

		DetailPanelContent =
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				EditorRows
			];
	}
	else if (bHasSelectedEntry)
	{
		const TArray<FString> RuleLines = Community ? Community->BuildRuleSummaryLines(SelectedEntry) : TArray<FString>{};
		TSharedRef<SVerticalBox> RuleList = SNew(SVerticalBox);
		for (const FString& RuleLine : RuleLines)
		{
			RuleList->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				MakeConstraintRow(RuleLine)
			];
		}

		TSharedRef<SVerticalBox> DetailLayout = SNew(SVerticalBox);
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(SBox)
			.WidthOverride(DetailColumnWidth - 18.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(SelectedEntry.Title))
				.Font(FT66FlatStyle::Tokens::FontBold(28))
				.ColorAndOpacity(ChallengeFantasyText)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 24.f)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
		];
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
			[
				SelectedEntry.AuthorAvatarUrl.IsEmpty()
				? StaticCastSharedRef<SWidget>(SNew(SSpacer).Size(FVector2D(0.f, 0.f)))
				: StaticCastSharedRef<SWidget>(
					SNew(SBox)
					.WidthOverride(52.f)
					.HeightOverride(52.f)
					[
						SNew(SImage)
						.Image(GetOrCreateAvatarBrush(SelectedEntry.AuthorAvatarUrl))
					])
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s%s%s"),
					*GetOriginLabel(SelectedEntry),
					SelectedEntry.AuthorDisplayName.IsEmpty() ? TEXT("") : TEXT(" by "),
					*SelectedEntry.AuthorDisplayName)))
				.Font(FT66FlatStyle::Tokens::FontBold(16))
				.ColorAndOpacity(ChallengeGoldText)
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 84.f)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
		[
			SNew(SBox)
			.HeightOverride(132.f)
			[
				MakeChallengeSpritePanel(
					SNew(STextBlock)
					.Text(FText::FromString(SelectedEntry.Description))
					.Font(FT66FlatStyle::Tokens::FontBold(16))
					.ColorAndOpacity(ChallengePaperText)
					.AutoWrapText(true)
					.WrapTextAt(DetailColumnWidth - 86.f)
					.Clipping(EWidgetClipping::ClipToBounds),
					nullptr,
					FMargin(26.f, 24.f),
					ChallengePanelInsetFill())
			]
		];
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(DetailListHeader))
			.Font(FT66FlatStyle::Tokens::FontBold(22))
			.ColorAndOpacity(ChallengeFantasyText)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
		];
		DetailLayout->AddSlot().AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(248.f)
			[
				MakeChallengeSpritePanel(
					SNew(SScrollBox)
					.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
					.ScrollBarVisibility(EVisibility::Visible)
					.ScrollBarThickness(FVector2D(14.f, 14.f))
					.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
					+ SScrollBox::Slot()
					[
						RuleList
					],
					nullptr,
					FMargin(28.f, 22.f),
					ChallengePanelInsetFill())
			]
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Community ? Community->BuildRewardSummary(SelectedEntry) : TEXT("No reward data")))
			.Font(FT66FlatStyle::Tokens::FontBold(18))
			.ColorAndOpacity(ChallengeGoldText)
			.AutoWrapText(true)
			.WrapTextAt(DetailColumnWidth - 12.f)
			.Clipping(EWidgetClipping::ClipToBounds)
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(Community ? Community->BuildSelectionSummary(SelectedEntry) : TEXT("No selection summary available.")))
				.Font(FT66FlatStyle::Tokens::FontRegular(11))
				.ColorAndOpacity(ChallengeFantasyMuted)
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 12.f)
				.Clipping(EWidgetClipping::ClipToBounds)
		];

		if (SelectedEntry.IsDraft())
		{
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(GetDraftSubmissionLabel(SelectedEntry)))
				.Font(FT66FlatStyle::Tokens::FontBold(14))
				.ColorAndOpacity(ChallengeDangerTint())
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 12.f)
				.Clipping(EWidgetClipping::ClipToBounds)
			];
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "EditDraft", "EDIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleEditDraftClicked), ET66ChallengeButtonFamily::CompactNeutral, 100.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SubmitSelectedDraft", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSubmitDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 112.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "PlaySelectedDraft", "PLAY"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandlePlayDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 112.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "DeleteSelectedDraft", "DELETE"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleDeleteDraftClicked), ET66ChallengeButtonFamily::ToggleOff, 112.f, 34.f, 12)
				]
			];
		}
		else
		{
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(bSelectedEntryConfirmed ? TEXT("Selected for the next run.") : TEXT("Confirm this entry to arm it for the next run.")))
				.Font(FT66FlatStyle::Tokens::FontRegular(11))
				.ColorAndOpacity(ChallengeFantasyMuted)
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 12.f)
				.Clipping(EWidgetClipping::ClipToBounds)
			];
			DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 16.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(
					FText::FromString(bSelectedEntryConfirmed ? TEXT("SELECTED") : TEXT("CONFIRM")),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleConfirmClicked),
					ET66ChallengeButtonFamily::ToggleOn,
					270.f,
					62.f,
					20,
					FMargin(22.f, 10.f, 22.f, 8.f))
			];
		}

		DetailPanelContent =
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				DetailLayout
			];
	}

	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;
	const FName ChallengeTabsGroup(TEXT("ChallengeTabs"));
	const FLinearColor Purple = FT66FlatStyle::PurpleAccent();
	const FLinearColor Red = FT66FlatStyle::SelectedText();
	const FLinearColor White = FT66FlatStyle::PrimaryText();
	const TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto DTag = [](const TCHAR* Name) -> FName
	{
		return FName(Name);
	};
	auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X * CanvasW, Y * CanvasH, W * CanvasW, H * CanvasH))
			[
				Widget
			];
	};
	auto MakeLabel = [](
		const FName Tag,
		const FText& Text,
		const int32 FontSize,
		const FLinearColor& Color,
		const bool bBold = true,
		const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
	{
		TSharedRef<STextBlock> Label = SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justification)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Visibility(EVisibility::HitTestInvisible);
		return FT66FlatStyle::AttachMetadata(
			Label,
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};
	auto MakeRect = [](const FLinearColor& Color, const FName Tag, const FString& Role = TEXT("Rect")) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(Color)
			.Visibility(EVisibility::HitTestInvisible),
			Tag,
			Role,
			ET66FlatState::Default);
	};
	auto MakeButtonShell = [](
		const ET66FlatState State,
		FOnClicked OnClicked,
		const FName Tag,
		const FName ToggleGroup = NAME_None,
		const bool bEnabled = true) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNullWidget::NullWidget,
			MoveTemp(OnClicked),
			FMargin(0.f),
			0.f,
			0.f,
			TAttribute<bool>(bEnabled),
			Tag,
			ToggleGroup);
	};
	auto MakeIcon = [&MakeLabel](const FName Tag, const TCHAR* Path, const FVector2D& Size, const FLinearColor& Tint, const FText& Fallback) -> TSharedRef<SWidget>
	{
		static TMap<FString, FT66ChallengeSpriteBrushEntry> FlatIconEntries;
		FT66ChallengeSpriteBrushEntry& Entry = FlatIconEntries.FindOrAdd(FString(Path));
		const FSlateBrush* Brush = ResolveChallengeSpriteBrush(
			Entry,
			FString(Path),
			Size,
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Trilinear);
		if (!Brush)
		{
			return MakeLabel(Tag, Fallback, 24, Tint, true, ETextJustify::Center);
		}
		TSharedRef<SImage> Image = SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(Tint)
			.Visibility(EVisibility::HitTestInvisible);
		return FT66FlatStyle::AttachMetadata(Image, Tag, TEXT("Icon"), ET66FlatState::Default);
	};

	const bool bOfficialSelected = !bDraftEditorActive && CurrentSourceTabIndex == static_cast<int32>(ESourceTabIndex::Official);
	const bool bCommunitySelected = !bDraftEditorActive && CurrentSourceTabIndex == static_cast<int32>(ESourceTabIndex::Community);
	const bool bCreateSelected = bDraftEditorActive;
	const FText CreateTabLabel = FText::FromString(ActiveKind == ET66CommunityContentKind::Mod ? TEXT("CREATE MOD") : TEXT("CREATE CHALLENGE"));
	const ET66FlatState ConfirmState = FooterConfirmFamily == ET66ChallengeButtonFamily::ToggleInactive
		? ET66FlatState::Disabled
		: ET66FlatState::Selected;
	const bool bConfirmEnabled = FooterConfirmFamily != ET66ChallengeButtonFamily::ToggleInactive;

	AddN(0.f, 0.f, 1.f, 1.f, MakeRect(FT66FlatStyle::BackgroundColor(), DTag(TEXT("Challenges.Background")), TEXT("Background")));
	AddN(0.f, 0.f, 1.f, 1.f, FT66FlatStyle::AttachMetadata(SNew(SBox), DTag(TEXT("Challenges.Root")), TEXT("ScreenRoot"), ET66FlatState::Default));

	AddN(0.012f, 0.021f, 0.132f, 0.071f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleBackClicked), DTag(TEXT("Challenges.TopRow.BackButton"))));
	AddN(0.033f, 0.044f, 0.022f, 0.034f, MakeIcon(DTag(TEXT("Challenges.TopRow.BackButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/back_chevron.png"), FVector2D(48.f, 48.f), Purple, FText::FromString(TEXT("<"))));
	AddN(0.071f, 0.039f, 0.054f, 0.044f, MakeLabel(DTag(TEXT("Challenges.TopRow.BackButton.Label")), NSLOCTEXT("T66.Challenges", "FlatBack", "BACK"), 28, Purple, true, ETextJustify::Left));

	AddN(0.374f, 0.033f, 0.254f, 0.066f, MakeLabel(DTag(TEXT("Challenges.Title")), FText::FromString(HeaderTitle.ToUpper()), 60, White, true, ETextJustify::Center));

	AddN(0.846f, 0.021f, 0.141f, 0.071f, MakeButtonShell(ConfirmState, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleConfirmClicked), DTag(TEXT("Challenges.TopRow.ConfirmButton")), NAME_None, bConfirmEnabled));
	AddN(0.870f, 0.039f, 0.070f, 0.044f, MakeLabel(DTag(TEXT("Challenges.TopRow.ConfirmButton.Label")), FooterConfirmLabel, 27, bConfirmEnabled ? Red : FT66FlatStyle::DisabledText(), true, ETextJustify::Left));
	AddN(0.947f, 0.044f, 0.025f, 0.034f, MakeIcon(DTag(TEXT("Challenges.TopRow.ConfirmButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/forward_chevron.png"), FVector2D(48.f, 48.f), bConfirmEnabled ? Red : FT66FlatStyle::DisabledText(), FText::FromString(TEXT(">"))));

	AddN(0.149f, 0.129f, 0.206f, 0.077f, MakeButtonShell(bOfficialSelected ? ET66FlatState::Selected : ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSourceTabSelected, static_cast<int32>(ESourceTabIndex::Official)), DTag(TEXT("Challenges.Tabs.OfficialButton")), ChallengeTabsGroup));
	AddN(0.181f, 0.145f, 0.032f, 0.048f, MakeIcon(DTag(TEXT("Challenges.Tabs.OfficialButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/target_crosshair.png"), FVector2D(56.f, 56.f), bOfficialSelected ? Red : Purple, FText::FromString(TEXT("+"))));
	AddN(0.222f, 0.148f, 0.076f, 0.044f, MakeLabel(DTag(TEXT("Challenges.Tabs.OfficialButton.Label")), NSLOCTEXT("T66.Challenges", "FlatOfficial", "OFFICIAL"), 26, bOfficialSelected ? Red : White, true, ETextJustify::Center));
	AddN(0.314f, 0.146f, 0.025f, 0.044f, MakeIcon(DTag(TEXT("Challenges.Tabs.OfficialButton.InfoIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(44.f, 44.f), bOfficialSelected ? Red : Purple, FText::FromString(TEXT("i"))));

	AddN(0.376f, 0.129f, 0.217f, 0.074f, MakeButtonShell(bCommunitySelected ? ET66FlatState::Selected : ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSourceTabSelected, static_cast<int32>(ESourceTabIndex::Community)), DTag(TEXT("Challenges.Tabs.CommunityButton")), ChallengeTabsGroup));
	AddN(0.406f, 0.145f, 0.032f, 0.048f, MakeIcon(DTag(TEXT("Challenges.Tabs.CommunityButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/people.png"), FVector2D(56.f, 56.f), bCommunitySelected ? Red : Purple, FText::FromString(TEXT("**"))));
	AddN(0.447f, 0.148f, 0.096f, 0.044f, MakeLabel(DTag(TEXT("Challenges.Tabs.CommunityButton.Label")), NSLOCTEXT("T66.Challenges", "FlatCommunity", "COMMUNITY"), 25, bCommunitySelected ? Red : White, true, ETextJustify::Center));
	AddN(0.552f, 0.146f, 0.025f, 0.044f, MakeIcon(DTag(TEXT("Challenges.Tabs.CommunityButton.InfoIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(44.f, 44.f), bCommunitySelected ? Red : Purple, FText::FromString(TEXT("i"))));

	AddN(0.615f, 0.129f, 0.236f, 0.074f, MakeButtonShell(bCreateSelected ? ET66FlatState::Selected : ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCreateDraftClicked), DTag(TEXT("Challenges.Tabs.CreateButton")), ChallengeTabsGroup));
	AddN(0.642f, 0.145f, 0.032f, 0.048f, MakeIcon(DTag(TEXT("Challenges.Tabs.CreateButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/pencil_edit.png"), FVector2D(56.f, 56.f), bCreateSelected ? Red : Purple, FText::FromString(TEXT("/"))));
	AddN(0.683f, 0.148f, 0.130f, 0.044f, MakeLabel(DTag(TEXT("Challenges.Tabs.CreateButton.Label")), CreateTabLabel, 23, bCreateSelected ? Red : White, true, ETextJustify::Center));
	AddN(0.819f, 0.146f, 0.025f, 0.044f, MakeIcon(DTag(TEXT("Challenges.Tabs.CreateButton.InfoIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(44.f, 44.f), bCreateSelected ? Red : Purple, FText::FromString(TEXT("i"))));

	AddN(0.027f, 0.230f, 0.466f, 0.704f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(22.f, 28.f),
			SNew(SBox)
			.WidthOverride(ListColumnWidth)
			[
				ListPanelContent
			],
			nullptr,
			DTag(TEXT("Challenges.LeftPanel"))));
	AddN(0.507f, 0.231f, 0.465f, 0.704f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Selected,
			FMargin(34.f, 28.f, 32.f, 28.f),
			SNew(SBox)
			.WidthOverride(DetailColumnWidth)
			[
				DetailPanelContent
			],
			nullptr,
			DTag(TEXT("Challenges.RightPanel"))));

	return FT66FlatStyle::WrapWithoutRetainer(
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				SNew(SBox)
				.WidthOverride(CanvasW)
				.HeightOverride(CanvasH)
				[
					Canvas
				]
			]
		],
		DTag(TEXT("Challenges.ViewportRoot")));
}

FReply UT66ChallengesScreen::HandleBackClicked()
{
	bSelectionStateInitialized = false;
	bRequestedCommunityRefresh = false;
	EndDraftEditor();
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleTabSelected(const int32 TabIndex)
{
	InitializeSelectionState();
	ActiveTabIndex = FMath::Clamp(TabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSourceTabSelected(const int32 SourceTabIndex)
{
	InitializeSelectionState();
	ActiveSourceTabIndex[ActiveTabIndex] = FMath::Clamp(SourceTabIndex, 0, static_cast<int32>(ESourceTabIndex::Count) - 1);
	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleEntrySelected(const int32 EntryIndex)
{
	const int32 SourceTabIndex = ActiveSourceTabIndex[ActiveTabIndex];
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(ActiveTabIndex, SourceTabIndex);
	if (Entries.IsValidIndex(EntryIndex))
	{
		PendingSelections[ActiveTabIndex][SourceTabIndex] = Entries[EntryIndex].LocalId;
	}

	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleConfirmClicked()
{
	bool bActivatedEntry = false;
	FT66CommunityContentEntry Entry;
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (FindCurrentSelectedEntry(Entry))
		{
			Community->ActivateEntry(Entry.LocalId);
			bActivatedEntry = true;
		}
	}

	if (!bActivatedEntry)
	{
		return FReply::Handled();
	}

	bSelectionStateInitialized = false;
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCreateDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		BeginDraftEditor(Community->CreateDraftTemplate(GetActiveKind()));
		RequestDeferredSlateRebuild();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleEditDraftClicked()
{
	FT66CommunityContentEntry Entry;
	if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
	{
		BeginDraftEditor(Entry);
		RequestDeferredSlateRebuild();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleDeleteDraftClicked()
{
	FT66CommunityContentEntry Entry;
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
		{
			Community->DeleteDraft(Entry.LocalId);
			EndDraftEditor();
			PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = NAME_None;
			RequestDeferredSlateRebuild();
		}
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSaveDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		Community->SaveDraft(DraftEditorEntry);
		PendingSelections[ActiveTabIndex][static_cast<int32>(ESourceTabIndex::Community)] = DraftEditorEntry.LocalId;
	}
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandlePlayDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (bDraftEditorActive)
		{
			Community->SaveDraft(DraftEditorEntry);
			Community->ActivateEntry(DraftEditorEntry.LocalId);
		}
		else
		{
			FT66CommunityContentEntry Entry;
			if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
			{
				Community->ActivateEntry(Entry.LocalId);
			}
		}
	}

	bSelectionStateInitialized = false;
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSubmitDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		const FName DraftId = bDraftEditorActive ? DraftEditorEntry.LocalId : GetSelectedEntryIdForView(ActiveTabIndex, ActiveSourceTabIndex[ActiveTabIndex]);
		if (bDraftEditorActive)
		{
			Community->SaveDraft(DraftEditorEntry);
		}
		Community->SubmitDraftForApproval(DraftId);
	}
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCancelDraftEditorClicked()
{
	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftReward(const int32 Delta)
{
	DraftEditorEntry.SuggestedRewardChadCoupons = T66CommunityContentLimits::ClampRewardChadCoupons(DraftEditorEntry.SuggestedRewardChadCoupons + Delta);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftRequiredStage(const int32 Delta)
{
	DraftEditorEntry.Rules.RequiredStageReached = T66CommunityContentLimits::ClampRequiredStageReached(DraftEditorEntry.Rules.RequiredStageReached + Delta);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftMaxRunTime(const int32 Delta)
{
	DraftEditorEntry.Rules.MaxRunTimeSeconds = T66CommunityContentLimits::ClampRunTimeSeconds(DraftEditorEntry.Rules.MaxRunTimeSeconds + Delta);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftPassiveClicked(const int32 Direction)
{
	CycleDraftPassive(Direction);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftUltimateClicked(const int32 Direction)
{
	CycleDraftUltimate(Direction);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftStartingItemClicked(const int32 Direction)
{
	CycleDraftStartingItem(Direction);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

void UT66ChallengesScreen::HandleDraftTitleChanged(const FText& NewText)
{
	DraftEditorEntry.Title = NewText.ToString();
}

void UT66ChallengesScreen::HandleDraftDescriptionChanged(const FText& NewText)
{
	DraftEditorEntry.Description = NewText.ToString();
}

void UT66ChallengesScreen::HandleDraftFullClearChanged(const ECheckBoxState NewState)
{
	DraftEditorEntry.Rules.bRequireFullClear = (NewState == ECheckBoxState::Checked);
}

void UT66ChallengesScreen::HandleDraftNoDamageChanged(const ECheckBoxState NewState)
{
	DraftEditorEntry.Rules.bRequireNoDamage = (NewState == ECheckBoxState::Checked);
}

void UT66ChallengesScreen::HandleCommunityContentChanged()
{
	RequestDeferredSlateRebuild();
}
