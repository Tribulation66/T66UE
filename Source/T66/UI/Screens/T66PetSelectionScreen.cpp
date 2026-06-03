// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PetSelectionScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66SessionSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

namespace
{
	constexpr float PetSelectionCanvasW = 1920.f;
	constexpr float PetSelectionCanvasH = 1080.f;

	FName PetSelectionTag(const TCHAR* Tag)
	{
		return FName(Tag);
	}

	TSharedRef<SWidget> MakePetLabel(
		const FText& Text,
		const ET66FlatLabelRole Role,
		const FName Tag,
		const ETextJustify::Type Justification = ETextJustify::Left,
		const float WrapAt = 0.f)
	{
		TSharedRef<STextBlock> Label = SNew(STextBlock)
			.Text(Text)
			.Font(Role == ET66FlatLabelRole::Title
				? FT66FlatStyle::MakeBoldFont(34)
				: (Role == ET66FlatLabelRole::Header
					? FT66FlatStyle::MakeBoldFont(22)
					: FT66FlatStyle::MakeFont(Role == ET66FlatLabelRole::Caption ? 14 : 17)))
			.ColorAndOpacity(Role == ET66FlatLabelRole::Caption || Role == ET66FlatLabelRole::Body
				? FT66FlatStyle::SecondaryText()
				: FT66FlatStyle::PrimaryText())
			.Justification(Justification)
			.AutoWrapText(WrapAt > 0.f)
			.WrapTextAt(WrapAt)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds);
		return FT66FlatStyle::AttachMetadata(Label, Tag, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true);
	}

	TSharedRef<SWidget> MakePetColorRect(const FLinearColor& Color, const FName Tag)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(Color)
			.Clipping(EWidgetClipping::ClipToBounds),
			Tag,
			TEXT("ColorFill"),
			ET66FlatState::Default);
	}

	TSharedRef<SWidget> MakePetButton(
		const FText& Label,
		FOnClicked OnClicked,
		const ET66FlatState State,
		const FName Tag,
		const float Height = 52.f,
		const int32 FontSize = 17,
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

	FText GetPetDisplayName(const FPetData& PetData, const FName FallbackID)
	{
		return PetData.DisplayName.IsEmpty() ? FText::FromName(FallbackID) : PetData.DisplayName;
	}
}

UT66PetSelectionScreen::UT66PetSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PetSelection;
	bIsModal = false;
}

void UT66PetSelectionScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::PetSelection);
		}
	}
	RefreshPetList();
}

void UT66PetSelectionScreen::RefreshScreen_Implementation()
{
	UpdatePetDisplay();
}

bool UT66PetSelectionScreen::HandleBackAction()
{
	OnBackClicked();
	return true;
}

