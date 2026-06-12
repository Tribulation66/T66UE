// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66CustomHeroBuilderScreen.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "UI/T66FrontendVideoCatalog.h"
#include "UI/T66FrontendVideoPlayer.h"
#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float BuilderCanvasW = 1920.f;
	constexpr float BuilderCanvasH = 1080.f;

	FName BuilderTag(const TCHAR* Tag)
	{
		return FName(Tag);
	}

	TSharedRef<SWidget> MakeBuilderLabel(const FText& Text, const int32 FontSize, const FName Tag, const bool bHeader = false)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(Text)
			.Font(bHeader ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(bHeader ? FT66FlatStyle::PrimaryText() : FT66FlatStyle::SecondaryText())
			.AutoWrapText(true)
			.WrapTextAt(520.f)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	}

	TSharedRef<SWidget> MakeBuilderButton(
		const FText& Label,
		FOnClicked OnClicked,
		const ET66FlatState State,
		const FName Tag,
		const float Height = 50.f,
		const int32 FontSize = 16,
		const TAttribute<bool> IsEnabled = true)
	{
		return FT66FlatStyle::MakeFlatButton(
			State,
			Label,
			MoveTemp(OnClicked),
			nullptr,
			nullptr,
			FMargin(12.f, 7.f),
			0.f,
			Height,
			IsEnabled,
			FontSize,
			Tag);
	}

	FText GetHeroName(UT66GameInstance* GI, UT66LocalizationSubsystem* Loc, const FName HeroID)
	{
		FHeroData HeroData;
		if (GI && GI->GetHeroData(HeroID, HeroData))
		{
			return Loc ? Loc->GetHeroDisplayName(HeroData) : HeroData.DisplayName;
		}
		return FText::FromName(HeroID);
	}

	int32& StatByName(FT66HeroStatBlock& Stats, const FName StatName)
	{
		if (StatName == FName(TEXT("Damage"))) return Stats.Damage;
		if (StatName == FName(TEXT("AttackSpeed"))) return Stats.AttackSpeed;
		if (StatName == FName(TEXT("AttackScale"))) return Stats.AttackScale;
		if (StatName == FName(TEXT("Accuracy"))) return Stats.Accuracy;
		if (StatName == FName(TEXT("Armor"))) return Stats.Armor;
		if (StatName == FName(TEXT("Evasion"))) return Stats.Evasion;
		if (StatName == FName(TEXT("Luck"))) return Stats.Luck;
		return Stats.Speed;
	}

	int32 StatValueByName(const FT66HeroStatBlock& Stats, const FName StatName)
	{
		FT66HeroStatBlock Mutable = Stats;
		return StatByName(Mutable, StatName);
	}
}

UT66CustomHeroBuilderScreen::UT66CustomHeroBuilderScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::CustomHeroBuilder;
	bIsModal = false;
}

void UT66CustomHeroBuilderScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	RefreshHeroIDs();
	LoadFromGameInstance();
}

void UT66CustomHeroBuilderScreen::RefreshScreen_Implementation()
{
	UpdatePreview();
}

bool UT66CustomHeroBuilderScreen::HandleBackAction()
{
	HandleBackClicked();
	return true;
}

void UT66CustomHeroBuilderScreen::RefreshHeroIDs()
{
	HeroIDs.Reset();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		for (const FName HeroID : GI->GetPlayableHeroIDs())
		{
			if (!HeroID.IsNone() && !UT66GameInstance::IsCustomHeroID(HeroID))
			{
				HeroIDs.AddUnique(HeroID);
			}
		}
	}
}

