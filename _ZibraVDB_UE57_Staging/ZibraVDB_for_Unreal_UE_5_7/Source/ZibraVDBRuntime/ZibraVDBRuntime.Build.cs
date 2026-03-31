// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ZibraVDBRuntime : ModuleRules
{
    private const bool bForceZibraVDBProfiler = false;

    public ZibraVDBRuntime(ReadOnlyTargetRules Target) : base(Target)
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

        var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = Path.GetFullPath(Path.Combine(ModuleDirectory, "Private/PCH.h"));

        PrivateIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(EngineDir, "Source/Runtime/Renderer/Private"),
                Path.Combine(EngineDir, "Source/Runtime/Renderer/Public"),
                Path.Combine(EngineDir, "Source/Runtime/Renderer/Internal"),
                Path.Combine(SDKPath, "include")
            }
        );

        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(ModuleDirectory, "Private/Render")
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Projects",
                "Engine",
                "GeometryFramework",
                "MovieScene",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Renderer",
                "RenderCore",
                "RHI",
                "CoreUObject",
                "InputCore",
                "Slate",
                "ZibraVDBShaders",
                "ZibraVDBSDKIntegration",
                "ZibraVDBRHI"
            }
        );

        if (Target.bCompileAgainstEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "EditorFramework",
                    "MainFrame",
                    "UnrealEd",
                    "DeveloperToolSettings",
                    "ZibraVDBLicensing",
                    "SlateCore"
                }
            );
        }

        string ZibraVDBSDKDirectory = Path.GetFullPath(SDKPath);
        string NativePluginConfiguration = File.ReadAllText(Path.GetFullPath(Path.Combine(ZibraVDBSDKDirectory, "BuildConfiguration.txt"))).Trim();

        bool bIsDebug = NativePluginConfiguration == "Debug";
        bool bIsProfile = NativePluginConfiguration == "Profile";
        bool bIsRelease = NativePluginConfiguration == "Release" || NativePluginConfiguration == "ReleaseNoCheck";
        bool bIsReleaseNoCheck = NativePluginConfiguration == "ReleaseNoCheck";
        bool bIsLicenseCheckEnabled = NativePluginConfiguration == "Release" && Target.bCompileAgainstEditor;

        PublicDefinitions.Add("ZCE_NO_STATIC_API_DECL");

        PublicDefinitions.Add($"ZIBRAVDB_DEBUG={(bIsDebug ? 1 : 0)}");
        PublicDefinitions.Add($"ZIBRAVDB_PROFILE={(bIsProfile ? 1 : 0)}");
        PublicDefinitions.Add($"ZIBRAVDB_RELEASE={(bIsRelease ? 1 : 0)}");
        PublicDefinitions.Add($"ZIBRAVDB_RELEASENOCHECK={(bIsReleaseNoCheck ? 1 : 0)}");
        PublicDefinitions.Add($"ZIBRAVDB_LICENSE_CHECK_ENABLED={(bIsLicenseCheckEnabled ? 1 : 0)}");

        PublicDefinitions.Add($"ZIBRAVDB_ENABLE_PROFILING={((bIsProfile || bForceZibraVDBProfiler) ? 1 : 0)}");
    }
}