TSharedRef<SWidget> UT66PetSelectionScreen::BuildSlateUI()
{
	RefreshPetList();

	const TSharedRef<SVerticalBox> PetList = SAssignNew(PetListBoxWidget, SVerticalBox);
	RebuildPetList();

	const TSharedRef<SVerticalBox> SkinList = SAssignNew(PetSkinListBoxWidget, SVerticalBox);
	RebuildPetSkinList();

	const TSharedRef<SVerticalBox> LeftPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakePetLabel(NSLOCTEXT("T66.Pets", "PetSelectionListHeader", "PETS"), ET66FlatLabelRole::Header, PetSelectionTag(TEXT("PetSelection.Collection.Header")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
		[
			MakePetButton(
				NSLOCTEXT("T66.Pets", "PetSelectionNoPet", "NONE"),
				FOnClicked::CreateUObject(this, &UT66PetSelectionScreen::HandleNoPetClicked),
				PreviewedPetID.IsNone() ? ET66FlatState::Selected : ET66FlatState::Default,
				PetSelectionTag(TEXT("PetSelection.Collection.NoPetButton")),
				54.f,
				14)
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 14.f, 0.f, 0.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				PetList
			]
		];

	const TSharedRef<SVerticalBox> CenterPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakePetLabel(NSLOCTEXT("T66.Pets", "PetSelectionPreviewHeader", "PET"), ET66FlatLabelRole::Header, PetSelectionTag(TEXT("PetSelection.Preview.Header")), ETextJustify::Center)
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 18.f, 0.f, 18.f)
		[
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(10.f),
				SAssignNew(PetPreviewColorBox, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FLinearColor(0.18f, 0.42f, 0.95f, 1.f))
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SBox)
				],
				nullptr,
				PetSelectionTag(TEXT("PetSelection.Preview.ColorPanel")))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakePetButton(
				NSLOCTEXT("T66.Pets", "PetSelectionPrevious", "PREV"),
				FOnClicked::CreateUObject(this, &UT66PetSelectionScreen::HandlePrevClicked),
				ET66FlatState::Default,
				PetSelectionTag(TEXT("PetSelection.Preview.PrevButton")),
				52.f,
				16,
				TAttribute<bool>::CreateLambda([this]() { return CapturedPetIDs.Num() > 0; }))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
		[
			MakePetButton(
				NSLOCTEXT("T66.Pets", "PetSelectionNext", "NEXT"),
				FOnClicked::CreateUObject(this, &UT66PetSelectionScreen::HandleNextClicked),
				ET66FlatState::Default,
				PetSelectionTag(TEXT("PetSelection.Preview.NextButton")),
				52.f,
				16,
				TAttribute<bool>::CreateLambda([this]() { return CapturedPetIDs.Num() > 0; }))
		];

	const TSharedRef<SVerticalBox> DetailPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			FT66FlatStyle::AttachMetadata(
				SAssignNew(PetNameWidget, STextBlock)
				.Text(NSLOCTEXT("T66.Pets", "PetSelectionNoPetTitle", "NONE"))
				.Font(FT66FlatStyle::MakeBoldFont(31))
				.ColorAndOpacity(FSlateColor(FT66FlatStyle::PrimaryText()))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
				PetSelectionTag(TEXT("PetSelection.Detail.Name")),
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 9.f, 0.f, 0.f)
		[
			FT66FlatStyle::AttachMetadata(
				SAssignNew(PetSourceWidget, STextBlock)
				.Text(NSLOCTEXT("T66.Pets", "PetSelectionSourceNone", "No active pet."))
				.Font(FT66FlatStyle::MakeFont(16))
				.ColorAndOpacity(FSlateColor(FT66FlatStyle::SecondaryText()))
				.AutoWrapText(true)
				.WrapTextAt(520.f),
				PetSelectionTag(TEXT("PetSelection.Detail.Source")),
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 34.f, 0.f, 0.f)
		[
			MakePetLabel(NSLOCTEXT("T66.Pets", "PetSelectionBondHeader", "UNION"), ET66FlatLabelRole::Header, PetSelectionTag(TEXT("PetSelection.Detail.BondHeader")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 11.f, 0.f, 0.f)
		[
			FT66FlatStyle::MakeFlatProgressBar(
				TAttribute<float>::CreateLambda([this]()
				{
					if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
					{
						if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
						{
							return Achievements->GetPetBondProgress01(PreviewedPetID);
						}
					}
					return 0.f;
				}),
				TOptional<FLinearColor>(FT66FlatStyle::ReadyBorder()),
				PetSelectionTag(TEXT("PetSelection.Detail.BondProgress")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
		[
			FT66FlatStyle::AttachMetadata(
				SAssignNew(PetBondTextWidget, STextBlock)
				.Text(FText::FromString(TEXT("--")))
				.Font(FT66FlatStyle::MakeFont(16))
				.ColorAndOpacity(FSlateColor(FT66FlatStyle::SecondaryText()))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
				PetSelectionTag(TEXT("PetSelection.Detail.BondText")),
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 9.f, 0.f, 0.f)
		[
			FT66FlatStyle::AttachMetadata(
				SAssignNew(PetMovementTextWidget, STextBlock)
				.Text(NSLOCTEXT("T66.Pets", "PetSelectionMovementOnly", "Union affects speed only."))
				.Font(FT66FlatStyle::MakeFont(16))
				.ColorAndOpacity(FSlateColor(FT66FlatStyle::SecondaryText()))
				.AutoWrapText(true)
				.WrapTextAt(520.f),
				PetSelectionTag(TEXT("PetSelection.Detail.MovementOnly")),
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 34.f, 0.f, 0.f)
		[
			MakePetLabel(NSLOCTEXT("T66.Pets", "PetSelectionSkinsHeader", "SKINS"), ET66FlatLabelRole::Header, PetSelectionTag(TEXT("PetSelection.Detail.SkinsHeader")))
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SkinList
			]
		];

	const TSharedRef<SHorizontalBox> Body = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.28f).Padding(0.f, 0.f, 18.f, 0.f)
		[
			FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(22.f), LeftPanel, nullptr, PetSelectionTag(TEXT("PetSelection.Collection.Panel")))
		]
		+ SHorizontalBox::Slot().FillWidth(0.31f).Padding(0.f, 0.f, 18.f, 0.f)
		[
			FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(22.f), CenterPanel, nullptr, PetSelectionTag(TEXT("PetSelection.Preview.Panel")))
		]
		+ SHorizontalBox::Slot().FillWidth(0.41f)
		[
			FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(24.f), DetailPanel, nullptr, PetSelectionTag(TEXT("PetSelection.Detail.Panel")))
		];

	const TSharedRef<SHorizontalBox> TopRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			MakePetButton(
				NSLOCTEXT("T66.Common", "Back", "BACK"),
				FOnClicked::CreateUObject(this, &UT66PetSelectionScreen::HandleBackClicked),
				ET66FlatState::Default,
				PetSelectionTag(TEXT("PetSelection.Top.BackButton")),
				56.f,
				19)
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(26.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
		[
			MakePetLabel(NSLOCTEXT("T66.Pets", "PetSelectionTitle", "CHOOSE PET"), ET66FlatLabelRole::Title, PetSelectionTag(TEXT("PetSelection.Top.Title")))
		];

	const TSharedRef<SHorizontalBox> BottomRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			SNew(SBox)
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			MakePetButton(
				NSLOCTEXT("T66.Pets", "PetSelectionConfirm", "CONFIRM"),
				FOnClicked::CreateUObject(this, &UT66PetSelectionScreen::HandleConfirmClicked),
				ET66FlatState::Selected,
				PetSelectionTag(TEXT("PetSelection.Bottom.ConfirmButton")),
				64.f,
				20,
				TAttribute<bool>::CreateLambda([this]() { return PreviewedPetID.IsNone() || IsPetCaptured(PreviewedPetID); }))
		];

	const TSharedRef<SVerticalBox> RootPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(30.f, 28.f, 30.f, 0.f)
		[
			TopRow
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(30.f, 24.f, 30.f, 0.f)
		[
			Body
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(30.f, 20.f, 30.f, 30.f)
		[
			BottomRow
		];

	TSharedRef<SWidget> Root = FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		.WidthOverride(PetSelectionCanvasW)
		.HeightOverride(PetSelectionCanvasH)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				RootPanel
			]
		],
		PetSelectionTag(TEXT("PetSelection.Root")),
		TEXT("ScreenRoot"),
		ET66FlatState::Default);

	UpdatePetDisplay();
	return Root;
}

void UT66PetSelectionScreen::RefreshPetList()
{
	CapturedPetIDs.Reset();
	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66AchievementsSubsystem* Achievements = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (Achievements)
	{
		CapturedPetIDs = Achievements->GetCapturedPetIDs();
	}

	const FName ActivePetID = Achievements ? Achievements->GetActivePetID() : NAME_None;
	const bool bCurrentStillCaptured = !PreviewedPetID.IsNone() && CapturedPetIDs.Contains(PreviewedPetID);
	if (!bCurrentStillCaptured)
	{
		PreviewedPetID = !ActivePetID.IsNone() && CapturedPetIDs.Contains(ActivePetID)
			? ActivePetID
			: NAME_None;
	}
	CurrentPetIndex = PreviewedPetID.IsNone() ? -1 : CapturedPetIDs.IndexOfByKey(PreviewedPetID);
}

void UT66PetSelectionScreen::RebuildPetList()
{
	if (!PetListBoxWidget.IsValid())
	{
		return;
	}

	PetListBoxWidget->ClearChildren();
	if (CapturedPetIDs.Num() == 0)
	{
		PetListBoxWidget->AddSlot().AutoHeight()
		[
			MakePetLabel(NSLOCTEXT("T66.Pets", "PetSelectionEmptyCollection", "No pets."), ET66FlatLabelRole::Body, PetSelectionTag(TEXT("PetSelection.Collection.Empty")), ETextJustify::Center, 140.f)
		];
		return;
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	for (const FName PetID : CapturedPetIDs)
	{
		FPetData PetData;
		if (T66GI)
		{
			T66GI->GetPetData(PetID, PetData);
		}
		const FText Label = GetPetDisplayName(PetData, PetID);
		PetListBoxWidget->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			MakePetButton(
				Label,
				FOnClicked::CreateUObject(this, &UT66PetSelectionScreen::HandlePetRowClicked, PetID),
				PreviewedPetID == PetID ? ET66FlatState::Selected : ET66FlatState::Default,
				FName(*FString::Printf(TEXT("PetSelection.Collection.Pet.%s"), *PetID.ToString())),
				54.f,
				16)
		];
	}
}

void UT66PetSelectionScreen::RebuildPetSkinList()
{
	if (!PetSkinListBoxWidget.IsValid())
	{
		return;
	}

	PetSkinListBoxWidget->ClearChildren();
	if (PreviewedPetID.IsNone())
	{
		PetSkinListBoxWidget->AddSlot().AutoHeight()
		[
			MakePetLabel(NSLOCTEXT("T66.Pets", "PetSelectionNoSkinRows", "No skin."), ET66FlatLabelRole::Body, PetSelectionTag(TEXT("PetSelection.Detail.Skins.None")), ETextJustify::Center, 150.f)
		];
		return;
	}

	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66AchievementsSubsystem* Achievements = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	TArray<FName> OwnedSkinIDs = Achievements ? Achievements->GetOwnedPetSkinIDs(PreviewedPetID) : TArray<FName>();
	OwnedSkinIDs.AddUnique(FName(TEXT("Default")));
	const FName EquippedSkinID = GetEquippedSkinID(PreviewedPetID);

	for (const FName SkinID : OwnedSkinIDs)
	{
		const bool bEquipped = SkinID == EquippedSkinID;
		PetSkinListBoxWidget->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			MakePetButton(
				bEquipped
					? FText::Format(NSLOCTEXT("T66.Pets", "PetSelectionEquippedSkinFormat", "{0} EQUIPPED"), FText::FromName(SkinID))
					: FText::FromName(SkinID),
				FOnClicked::CreateUObject(this, &UT66PetSelectionScreen::HandleSkinClicked, SkinID),
				bEquipped ? ET66FlatState::Selected : ET66FlatState::Default,
				FName(*FString::Printf(TEXT("PetSelection.Detail.Skin.%s"), *SkinID.ToString())),
				46.f,
				15)
		];
	}
}

