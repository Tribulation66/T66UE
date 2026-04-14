// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniShopScreen.generated.h"

class STextBlock;

UCLASS(Blueprintable)
class T66MINI_API UT66MiniShopScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniShopScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EMiniCircusTab : uint8
	{
		Vendor = 0,
		Gambling,
		Alchemy
	};

	FReply HandleBuyItemClicked(FName ItemID);
	FReply HandleSellItemClicked(FName ItemID);
	FReply HandleLockClicked(FName ItemID);
	FReply HandleRerollClicked();
	FReply HandleContinueClicked();
	FReply HandleVendorTabClicked();
	FReply HandleGamblingTabClicked();
	FReply HandleAlchemyTabClicked();
	void SetStatus(const FText& InText);

	TSharedPtr<STextBlock> StatusTextBlock;
	EMiniCircusTab ActiveTab = EMiniCircusTab::Vendor;
};
