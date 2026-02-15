// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66LoadPreviewOverlayWidget.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66Style.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"

TSharedRef<SWidget> UT66LoadPreviewOverlayWidget::RebuildWidget()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const FText LoadText = NSLOCTEXT("T66.LoadPreview", "Load", "LOAD");

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(20.f, 0.f, 0.f, 20.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(LoadText, FOnClicked::CreateUObject(this, &UT66LoadPreviewOverlayWidget::OnLoadClicked), ET66ButtonType::Success)
				.SetMinWidth(160.f))
		];
}

FReply UT66LoadPreviewOverlayWidget::OnLoadClicked()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	AT66PlayerController* PC = GetOwningPlayer() ? Cast<AT66PlayerController>(GetOwningPlayer()) : nullptr;

	if (T66GI)
	{
		T66GI->bLoadAsPreview = false;
	}
	if (PC)
	{
		PC->SetPause(false);
	}
	RemoveFromParent();
	return FReply::Handled();
}