void UT66PetSelectionScreen::UpdatePetDisplay()
{
	FPetData PetData;
	const bool bHasPet = GetPreviewedPetData(PetData);
	const FText PetName = bHasPet
		? GetPetDisplayName(PetData, PreviewedPetID)
		: NSLOCTEXT("T66.Pets", "PetSelectionNoPetTitle", "NONE");

	if (PetNameWidget.IsValid())
	{
		PetNameWidget->SetText(PetName);
	}
	if (PetSourceWidget.IsValid())
	{
		PetSourceWidget->SetText(bHasPet
			? FText::Format(NSLOCTEXT("T66.Pets", "PetSelectionSourceFormat", "Captured from {0}."), FText::FromName(PetData.SourceBossID.IsNone() ? PreviewedPetID : PetData.SourceBossID))
			: NSLOCTEXT("T66.Pets", "PetSelectionNoActivePet", "No active pet."));
	}
	if (PetPreviewColorBox.IsValid())
	{
		PetPreviewColorBox->SetBorderBackgroundColor(bHasPet ? PetData.PlaceholderColor : FLinearColor(0.035f, 0.035f, 0.044f, 1.f));
	}

	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66AchievementsSubsystem* Achievements = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	const int32 BondStages = Achievements && bHasPet ? Achievements->GetPetBondStagesCleared(PreviewedPetID) : 0;
	const float MovementMultiplier = Achievements && bHasPet ? Achievements->GetPetBondMovementSpeedMultiplier(PreviewedPetID) : 1.f;
	if (PetBondTextWidget.IsValid())
	{
		PetBondTextWidget->SetText(bHasPet
			? FText::Format(NSLOCTEXT("T66.Pets", "PetSelectionBondFormat", "{0} stages cleared"), FText::AsNumber(BondStages))
			: FText::FromString(TEXT("--")));
	}
	if (PetMovementTextWidget.IsValid())
	{
		PetMovementTextWidget->SetText(bHasPet
			? FText::Format(NSLOCTEXT("T66.Pets", "PetSelectionMovementFormat", "Fetch movement speed x{0}; collection amount is unchanged."), FText::AsNumber(MovementMultiplier))
			: NSLOCTEXT("T66.Pets", "PetSelectionMovementOnly", "Union affects speed only."));
	}

	RebuildPetList();
	RebuildPetSkinList();
}

