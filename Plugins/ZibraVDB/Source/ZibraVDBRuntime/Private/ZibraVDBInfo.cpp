// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#if WITH_EDITOR

#include "ZibraVDBInfo.h"
#include "Interfaces/IPluginManager.h"

const FString& FZibraVDBInfo::GetVersion()
{
	static FString Version;
	if (Version.IsEmpty())
	{
		TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ZibraVDB"));
		if (Plugin)
		{
			Version = Plugin->GetDescriptor().VersionName;
		}
		else
		{
			Version = "Failed to retreive version from Plugin Manager";
		}
	}
	return Version;
}

#endif
