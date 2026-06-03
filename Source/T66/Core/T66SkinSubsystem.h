// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66SkinSubsystem.generated.h"

class UT66AchievementsSubsystem;
class UT66ProfileSaveGame;
class UTexture2D;

/** Entity type for skin ownership (hero vs companion). */
UENUM(BlueprintType)
enum class ET66SkinEntityType : uint8
{
	Hero,
	Companion
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkinStateChanged);

/**
 * Central subsystem for all skin management (heroes and companions).
 * Ownership, purchase, and equipped state live here; persistence uses the profile via AchievementsSubsystem.
 */
UCLASS()
class T66_API UT66SkinSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Broadcast when ownership or equipped state changes (UI can refresh). */
	UPROPERTY(BlueprintAssignable, Category = "Skins")
	FOnSkinStateChanged OnSkinStateChanged;

	/** Default skin ID (always considered owned). */
	static const FName DefaultSkinID;

	/** Demo-facing purchasable skin ID. Offered only for the first five heroes. */
	static const FName DemoSkinID;

	/** Secret skin unlocked for the hero that gives Kromer to the Saint. */
	static const FName SaintSkinID;

	/** Price in Chad Coupons for the purchasable demo skin. */
	static constexpr int32 DefaultSkinPriceAC = 50;

	/** All globally known skin IDs. Entity-specific offer lists are filtered by GetSkinsForEntity. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	static TArray<FName> GetAllSkinIDs();

	// ---- Unified API (works for both heroes and companions) ----

	/** True if the entity owns this skin. Default is always owned. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	bool IsSkinOwned(ET66SkinEntityType EntityType, FName EntityID, FName SkinID) const;

	/** Purchase a skin for the entity with Chad Coupons. Returns true if purchased. */
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool PurchaseSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID, int32 CostAC);

	/** Grants a skin without spending currency; used by achievement/secret-ending rewards. */
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool GrantSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID, bool bEquip = true);

	/** Refund a previously owned skin for the entity. Returns true if refunded. */
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool RefundSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID, int32 RefundAC);

	/** Currently equipped skin for the entity (Default if none). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	FName GetEquippedSkin(ET66SkinEntityType EntityType, FName EntityID) const;

	/** Set equipped skin (must be owned). */
	UFUNCTION(BlueprintCallable, Category = "Skins")
	void SetEquippedSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID);

	/** Get skin list for UI: each skin has bIsOwned, bIsEquipped, CoinCost filled for the given entity. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	TArray<FSkinData> GetSkinsForEntity(ET66SkinEntityType EntityType, FName EntityID) const;

	/** Resolve the portrait used for an entity skin; falls back to the entity portrait when no skin-specific override is defined. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	TSoftObjectPtr<UTexture2D> GetSkinPortrait(ET66SkinEntityType EntityType, FName EntityID, FName SkinID, bool bSelectionPortrait = false) const;

	/** Current Chad Coupons balance (from profile). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	int32 GetAchievementCoinsBalance() const;

	// ---- Legacy compatibility (forward to unified API) ----
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	bool IsHeroSkinOwned(FName HeroID, FName SkinID) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	bool IsCompanionSkinOwned(FName CompanionID, FName SkinID) const;
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool PurchaseHeroSkin(FName HeroID, FName SkinID, int32 CostAC);
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool GrantHeroSkin(FName HeroID, FName SkinID, bool bEquip = true);
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool PurchaseCompanionSkin(FName CompanionID, FName SkinID, int32 CostAC);
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool RefundHeroSkin(FName HeroID, FName SkinID, int32 RefundAC);
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool RefundCompanionSkin(FName CompanionID, FName SkinID, int32 RefundAC);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	FName GetEquippedHeroSkinID(FName HeroID) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	FName GetEquippedCompanionSkinID(FName CompanionID) const;
	UFUNCTION(BlueprintCallable, Category = "Skins")
	void SetEquippedHeroSkinID(FName HeroID, FName SkinID);
	UFUNCTION(BlueprintCallable, Category = "Skins")
	void SetEquippedCompanionSkinID(FName CompanionID, FName SkinID);

	/** Debug: clear all hero skin ownership and reset equipped to Default. */
	UFUNCTION(BlueprintCallable, Category = "Skins")
	void ResetAllHeroSkinOwnership();

private:
	UT66ProfileSaveGame* GetProfile() const;
	void MarkProfileDirtyAndSave(bool bBroadcastCoinsChanged);
	bool IsSkinOfferedForEntity(ET66SkinEntityType EntityType, FName EntityID, FName SkinID) const;
	TArray<FName> GetSkinIDsForEntity(ET66SkinEntityType EntityType, FName EntityID) const;
	TSoftObjectPtr<UTexture2D> GetCompanionSkinPortraitOverride(FName CompanionID, FName SkinID, bool bSelectionPortrait) const;
	static const FName LegacyBeachgoerSkinID;
	static FName NormalizeSkinID(FName SkinID);
};
