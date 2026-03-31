// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ZibraVDBRHI : ModuleRules
{
    public ZibraVDBRHI(ReadOnlyTargetRules Target) : base(Target)
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

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = Path.GetFullPath(Path.Combine(ModuleDirectory, "Private/PCH.h"));

        PublicIncludePaths.Add(Path.Combine(SDKPath, "include"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Projects",
                "CoreUObject",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "RHI",
                "RenderCore",
                "ZibraVDBShaders",
            }
        );
    }
}