void UT66CustomHeroBuilderScreen::LoadFromGameInstance()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	const FName FallbackHeroID = HeroIDs.Num() > 0 ? HeroIDs[0] : FName(TEXT("Hero_1"));
	WeaponSourceHeroID = GI && !GI->CustomHeroBuild.WeaponSourceHeroID.IsNone() ? GI->CustomHeroBuild.WeaponSourceHeroID : FallbackHeroID;
	VisualSourceHeroID = GI && !GI->CustomHeroBuild.VisualSourceHeroID.IsNone() ? GI->CustomHeroBuild.VisualSourceHeroID : FallbackHeroID;
	BodyType = GI ? GI->ResolveCustomHeroBodyType(UT66GameInstance::GetCustomHeroID(), GI->SelectedHeroBodyType) : ET66BodyType::Chad;
	Stats = GI && GI->CustomHeroBuild.bConfigured ? GI->CustomHeroBuild.Stats : FT66HeroStatBlock{};
	if (!GI || !GI->CustomHeroBuild.bConfigured)
	{
		Stats.Damage = 3;
		Stats.AttackSpeed = 3;
		Stats.AttackScale = 3;
		Stats.Accuracy = 3;
		Stats.Armor = 3;
		Stats.Evasion = 3;
		Stats.Luck = 3;
		Stats.Speed = 3;
	}
	ClampStatsToBudget();
}

int32 UT66CustomHeroBuilderScreen::GetSpentPoints() const
{
	return Stats.Damage
		+ Stats.AttackSpeed
		+ Stats.AttackScale
		+ Stats.Accuracy
		+ Stats.Armor
		+ Stats.Evasion
		+ Stats.Luck
		+ Stats.Speed;
}

int32 UT66CustomHeroBuilderScreen::GetRemainingPoints() const
{
	return FMath::Max(0, StatBudget - GetSpentPoints());
}

void UT66CustomHeroBuilderScreen::ClampStatsToBudget()
{
	const FName StatNames[] = {
		FName(TEXT("Damage")), FName(TEXT("AttackSpeed")), FName(TEXT("AttackScale")), FName(TEXT("Accuracy")),
		FName(TEXT("Armor")), FName(TEXT("Evasion")), FName(TEXT("Luck")), FName(TEXT("Speed"))
	};
	for (const FName StatName : StatNames)
	{
		int32& Value = StatByName(Stats, StatName);
		Value = FMath::Clamp(Value, MinStatValue, MaxStatValue);
	}
	while (GetSpentPoints() > StatBudget)
	{
		bool bReduced = false;
		for (const FName StatName : StatNames)
		{
			int32& Value = StatByName(Stats, StatName);
			if (Value > MinStatValue)
			{
				--Value;
				bReduced = true;
				if (GetSpentPoints() <= StatBudget)
				{
					return;
				}
			}
		}
		if (!bReduced)
		{
			return;
		}
	}
}

void UT66CustomHeroBuilderScreen::RebuildWeaponList()
{
	if (!WeaponListBox.IsValid()) return;
	WeaponListBox->ClearChildren();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	for (const FName HeroID : HeroIDs)
	{
		WeaponListBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeBuilderButton(
				GetHeroName(GI, Loc, HeroID),
				FOnClicked::CreateUObject(this, &UT66CustomHeroBuilderScreen::HandleWeaponHeroClicked, HeroID),
				HeroID == WeaponSourceHeroID ? ET66FlatState::Selected : ET66FlatState::Default,
				FName(*FString::Printf(TEXT("CustomHero.Weapon.%s"), *HeroID.ToString())))
		];
	}
}

void UT66CustomHeroBuilderScreen::RebuildVisualList()
{
	if (!VisualListBox.IsValid()) return;
	VisualListBox->ClearChildren();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	for (const FName HeroID : HeroIDs)
	{
		VisualListBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeBuilderButton(
				GetHeroName(GI, Loc, HeroID),
				FOnClicked::CreateUObject(this, &UT66CustomHeroBuilderScreen::HandleVisualHeroClicked, HeroID),
				HeroID == VisualSourceHeroID ? ET66FlatState::Selected : ET66FlatState::Default,
				FName(*FString::Printf(TEXT("CustomHero.Model.%s"), *HeroID.ToString())))
		];
	}
}

