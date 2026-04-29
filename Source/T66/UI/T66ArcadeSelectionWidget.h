// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ArcadePopupWidget.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "T66ArcadeSelectionWidget.generated.h"

class UTexture2D;

UCLASS(Blueprintable)
class T66_API UT66ArcadeSelectionWidget : public UT66ArcadePopupWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual bool ReportsArcadeResult() const override { return false; }

private:
	TSharedRef<SWidget> BuildGameButton(const FT66ArcadeInteractableData& GameData, int32 Index);
	FReply HandleGameClicked(ET66ArcadeGameType GameType);
	FReply HandleExitClicked();
	FText ResolveGameCode(ET66ArcadeGameType GameType) const;
	FText ResolveGameFlavorText(ET66ArcadeGameType GameType) const;
	FLinearColor ResolveGameAccentColor(ET66ArcadeGameType GameType, int32 Index) const;
	void LoadCabinetArtworkBrush();
	TSharedRef<SWidget> BuildCabinetArtworkLayer(float Opacity) const;
	TSharedRef<SWidget> BuildCrtOverlay() const;

	TArray<FT66ArcadeInteractableData> GameOptions;
	TStrongObjectPtr<UTexture2D> CabinetArtworkTexture;
	FSlateBrush CabinetArtworkBrush;
};
