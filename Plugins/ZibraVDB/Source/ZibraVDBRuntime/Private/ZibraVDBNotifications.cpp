// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBNotifications.h"

#include "Framework/Notifications/NotificationManager.h"

TArray<FString> FZibraVDBNotifications::ShownMessages = TArray<FString>();
FCriticalSection FZibraVDBNotifications::ShownMessagesMutex;

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

DEFINE_LOG_CATEGORY_STATIC(LogZibraVDBNotifications, Log, All);

FZibraVDBNotifications::FZibraVDBNotifications()
{
}

FZibraVDBNotifications::~FZibraVDBNotifications()
{
}

void FZibraVDBNotifications::ResetShownMessages()
{
	FScopeLock Lock(&ShownMessagesMutex);
	ShownMessages.Empty();
}

void FZibraVDBNotifications::AddNotification(const FText& Message, float Duration, bool bAllowRepeat)
{
	checkf(IsInGameThread(),
		TEXT("FZibraVDBNotifications::AddNotification must be called on game thread. Use AddNotification_RenderThread if necessary."));

	const TOptional<FNotificationInfo> Info = CreateNotification(Message, Duration, bAllowRepeat);

	if (Info.IsSet())
	{
		FSlateNotificationManager::Get().AddNotification(Info.GetValue());
	}
}

void FZibraVDBNotifications::AddNotification_RenderThread(
	const FText& Message, float Duration /*= 5*/, bool bAllowRepeat /*= false*/)
{
	checkf(IsInRenderingThread(),
		TEXT(
			"FZibraVDBNotifications::AddNotification_RenderThread must be called on render thread. Use AddNotification if necessary."));

	FNotificationInfo* Info = CreateNotification_RenderThread(Message, Duration, bAllowRepeat);

	if (Info)
	{
		FSlateNotificationManager::Get().QueueNotification(Info);
	}
}

void FZibraVDBNotifications::AddNotificationWithCheckbox(const FText& Message, const FText& CheckBoxText,
	FOnCheckStateChanged CheckboxDelegate,
	ECheckBoxState CheckboxState, float Duration, bool bAllowRepeat)
{
	TOptional<FNotificationInfo> Info = CreateNotification(Message, Duration, bAllowRepeat);

	if (!Info.IsSet())
	{
		return;
	}

	// Set checkbox Message.
	if (!CheckBoxText.IsEmpty())
	{
		Info->CheckBoxText = Message;
		Info->CheckBoxState = CheckboxState;
		Info->CheckBoxStateChanged = CheckboxDelegate;
	}

	// And call Add Notification, this is pretty much it!
	FSlateNotificationManager::Get().AddNotification(Info.GetValue());
}

TOptional<FNotificationInfo> FZibraVDBNotifications::CreateNotification(const FText& Message, float Duration, bool bAllowRepeat)
{
	FString MessageAsString = Message.ToString();

	if (!bAllowRepeat)
	{
		FScopeLock Lock(&ShownMessagesMutex);
		if (ShownMessages.Contains(MessageAsString))
			return {};
		ShownMessages.Add(MessageAsString);
	}

	UE_LOG(LogZibraVDBNotifications, Warning, TEXT("%s"), *MessageAsString);
	FText NotificationMessage = FText::Format(LOCTEXT("ZibraVDBNotificationPrefix", "ZibraVDB: {0}"), Message);

	FNotificationInfo Info(NotificationMessage);

	// Set a default expire duration.
	Info.ExpireDuration = Duration;

	return TOptional<FNotificationInfo>(Info);
}

FNotificationInfo* FZibraVDBNotifications::CreateNotification_RenderThread(const FText& Message, float Duration, bool bAllowRepeat)
{
	FString MessageAsString = Message.ToString();

	if (!bAllowRepeat)
	{
		FScopeLock Lock(&ShownMessagesMutex);
		if (ShownMessages.Contains(MessageAsString))
			return nullptr;
		ShownMessages.Add(MessageAsString);
	}

	UE_LOG(LogZibraVDBNotifications, Warning, TEXT("%s"), *MessageAsString);
	FText NotificationMessage = FText::Format(LOCTEXT("ZibraVDBNotificationPrefix", "ZibraVDB: {0}"), Message);

	// Freed by FSlateNotificationManager
	FNotificationInfo* Info = new FNotificationInfo(NotificationMessage);

	// Set a default expire duration.
	Info->ExpireDuration = Duration;

	return Info;
}

#undef LOCTEXT_NAMESPACE
