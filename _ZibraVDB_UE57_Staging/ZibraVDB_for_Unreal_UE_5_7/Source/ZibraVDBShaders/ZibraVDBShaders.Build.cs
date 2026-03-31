// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ZibraVDBShaders : ModuleRules
{
    public ZibraVDBShaders(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = Path.GetFullPath(Path.Combine(ModuleDirectory, "Private/PCH.h"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Projects",
                "RenderCore",
                "RHI",
                "Engine"
            }
        );
        
        PrivateIncludePaths.AddRange(
            new string[]
            {
                System.IO.Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Private")
            }
        );
    }
}
