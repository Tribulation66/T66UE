// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ZibraVDBEditor : ModuleRules
{
    public ZibraVDBEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        string SDKPath;

        if (Target.bCompileAgainstEditor)
        {
            SDKPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source/ZibraVDBSDK/Editor/"));
        }
        else
        {
            throw new BuildException("ZibraVDBEditor module can only be built in the editor configuration.");
        }

        bUseRTTI = true;

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = Path.GetFullPath(Path.Combine(ModuleDirectory, "Private/PCH.h"));

        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(SDKPath, "include"),
                Path.Combine(SDKPath, "OpenVDBHelper/include")
            });

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ZibraVDBRuntime",
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
                "ApplicationCore",
                "AssetTools",
                "Core",
                "CoreUObject",
                "DesktopWidgets",
                "EditorStyle",
                "Engine",
                "InputCore",
                "Json",
                "JsonUtilities",
                "MainFrame",
                "MovieScene",
                "Projects",
                "RHI",
                "RenderCore",
                "Sequencer",
                "SequencerCore",
                "Slate",
                "SlateCore",
                "ToolMenus",
                "UnrealEd",
#if UE_5_5_OR_LATER
                "WindowsTargetPlatformSettings",
#else
                "WindowsTargetPlatform",
#endif
                "ZibraVDBLicensing",
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