void UT66CustomHeroBuilderScreen::RebuildStatsList()
{
	if (!StatsListBox.IsValid()) return;
	StatsListBox->ClearChildren();

	const struct FStatRow
	{
		FName Name;
		FText Label;
	} Rows[] = {
		{ FName(TEXT("Damage")), NSLOCTEXT("T66.CustomHero", "StatDamage", "Damage") },
		{ FName(TEXT("AttackSpeed")), NSLOCTEXT("T66.CustomHero", "StatAttackSpeed", "Attack Speed") },
		{ FName(TEXT("AttackScale")), NSLOCTEXT("T66.CustomHero", "StatAttackScale", "Attack Scale") },
		{ FName(TEXT("Accuracy")), NSLOCTEXT("T66.CustomHero", "StatAccuracy", "Accuracy") },
		{ FName(TEXT("Armor")), NSLOCTEXT("T66.CustomHero", "StatArmor", "Armor") },
		{ FName(TEXT("Evasion")), NSLOCTEXT("T66.CustomHero", "StatEvasion", "Evasion") },
		{ FName(TEXT("Luck")), NSLOCTEXT("T66.CustomHero", "StatLuck", "Luck") },
		{ FName(TEXT("Speed")), NSLOCTEXT("T66.CustomHero", "StatSpeed", "Speed") },
	};

	for (const FStatRow& Row : Rows)
	{
		const int32 Value = StatValueByName(Stats, Row.Name);
		StatsListBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				MakeBuilderLabel(Row.Label, 17, FName(*FString::Printf(TEXT("CustomHero.Stat.%s.Label"), *Row.Name.ToString())))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
			[
				MakeBuilderButton(
					FText::FromString(TEXT("-")),
					FOnClicked::CreateUObject(this, &UT66CustomHeroBuilderScreen::HandleAdjustStatClicked, Row.Name, -1),
					ET66FlatState::Default,
					FName(*FString::Printf(TEXT("CustomHero.Stat.%s.Minus"), *Row.Name.ToString())),
					38.f,
					20,
					Value > MinStatValue)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				MakeBuilderLabel(FText::AsNumber(Value), 19, FName(*FString::Printf(TEXT("CustomHero.Stat.%s.Value"), *Row.Name.ToString())), true)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
			[
				MakeBuilderButton(
					FText::FromString(TEXT("+")),
					FOnClicked::CreateUObject(this, &UT66CustomHeroBuilderScreen::HandleAdjustStatClicked, Row.Name, +1),
					ET66FlatState::Selected,
					FName(*FString::Printf(TEXT("CustomHero.Stat.%s.Plus"), *Row.Name.ToString())),
					38.f,
					20,
					Value < MaxStatValue && GetRemainingPoints() > 0)
			]
		];
	}

	if (RemainingText.IsValid())
	{
		RemainingText->SetText(FText::Format(
			NSLOCTEXT("T66.CustomHero", "RemainingPoints", "Remaining: {0} / {1}"),
			FText::AsNumber(GetRemainingPoints()),
			FText::AsNumber(StatBudget)));
	}
}

