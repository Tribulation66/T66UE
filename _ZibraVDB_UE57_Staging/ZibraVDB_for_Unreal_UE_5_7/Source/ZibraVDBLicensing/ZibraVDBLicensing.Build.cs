// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ZibraVDBLicensing : ModuleRules
{
    public ZibraVDBLicensing(ReadOnlyTargetRules Target) : base(Target)
    {
        string SDKPath;

        if (Target.bCompileAgainstEditor)
        {
            SDKPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source/ZibraVDBSDK/Editor/"));
        }
        else
        {
            SDKPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source/ZibraVDBSDK/Build/"));
        }

        bUseRTTI = true;

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = Path.GetFullPath(Path.Combine(ModuleDirectory, "Private/PCH.h"));

        PublicIncludePaths.Add(Path.Combine(SDKPath, "include"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject"
            }
        );

#if UE_5_0_OR_LATER
        PublicDependencyModuleNames.Add("HTTP");
#else
        PublicDependencyModuleNames.Add("Http");
#endif

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Json",
                "JsonUtilities",
                "Slate",
                "ZibraVDBSDKIntegration"
            }
        );

        string ZibraVDBSDKDirectory = Path.GetFullPath(SDKPath);
        string NativePluginConfiguration = File.ReadAllText(Path.GetFullPath(Path.Combine(ZibraVDBSDKDirectory, "BuildConfiguration.txt"))).Trim();

        bool bIsDebug = NativePluginConfiguration == "Debug";
        bool bIsProfile = NativePluginConfiguration == "Profile";
        bool bIsRelease = NativePluginConfiguration == "Release" || NativePluginConfiguration == "ReleaseNoCheck";
        bool bIsReleaseNoCheck = NativePluginConfiguration == "ReleaseNoCheck";
        bool bIsLicenseCheckEnabled = NativePluginConfiguration == "Release" && Target.bCompileAgainstEditor;

        PublicDefinitions.Add($"ZIBRAVDB_DEBUG={(bIsDebug ? 1 : 0)}");
        PublicDefinitions.Add($"ZIBRAVDB_PROFILE={(bIsProfile ? 1 : 0)}");
        PublicDefinitions.Add($"ZIBRAVDB_RELEASE={(bIsRelease ? 1 : 0)}");
        PublicDefinitions.Add($"ZIBRAVDB_RELEASENOCHECK={(bIsReleaseNoCheck ? 1 : 0)}");
        PublicDefinitions.Add($"ZIBRAVDB_LICENSE_CHECK_ENABLED={(bIsLicenseCheckEnabled ? 1 : 0)}");
    }
}
