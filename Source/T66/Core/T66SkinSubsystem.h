// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66SkinSubsystem.generated.h"

class UT66AchievementsSubsystem;
class UT66ProfileSaveGame;

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

	/** Beachgoer skin ID. */
	static const FName BeachgoerSkinID;

	/** Price in AC for Beachgoer (and other purchasable skins unless overridden). */
	static constexpr int32 DefaultSkinPriceAC = 250;

	/** All skin IDs offered in UI (Default + Beachgoer). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	static TArray<FName> GetAllSkinIDs();

	// ---- Unified API (works for both heroes and companions) ----

	/** True if the entity owns this skin. Default is always owned. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	bool IsSkinOwned(ET66SkinEntityType EntityType, FName EntityID, FName SkinID) const;

	/** Purchase a skin for the entity with AC. Returns true if purchased. */
	UFUNCTION(BlueprintCallable, Category = "Skins")
	bool PurchaseSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID, int32 CostAC);

	/** Currently equipped skin for the entity (Default if none). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	FName GetEquippedSkin(ET66SkinEntityType EntityType, FName EntityID) const;

	/** Set equipped skin (must be owned). */
	UFUNCTION(BlueprintCallable, Category = "Skins")
	void SetEquippedSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID);

	/** Get skin list for UI: each skin has bIsOwned, bIsEquipped, CoinCost filled for the given entity. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skins")
	TArray<FSkinData> GetSkinsForEntity(ET66SkinEntityType EntityType, FName EntityID) const;

	/** Current AC balance (from profile). */
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
	bool PurchaseCompanionSkin(FName CompanionID, FName SkinID, int32 CostAC);
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
};
