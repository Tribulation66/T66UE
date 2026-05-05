// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniShopScreen.generated.h"

class STextBlock;

USTRUCT()
struct T66MINI_API FT66MiniShopRequestPayload
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Revision = 0;

	UPROPERTY()
	FName Action = NAME_None;

	UPROPERTY()
	FName ItemID = NAME_None;

	UPROPERTY()
	FName GameID = NAME_None;

	UPROPERTY()
	int32 Amount = 0;
};

UCLASS(Blueprintable)
class T66MINI_API UT66MiniShopScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniShopScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EMiniCircusTab : uint8
	{
		Vendor = 0,
		Gambling,
		Alchemy
	};

	FReply HandleBuyItemClicked(FName ItemID);
	FReply HandleStealItemClicked(FName ItemID);
	FReply HandleSellItemClicked(FName ItemID);
	FReply HandleBuybackClicked(FName ItemID);
	FReply HandleLockClicked(FName ItemID);
	FReply HandleRerollClicked();
	FReply HandleBorrowGoldClicked(int32 Amount);
	FReply HandlePayDebtClicked(int32 Amount);
	FReply HandleCircusGameClicked(FName GameID, FString GameTitle);
	FReply HandleAlchemyTransmuteClicked();
	FReply HandleAlchemyDissolveClicked();
	FReply HandleContinueClicked();
	FReply HandleVendorTabClicked();
	FReply HandleGamblingTabClicked();
	FReply HandleAlchemyTabClicked();
	FString BuildShopUiStateKey() const;
	void RequestShopRebuildIfStateChanged();
	bool ExecuteLocalRequest(const FT66MiniShopRequestPayload& Request);
	void SetStatus(const FText& InText);

	TSharedPtr<STextBlock> StatusTextBlock;
	FText CurrentStatusText;
	EMiniCircusTab ActiveTab = EMiniCircusTab::Vendor;
	int32 LastAppliedStateRevision = 0;
	FString LastShopUiStateKey;
};
