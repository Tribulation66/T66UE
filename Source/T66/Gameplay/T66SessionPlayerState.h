// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/PlayerState.h"
#include "UI/T66UITypes.h"
#include "T66SessionPlayerState.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66LobbyPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	FString SteamId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	FString DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	FName SelectedHeroID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	FName SelectedCompanionID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	ET66BodyType SelectedHeroBodyType = ET66BodyType::TypeA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	ET66BodyType SelectedCompanionBodyType = ET66BodyType::TypeA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	FName SelectedHeroSkinID = FName(TEXT("Default"));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	ET66Difficulty LobbyDifficulty = ET66Difficulty::Easy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	ET66ScreenType FrontendScreen = ET66ScreenType::MainMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bLobbyReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bPartyHost = false;
};

DECLARE_MULTICAST_DELEGATE(FOnT66LobbyInfoChanged);

UCLASS()
class T66_API AT66SessionPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AT66SessionPlayerState();

	const FT66LobbyPlayerInfo& GetLobbyInfo() const { return LobbyInfo; }
	const FString& GetSteamId() const { return LobbyInfo.SteamId; }
	const FString& GetDisplayName() const { return LobbyInfo.DisplayName; }
	FName GetSelectedHeroID() const { return LobbyInfo.SelectedHeroID; }
	FName GetSelectedCompanionID() const { return LobbyInfo.SelectedCompanionID; }
	ET66BodyType GetSelectedHeroBodyType() const { return LobbyInfo.SelectedHeroBodyType; }
	ET66BodyType GetSelectedCompanionBodyType() const { return LobbyInfo.SelectedCompanionBodyType; }
	FName GetSelectedHeroSkinID() const { return LobbyInfo.SelectedHeroSkinID; }
	bool IsLobbyReady() const { return LobbyInfo.bLobbyReady; }
	bool IsPartyHost() const { return LobbyInfo.bPartyHost; }

	void ApplyLobbyInfo(const FT66LobbyPlayerInfo& NewInfo);

	FOnT66LobbyInfoChanged& OnLobbyInfoChanged() { return LobbyInfoChanged; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_LobbyInfo)
	FT66LobbyPlayerInfo LobbyInfo;

	UFUNCTION()
	void OnRep_LobbyInfo();

private:
	void BroadcastLobbyInfoChanged();

	FOnT66LobbyInfoChanged LobbyInfoChanged;
};
