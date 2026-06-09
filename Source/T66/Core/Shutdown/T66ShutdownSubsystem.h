// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66ShutdownSubsystem.generated.h"

UENUM(BlueprintType)
enum class ET66ShutdownReason : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	UserQuit UMETA(DisplayName = "User Quit"),
	EnginePreExit UMETA(DisplayName = "Engine Pre-Exit"),
	TestHarness UMETA(DisplayName = "Test Harness")
};

UENUM(BlueprintType)
enum class ET66ShutdownPhase : uint8
{
	InputLock UMETA(DisplayName = "Input Lock"),
	NativeExternal UMETA(DisplayName = "Native / External"),
	NetworkPlatform UMETA(DisplayName = "Network / Platform"),
	DurableState UMETA(DisplayName = "Durable State"),
	AsyncWork UMETA(DisplayName = "Async Work"),
	RuntimeTick UMETA(DisplayName = "Runtime Tick"),
	MediaAudio UMETA(DisplayName = "Media / Audio"),
	GameplayWorld UMETA(DisplayName = "Gameplay World"),
	FinalReport UMETA(DisplayName = "Final Report")
};

struct T66_API FT66ShutdownContext
{
	ET66ShutdownReason Reason = ET66ShutdownReason::Unknown;
	ET66ShutdownPhase Phase = ET66ShutdownPhase::InputLock;
	FName ParticipantName = NAME_None;
	double ShutdownStartedSeconds = 0.0;
	bool bWillRequestEngineExit = false;
};

DECLARE_DELEGATE_RetVal_OneParam(bool, FT66ShutdownParticipantDelegate, const FT66ShutdownContext&);

struct T66_API FT66ShutdownParticipantHandle
{
	int32 Id = INDEX_NONE;

	bool IsValid() const { return Id != INDEX_NONE; }
	void Reset() { Id = INDEX_NONE; }
};

UCLASS()
class T66_API UT66ShutdownSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	FT66ShutdownParticipantHandle RegisterParticipant(
		UObject* Owner,
		FName ParticipantName,
		ET66ShutdownPhase Phase,
		int32 Priority,
		double TimeoutSeconds,
		bool bRequired,
		FT66ShutdownParticipantDelegate Delegate);

	void UnregisterParticipant(FT66ShutdownParticipantHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "T66|Shutdown")
	void RequestQuitGame(ET66ShutdownReason Reason = ET66ShutdownReason::UserQuit, int32 ExitCode = 0);

	bool RunShutdown(ET66ShutdownReason Reason, bool bRequestEngineExit, int32 ExitCode = 0, const TCHAR* ExitTag = TEXT("T66ShutdownSystem"));

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Shutdown")
	bool IsShutdownInProgress() const { return bShutdownInProgress; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Shutdown")
	bool HasShutdownCompleted() const { return bShutdownCompleted; }

	static const TCHAR* LexToString(ET66ShutdownReason Reason);
	static const TCHAR* LexToString(ET66ShutdownPhase Phase);

private:
	struct FParticipant
	{
		FT66ShutdownParticipantHandle Handle;
		TWeakObjectPtr<UObject> Owner;
		FName Name = NAME_None;
		ET66ShutdownPhase Phase = ET66ShutdownPhase::InputLock;
		int32 Priority = 0;
		double TimeoutSeconds = 1.0;
		bool bRequired = false;
		FT66ShutdownParticipantDelegate Delegate;
	};

	TArray<FParticipant> Participants;
	int32 NextParticipantId = 1;
	bool bShutdownInProgress = false;
	bool bShutdownCompleted = false;
	bool bEngineExitRequested = false;
	double LastShutdownStartedSeconds = 0.0;

	static int32 GetPhaseSortKey(ET66ShutdownPhase Phase);
};