TArray<FPetData> UT66PetSelectionScreen::GetCapturedPets() const
{
	TArray<FPetData> Pets;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!T66GI)
	{
		return Pets;
	}

	for (const FName PetID : CapturedPetIDs)
	{
		FPetData PetData;
		if (T66GI->GetPetData(PetID, PetData))
		{
			Pets.Add(PetData);
		}
	}
	return Pets;
}

bool UT66PetSelectionScreen::GetPreviewedPetData(FPetData& OutPetData) const
{
	if (PreviewedPetID.IsNone())
	{
		return false;
	}
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return T66GI->GetPetData(PreviewedPetID, OutPetData);
	}
	return false;
}

void UT66PetSelectionScreen::PreviewPet(FName PetID)
{
	if (PetID.IsNone())
	{
		SelectNoPet();
		return;
	}
	if (!IsPetCaptured(PetID))
	{
		return;
	}

	PreviewedPetID = PetID;
	CurrentPetIndex = CapturedPetIDs.IndexOfByKey(PetID);
	UpdatePetDisplay();
}

void UT66PetSelectionScreen::SelectNoPet()
{
	PreviewedPetID = NAME_None;
	CurrentPetIndex = -1;
	UpdatePetDisplay();
}

void UT66PetSelectionScreen::PreviewNextPet()
{
	if (CapturedPetIDs.Num() == 0)
	{
		SelectNoPet();
		return;
	}
	const int32 StartIndex = CurrentPetIndex < 0 ? -1 : CurrentPetIndex;
	const int32 NextIndex = (StartIndex + 1 + CapturedPetIDs.Num()) % CapturedPetIDs.Num();
	PreviewPet(CapturedPetIDs[NextIndex]);
}

