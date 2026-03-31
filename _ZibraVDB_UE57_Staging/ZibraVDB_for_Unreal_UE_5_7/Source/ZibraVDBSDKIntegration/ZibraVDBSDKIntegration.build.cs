// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ZibraVDBSDKIntegration : ModuleRules
{
    public ZibraVDBSDKIntegration(ReadOnlyTargetRules Target) : base(Target)
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

        bEnableExceptions = true;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(SDKPath, "include"),
            }
        );

        if (Target.bCompileAgainstEditor)
        {
            PublicIncludePaths.AddRange(
                new string[]
                {
                Path.Combine(SDKPath, "OpenVDBHelper/include")
                }
            );
        }

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                    "Projects",
                    "Engine",
                    "ZibraVDBRHI",
                    "ZibraVDBShaders"
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "InputCore",
                "RenderCore",
                "RHI",
                "Slate"
            }
        );

        if (Target.bCompileAgainstEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Slate"
                }
            );
        }

        string ZibraVDBSDKDirectory = Path.GetFullPath(Path.Combine(SDKPath, "bin"));
        string DynamicLibName = "ZibraVDBSDK.dll";

        // Ensure that the DLL and Libs are staged along with the executable.
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Ensure that the DLL is staged along with the executable
            RuntimeDependencies.Add(Path.Combine(ZibraVDBSDKDirectory, DynamicLibName));
        }

        if (Target.bCompileAgainstEditor)
        {
            string OpenVDBHelperPath = Path.GetFullPath(Path.Combine(SDKPath, "OpenVDBHelper/bin/OpenVDBHelper.dll"));
            RuntimeDependencies.Add(OpenVDBHelperPath);
                
            string OpenVDBDependenciesDirectory = Path.GetFullPath(Path.Combine(PluginDirectory, "Source/ThirdParty"));
            foreach (string DllPath in Directory.GetFiles(OpenVDBDependenciesDirectory, "*.dll"))
            {
                RuntimeDependencies.Add(DllPath);
            }
        }

        PublicDefinitions.Add("ZCE_NO_STATIC_API_DECL");
        PublicDefinitions.Add("ZIB_NO_STATIC_API_DECL");
        PublicDefinitions.Add("ZRHI_NO_STATIC_API_DECL");
    }
}