TSharedRef<SWidget> UT66CustomHeroBuilderScreen::BuildSlateUI()
{
	RefreshHeroIDs();
	LoadFromGameInstance();

	const TSharedRef<SVerticalBox> WeaponList = SAssignNew(WeaponListBox, SVerticalBox);
	const TSharedRef<SVerticalBox> VisualList = SAssignNew(VisualListBox, SVerticalBox);
	const TSharedRef<SVerticalBox> StatsList = SAssignNew(StatsListBox, SVerticalBox);
	RebuildWeaponList();
	RebuildVisualList();
	RebuildStatsList();

	const TSharedRef<SVerticalBox> WeaponPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBuilderLabel(NSLOCTEXT("T66.CustomHero", "WeaponHeader", "WEAPON SET"), 26, BuilderTag(TEXT("CustomHero.Weapon.Header")), true)
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 16.f, 0.f, 0.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				WeaponList
			]
		];

	const TSharedRef<SVerticalBox> ModelPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBuilderLabel(NSLOCTEXT("T66.CustomHero", "ModelHeader", "MODEL"), 26, BuilderTag(TEXT("CustomHero.Model.Header")), true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 10.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeBuilderButton(
					FText::FromString(TEXT("CHAD")),
					FOnClicked::CreateUObject(this, &UT66CustomHeroBuilderScreen::HandleBodyClicked, T66BodyTypeAliases::Chad),
					T66BodyTypeAliases::IsChad(BodyType) ? ET66FlatState::Selected : ET66FlatState::Default,
					BuilderTag(TEXT("CustomHero.Model.Body.Chad")))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeBuilderButton(
					FText::FromString(TEXT("STACY")),
					FOnClicked::CreateUObject(this, &UT66CustomHeroBuilderScreen::HandleBodyClicked, T66BodyTypeAliases::Stacy),
					T66BodyTypeAliases::IsStacy(BodyType) ? ET66FlatState::Selected : ET66FlatState::Default,
					BuilderTag(TEXT("CustomHero.Model.Body.Stacy")))
			]
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				VisualList
			]
		];

	const TSharedRef<SVerticalBox> PreviewPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBuilderLabel(NSLOCTEXT("T66.CustomHero", "PreviewHeader", "CUSTOM HERO"), 26, BuilderTag(TEXT("CustomHero.Preview.Header")), true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
		[
			SAssignNew(PreviewNameText, STextBlock)
			.Text(NSLOCTEXT("T66.CustomHero", "PreviewNameDefault", "Custom Hero"))
			.Font(FT66FlatStyle::MakeBoldFont(22))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 18.f, 0.f, 18.f)
		[
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(8.f),
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SAssignNew(PreviewFallbackBox, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
					.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.11f, 1.f))
					[
						SNew(SBox)
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image_UObject(this, &UT66CustomHeroBuilderScreen::GetPreviewVideoBrush)
				],
				nullptr,
				BuilderTag(TEXT("CustomHero.Preview.VideoPanel")))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(RemainingText, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FT66FlatStyle::MakeBoldFont(18))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
		[
			StatsList
		];

	const TSharedRef<SVerticalBox> ActionPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBuilderButton(
				NSLOCTEXT("T66.CustomHero", "ConfirmButton", "USE CUSTOM HERO"),
				FOnClicked::CreateUObject(this, &UT66CustomHeroBuilderScreen::HandleConfirmClicked),
				ET66FlatState::Selected,
				BuilderTag(TEXT("CustomHero.Actions.Confirm")),
				72.f,
				22)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
		[
			MakeBuilderButton(
				NSLOCTEXT("T66.Common", "Back", "BACK"),
				FOnClicked::CreateUObject(this, &UT66CustomHeroBuilderScreen::HandleBackClicked),
				ET66FlatState::Default,
				BuilderTag(TEXT("CustomHero.Actions.Back")),
				56.f,
				18)
		];

	const TSharedRef<SWidget> Root = SNew(SBox)
		.WidthOverride(BuilderCanvasW)
		.HeightOverride(BuilderCanvasH)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
					.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
				]
				+ SOverlay::Slot()
				.Padding(70.f, 56.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeBuilderLabel(NSLOCTEXT("T66.CustomHero", "Title", "BUILD YOUR OWN HERO"), 38, BuilderTag(TEXT("CustomHero.Title")), true)
					]
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 34.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(0.24f).Padding(0.f, 0.f, 18.f, 0.f)
						[
							FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(18.f), WeaponPanel, nullptr, BuilderTag(TEXT("CustomHero.Weapon.Panel")))
						]
						+ SHorizontalBox::Slot().FillWidth(0.24f).Padding(0.f, 0.f, 18.f, 0.f)
						[
							FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(18.f), ModelPanel, nullptr, BuilderTag(TEXT("CustomHero.Model.Panel")))
						]
						+ SHorizontalBox::Slot().FillWidth(0.36f).Padding(0.f, 0.f, 18.f, 0.f)
						[
							FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(18.f), PreviewPanel, nullptr, BuilderTag(TEXT("CustomHero.Preview.Panel")))
						]
						+ SHorizontalBox::Slot().FillWidth(0.16f)
						[
							FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(18.f), ActionPanel, nullptr, BuilderTag(TEXT("CustomHero.Actions.Panel")))
						]
					]
				]
			]
		];

	RebuildStatsList();
	UpdatePreview();
	return FT66FlatStyle::AttachMetadata(Root, BuilderTag(TEXT("CustomHero.Root")), TEXT("ScreenRoot"), ET66FlatState::Default);
}

