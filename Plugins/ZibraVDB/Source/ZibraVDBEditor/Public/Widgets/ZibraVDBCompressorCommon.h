// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Containers/StaticArray.h"
#include "Containers/UnrealString.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"
#include "ZibraVDBRuntime/Public/ZibraVDBVolume4.h"
#include <Zibra/OpenVDBHelper.h>

struct FSequenceInfo;

enum class EZibraCompressionState
{
	Uninitialized = 0,
	Started = 1,
	Finished = 2
};

enum class EMetadataReadingState
{
	Running,
	Finished,
	Error
};

class FZibraVDBCompressorCommon
{
public:
	static void ReleaseZibraCompressionSetting(struct FBridgeZibraVDBCompressionSettings& ZibraCompressionSetting);
	static void DeleteTempCompressedFile(const FString& FilePath);
	static EMetadataReadingState ReadSequenceInfo(const FString& InputFileMask,
		TFunction<void(Zibra::OpenVDBHelper::SequenceInfo*)> InCompletionCallback, TFunction<void()> InErrorCallback);
	static TSharedRef<SHorizontalBox> ConstructPerChannelCompressionSettingsBox(
		TArray<FCompressionSettings>& PerChannelCompressionSettings, float DefaultQuality);
};