void UT66PetSelectionScreen::PreviewPreviousPet()
{
	if (CapturedPetIDs.Num() == 0)
	{
		SelectNoPet();
		return;
	}
	const int32 StartIndex = CurrentPetIndex < 0 ? 0 : CurrentPetIndex;
	const int32 PrevIndex = (StartIndex - 1 + CapturedPetIDs.Num()) % CapturedPetIDs.Num();
	PreviewPet(CapturedPetIDs[PrevIndex]);
}

void UT66PetSelectionScreen::OnConfirmPetClicked()
{
	if (!PreviewedPetID.IsNone() && !IsPetCaptured(PreviewedPetID))
	{
		return;
	}

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		T66GI->SelectedPetID = PreviewedPetID;
		if (UT66AchievementsSubsystem* Achievements = T66GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achievements->SetActivePetID(PreviewedPetID);
		}
	}
	NavigateBack();
}

void UT66PetSelectionScreen::OnBackClicked()
{
	NavigateBack();
}

bool UT66PetSelectionScreen::IsPetCaptured(FName PetID) const
{
	if (PetID.IsNone())
	{
		return true;
	}
	if (UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66AchievementsSubsystem* Achievements = GIBase->GetSubsystem<UT66AchievementsSubsystem>())
		{
			return Achievements->IsPetCaptured(PetID);
		}
	}
	return false;
}

FName UT66PetSelectionScreen::GetEquippedSkinID(FName PetID) const
{
	if (UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66AchievementsSubsystem* Achievements = GIBase->GetSubsystem<UT66AchievementsSubsystem>())
		{
			return Achievements->GetEquippedPetSkinID(PetID);
		}
	}
	return FName(TEXT("Default"));
}

FReply UT66PetSelectionScreen::HandlePrevClicked()
{
	PreviewPreviousPet();
	return FReply::Handled();
}

FReply UT66PetSelectionScreen::HandleNextClicked()
{
	PreviewNextPet();
	return FReply::Handled();
}

FReply UT66PetSelectionScreen::HandleNoPetClicked()
{
	SelectNoPet();
	return FReply::Handled();
}

FReply UT66PetSelectionScreen::HandlePetRowClicked(FName PetID)
{
	PreviewPet(PetID);
	return FReply::Handled();
}

FReply UT66PetSelectionScreen::HandleSkinClicked(FName SkinID)
{
	if (!PreviewedPetID.IsNone())
	{
		if (UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66AchievementsSubsystem* Achievements = GIBase->GetSubsystem<UT66AchievementsSubsystem>())
			{
				Achievements->SetEquippedPetSkinID(PreviewedPetID, SkinID);
			}
		}
	}
	UpdatePetDisplay();
	return FReply::Handled();
}

FReply UT66PetSelectionScreen::HandleConfirmClicked()
{
	OnConfirmPetClicked();
	return FReply::Handled();
}

FReply UT66PetSelectionScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}