void UT66CustomHeroBuilderScreen::UpdatePreview()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	if (PreviewNameText.IsValid())
	{
		PreviewNameText->SetText(FText::Format(
			NSLOCTEXT("T66.CustomHero", "PreviewNameFormat", "{0} model / {1} weapons"),
			GetHeroName(GI, Loc, VisualSourceHeroID),
			GetHeroName(GI, Loc, WeaponSourceHeroID)));
	}

	bool bVideoOpened = false;
	FT66FrontendVideoAsset VideoAsset;
	if (T66FrontendVideoCatalog::ResolveHeroSelection(VisualSourceHeroID, FName(TEXT("Default")), BodyType, VideoAsset))
	{
		if (!PreviewVideoPlayer)
		{
			PreviewVideoPlayer = NewObject<UT66FrontendVideoPlayer>(this);
		}
		if (PreviewVideoPlayer)
		{
			bVideoOpened = PreviewVideoPlayer->OpenVideo(VideoAsset, FVector2D(620.f, 520.f), FName(TEXT("CustomHeroBuilderPreview")));
		}
	}
	if (!bVideoOpened && PreviewVideoPlayer)
	{
		PreviewVideoPlayer->CloseVideo();
	}
	if (PreviewFallbackBox.IsValid())
	{
		PreviewFallbackBox->SetBorderBackgroundColor(bVideoOpened ? FLinearColor::Transparent : FLinearColor(0.22f, 0.17f, 0.33f, 1.f));
	}
	if (RemainingText.IsValid())
	{
		RemainingText->SetText(FText::Format(
			NSLOCTEXT("T66.CustomHero", "RemainingPoints", "Remaining: {0} / {1}"),
			FText::AsNumber(GetRemainingPoints()),
			FText::AsNumber(StatBudget)));
	}
}

FReply UT66CustomHeroBuilderScreen::HandleWeaponHeroClicked(const FName HeroID)
{
	WeaponSourceHeroID = HeroID;
	RebuildWeaponList();
	UpdatePreview();
	return FReply::Handled();
}

FReply UT66CustomHeroBuilderScreen::HandleVisualHeroClicked(const FName HeroID)
{
	VisualSourceHeroID = HeroID;
	RebuildVisualList();
	UpdatePreview();
	return FReply::Handled();
}

FReply UT66CustomHeroBuilderScreen::HandleBodyClicked(const ET66BodyType InBodyType)
{
	BodyType = InBodyType;
	UpdatePreview();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66CustomHeroBuilderScreen::HandleAdjustStatClicked(const FName StatName, const int32 Delta)
{
	int32& Value = StatByName(Stats, StatName);
	if (Delta > 0 && GetRemainingPoints() <= 0)
	{
		return FReply::Handled();
	}
	Value = FMath::Clamp(Value + Delta, MinStatValue, MaxStatValue);
	ClampStatsToBudget();
	RebuildStatsList();
	UpdatePreview();
	return FReply::Handled();
}

FReply UT66CustomHeroBuilderScreen::HandleConfirmClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->ConfigureCustomHero(WeaponSourceHeroID, VisualSourceHeroID, BodyType, Stats);
	}
	NavigateTo(ET66ScreenType::HeroSelection);
	return FReply::Handled();
}

FReply UT66CustomHeroBuilderScreen::HandleBackClicked()
{
	NavigateTo(ET66ScreenType::HeroSelection);
	return FReply::Handled();
}

const FSlateBrush* UT66CustomHeroBuilderScreen::GetPreviewVideoBrush() const
{
	return PreviewVideoPlayer ? PreviewVideoPlayer->GetVideoBrush() : nullptr;
}
