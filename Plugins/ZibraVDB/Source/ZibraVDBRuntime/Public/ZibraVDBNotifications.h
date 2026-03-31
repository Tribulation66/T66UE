// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Logging/LogVerbosity.h"
#include "Misc/Optional.h"
#include "Widgets/Notifications/SNotificationList.h"

class ZIBRAVDBRUNTIME_API FZibraVDBNotifications
{
public:
	FZibraVDBNotifications();
	~FZibraVDBNotifications();

	static void AddNotification(const FText& Message, float Duration = 5, bool bAllowRepeat = true);
	static void AddNotification_RenderThread(const FText& Message, float Duration = 5, bool bAllowRepeat = true);
	static void AddNotificationWithCheckbox(const FText& Message, const FText& CheckBoxText,
		FOnCheckStateChanged CheckboxDelegate,
		ECheckBoxState CheckboxState = ECheckBoxState::Unchecked, float Duration = 5, bool bAllowRepeat = true);
	static void ResetShownMessages();

private:
	static TOptional<FNotificationInfo> CreateNotification(const FText& Message, float Duration, bool bAllowRepeat);
	static FNotificationInfo* CreateNotification_RenderThread(const FText& Message, float Duration, bool bAllowRepeat);

	static TArray<FString> ShownMessages;
	static FCriticalSection ShownMessagesMutex;
};
