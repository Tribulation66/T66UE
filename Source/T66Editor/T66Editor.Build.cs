// Copyright Tribulation 66. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class T66Editor : ModuleRules
{
	public T66Editor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"T66"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"Slate",
			"SlateCore",
			"UMG",
			"UMGEditor",
			"Blutility",
			"EditorSubsystem",
			"AssetTools",
			"AssetRegistry",
			"Landscape",
			"LandscapeEditor",
			"Foliage",
			"LevelEditor",
			"ToolMenus",
			"EditorStyle"
		});

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory));
	}
}
