// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FT66DragPreviewDeltaDelegate, float);

class ST66DragRotateStagePreview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66DragRotateStagePreview)
		: _DegreesPerPixel(0.25f)
	{}
		SLATE_ARGUMENT(float, DegreesPerPixel)
		SLATE_EVENT(FT66DragPreviewDeltaDelegate, OnRotateYaw)
		SLATE_EVENT(FT66DragPreviewDeltaDelegate, OnZoom)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		DegreesPerPixel = InArgs._DegreesPerPixel;
		OnRotateYaw = InArgs._OnRotateYaw;
		OnZoom = InArgs._OnZoom;
		bDragging = false;
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bDragging = true;
			LastPos = MouseEvent.GetScreenSpacePosition();
			return FReply::Handled().CaptureMouse(SharedThis(this));
		}

		return FReply::Unhandled();
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bDragging = false;
			return FReply::Handled().ReleaseMouseCapture();
		}

		return FReply::Unhandled();
	}

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (!bDragging)
		{
			return FReply::Unhandled();
		}

		const FVector2D Pos = MouseEvent.GetScreenSpacePosition();
		const FVector2D Delta = Pos - LastPos;
		LastPos = Pos;
		OnRotateYaw.ExecuteIfBound(Delta.X * DegreesPerPixel);
		return FReply::Handled();
	}

	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		OnZoom.ExecuteIfBound(MouseEvent.GetWheelDelta());
		return OnZoom.IsBound() ? FReply::Handled() : FReply::Unhandled();
	}

private:
	FT66DragPreviewDeltaDelegate OnRotateYaw;
	FT66DragPreviewDeltaDelegate OnZoom;
	bool bDragging = false;
	FVector2D LastPos = FVector2D::ZeroVector;
	float DegreesPerPixel = 0.25f;
};

namespace T66SelectionScreenUtils
{
	inline void PopulateDefaultOwnedSkins(TArray<FSkinData>& OutSkins)
	{
		for (const FName SkinID : UT66SkinSubsystem::GetAllSkinIDs())
		{
			FSkinData SkinData;
			SkinData.SkinID = SkinID;
			SkinData.bIsDefault = (SkinID == UT66SkinSubsystem::DefaultSkinID);
			SkinData.bIsOwned = SkinData.bIsDefault;
			SkinData.bIsEquipped = SkinData.bIsDefault;
			SkinData.CoinCost = SkinData.bIsDefault ? 0 : UT66SkinSubsystem::DefaultSkinPriceAC;
			OutSkins.Add(SkinData);
		}
	}

	inline int32 GetAchievementCoinBalance(const UObject* ContextObject)
	{
		const UGameInstance* GameInstance = ContextObject ? UGameplayStatics::GetGameInstance(ContextObject) : nullptr;
		const UT66SkinSubsystem* SkinSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66SkinSubsystem>() : nullptr;
		return SkinSubsystem ? SkinSubsystem->GetAchievementCoinsBalance() : 0;
	}

	inline FText FormatAchievementCoinBalance(UT66LocalizationSubsystem* Loc, const int32 Amount)
	{
		return Loc
			? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Amount))
			: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} CC"), FText::AsNumber(Amount));
	}
}
