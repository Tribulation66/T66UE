// Copyright Tribulation 66. All Rights Reserved.

#include "T66Editor.h"

DEFINE_LOG_CATEGORY(LogT66Editor);

IMPLEMENT_MODULE(FT66EditorModule, T66Editor)

void FT66EditorModule::StartupModule()
{
	UE_LOG(LogT66Editor, Log, TEXT("T66Editor module started"));
}

void FT66EditorModule::ShutdownModule()
{
	UE_LOG(LogT66Editor, Log, TEXT("T66Editor module shutdown"));
}
