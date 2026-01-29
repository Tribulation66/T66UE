// Copyright Tribulation 66. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class T66 : ModuleRules
{
	public T66(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "NavigationSystem", "Slate", "SlateCore", "UMG" });

		// Add all subdirectories as include paths
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Core"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Data"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UI"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UI", "Screens"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UI", "Components"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Gameplay"));
	}
}
